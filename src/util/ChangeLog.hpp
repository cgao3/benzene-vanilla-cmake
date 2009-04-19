//----------------------------------------------------------------------------
/** @file
 */
//----------------------------------------------------------------------------

#ifndef CHANGELOG_HPP
#define CHANGELOG_HPP

#include <list>
#include <vector>
#include <string>
#include <sstream>
#include "Benzene.hpp"

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
    bool empty() const;

    /** Returns size of changelog. */
    int size() const;

    /** Adds an entry onto the changelog. */
    void push(Action action, const T& data);

    /** Pops the top entry off of the changelog. Asserts log is not
	empty.
    */
    void pop();

    /** Returns the action on top of the changelog. Asserts log is not
	empty.
        
        @todo how to write this method using the 'inline' syntax?  I
        keep getting compile errors.
    */
    Action topAction() const { assert(!empty()); return m_action.back(); }

    /** Returns a copy of the data on top of the changelog. Asserts log is
	not empty.
    */
    T topData() const;

    /** Clears the log. */
    void clear();

    /** Dump the contents of the log to a string. */
    std::string dump() const;

private:
    std::vector<T> m_data;
    std::vector<Action> m_action;
};

template<typename T>
inline ChangeLog<T>::ChangeLog()
{
}

template<typename T> 
inline bool ChangeLog<T>::empty() const
{
    return m_action.empty();
}

template<typename T> 
inline int ChangeLog<T>::size() const
{
    return m_action.size();
}

template<typename T> 
inline void ChangeLog<T>::push(Action action, const T& data)
{
    m_action.push_back(action);
    m_data.push_back(data);
}

template<typename T>
inline void ChangeLog<T>::pop()
{
    assert(!empty());
    m_action.pop_back();
    m_data.pop_back();
}

template<typename T>
inline T ChangeLog<T>::topData() const
{
    assert(!empty());
    return m_data.back();
}

/// @todo vector::clear() takes linear time.  Is this a problem?
template<typename T>
inline void ChangeLog<T>::clear()
{
    m_action.clear();
    m_data.clear();
}

template<typename T>
std::string ChangeLog<T>::dump() const
{
    std::ostringstream os;

    for (int i=0; i<(int)m_action.size(); ++i) {
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
        os << std::endl;
    }
    return os.str();
}

//----------------------------------------------------------------------------

_END_BENZENE_NAMESPACE_

#endif // CHANGELOG_HPP
