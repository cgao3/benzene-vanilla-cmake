//----------------------------------------------------------------------------
/** @file BenzeneAssert.hpp */
//----------------------------------------------------------------------------

#ifndef BENZENEASSERT_HPP
#define BENZENEASSERT_HPP

//----------------------------------------------------------------------------

#define __BENZENE_ASSERT_VOID_CAST static_cast<void>

extern void BenzeneAssertShutdown(const char* assertion, 
                                  const char* file, int line,
                                  const char* function)
        __attribute__ ((__noreturn__));

#ifdef NDEBUG

#define BenzeneAssert(expr)     __BENZENE_ASSERT_VOID_CAST(0)

#else

#define BenzeneAssert(expr) \
    (__BENZENE_ASSERT_VOID_CAST ((expr) ? 0 :                               \
                             (BenzeneAssertShutdown(#expr,                  \
                                                    __FILE__, __LINE__,     \
                                                    __PRETTY_FUNCTION__), 0)))

#endif // NDEBUG

//----------------------------------------------------------------------------

#endif // BENZENEASSERT_HPP

