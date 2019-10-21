/* Add __has_feature () macro
   Verify that errors are detected and handled gracefully.
{ dg-do compile }
{ dg-options "-fsyntax-only" }
*/

#ifndef __has_feature
#  error "__has_feature is not defined"
#endif

#if __has_feature             // { dg-error "missing '\\\(' after \"__has_feature\"" }
#endif

#if __has_feature (           // { dg-error "macro \"__has_feature\" requires an identifier" }
#endif

#if __has_feature ()          // { dg-error "macro \"__has_feature\" requires an identifier" }
#endif

#if __has_feature (1)         // { dg-error "macro \"__has_feature\" requires an identifier" }
#endif

#if __has_feature (1, 2)      // { dg-error "macro \"__has_feature\" requires an identifier" }
#endif

#if __has_feature (1 + 2)     // { dg-error "macro \"__has_feature\" requires an identifier" }
#endif

#if __has_feature (x, y)      // { dg-error "expected '\\\)' after \"x\"" } */
#endif

#if __has_feature (x + 1)     // { dg-error "expected '\\\)' after \"x\"" } */
#endif

#if __has_feature (p->i)      // { dg-error "expected '\\\)' after \"p\"" } */
#endif

#if __has_feature ((x))       // { dg-error "macro \"__has_feature\" requires an identifier" }
#endif

#if __has_feature ((y)        // { dg-error "macro \"__has_feature\" requires an identifier" }
#endif

#if __has_feature ((((z)      // { dg-error "macro \"__has_feature\" requires an identifier" }
#endif

#if __has_feature (x)))       // { dg-error "missing '\\\('" }"
#endif

#if __has_feature (f ())      // { dg-error "expected '\\\)' after \"f\"" }"
#endif
