//----------------------------------------------------------------------------
/** @file Benzene.hpp
 */
//----------------------------------------------------------------------------

#ifndef BENZENE_HPP
#define BENZENE_HPP

//----------------------------------------------------------------------------

/** Suppresses compiler warnings about unused variables. 

    This is not the best solution, since it's possible to call this
    function on a variable and then use it later on. The macro defined
    below is better in that it prevents this from happening, but
    confuses doxygen. We use this one for now as a compromise.
*/
template <typename T>
inline void UNUSED(const T&)
{
}

#if 0

/** Marks a parameter as unusable and suppresses compiler the warning
    that it is unused. 
    
    CURRENTLY UNUSED since mangling names in function definitions
    confuses doxygen.
*/
#ifdef UNUSED
#elif defined(__GNUC__)
# define UNUSED(x) UNUSED_ ## x __attribute__((unused))
#elif defined(__LCLINT__)
# define UNUSED(x) /*@unused@*/ x
#else
# define UNUSED(x) x
#endif

#endif

//----------------------------------------------------------------------------

/** Begins the benzene namespace. 
    Should be used in all header files, immediately after the include
    directives. 
*/
#define _BEGIN_BENZENE_NAMESPACE_ namespace benzene {

/** Ends the benzene namespace. 
    Should be used at the end of every header file. 
*/
#define _END_BENZENE_NAMESPACE_ }

//----------------------------------------------------------------------------

#endif // BENZENE_HPP
