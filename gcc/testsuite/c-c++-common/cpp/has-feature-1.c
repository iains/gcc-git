/* Add __has_feature () macro
   Verify __has_feature evaluation...
{ dg-do compile }
{ dg-options "-fsyntax-only" }
*/

#ifndef __has_feature
#  error "__has_feature is not defined"
#endif

#if !__has_feature (test_feature)
#  error "__has_feature (test_feature) failed"
#endif
