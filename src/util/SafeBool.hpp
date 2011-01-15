//----------------------------------------------------------------------------
/** @file SafeBool.hpp */
//----------------------------------------------------------------------------

#ifndef SAFEBOOL_HPP
#define SAFEBOOL_HPP

#include "Benzene.hpp"

_BEGIN_BENZENE_NAMESPACE_

//----------------------------------------------------------------------------

/** Implements the Safe Bool Idiom.  Classes deriving off of SafeBool
    need to implement a 'bool boolean_test()' method.  See:
    http://www.artima.com/cppsource/safebool.html. */
template <typename T> 
class SafeBool
{
protected:
    typedef void (SafeBool::*bool_type)() const;

    void this_type_does_not_support_comparisons() const {}

public:
    operator bool_type() const
    {
        return (static_cast<const T*>(this))->boolean_test() 
            ? &SafeBool::this_type_does_not_support_comparisons : 0;
    }
};

/** Invalidates operator!= */
template <typename T, typename U> 
bool operator!=(const SafeBool<T>& lhs, const SafeBool<U>& rhs) 
{
    lhs.this_type_does_not_support_comparisons();
    return false;	
} 

/** Invalidates operator== */
template <typename T, typename U> 
bool operator==(const SafeBool<T>& lhs, const SafeBool<U>& rhs) 
{
    lhs.this_type_does_not_support_comparisons();
    return false;	
} 

//---------------------------------------------------------------------------

_END_BENZENE_NAMESPACE_

#endif // SAFEBOOL_HPP
