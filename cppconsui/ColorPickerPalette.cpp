/*
 * Copyright (C) 2007 by Mark Pustjens <pustjens@dds.nl>
 * Copyright (C) 2010-2011 by CenterIM developers
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
 * */

/**
 * @file
 * ColorPickerPalette class implementation.
 *
 * @ingroup cppconsui
 */

#include "ColorPickerPalette.h"

#include "ConsuiCurses.h"
#include "ColorScheme.h"

namespace CppConsUI
{

ColorPickerPalette::ColorPickerPalette(int defaultcolor, int flags)
: Container(0, 0)
{
  if (flags == (FLAG_HIDE_ANSI | FLAG_HIDE_GRAYSCALE | FLAG_HIDE_COLORCUBE))
    flags = (FLAG_HIDE_GRAYSCALE | FLAG_HIDE_COLORCUBE);

  if (Curses::nrcolors() < 256)
    flags |= (FLAG_HIDE_GRAYSCALE | FLAG_HIDE_COLORCUBE);

  if (!(flags & FLAG_HIDE_ANSI))
    /* Default 16 colors */
    AddAnsi(defaultcolor);

  if (!(flags & FLAG_HIDE_GRAYSCALE))
    /* Grayscale ladder */
    AddGrayscale(defaultcolor);

  if (!(flags & FLAG_HIDE_COLORCUBE))
    /* 6x6x6 Color cube*/
    AddColorCube(defaultcolor);
}

void ColorPickerPalette::OnSelectColor(Button& activator)
{
  ColorPickerPaletteButton *btn = dynamic_cast<ColorPickerPaletteButton*>(&activator);
  g_assert(btn);

  signal_color_selected (*this, btn->color);
}

void ColorPickerPalette::AddButton(int x, int y, int color, int defaultcolor)
{
  ColorPickerPaletteButton *btn = new ColorPickerPaletteButton(color);

  btn->signal_activate.connect(sigc::mem_fun(this,
      &ColorPickerPalette::OnSelectColor));
  AddWidget (*btn, x, y);

  if (color == defaultcolor)
    btn->GrabFocus();
}

void ColorPickerPalette::AddAnsi(int defaultcolor)
{
  int w, h, x, y;

  // Resize the ColorPickerPalette
  w = GetWidth();
  h = y = GetHeight();

  w = MAX(w, 16);
  h = h + 2;

  Resize (w, h);

  // Add the color picker buttons
  for (x = 0; x < 16; x++)
  {
    if (x >= 8)
      AddButton((x-8)*2, y+1, x, defaultcolor);
    else
      AddButton(x*2, y, x, defaultcolor);
  }
}

void ColorPickerPalette::AddGrayscale(int defaultcolor)
{
  int w, h, x, y, color;

  // Resize the ColorPickerPalette
  w = GetWidth();
  h = GetHeight();

  // Add space between this and previous section
  if (h)
    h++;

  y = h;
  w = MAX(w, (256-232)*2);
  h = h + 1;

  Resize (w, h);

  // Add the color picker buttons
  for (color = 232, x = 0; color < 256; color++, x += 2)
    AddButton(x, y, color, defaultcolor);

  AddButton(x, y, Curses::Color::WHITE, defaultcolor);
}

void ColorPickerPalette::AddColorCube(int defaultcolor)
{
  int w, h, x, y;
  // Resize the ColorPickerPalette
  w = GetWidth();
  h = GetHeight();

  // Add space between this and previous section
  if (h)
    h++;

  y = h;

  w = MAX(w, (6*6*2)+5);
  h = h + 6;

  Resize (w, h);

  // Add the color picker buttons
  x = 0;
  for (int g = 0; g < 6; g++)
  {
    for (int r = 0; r < 6; r++)
    {
      for (int b = 0; b < 6; b++)
      {
        AddButton(x, y, 16 + (r * 36) + (g * 6) + b, defaultcolor);
        x += 2;
      }

      x++;
    }

    y++;
    x = 0;
  }
}

ColorPickerPalette::ColorPickerPaletteButton::ColorPickerPaletteButton (const int color)
: Button(2, 1, ""), color(color)
{
}

void ColorPickerPalette::ColorPickerPaletteButton::Draw()
{
  ProceedUpdateArea();

  if (!area)
    return;

  //@todo get proper color for cursor in more cases
  int cursor = Curses::Color::BLACK;
  if ((color == 0) || (color == 1) || (color == 8) || (color == 9)
      || (color >= 232 && color < 244)
      || (color == 16))
    cursor = Curses::Color::WHITE;

  ColorScheme::Color c(cursor, color);
  int colorpair = COLORSCHEME->GetColorPair(c);

  area->fill(colorpair, 0, 0, 2, 1);

  // print cursor
  if (has_focus)
    area->mvaddstring(0, 0, "<>");
}

} // namespace CppConsUI

/* vim: set tabstop=2 shiftwidth=2 expandtab : */