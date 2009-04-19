//----------------------------------------------------------------------------
/** @file
 */
//----------------------------------------------------------------------------

#ifndef HEXASSERT_HPP
#define HEXASSERT_HPP

//----------------------------------------------------------------------------

#define __HEX_ASSERT_VOID_CAST static_cast<void>

extern void HexAssertShutdown(const char* assertion, 
                              const char* file, int line,
                              const char* function)
        __attribute__ ((__noreturn__));


#ifdef NDEBUG

#define HexAssert(expr)     __HEX_ASSERT_VOID_CAST(0)

#else

#define HexAssert(expr) \
    (__HEX_ASSERT_VOID_CAST ((expr) ? 0 :                               \
                             (HexAssertShutdown(#expr,                  \
                                                __FILE__, __LINE__,     \
                                                __PRETTY_FUNCTION__), 0)))

#endif // NDEBUG

//----------------------------------------------------------------------------

#endif // HEXASSERT_HPP

