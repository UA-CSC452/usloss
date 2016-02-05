
#if !defined(_project_h)
#define _project_h

/*
 *  Project-specific "storage classes."  Labelled as "dynamic" as a pun on
 *	the "static" storage class.  All "dynamic"ally declared objects
 *	should only be called by other USLOSS routines.  Naturally, this
 *	is only enforced when building a single monolithic simulator object
 *	file, by making everything declared "dynamic" to be static, and
 *	thus not visible to routines linked with the object file.
 *
 *	dynamic_dcl	- used to declare functions or variables.
 *	dynamic_fun	- used to define functions.
 *	dynamic_def()	- used to define variables.  This is done to avoid
 *			  the error message generated when a variable is
 *			  declared extern or 'normal' in a module and later
 *			  defined as a static variable.  The result is that
 *			  the variable is defined static by the previous
 *			  "declaration" (usually in a header file) and the
 *			  "definition" made by dynamic_def() is eliminated
 *			  and replaced with an innocuous 'extern' declaration.
 */

#if defined(MAKELIB)
#define dynamic_dcl extern
#define dynamic_fun
#define dynamic_def(a) a
#else
#define dynamic_dcl static
#define dynamic_fun static
#define dynamic_def(a) extern int __bogus ## __LINE__
#endif

/*
 *  So you think ANSI C++ is upwards compatible with ANSI C??  HA!!  Check out
 *  this little hack to get around some function typecasting problems (as shown
 *  in sig_ints.c[c]).
 */
#if defined(__cplusplus)
#define FN_CAST void (*)(...)
#else
#define FN_CAST void (*)(void)
#endif

#endif	/*  _project_h */

