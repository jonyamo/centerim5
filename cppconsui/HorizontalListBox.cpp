/*
 * Copyright (C) 2007 by Mark Pustjens <pustjens@dds.nl>
 * Copyright (C) 2010-2013 by CenterIM developers
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
 * HorizontalListBox class implementation.
 *
 * @ingroup cppconsui
 */

#include "HorizontalListBox.h"

namespace CppConsUI
{

HorizontalListBox::HorizontalListBox(int w, int h)
: AbstractListBox(w, h), children_width(0), autosize_children(0)
, autosize_width(0), reposition_widgets(false)
{
}

void HorizontalListBox::Draw()
{
  ProceedUpdateArea();
  // set virtual scroll area width
  if (screen_area)
    SetScrollHeight(screen_area->getmaxy());
  UpdateScrollWidth();
  if (update_area)
    reposition_widgets = true;
  ProceedUpdateVirtualArea();

  if (!area) {
    // scrollpane will clear the screen (real) area
    AbstractListBox::Draw();
    return;
  }

  if (reposition_widgets) {
    autosize_width = 1;
    int autosize_width_extra = 0;
    int realw = area->getmaxx();
    if (autosize_children && children_width < realw) {
      int space = realw - (children_width - autosize_children);
      autosize_width = space / autosize_children;
      autosize_width_extra = space % autosize_children;
    }
    autosize_extra.clear();

    int x = 0;
    for (Children::iterator i = children.begin(); i != children.end(); i++) {
      Widget *widget = i->widget;
      if (!widget->IsVisible())
        continue;

      int w = widget->GetWidth();
      if (w == AUTOSIZE) {
        w = autosize_width;
        if (autosize_width_extra) {
          autosize_extra.insert(widget);
          autosize_width_extra--;
          w++;
        }

        // make sure the area is updated
        widget->UpdateArea();
      }

      widget->Move(x, 0);
      x += w;
    }
    reposition_widgets = false;
  }

  // make sure that currently focused widget is visible
  if (focus_child) {
    int w = focus_child->GetWidth();
    if (w == AUTOSIZE) {
      w = autosize_width;
      if (autosize_extra.find(focus_child) != autosize_extra.end())
        w++;
    }

    MakeVisible(focus_child->GetLeft(), focus_child->GetTop(), w, 1);
  }

  AbstractListBox::Draw();
}

VerticalLine *HorizontalListBox::InsertSeparator(size_t pos)
{
  VerticalLine *l = new VerticalLine(AUTOSIZE);
  InsertWidget(pos, *l);
  return l;
}

VerticalLine *HorizontalListBox::AppendSeparator()
{
  VerticalLine *l = new VerticalLine(AUTOSIZE);
  AppendWidget(*l);
  return l;
}

void HorizontalListBox::InsertWidget(size_t pos, Widget& widget)
{
  if (widget.IsVisible()) {
    int w = widget.GetWidth();
    if (w == AUTOSIZE) {
      w = 1;
      autosize_children++;
    }
    children_width += w;
    UpdateScrollWidth();
  }

  // note: widget is moved to a correct position in Draw() method
  ScrollPane::InsertWidget(pos, widget, 0, 0);
  reposition_widgets = true;

  if (widget.IsVisible())
    signal_children_width_change(*this, children_width);
}

void HorizontalListBox::AppendWidget(Widget& widget)
{
  InsertWidget(children.size(), widget);
}

Curses::Window *HorizontalListBox::GetSubPad(const Widget& child, int begin_x,
    int begin_y, int ncols, int nlines)
{
  // autosize
  if (ncols == AUTOSIZE) {
    ncols = autosize_width;
    if (autosize_extra.find(&child) != autosize_extra.end())
      ncols++;
  }

  return AbstractListBox::GetSubPad(child, begin_x, begin_y, ncols, nlines);
}

void HorizontalListBox::OnChildMoveResize(Widget& /*activator*/,
    const Rect& oldsize, const Rect& newsize)
{
  int old_width = oldsize.GetWidth();
  int new_width = newsize.GetWidth();
  if (old_width != new_width) {
    if (old_width == AUTOSIZE) {
      old_width = 1;
      autosize_children--;
    }
    if (new_width == AUTOSIZE) {
      new_width = 1;
      autosize_children++;
    }
    children_width += new_width - old_width;
    reposition_widgets = true;
    UpdateScrollWidth();

    signal_children_width_change(*this, children_width);
  }
}

void HorizontalListBox::OnChildVisible(Widget& activator, bool visible)
{
  // the widget is being hidden or deleted
  int width = activator.GetWidth();
  int sign = visible ? 1 : - 1;
  if (width == AUTOSIZE) {
    autosize_children += sign;
    width = 1;
  }
  children_width += sign * width;
  reposition_widgets = true;
  UpdateScrollWidth();

  signal_children_width_change(*this, children_width);
}

void HorizontalListBox::UpdateScrollWidth()
{
  int realw = 0;
  if (screen_area)
    realw = screen_area->getmaxx();

  SetScrollWidth(MAX(realw, children_width));
}

} // namespace CppConsUI

/* vim: set tabstop=2 shiftwidth=2 textwidth=78 expandtab : */
