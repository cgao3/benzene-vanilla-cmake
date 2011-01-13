//----------------------------------------------------------------------------
/** @file LinkedList.hpp
    Lock-free sorted linked list.
 */
//----------------------------------------------------------------------------

#ifndef LINKEDLIST_HPP
#define LINKEDLIST_HPP

#include "AtomicMemory.hpp"
#include "Benzene.hpp"
#include "BenzeneAssert.hpp"
#include "SafeBool.hpp"
#include <iostream>
#include <boost/utility.hpp>

_BEGIN_BENZENE_NAMESPACE_

//----------------------------------------------------------------------------

/** A node occuring in an instance of LinkedList. */
template<typename T>
class ListNode
{
public:
    ListNode();

    ListNode(const T& data);

    bool AddChild(ListNode<T>* child, ListNode<T>* successor) volatile;

    void Delete() volatile;

private:
    T m_data;

    bool m_locked;

    bool m_deleted;

    ListNode<T>* m_next;

    ListNode<T>* m_dead;

    struct ScopedLock
    {
        volatile ListNode<T>& m_obj;

        ScopedLock(volatile ListNode<T>& obj) 
            : m_obj(obj)
        { 
            while (!CompareAndSwap(&m_obj.m_locked, false, true))
                ;
        }
        ~ScopedLock()
        {
            m_obj.m_locked = false;
        }
    };

    ListNode<T>* GetNext() volatile;

    void TryFixLink(ListNode<T>* node) volatile;

    template<typename U> friend class Pool;
    
    template<typename U> friend class ListIterator;

    template<typename U> friend class LinkedList;
};


template<typename T>
ListNode<T>::ListNode()
    : m_locked(false),
      m_deleted(false),
      m_next(0),
      m_dead(0)
{
}

template<typename T>
ListNode<T>::ListNode(const T& data)
    : m_data(data),
      m_locked(false),
      m_deleted(false),
      m_next(0),
      m_dead(0)
{
}

/** Locks the node and adds the given child. */
template<typename T>
bool ListNode<T>::AddChild(ListNode<T>* child, ListNode<T>* successor) volatile
{
    ScopedLock lock(*this);
    if (!m_deleted && m_next == successor)
    {
        child->m_next = m_next;
        m_next = child;
        return true;
    }
    return false;
}

/** Logically deletes this node. 
    Node must be locked while doing so to prevent the node from being
    deleted inside another thread's AddChild() call. */
template<typename T>
void ListNode<T>::Delete() volatile
{
    ScopedLock lock(*this);
    m_deleted = true;
}

/** Attempts to physically delete a logically deleted node. */
template<typename T>
void ListNode<T>::TryFixLink(ListNode<T>* node) volatile
{
    ScopedLock lock(*this);
    if (m_next == node)
        m_next = node->m_next;
}

/** Returns the next node in the list. 
    Tries to physically delete any logically deleted nodes it
    encounters along the way. */
template<typename T>
ListNode<T>* ListNode<T>::GetNext() volatile
{
    ListNode<T>* node = m_next;
    while ((node != 0) && node->m_deleted)
    {
        TryFixLink(node);
        node = node->m_next;
    }
    return node;
}

//----------------------------------------------------------------------------

/** Pool of pre-allocated memory. 
    Allocates more memory when current amount is used up.
*/
template<typename T>
class Pool : boost::noncopyable
{
public:

    static const std::size_t CHUNK_SIZE = 1 << 24; // 16MB

    Pool();

    ~Pool();

    std::size_t Allocated() const;

    std::size_t ChunkSize() const;

    /** Grabs memory from pool in threadsafe manner. */
    ListNode<T>* Get() volatile;

    /** Adds a node to the list of dead nodes in a threadsafe manner. */
    void AddToDeadList(ListNode<T>* node) volatile;

    /** Puts node back on pool, not threadsafe. */
    void Put(ListNode<T>* node);

    /** Returns dead nodes to pool. */
    void RaiseTheDead();

private:
    ListNode<T>* m_head;
    
    ListNode<T>* m_dead;

    bool m_lockedHead;

    bool m_lockedDead;

    std::size_t m_allocated;

    std::size_t m_chunkSize;

    std::vector<ListNode<T>* > m_chunks;

    void Allocate();
};

template<typename T>
Pool<T>::Pool()
    : m_head(0),
      m_dead(0),
      m_lockedHead(false),
      m_lockedDead(false),
      m_allocated(0),
      m_chunkSize(CHUNK_SIZE)
{
    Allocate();
}

template<typename T>
Pool<T>::~Pool()
{
    for (std::size_t i = 0; i < m_chunks.size(); ++i)
        delete [] m_chunks[i];
}

template<typename T>
std::size_t Pool<T>::Allocated() const
{
    return m_allocated;
}

template<typename T>
std::size_t Pool<T>::ChunkSize() const
{
    return m_chunkSize;
}

template<typename T>
void Pool<T>::Allocate()
{
    BenzeneAssert(m_head == 0);
    std::size_t num = m_chunkSize / sizeof(ListNode<T>);
    ListNode<T>* data = new ListNode<T>[num];
    m_chunks.push_back(data);
    for (std::size_t i = 0; i < num - 1; ++i)
        data[i].m_next = &data[i + 1];
    data[num - 1].m_next = 0;
    m_head = data;
    m_allocated += num * sizeof(ListNode<T>);
}

template<typename T>
ListNode<T>* Pool<T>::Get() volatile
{
    while (!CompareAndSwap(&m_lockedHead, false, true));
    if (m_head == 0)
    {
        // Strip volatile-ness since we are locked
        Pool<T>& ref = const_cast<Pool<T>&>(*this);
        ref.Allocate();
    }
    ListNode<T>* ret = m_head;
    m_head = m_head->m_next;
    m_lockedHead = false;
    return ret;
}

template<typename T>
void Pool<T>::AddToDeadList(ListNode<T>* node) volatile
{
    while (!CompareAndSwap(&m_lockedDead, false, true));
    node->m_dead = m_dead;
    m_dead = node;
    m_lockedDead = false;
}

template<typename T>
void Pool<T>::RaiseTheDead()
{
    ListNode<T>* node = m_dead;
    while (node)
    {
        ListNode<T>* next = node->m_dead;
        node->m_dead = 0;
        Put(node);
        node = next;
    }
    m_dead = 0;
}

template<typename T>
void Pool<T>::Put(ListNode<T>* node)
{
    node->m_next = m_head;
    m_head = node;
}

//----------------------------------------------------------------------------

/** Lock-free linked list. 
    Uses memory from the supplied Pool object. 
*/
template<typename T>
class LinkedList
{
public:
    LinkedList(Pool<T>& pool);

    LinkedList(const LinkedList<T>& other);

    virtual ~LinkedList();

    bool Empty() const;

    void Add(const T& data);

    void Remove(const T& data);
    
    void Clear();

    void operator=(const LinkedList<T>& other);

    bool operator==(const LinkedList<T>& other) const;

private:
    template <typename U> friend class ListIterator;

    Pool<T>& m_pool;

    mutable ListNode<T> m_head;

    void CopyList(const LinkedList<T>& other);
};

template<typename T>
LinkedList<T>::LinkedList(Pool<T>& pool)
    : m_pool(pool)
{
    m_head.m_next = 0;
}

template<typename T>
LinkedList<T>::LinkedList(const LinkedList<T>& other)
    : m_pool(other.m_pool)
{
    CopyList(other);
}

template<typename T>
LinkedList<T>::~LinkedList()
{
}

template<typename T>
bool LinkedList<T>::Empty() const
{
    return m_head.GetNext() == 0;
}

template<typename T>
void LinkedList<T>::Clear()
{
    ListNode<T>* node = m_head.GetNext();
    while (node != 0)
    {
        ListNode<T>* next = node.GetNext();
        m_pool.Put(node);
        node = next;
    }
    m_head.m_next = 0;
}

template<typename T>
void LinkedList<T>::CopyList(const LinkedList<T>& other)
{
    BenzeneAssert(m_head.m_next == 0);
    ListNode<T>* them = other.m_head.GetNext();
    ListNode<T>* mine = m_head;
    while (them != 0)
    {
        ListNode<T>* child = new (m_pool.Get()) ListNode<T>(them->m_data);
        mine->AddChild(child);
        mine = mine->GetNext();
        them = them->GetNext();
    }
}

template<typename T>
void LinkedList<T>::Add(const T& data)
{
    ListNode<T>* node = 0;
    while (true)
    {
        ListNode<T>* current = &m_head;
        ListNode<T>* next = current->GetNext();
        while ((next != 0) && next->m_data < data)
        {
            current = next;
            next = current->GetNext();
        }
        // At this point: (next == 0 || next->m_data >= data)

        // Check for duplicate
        if ((next != 0) && next->m_data == data)
            break;

        // Try to add it in this location
        if (node == 0)
            node = new (m_pool.Get()) ListNode<T>(data);
        if (current->AddChild(node, next))
            break;
    }
}

template<typename T>
void LinkedList<T>::Remove(const T& data)
{
    ListNode<T>* parent = &m_head;
    ListNode<T>* next = parent->GetNext();
    while ((next != 0) && next->m_data != data)
    {
        parent = next;
        next = parent->GetNext();
    }
    if (next != 0) 
    {
        next->Delete();
        parent->TryFixLink(next);
        m_pool.AddToDeadList(next);
    }
}

template<typename T>
void LinkedList<T>::operator=(const LinkedList<T>& other)
{
    Clear();
    CopyList(other);
}

template<typename T>
bool LinkedList<T>::operator==(const LinkedList<T>& other) const
{
    ListNode<T>* mine = m_head.GetNext();
    ListNode<T>* them = other.m_head.GetNext();
    while ((mine != 0) && (them != 0))
    {
        if (mine->m_data != them->m_data)
            return false;
        mine = mine->GetNext();
        them = them->GetNext();
    }
    return (mine != 0) == (them != 0);
}

//----------------------------------------------------------------------------

/** Iterates over an instance of LinkedList. */
template<typename T>
class ListIterator : public SafeBool<ListIterator<T> >
{
public:
    ListIterator(LinkedList<T>& lst);

    T& operator*();

    ListIterator<T>& operator++();
    
    bool boolean_test() const;

private:
    ListNode<T>* m_current;

    template <typename U> friend class LinkedList;
};

template<typename T>
ListIterator<T>::ListIterator(LinkedList<T>& lst)
    : m_current(lst.m_head.GetNext())
{
}

template<typename T>
T& ListIterator<T>::operator*()
{
    return m_current->m_data;
}

template<typename T>
ListIterator<T>& ListIterator<T>::operator++()
{
    m_current = m_current->GetNext();
    return *this;
}

template<typename T>
bool ListIterator<T>::boolean_test() const
{
    return m_current != 0;
}

//----------------------------------------------------------------------------

_END_BENZENE_NAMESPACE_

#endif // LINKEDLIST_HPP
