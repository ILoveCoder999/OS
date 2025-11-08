

#ifndef _ASSERT_H_
#define _ASSERT_H_

/*
 * Required by the C standard
 */
#undef assert

/*
 * Function to call when an assert fails.
 */
void __bad_assert(const char *file, int line, const char *msg);

/*
 * Asserts are only "on" if NDEBUG isn't set. (This is standard C.)
 */

#ifdef NDEBUG

#if 0 /* not allowed by the C standard */
#define assert(x) ((void)(x))	/* retain any side effects of X */
#else
#define assert(x) ((void)0)	/* mysteriously hide any side effects of X */
#endif

#else
#define assert(x) ((x) ? (void)0 : __bad_assert(__FILE__, __LINE__, #x))
#endif


#endif /* _ASSERT_H_ */
