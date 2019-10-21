/* Add __has_extension () macro
   Verify that errors are detected and handled gracefully.
{ dg-do compile }
{ dg-options "-fsyntax-only" }
*/

#ifndef __has_extension
#  error "__has_extension is not defined"
#endif

#if __has_extension             // { dg-error "missing '\\\(' after \"__has_extension\"" }
#endif

#if __has_extension (           // { dg-error "macro \"__has_extension\" requires an identifier" }
#endif

#if __has_extension ()          // { dg-error "macro \"__has_extension\" requires an identifier" }
#endif

#if __has_extension (1)         // { dg-error "macro \"__has_extension\" requires an identifier" }
#endif

#if __has_extension (1, 2)      // { dg-error "macro \"__has_extension\" requires an identifier" }
#endif

#if __has_extension (1 + 2)     // { dg-error "macro \"__has_extension\" requires an identifier" }
#endif

#if __has_extension (x, y)      // { dg-error "expected '\\\)' after \"x\"" } */
#endif

#if __has_extension (x + 1)     // { dg-error "expected '\\\)' after \"x\"" } */
#endif

#if __has_extension (p->i)      // { dg-error "expected '\\\)' after \"p\"" } */
#endif

#if __has_extension ((x))       // { dg-error "macro \"__has_extension\" requires an identifier" }
#endif

#if __has_extension ((y)        // { dg-error "macro \"__has_extension\" requires an identifier" }
#endif

#if __has_extension ((((z)      // { dg-error "macro \"__has_extension\" requires an identifier" }
#endif

#if __has_extension (x)))       // { dg-error "missing '\\\('" }"
#endif

#if __has_extension (f ())      // { dg-error "expected '\\\)' after \"f\"" }"
#endif
