// 2002-01-23  Loren J. Rittle <rittle@labs.mot.com> <ljrittle@acm.org>
// Adpated from libstdc++/5347 submitted by markus.breuer@materna.de
//
// Copyright (C) 2002-2025 Free Software Foundation, Inc.
//
// This file is part of the GNU ISO C++ Library.  This library is free
// software; you can redistribute it and/or modify it under the
// terms of the GNU General Public License as published by the
// Free Software Foundation; either version 3, or (at your option)
// any later version.
//
// This library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License along
// with this library; see the file COPYING3.  If not see
// <http://www.gnu.org/licenses/>.

// { dg-do run }
// { dg-options "-pthread"  }
// { dg-require-effective-target pthread }

#include <sstream>
#include <pthread.h>

const int max_thread_count = 2;
const int max_loop_count = 1000000;

void*
thread_main (void *)
{
   for (int i = 0; i < max_loop_count; i++)
     std::ostringstream oss;

   return 0;
}

int
main()
{
  pthread_t tid[max_thread_count];

#if defined(__sun) && defined(__svr4__) && _XOPEN_VERSION >= 500
  pthread_setconcurrency (max_thread_count);
#endif

  for (int i = 0; i < max_thread_count; i++)
    pthread_create (&tid[i], 0, thread_main, 0);

  for (int i = 0; i < max_thread_count; i++)
    pthread_join (tid[i], 0);

  return 0;
}
