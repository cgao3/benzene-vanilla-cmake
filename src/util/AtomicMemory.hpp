//----------------------------------------------------------------------------
/** @file AtomicMemory.hpp */
//----------------------------------------------------------------------------

#ifndef ATOMICMEMORY_HPP
#define ATOMICMEMORY_HPP

#include "config.h"
#include "Benzene.hpp"

_BEGIN_BENZENE_NAMESPACE_

//----------------------------------------------------------------------------

/** Performs fetch and add.
    Equivalent to @c { T tmp = *ptr; *ptr += value; return tmp; } @c
    Guaranteed to be atomic if HAVE_GCC_ATOMIC_BUILTINS is defined.
 */
template<typename T>
inline T FetchAndAdd(volatile T* ptr, T value)
{
#if HAVE_GCC_ATOMIC_BUILTINS    
    return __sync_fetch_and_add(ptr, value);
#else
    T tmp = *ptr;
    *ptr += value;
    return tmp;
#endif
} 

/** Performs compare and swap.  
    If the contents of ptr equal to oldValue, stores newValue in ptr
    and returns true, otherwise returns false. This operation is
    guaranteed to be atomic if HAVE_GCC_ATOMIC_BUILTINS is defined. */
template<typename T>
inline bool CompareAndSwap(volatile T* ptr, T oldValue, T newValue)
{
#if HAVE_GCC_ATOMIC_BUILTINS
    return __sync_bool_compare_and_swap(ptr, oldValue, newValue);
#else
    if (*ptr == oldValue)
    {
        *ptr = newValue;
        return true;
    }
    return false;
#endif
}

//----------------------------------------------------------------------------

_END_BENZENE_NAMESPACE_

#endif // ATOMICMEMORY_HPP
