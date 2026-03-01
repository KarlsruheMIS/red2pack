//
// From KaHIP
//

#ifndef RED2PACK_MACROS_H
#define RED2PACK_MACROS_H

// Helper macro for STR.
#define ASSERT_H_XSTR(x) (#x)

// This macro allows to convert an expression to a string.
#define STR(x) ASSERT_H_XSTR(x)

// A custom assertion macro that does not kill the program but prints to
// stderr instead.
#if (defined(NDEBUG) || defined(SPEEDPROFILING))
# define RED2PACK_ASSERT_TRUE(x) do {} while (false);
#else
# define RED2PACK_ASSERT_TRUE(expression) \
        do { \
                if (not (expression)) { \
                        std::cerr << "ASSERTION FAILED [" << __FILE__ << ":" << __LINE__ << \
                        "]. Asserted: " << STR(expression) << std::endl; \
                        abort(); \
                } \
        } while (false)
#endif


#endif  // RED2PACK_MACROS_H
