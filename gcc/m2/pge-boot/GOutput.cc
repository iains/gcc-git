/* do not edit automatically generated by mc from Output.  */
/* Output.mod redirect output.

Copyright (C) 2021-2025 Free Software Foundation, Inc.
Contributed by Gaius Mulley <gaius.mulley@southwales.ac.uk>.

This file is part of GNU Modula-2.

GNU Modula-2 is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 3, or (at your option)
any later version.

GNU Modula-2 is distributed in the hope that it will be useful, but
WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
General Public License for more details.

You should have received a copy of the GNU General Public License
along with GNU Modula-2; see the file COPYING3.  If not see
<http://www.gnu.org/licenses/>.  */

#include <stdbool.h>
#   if !defined (PROC_D)
#      define PROC_D
       typedef void (*PROC_t) (void);
       typedef struct { PROC_t proc; } PROC;
#   endif

#   if !defined (TRUE)
#      define TRUE (1==1)
#   endif

#   if !defined (FALSE)
#      define FALSE (1==0)
#   endif

#include <stddef.h>
#include <string.h>
#include <limits.h>
#if defined(__cplusplus)
#   undef NULL
#   define NULL 0
#endif
#define _Output_C

#include "GOutput.h"
#   include "GFIO.h"
#   include "GSFIO.h"
#   include "GStrLib.h"
#   include "GNameKey.h"
#   include "GNumberIO.h"
#   include "GASCII.h"
#   include "GDynamicStrings.h"

static bool stdout_;
static FIO_File outputFile;
static DynamicStrings_String buffer;

/*
   Open - attempt to open filename as the output file.
          TRUE is returned if success, FALSE otherwise.
*/

extern "C" bool Output_Open (const char *filename_, unsigned int _filename_high);

/*
   Close - close the output file.
*/

extern "C" void Output_Close (void);

/*
   Write - write a single character to the output file.
*/

extern "C" void Output_Write (char ch);

/*
   WriteString - write an unformatted string to the output.
*/

extern "C" void Output_WriteString (const char *s_, unsigned int _s_high);

/*
   KillWriteS - write a string to the output and free the string afterwards.
*/

extern "C" void Output_KillWriteS (DynamicStrings_String s);

/*
   WriteS - write a string to the output.  The string is not freed.
*/

extern "C" void Output_WriteS (DynamicStrings_String s);

/*
   WriteKey - write a key to the output.
*/

extern "C" void Output_WriteKey (NameKey_Name key);

/*
   WriteLn - write a newline to the output.
*/

extern "C" void Output_WriteLn (void);

/*
   WriteCard - write a cardinal using fieldlength characters.
*/

extern "C" void Output_WriteCard (unsigned int card, unsigned int fieldlength);

/*
   StartBuffer - create a buffer into which any output is redirected.
*/

extern "C" void Output_StartBuffer (void);

/*
   EndBuffer - end the redirection and return the contents of the buffer.
*/

extern "C" DynamicStrings_String Output_EndBuffer (void);


/*
   Open - attempt to open filename as the output file.
          TRUE is returned if success, FALSE otherwise.
*/

extern "C" bool Output_Open (const char *filename_, unsigned int _filename_high)
{
  char filename[_filename_high+1];

  /* make a local copy of each unbounded array.  */
  memcpy (filename, filename_, _filename_high+1);

  if ((StrLib_StrEqual ((const char *) filename, _filename_high, (const char *) "<stdout>", 8)) || (StrLib_StrEqual ((const char *) filename, _filename_high, (const char *) "-", 1)))
    {
      outputFile = FIO_StdOut;
      stdout_ = true;
      return true;
    }
  else
    {
      outputFile = FIO_OpenToWrite ((const char *) filename, _filename_high);
      stdout_ = false;
      return FIO_IsNoError (outputFile);
    }
  /* static analysis guarentees a RETURN statement will be used before here.  */
  __builtin_unreachable ();
}


/*
   Close - close the output file.
*/

extern "C" void Output_Close (void)
{
  FIO_Close (outputFile);
}


/*
   Write - write a single character to the output file.
*/

extern "C" void Output_Write (char ch)
{
  if (buffer == NULL)
    {
      FIO_WriteChar (outputFile, ch);
    }
  else
    {
      buffer = DynamicStrings_ConCatChar (buffer, ch);
    }
}


/*
   WriteString - write an unformatted string to the output.
*/

extern "C" void Output_WriteString (const char *s_, unsigned int _s_high)
{
  char s[_s_high+1];

  /* make a local copy of each unbounded array.  */
  memcpy (s, s_, _s_high+1);

  if (buffer == NULL)
    {
      FIO_WriteString (outputFile, (const char *) s, _s_high);
    }
  else
    {
      buffer = DynamicStrings_ConCat (buffer, DynamicStrings_Mark (DynamicStrings_InitString ((const char *) s, _s_high)));
    }
}


/*
   KillWriteS - write a string to the output and free the string afterwards.
*/

extern "C" void Output_KillWriteS (DynamicStrings_String s)
{
  if ((DynamicStrings_KillString (SFIO_WriteS (outputFile, s))) == NULL)
    {}  /* empty.  */
}


/*
   WriteS - write a string to the output.  The string is not freed.
*/

extern "C" void Output_WriteS (DynamicStrings_String s)
{
  if ((SFIO_WriteS (outputFile, s)) == s)
    {}  /* empty.  */
}


/*
   WriteKey - write a key to the output.
*/

extern "C" void Output_WriteKey (NameKey_Name key)
{
  if (buffer == NULL)
    {
      Output_KillWriteS (DynamicStrings_InitStringCharStar (NameKey_KeyToCharStar (key)));
    }
  else
    {
      buffer = DynamicStrings_ConCat (buffer, DynamicStrings_Mark (DynamicStrings_InitStringCharStar (NameKey_KeyToCharStar (key))));
    }
}


/*
   WriteLn - write a newline to the output.
*/

extern "C" void Output_WriteLn (void)
{
  if (buffer == NULL)
    {
      FIO_WriteLine (outputFile);
    }
  else
    {
      Output_Write (ASCII_nl);
    }
}


/*
   WriteCard - write a cardinal using fieldlength characters.
*/

extern "C" void Output_WriteCard (unsigned int card, unsigned int fieldlength)
{
  typedef struct WriteCard__T1_a WriteCard__T1;

  struct WriteCard__T1_a { char array[20+1]; };
  WriteCard__T1 s;

  NumberIO_CardToStr (card, fieldlength, (char *) &s.array[0], 20);
  Output_WriteString ((const char *) &s.array[0], 20);
}


/*
   StartBuffer - create a buffer into which any output is redirected.
*/

extern "C" void Output_StartBuffer (void)
{
  if (buffer != NULL)
    {
      buffer = DynamicStrings_KillString (buffer);
    }
  buffer = DynamicStrings_InitString ((const char *) "", 0);
}


/*
   EndBuffer - end the redirection and return the contents of the buffer.
*/

extern "C" DynamicStrings_String Output_EndBuffer (void)
{
  DynamicStrings_String s;

  s = buffer;
  buffer = static_cast<DynamicStrings_String> (NULL);
  return s;
  /* static analysis guarentees a RETURN statement will be used before here.  */
  __builtin_unreachable ();
}

extern "C" void _M2_Output_init (__attribute__((unused)) int argc,__attribute__((unused)) char *argv[],__attribute__((unused)) char *envp[])
{
  stdout_ = true;
  buffer = static_cast<DynamicStrings_String> (NULL);
  outputFile = FIO_StdOut;
}

extern "C" void _M2_Output_fini (__attribute__((unused)) int argc,__attribute__((unused)) char *argv[],__attribute__((unused)) char *envp[])
{
}
