/* Simple implementation of range_label.
   Copyright (C) 2014-2025 Free Software Foundation, Inc.

This file is part of GCC.

GCC is free software; you can redistribute it and/or modify it under
the terms of the GNU General Public License as published by the Free
Software Foundation; either version 3, or (at your option) any later
version.

GCC is distributed in the hope that it will be useful, but WITHOUT ANY
WARRANTY; without even the implied warranty of MERCHANTABILITY or
FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
for more details.

You should have received a copy of the GNU General Public License
along with GCC; see the file COPYING3.  If not see
<http://www.gnu.org/licenses/>.  */

#ifndef GCC_TEXT_RANGE_LABEL_H
#define GCC_TEXT_RANGE_LABEL_H

#include "rich-location.h"

/* Concrete subclass of libcpp's range_label.
   Simple implementation using a string literal.  */

class text_range_label : public range_label
{
 public:
  text_range_label (const char *text) : m_text (text) {}

  label_text get_text (unsigned /*range_idx*/) const final override
  {
    return label_text::borrow (m_text);
  }

 private:
  const char *m_text;
};

#endif /* GCC_TEXT_RANGE_LABEL_H */
