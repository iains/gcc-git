/* { dg-final { check-function-bodies "**" "" "-DCHECK_ASM" { target { ! ilp32 } } } } */

#include "test_sve_acle.h"

/*
** ldnt1_mf8_base:
**	ldnt1b	z0\.b, p0/z, \[x0\]
**	ret
*/
TEST_LOAD (ldnt1_mf8_base, svmfloat8_t, mfloat8_t,
	   z0 = svldnt1_mf8 (p0, x0),
	   z0 = svldnt1 (p0, x0))

/*
** ldnt1_mf8_index:
**	ldnt1b	z0\.b, p0/z, \[x0, x1\]
**	ret
*/
TEST_LOAD (ldnt1_mf8_index, svmfloat8_t, mfloat8_t,
	   z0 = svldnt1_mf8 (p0, x0 + x1),
	   z0 = svldnt1 (p0, x0 + x1))

/*
** ldnt1_mf8_1:
**	ldnt1b	z0\.b, p0/z, \[x0, #1, mul vl\]
**	ret
*/
TEST_LOAD (ldnt1_mf8_1, svmfloat8_t, mfloat8_t,
	   z0 = svldnt1_mf8 (p0, x0 + svcntb ()),
	   z0 = svldnt1 (p0, x0 + svcntb ()))

/*
** ldnt1_mf8_7:
**	ldnt1b	z0\.b, p0/z, \[x0, #7, mul vl\]
**	ret
*/
TEST_LOAD (ldnt1_mf8_7, svmfloat8_t, mfloat8_t,
	   z0 = svldnt1_mf8 (p0, x0 + svcntb () * 7),
	   z0 = svldnt1 (p0, x0 + svcntb () * 7))

/* Moving the constant into a register would also be OK.  */
/*
** ldnt1_mf8_8:
**	incb	x0, all, mul #8
**	ldnt1b	z0\.b, p0/z, \[x0\]
**	ret
*/
TEST_LOAD (ldnt1_mf8_8, svmfloat8_t, mfloat8_t,
	   z0 = svldnt1_mf8 (p0, x0 + svcntb () * 8),
	   z0 = svldnt1 (p0, x0 + svcntb () * 8))

/*
** ldnt1_mf8_m1:
**	ldnt1b	z0\.b, p0/z, \[x0, #-1, mul vl\]
**	ret
*/
TEST_LOAD (ldnt1_mf8_m1, svmfloat8_t, mfloat8_t,
	   z0 = svldnt1_mf8 (p0, x0 - svcntb ()),
	   z0 = svldnt1 (p0, x0 - svcntb ()))

/*
** ldnt1_mf8_m8:
**	ldnt1b	z0\.b, p0/z, \[x0, #-8, mul vl\]
**	ret
*/
TEST_LOAD (ldnt1_mf8_m8, svmfloat8_t, mfloat8_t,
	   z0 = svldnt1_mf8 (p0, x0 - svcntb () * 8),
	   z0 = svldnt1 (p0, x0 - svcntb () * 8))

/* Moving the constant into a register would also be OK.  */
/*
** ldnt1_mf8_m9:
**	decb	x0, all, mul #9
**	ldnt1b	z0\.b, p0/z, \[x0\]
**	ret
*/
TEST_LOAD (ldnt1_mf8_m9, svmfloat8_t, mfloat8_t,
	   z0 = svldnt1_mf8 (p0, x0 - svcntb () * 9),
	   z0 = svldnt1 (p0, x0 - svcntb () * 9))

/*
** ldnt1_vnum_mf8_0:
**	ldnt1b	z0\.b, p0/z, \[x0\]
**	ret
*/
TEST_LOAD (ldnt1_vnum_mf8_0, svmfloat8_t, mfloat8_t,
	   z0 = svldnt1_vnum_mf8 (p0, x0, 0),
	   z0 = svldnt1_vnum (p0, x0, 0))

/*
** ldnt1_vnum_mf8_1:
**	ldnt1b	z0\.b, p0/z, \[x0, #1, mul vl\]
**	ret
*/
TEST_LOAD (ldnt1_vnum_mf8_1, svmfloat8_t, mfloat8_t,
	   z0 = svldnt1_vnum_mf8 (p0, x0, 1),
	   z0 = svldnt1_vnum (p0, x0, 1))

/*
** ldnt1_vnum_mf8_7:
**	ldnt1b	z0\.b, p0/z, \[x0, #7, mul vl\]
**	ret
*/
TEST_LOAD (ldnt1_vnum_mf8_7, svmfloat8_t, mfloat8_t,
	   z0 = svldnt1_vnum_mf8 (p0, x0, 7),
	   z0 = svldnt1_vnum (p0, x0, 7))

/* Moving the constant into a register would also be OK.  */
/*
** ldnt1_vnum_mf8_8:
**	incb	x0, all, mul #8
**	ldnt1b	z0\.b, p0/z, \[x0\]
**	ret
*/
TEST_LOAD (ldnt1_vnum_mf8_8, svmfloat8_t, mfloat8_t,
	   z0 = svldnt1_vnum_mf8 (p0, x0, 8),
	   z0 = svldnt1_vnum (p0, x0, 8))

/*
** ldnt1_vnum_mf8_m1:
**	ldnt1b	z0\.b, p0/z, \[x0, #-1, mul vl\]
**	ret
*/
TEST_LOAD (ldnt1_vnum_mf8_m1, svmfloat8_t, mfloat8_t,
	   z0 = svldnt1_vnum_mf8 (p0, x0, -1),
	   z0 = svldnt1_vnum (p0, x0, -1))

/*
** ldnt1_vnum_mf8_m8:
**	ldnt1b	z0\.b, p0/z, \[x0, #-8, mul vl\]
**	ret
*/
TEST_LOAD (ldnt1_vnum_mf8_m8, svmfloat8_t, mfloat8_t,
	   z0 = svldnt1_vnum_mf8 (p0, x0, -8),
	   z0 = svldnt1_vnum (p0, x0, -8))

/* Moving the constant into a register would also be OK.  */
/*
** ldnt1_vnum_mf8_m9:
**	decb	x0, all, mul #9
**	ldnt1b	z0\.b, p0/z, \[x0\]
**	ret
*/
TEST_LOAD (ldnt1_vnum_mf8_m9, svmfloat8_t, mfloat8_t,
	   z0 = svldnt1_vnum_mf8 (p0, x0, -9),
	   z0 = svldnt1_vnum (p0, x0, -9))

/*
** ldnt1_vnum_mf8_x1:
**	cntb	(x[0-9]+)
** (
**	madd	(x[0-9]+), (?:x1, \1|\1, x1), x0
**	ldnt1b	z0\.b, p0/z, \[\2\]
** |
**	mul	(x[0-9]+), (?:x1, \1|\1, x1)
**	ldnt1b	z0\.b, p0/z, \[x0, \3\]
** )
**	ret
*/
TEST_LOAD (ldnt1_vnum_mf8_x1, svmfloat8_t, mfloat8_t,
	   z0 = svldnt1_vnum_mf8 (p0, x0, x1),
	   z0 = svldnt1_vnum (p0, x0, x1))
