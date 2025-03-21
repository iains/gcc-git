/* Copyright (C) 2003-2025 Free Software Foundation, Inc.

   This file is part of GCC.

   GCC is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 3, or (at your option)
   any later version.

   GCC is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   Under Section 7 of GPL version 3, you are granted additional
   permissions described in the GCC Runtime Library Exception, version
   3.1, as published by the Free Software Foundation.

   You should have received a copy of the GNU General Public License and
   a copy of the GCC Runtime Library Exception along with this program;
   see the files COPYING3 and COPYING.RUNTIME respectively.  If not, see
   <http://www.gnu.org/licenses/>.  */

/* Implemented from the specification included in the Intel C++ Compiler
   User Guide and Reference, version 9.0.  */

#ifndef NO_WARN_X86_INTRINSICS
/* This header is distributed to simplify porting x86_64 code that
   makes explicit use of Intel intrinsics to powerpc64le.
   It is the user's responsibility to determine if the results are
   acceptable and make additional changes as necessary.
   Note that much code that uses Intel intrinsics can be rewritten in
   standard C or GNU C extensions, which are more portable and better
   optimized across multiple targets.  */
#endif

#ifndef TMMINTRIN_H_
#define TMMINTRIN_H_

#include <altivec.h>
#include <assert.h>

/* We need definitions from the SSE header files.  */
#include <pmmintrin.h>

extern __inline __m128i
__attribute__((__gnu_inline__, __always_inline__, __artificial__))
_mm_abs_epi16 (__m128i __A)
{
  return (__m128i) vec_abs ((__v8hi) __A);
}

extern __inline __m128i
__attribute__((__gnu_inline__, __always_inline__, __artificial__))
_mm_abs_epi32 (__m128i __A)
{
  return (__m128i) vec_abs ((__v4si) __A);
}

extern __inline __m128i
__attribute__((__gnu_inline__, __always_inline__, __artificial__))
_mm_abs_epi8 (__m128i __A)
{
  return (__m128i) vec_abs ((__v16qi) __A);
}

extern __inline __m64
__attribute__((__gnu_inline__, __always_inline__, __artificial__))
_mm_abs_pi16 (__m64 __A)
{
  __v8hi __B = (__v8hi) (__v2du) { __A, __A };
  return (__m64) ((__v2du) vec_abs (__B))[0];
}

extern __inline __m64
__attribute__((__gnu_inline__, __always_inline__, __artificial__))
_mm_abs_pi32 (__m64 __A)
{
  __v4si __B = (__v4si) (__v2du) { __A, __A };
  return (__m64) ((__v2du) vec_abs (__B))[0];
}

extern __inline __m64
__attribute__((__gnu_inline__, __always_inline__, __artificial__))
_mm_abs_pi8 (__m64 __A)
{
  __v16qi __B = (__v16qi) (__v2du) { __A, __A };
  return (__m64) ((__v2du) vec_abs (__B))[0];
}

extern __inline __m128i
__attribute__((__gnu_inline__, __always_inline__, __artificial__))
_mm_alignr_epi8 (__m128i __A, __m128i __B, const unsigned int __count)
{
  if (__builtin_constant_p (__count) && __count < 16)
    {
#ifdef __LITTLE_ENDIAN__
      __A = (__m128i) vec_reve ((__v16qu) __A);
      __B = (__m128i) vec_reve ((__v16qu) __B);
#endif
      __A = (__m128i) vec_sld ((__v16qu) __B, (__v16qu) __A, __count);
#ifdef __LITTLE_ENDIAN__
      __A = (__m128i) vec_reve ((__v16qu) __A);
#endif
      return __A;
    }

  if (__count == 0)
    return __B;

  if (__count >= 16)
    {
      if (__count >= 32)
	{
	  const __v16qu __zero = { 0 };
	  return (__m128i) __zero;
	}
      else
	{
	  const __v16qu __shift =
	    vec_splats ((unsigned char) ((__count - 16) * 8));
#ifdef __LITTLE_ENDIAN__
	  return (__m128i) vec_sro ((__v16qu) __A, __shift);
#else
	  return (__m128i) vec_slo ((__v16qu) __A, __shift);
#endif
	}
    }
  else
    {
      const __v16qu __shiftA =
	vec_splats ((unsigned char) ((16 - __count) * 8));
      const __v16qu __shiftB = vec_splats ((unsigned char) (__count * 8));
#ifdef __LITTLE_ENDIAN__
      __A = (__m128i) vec_slo ((__v16qu) __A, __shiftA);
      __B = (__m128i) vec_sro ((__v16qu) __B, __shiftB);
#else
      __A = (__m128i) vec_sro ((__v16qu) __A, __shiftA);
      __B = (__m128i) vec_slo ((__v16qu) __B, __shiftB);
#endif
      return (__m128i) vec_or ((__v16qu) __A, (__v16qu) __B);
    }
}

extern __inline __m64
__attribute__((__gnu_inline__, __always_inline__, __artificial__))
_mm_alignr_pi8 (__m64 __A, __m64 __B, unsigned int __count)
{
  if (__count < 16)
    {
      __v2du __C = { __B, __A };
#ifdef __LITTLE_ENDIAN__
      const __v4su __shift = { __count << 3, 0, 0, 0 };
      __C = (__v2du) vec_sro ((__v16qu) __C, (__v16qu) __shift);
#else
      const __v4su __shift = { 0, 0, 0, __count << 3 };
      __C = (__v2du) vec_slo ((__v16qu) __C, (__v16qu) __shift);
#endif
      return (__m64) __C[0];
    }
  else
    {
      const __m64 __zero = { 0 };
      return __zero;
    }
}

extern __inline __m128i
__attribute__((__gnu_inline__, __always_inline__, __artificial__))
_mm_hadd_epi16 (__m128i __A, __m128i __B)
{
  const __v16qu __P =
    {  0,  1,  4,  5,  8,  9, 12, 13, 16, 17, 20, 21, 24, 25, 28, 29 };
  const __v16qu __Q =
    {  2,  3,  6,  7, 10, 11, 14, 15, 18, 19, 22, 23, 26, 27, 30, 31 };
  __v8hi __C = vec_perm ((__v8hi) __A, (__v8hi) __B, __P);
  __v8hi __D = vec_perm ((__v8hi) __A, (__v8hi) __B, __Q);
  return (__m128i) vec_add (__C, __D);
}

extern __inline __m128i
__attribute__((__gnu_inline__, __always_inline__, __artificial__))
_mm_hadd_epi32 (__m128i __A, __m128i __B)
{
  const __v16qu __P =
    {  0,  1,  2,  3,  8,  9, 10, 11, 16, 17, 18, 19, 24, 25, 26, 27 };
  const __v16qu __Q =
    {  4,  5,  6,  7, 12, 13, 14, 15, 20, 21, 22, 23, 28, 29, 30, 31 };
  __v4si __C = vec_perm ((__v4si) __A, (__v4si) __B, __P);
  __v4si __D = vec_perm ((__v4si) __A, (__v4si) __B, __Q);
  return (__m128i) vec_add (__C, __D);
}

extern __inline __m64
__attribute__((__gnu_inline__, __always_inline__, __artificial__))
_mm_hadd_pi16 (__m64 __A, __m64 __B)
{
  __v8hi __C = (__v8hi) (__v2du) { __A, __B };
  const __v16qu __P =
    {  0,  1,  4,  5,  8,  9, 12, 13,  0,  1,  4,  5,  8,  9, 12, 13 };
  const __v16qu __Q =
    {  2,  3,  6,  7, 10, 11, 14, 15,  2,  3,  6,  7, 10, 11, 14, 15 };
  __v8hi __D = vec_perm (__C, __C, __Q);
  __C = vec_perm (__C, __C, __P);
  __C = vec_add (__C, __D);
  return (__m64) ((__v2du) __C)[1];
}

extern __inline __m64
__attribute__((__gnu_inline__, __always_inline__, __artificial__))
_mm_hadd_pi32 (__m64 __A, __m64 __B)
{
  __v4si __C = (__v4si) (__v2du) { __A, __B };
  const __v16qu __P =
    {  0,  1,  2,  3,  8,  9, 10, 11,  0,  1,  2,  3,  8,  9, 10, 11 };
  const __v16qu __Q =
    {  4,  5,  6,  7, 12, 13, 14, 15,  4,  5,  6,  7, 12, 13, 14, 15 };
  __v4si __D = vec_perm (__C, __C, __Q);
  __C = vec_perm (__C, __C, __P);
  __C = vec_add (__C, __D);
  return (__m64) ((__v2du) __C)[1];
}

extern __inline __m128i
__attribute__((__gnu_inline__, __always_inline__, __artificial__))
_mm_hadds_epi16 (__m128i __A, __m128i __B)
{
  __v4si __C = { 0 }, __D = { 0 };
  __C = vec_sum4s ((__v8hi) __A, __C);
  __D = vec_sum4s ((__v8hi) __B, __D);
  __C = (__v4si) vec_packs (__C, __D);
  return (__m128i) __C;
}

extern __inline __m64
__attribute__((__gnu_inline__, __always_inline__, __artificial__))
_mm_hadds_pi16 (__m64 __A, __m64 __B)
{
  const __v4si __zero = { 0 };
  __v8hi __C = (__v8hi) (__v2du) { __A, __B };
  __v4si __D = vec_sum4s (__C, __zero);
  __C = vec_packs (__D, __D);
  return (__m64) ((__v2du) __C)[1];
}

extern __inline __m128i
__attribute__((__gnu_inline__, __always_inline__, __artificial__))
_mm_hsub_epi16 (__m128i __A, __m128i __B)
{
  const __v16qu __P =
    {  0,  1,  4,  5,  8,  9, 12, 13, 16, 17, 20, 21, 24, 25, 28, 29 };
  const __v16qu __Q =
    {  2,  3,  6,  7, 10, 11, 14, 15, 18, 19, 22, 23, 26, 27, 30, 31 };
  __v8hi __C = vec_perm ((__v8hi) __A, (__v8hi) __B, __P);
  __v8hi __D = vec_perm ((__v8hi) __A, (__v8hi) __B, __Q);
  return (__m128i) vec_sub (__C, __D);
}

extern __inline __m128i
__attribute__((__gnu_inline__, __always_inline__, __artificial__))
_mm_hsub_epi32 (__m128i __A, __m128i __B)
{
  const __v16qu __P =
    {  0,  1,  2,  3,  8,  9, 10, 11, 16, 17, 18, 19, 24, 25, 26, 27 };
  const __v16qu __Q =
    {  4,  5,  6,  7, 12, 13, 14, 15, 20, 21, 22, 23, 28, 29, 30, 31 };
  __v4si __C = vec_perm ((__v4si) __A, (__v4si) __B, __P);
  __v4si __D = vec_perm ((__v4si) __A, (__v4si) __B, __Q);
  return (__m128i) vec_sub (__C, __D);
}

extern __inline __m64
__attribute__((__gnu_inline__, __always_inline__, __artificial__))
_mm_hsub_pi16 (__m64 __A, __m64 __B)
{
  const __v16qu __P =
    {  0,  1,  4,  5,  8,  9, 12, 13,  0,  1,  4,  5,  8,  9, 12, 13 };
  const __v16qu __Q =
    {  2,  3,  6,  7, 10, 11, 14, 15,  2,  3,  6,  7, 10, 11, 14, 15 };
  __v8hi __C = (__v8hi) (__v2du) { __A, __B };
  __v8hi __D = vec_perm (__C, __C, __Q);
  __C = vec_perm (__C, __C, __P);
  __C = vec_sub (__C, __D);
  return (__m64) ((__v2du) __C)[1];
}

extern __inline __m64
__attribute__((__gnu_inline__, __always_inline__, __artificial__))
_mm_hsub_pi32 (__m64 __A, __m64 __B)
{
  const __v16qu __P =
    {  0,  1,  2,  3,  8,  9, 10, 11,  0,  1,  2,  3,  8,  9, 10, 11 };
  const __v16qu __Q =
    {  4,  5,  6,  7, 12, 13, 14, 15,  4,  5,  6,  7, 12, 13, 14, 15 };
  __v4si __C = (__v4si) (__v2du) { __A, __B };
  __v4si __D = vec_perm (__C, __C, __Q);
  __C = vec_perm (__C, __C, __P);
  __C = vec_sub (__C, __D);
  return (__m64) ((__v2du) __C)[1];
}

extern __inline __m128i
__attribute__((__gnu_inline__, __always_inline__, __artificial__))
_mm_hsubs_epi16 (__m128i __A, __m128i __B)
{
  const __v16qu __P =
    {  0,  1,  4,  5,  8,  9, 12, 13, 16, 17, 20, 21, 24, 25, 28, 29 };
  const __v16qu __Q =
    {  2,  3,  6,  7, 10, 11, 14, 15, 18, 19, 22, 23, 26, 27, 30, 31 };
  __v8hi __C = vec_perm ((__v8hi) __A, (__v8hi) __B, __P);
  __v8hi __D = vec_perm ((__v8hi) __A, (__v8hi) __B, __Q);
  return (__m128i) vec_subs (__C, __D);
}

extern __inline __m64
__attribute__((__gnu_inline__, __always_inline__, __artificial__))
_mm_hsubs_pi16 (__m64 __A, __m64 __B)
{
  const __v16qu __P =
    {  0,  1,  4,  5,  8,  9, 12, 13,  0,  1,  4,  5,  8,  9, 12, 13 };
  const __v16qu __Q =
    {  2,  3,  6,  7, 10, 11, 14, 15,  2,  3,  6,  7, 10, 11, 14, 15 };
  __v8hi __C = (__v8hi) (__v2du) { __A, __B };
  __v8hi __D = vec_perm (__C, __C, __P);
  __v8hi __E = vec_perm (__C, __C, __Q);
  __C = vec_subs (__D, __E);
  return (__m64) ((__v2du) __C)[1];
}

extern __inline __m128i
__attribute__((__gnu_inline__, __always_inline__, __artificial__))
_mm_shuffle_epi8 (__m128i __A, __m128i __B)
{
  const __v16qi __zero = { 0 };
  __vector __bool char __select = vec_cmplt ((__v16qi) __B, __zero);
  __v16qi __C = vec_perm ((__v16qi) __A, (__v16qi) __A, (__v16qu) __B);
  return (__m128i) vec_sel (__C, __zero, __select);
}

extern __inline __m64
__attribute__((__gnu_inline__, __always_inline__, __artificial__))
_mm_shuffle_pi8 (__m64 __A, __m64 __B)
{
  const __v16qi __zero = { 0 };
  __v16qi __C = (__v16qi) (__v2du) { __A, __A };
  __v16qi __D = (__v16qi) (__v2du) { __B, __B };
  __vector __bool char __select = vec_cmplt ((__v16qi) __D, __zero);
  __C = vec_perm ((__v16qi) __C, (__v16qi) __C, (__v16qu) __D);
  __C = vec_sel (__C, __zero, __select);
  return (__m64) ((__v2du) (__C))[0];
}

#ifdef _ARCH_PWR8
extern __inline __m128i
__attribute__((__gnu_inline__, __always_inline__, __artificial__))
_mm_sign_epi8 (__m128i __A, __m128i __B)
{
  const __v16qi __zero = { 0 };
  __v16qi __selectneg = (__v16qi) vec_cmplt ((__v16qi) __B, __zero);
  __v16qi __selectpos =
    (__v16qi) vec_neg ((__v16qi) vec_cmpgt ((__v16qi) __B, __zero));
  __v16qi __conv = vec_add (__selectneg, __selectpos);
  return (__m128i) vec_mul ((__v16qi) __A, (__v16qi) __conv);
}
#endif

#ifdef _ARCH_PWR8
extern __inline __m128i
__attribute__((__gnu_inline__, __always_inline__, __artificial__))
_mm_sign_epi16 (__m128i __A, __m128i __B)
{
  const __v8hi __zero = { 0 };
  __v8hi __selectneg = (__v8hi) vec_cmplt ((__v8hi) __B, __zero);
  __v8hi __selectpos =
    (__v8hi) vec_neg ((__v8hi) vec_cmpgt ((__v8hi) __B, __zero));
  __v8hi __conv = vec_add (__selectneg, __selectpos);
  return (__m128i) vec_mul ((__v8hi) __A, (__v8hi) __conv);
}
#endif

#ifdef _ARCH_PWR8
extern __inline __m128i
__attribute__((__gnu_inline__, __always_inline__, __artificial__))
_mm_sign_epi32 (__m128i __A, __m128i __B)
{
  const __v4si __zero = { 0 };
  __v4si __selectneg = (__v4si) vec_cmplt ((__v4si) __B, __zero);
  __v4si __selectpos =
    (__v4si) vec_neg ((__v4si) vec_cmpgt ((__v4si) __B, __zero));
  __v4si __conv = vec_add (__selectneg, __selectpos);
  return (__m128i) vec_mul ((__v4si) __A, (__v4si) __conv);
}
#endif

#ifdef _ARCH_PWR8
extern __inline __m64
__attribute__((__gnu_inline__, __always_inline__, __artificial__))
_mm_sign_pi8 (__m64 __A, __m64 __B)
{
  const __v16qi __zero = { 0 };
  __v16qi __C = (__v16qi) (__v2du) { __A, __A };
  __v16qi __D = (__v16qi) (__v2du) { __B, __B };
  __C = (__v16qi) _mm_sign_epi8 ((__m128i) __C, (__m128i) __D);
  return (__m64) ((__v2du) (__C))[0];
}
#endif

#ifdef _ARCH_PWR8
extern __inline __m64
__attribute__((__gnu_inline__, __always_inline__, __artificial__))
_mm_sign_pi16 (__m64 __A, __m64 __B)
{
  const __v8hi __zero = { 0 };
  __v8hi __C = (__v8hi) (__v2du) { __A, __A };
  __v8hi __D = (__v8hi) (__v2du) { __B, __B };
  __C = (__v8hi) _mm_sign_epi16 ((__m128i) __C, (__m128i) __D);
  return (__m64) ((__v2du) (__C))[0];
}
#endif

#ifdef _ARCH_PWR8
extern __inline __m64
__attribute__((__gnu_inline__, __always_inline__, __artificial__))
_mm_sign_pi32 (__m64 __A, __m64 __B)
{
  const __v4si __zero = { 0 };
  __v4si __C = (__v4si) (__v2du) { __A, __A };
  __v4si __D = (__v4si) (__v2du) { __B, __B };
  __C = (__v4si) _mm_sign_epi32 ((__m128i) __C, (__m128i) __D);
  return (__m64) ((__v2du) (__C))[0];
}
#endif

extern __inline __m128i
__attribute__((__gnu_inline__, __always_inline__, __artificial__))
_mm_maddubs_epi16 (__m128i __A, __m128i __B)
{
  __v8hi __unsigned = vec_splats ((signed short) 0x00ff);
  __v8hi __C = vec_and (vec_unpackh ((__v16qi) __A), __unsigned);
  __v8hi __D = vec_and (vec_unpackl ((__v16qi) __A), __unsigned);
  __v8hi __E = vec_unpackh ((__v16qi) __B);
  __v8hi __F = vec_unpackl ((__v16qi) __B);
  __C = vec_mul (__C, __E);
  __D = vec_mul (__D, __F);
  const __v16qu __odds  =
    {  0,  1,  4,  5,  8,  9, 12, 13, 16, 17, 20, 21, 24, 25, 28, 29 };
  const __v16qu __evens =
    {  2,  3,  6,  7, 10, 11, 14, 15, 18, 19, 22, 23, 26, 27, 30, 31 };
  __E = vec_perm (__C, __D, __odds);
  __F = vec_perm (__C, __D, __evens);
  return (__m128i) vec_adds (__E, __F);
}

extern __inline __m64
__attribute__((__gnu_inline__, __always_inline__, __artificial__))
_mm_maddubs_pi16 (__m64 __A, __m64 __B)
{
  __v8hi __C = (__v8hi) (__v2du) { __A, __A };
  __C = vec_unpackl ((__v16qi) __C);
  const __v8hi __unsigned = vec_splats ((signed short) 0x00ff);
  __C = vec_and (__C, __unsigned);
  __v8hi __D = (__v8hi) (__v2du) { __B, __B };
  __D = vec_unpackl ((__v16qi) __D);
  __D = vec_mul (__C, __D);
  const __v16qu __odds  =
    {  0,  1,  4,  5,  8,  9, 12, 13, 16, 17, 20, 21, 24, 25, 28, 29 };
  const __v16qu __evens =
    {  2,  3,  6,  7, 10, 11, 14, 15, 18, 19, 22, 23, 26, 27, 30, 31 };
  __C = vec_perm (__D, __D, __odds);
  __D = vec_perm (__D, __D, __evens);
  __C = vec_adds (__C, __D);
  return (__m64) ((__v2du) (__C))[0];
}

extern __inline __m128i
__attribute__((__gnu_inline__, __always_inline__, __artificial__))
_mm_mulhrs_epi16 (__m128i __A, __m128i __B)
{
  __v4si __C = vec_unpackh ((__v8hi) __A);
  __v4si __D = vec_unpackh ((__v8hi) __B);
  __C = vec_mul (__C, __D);
  __D = vec_unpackl ((__v8hi) __A);
  __v4si __E = vec_unpackl ((__v8hi) __B);
  __D = vec_mul (__D, __E);
  const __v4su __shift = vec_splats ((unsigned int) 14);
  __C = vec_sr (__C, __shift);
  __D = vec_sr (__D, __shift);
  const __v4si __ones = vec_splats ((signed int) 1);
  __C = vec_add (__C, __ones);
  __C = vec_sr (__C, (__v4su) __ones);
  __D = vec_add (__D, __ones);
  __D = vec_sr (__D, (__v4su) __ones);
  return (__m128i) vec_pack (__C, __D);
}

extern __inline __m64
__attribute__((__gnu_inline__, __always_inline__, __artificial__))
_mm_mulhrs_pi16 (__m64 __A, __m64 __B)
{
  __v4si __C = (__v4si) (__v2du) { __A, __A };
  __C = vec_unpackh ((__v8hi) __C);
  __v4si __D = (__v4si) (__v2du) { __B, __B };
  __D = vec_unpackh ((__v8hi) __D);
  __C = vec_mul (__C, __D);
  const __v4su __shift = vec_splats ((unsigned int) 14);
  __C = vec_sr (__C, __shift);
  const __v4si __ones = vec_splats ((signed int) 1);
  __C = vec_add (__C, __ones);
  __C = vec_sr (__C, (__v4su) __ones);
  __v8hi __E = vec_pack (__C, __D);
  return (__m64) ((__v2du) (__E))[0];
}

#endif
