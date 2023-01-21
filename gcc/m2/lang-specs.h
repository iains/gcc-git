/* Definitions for specs for GNU Modula-2.
   Copyright (C) 2001-2023 Free Software Foundation, Inc.
   Contributed by Gaius Mulley.

This file is part of GCC.

GCC is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 3, or (at your option)
any later version.

GCC is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with GCC; see the file COPYING3.  If not see
<http://www.gnu.org/licenses/>.  */

/* This is the contribution to the `default_compilers' array in gcc.c for
   GNU Modula-2.  */

/* Pass the preprocessor options on the command line together with
   the exec prefix.  */

#define M2CPP \
  "%{fcpp:-fcpp-begin " \
  "      -E -lang-asm -traditional-cpp " \
  "      %(cpp_unique_options) -fcpp-end; \
     : %I } "

  {".mod", "@modula-2", 0, 0, 0},
  {"@modula-2",
      "cc1gm2 " M2CPP
      "      %(cc1_options) %{B*} %{c*} %{+e*} %{I*} "
      "      %{i*} %{save-temps*} %{v} "
      "      %i %{!fsyntax-only:%(invoke_as)}",
      0, 0, 0},
