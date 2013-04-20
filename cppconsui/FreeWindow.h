/*
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
 * FreeWindow class.
 *
 * @ingroup cppconsui
 */

#ifndef __FREEWINDOW_H__
#define __FREEWINDOW_H__

#include "Container.h"

namespace CppConsUI
{

class FreeWindow
: public Container
{
public:
  enum Type {
    TYPE_NON_FOCUSABLE,
    TYPE_NORMAL,
    TYPE_TOP
  };

  FreeWindow(int x, int y, int w, int h, Type t = TYPE_NORMAL);
  virtual ~FreeWindow();

  // Widget
  virtual void MoveResize(int newx, int newy, int neww, int newh);
  virtual void MoveResizeRect(const Rect &rect)
    { MoveResize(rect.x, rect.y, rect.width, rect.height); }
  virtual void Draw();
  virtual void SetVisibility(bool visible);
  virtual bool IsVisibleRecursive() const { return IsVisible(); }
  virtual int GetLeft() const { return win_x; }
  virtual int GetTop() const { return win_y; }
  virtual int GetWidth() const { return win_w; }
  virtual int GetHeight() const { return win_h; }
  virtual Point GetAbsolutePosition();
  virtual void SetWishSize(int neww, int newh);

  // Container
  virtual bool IsWidgetVisible(const Widget& widget) const;
  virtual bool SetFocusChild(Widget& child);

  virtual void Show();
  virtual void Hide();
  virtual void Close();

  //virtual void SetType(Type t) { type = t; }
  virtual Type GetType() { return type; }

  /**
   * This function is called when the screen is resized.
   */
  virtual void OnScreenResized() {}

  sigc::signal<void, FreeWindow&> signal_close;
  sigc::signal<void, FreeWindow&> signal_show;
  sigc::signal<void, FreeWindow&> signal_hide;

protected:
  /**
   * The window on-screen dimensions.
   */
  int win_x, win_y, win_w, win_h;
  /**
   * Dimensions to use when copying from pad to window.
   */
  int copy_x, copy_y, copy_w, copy_h;
  /**
   * The 'real' window for this window.
   */
  Curses::Window *realwindow;

  Type type;

  // Widget
  virtual void SetParent(Container& parent);
  virtual void ProceedUpdateArea();
  virtual void Redraw();

  /**
   * Internal callback triggered when the screen is resized. It should be used
   * (overwritten) only by CppConsUI widgets, users should use
   * OnScreenResized() instead.
   */
  virtual void OnScreenResizedInternal();

  /**
   * Updates window's width and height and calls UpdateArea(). Autosize hints
   * are taken into account during size calculations.
   */
  virtual void ResizeAndUpdateArea();

private:
  FreeWindow(const FreeWindow&);
  FreeWindow& operator=(const FreeWindow&);

  void ActionClose();

  void DeclareBindables();
};

} // namespace CppConsUI

#endif // __FREEWINDOW_H__

/* vim: set tabstop=2 shiftwidth=2 textwidth=78 expandtab : */
