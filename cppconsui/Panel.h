/*
 * Copyright (C) 2007 by Mark Pustjens <pustjens@dds.nl>
 * Copyright (C) 2010-2012 by CenterIM developers
 *
 * This file is part of CenterIM.
 *
 * CenterIM is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * CenterIM is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

/**
 * @file
 * Panel class.
 *
 * @ingroup cppconsui
 */

#ifndef __PANEL_H__
#define __PANEL_H__

#include "Widget.h"

namespace CppConsUI
{

/**
 * A widget representing a rectangular border with an optional caption on the
 * top border line.
 */
class Panel
: public Widget
{
public:
  Panel(int w, int h, const char *text = NULL);
  virtual ~Panel();

  // Widget
  virtual void Draw();

  /**
   * Sets the caption text.
   */
  virtual void SetTitle(const char *text);
  /**
   * Returns caption text.
   */
  virtual const char *GetTitle() const;

protected:
  /**
   * Caption text.
   */
  char *title;
  /**
   * On-screen caption width.
   */
  int title_width;

private:
  Panel(const Panel&);
  Panel& operator=(const Panel&);
};

} // namespace CppConsUI

#endif // __PANEL_H__

/* vim: set tabstop=2 shiftwidth=2 textwidth=78 expandtab : */
