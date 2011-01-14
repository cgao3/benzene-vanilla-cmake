//----------------------------------------------------------------------------
/** @file ChangeLog.hpp */
//----------------------------------------------------------------------------

#ifndef CHANGELOG_HPP
#define CHANGELOG_HPP

#include <list>
#include <vector>
#include <string>
#include <sstream>
#include "Benzene.hpp"
#include "BenzeneAssert.hpp"

_BEGIN_BENZENE_NAMESPACE_

//----------------------------------------------------------------------------

/** General purpose changelog; allows incremental changes made to a
    datastructure to be undone quickly.
    
    A changelog is a stack that tracks the changes to some data
    structure with data type T.  There are three actions: ADD, REMOVE,
    and MARKER.  And ADD action means the data was recently added to the
    datastructure, and REMOVE means it was recently removed.  MARKER is
    used to mark how far back to go when you want to undo the changes
    made.
*/
template<typename T>
class ChangeLog
{
public:
    /** Constructor. */
    ChangeLog();

    /** Available actions. */
    typedef enum {ADD, REMOVE, PROCESSED, MARKER} Action;

    /** Returns true if changelog is empty. */
    bool Empty() const;

    /** Returns size of changelog. */
    std::size_t Size() const;

    /** Adds an entry onto the changelog. */
    void Push(Action action, const T& data);

    /** Pops the top entry off of the changelog. Asserts log is not
	empty. */
    void Pop();

    /** Returns the action on top of the changelog. Asserts log is not
	empty. */
    Action TopAction() const;

    /** Returns a copy of the data on top of the changelog. Asserts log is
	not empty. */
    T TopData() const;

    /** Clears the log. */
    void Clear();

    /** Dump the contents of the log to a string. */
    std::string Dump() const;

private:
    std::vector<T> m_data;

    std::vector<Action> m_action;
};

template<typename T>
inline ChangeLog<T>::ChangeLog()
{
}

template<typename T> 
inline bool ChangeLog<T>::Empty() const
{
    return m_action.empty();
}

template<typename T> 
inline std::size_t ChangeLog<T>::Size() const
{
    return m_action.size();
}

template<typename T> 
inline void ChangeLog<T>::Push(Action action, const T& data)
{
    m_action.push_back(action);
    m_data.push_back(data);
}

template<typename T>
inline void ChangeLog<T>::Pop()
{
    BenzeneAssert(!Empty());
    m_action.pop_back();
    m_data.pop_back();
}

template<typename T>
inline typename ChangeLog<T>::Action ChangeLog<T>::TopAction() const 
{ 
    BenzeneAssert(!Empty()); 
    return m_action.back();
}

template<typename T>
inline T ChangeLog<T>::TopData() const
{
    BenzeneAssert(!Empty());
    return m_data.back();
}

/// @todo Takes linear time--is this a problem?
template<typename T>
inline void ChangeLog<T>::Clear()
{
    m_action.clear();
    m_data.clear();
}

template<typename T>
std::string ChangeLog<T>::Dump() const
{
    std::ostringstream os;
    for (std::size_t i = 0; i < m_action.size(); ++i) 
    {
        os << i << ": ";
        if (m_action[i] == MARKER)
            os << "MARKER";
        else {
            if (m_action[i] == ADD)
                os << "   ADD: ";
            else if (m_action[i] == REMOVE)
                os << "REMOVE: ";
            else if (m_action[i] == PROCESSED)
                os << "PROCESSED: ";
            os << m_data[i];
        }
        os << '\n';
    }
    return os.str();
}

//----------------------------------------------------------------------------

_END_BENZENE_NAMESPACE_

#endif // CHANGELOG_HPP
