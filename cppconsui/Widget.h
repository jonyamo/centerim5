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
 * Widget class.
 *
 * @ingroup cppconsui
 */

#ifndef __WIDGET_H__
#define __WIDGET_H__

#define AUTOSIZE -1024
#define UNSET -2048

#include "ConsuiCurses.h"
#include "CppConsUI.h"
#include "InputProcessor.h"

namespace CppConsUI
{

class Container;
class Window;

class Widget
: public sigc::trackable
, public InputProcessor
{
public:
  Widget(int w, int h);
  virtual ~Widget();

  /**
   * Moves and resizes this widget. Emits signal_moveresize.
   */
  virtual void MoveResize(int newx, int newy, int neww, int newh);
  /**
   * The @ref area is recreated, redraw signal is emitted. It is called
   * whenever the coordinates of the widget change.
   */
  virtual void UpdateArea();
  /**
   * The Draw() method does the actual drawing on a (virtual) area of the
   * screen. The @ref CoreManager singleton calls Draw() on all on-screen
   * Windows. This causes all Draw() implementations needed to draw the screen
   * to be called.
   */
  virtual void Draw() = 0;
  /**
   * Finds the widget that could be the focus widget from the focus chain
   * starting with this widget:
   */
  virtual Widget *GetFocusWidget();
  /**
   * Deletes the focus (and input) chain starting from this widget.
   */
  virtual void CleanFocus();
  /**
   * @todo
   */
  virtual bool RestoreFocus();
  /**
   * Takes focus from the widget. Used when this window is no longer a top
   * window.
   */
  virtual void UngrabFocus();
  /**
   * Makes this widget the widget that has the focus. This operation is
   * successful if the widget is visible and all predecessors are visible too.
   */
  virtual bool GrabFocus();

  virtual bool CanFocus() const { return can_focus; }
  virtual bool HasFocus() const { return has_focus; }

  virtual void SetVisibility(bool visible);
  virtual bool IsVisible() const { return visible; }
  virtual bool IsVisibleRecursive() const;

  virtual void SetParent(Container& parent);
  virtual Container *GetParent() const { return parent; }

  virtual int GetLeft() const { return xpos; }
  virtual int GetTop() const { return ypos; }
  virtual int GetWidth() const { return width; }
  virtual int GetHeight() const { return height; }

  /**
   * Convenient MoveResize() wrapper.
   */
  virtual void Move(int newx, int newy);
  /**
   * Convenient MoveResize() wrapper.
   */
  virtual void Resize(int neww, int newh);
  /**
   * Convenient MoveResize() wrapper.
   */
  virtual void SetLeft(int newx);
  /**
   * Convenient MoveResize() wrapper.
   */
  virtual void SetTop(int newy);
  /**
   * Convenient MoveResize() wrapper.
   */
  virtual void SetWidth(int neww);
  /**
   * Convenient MoveResize() wrapper.
   */
  virtual void SetHeight(int newh);

  /**
   * Returns an absolute position of the widget. Note: special care has to be
   * taken when calling this method because CppConsUI uses deferred drawing.
   * The returned value is correct only if the drawing of all parent widgets
   * already occured.
   */
  virtual Point GetAbsolutePosition() const;
  /**
   * Returns a relative position of the widget to a given predecessor. See
   * note in GetAbsolutePosition() method.
   */
  virtual Point GetRelativePosition(const Container& ref) const;
  /**
   * Returns a real (on-screen) width. See note in GetAbsolutePosition()
   * method.
   */
  virtual int GetRealWidth() const;
  /**
   * Returns a real (on-screen) height. See note in GetAbsolutePosition()
   * method.
   */
  virtual int GetRealHeight() const;

  /**
   * Returns an area width that is requested by the widget. This method can
   * be used by a parent widget in GetSubPad() if width is set to AUTOSIZE.
   * The AUTOSIZE returned value means "as much as possible".
   */
  virtual int GetWishWidth() const;
  /**
   * Returns an area height that is requested by the widget. This method can
   * be used by a parent widget in GetSubPad() if height is set to AUTOSIZE.
   * The AUTOSIZE returned value means "as much as possible".
   */
  virtual int GetWishHeight() const;

  virtual void SetColorScheme(const char *scheme);
  virtual const char *GetColorScheme() const;

  sigc::signal<void, Widget&, const Rect&, const Rect&> signal_moveresize;
  sigc::signal<void, Widget&, const Size&, const Size&>
    signal_wish_size_change;
  /**
   * Signal emitted whenever a widget grabs or looses the focus.
   */
  sigc::signal<void, Widget&, bool> signal_focus;
  /**
   * Signal emmited whenever a visible property is changed.
   */
  sigc::signal<void, Widget&, bool> signal_visible;

protected:
  /**
   * Screen area relative to parent area. Note that this is a requested area
   * if parent window is big enough, real width/height can differ.
   */
  int xpos, ypos, width, height;
  /**
   *
   */
  int wish_width, wish_height;
  /**
   * Flag if the widget can grab the focus.
   */
  bool can_focus;
  /**
   * Flag if the widget has the focus. Only one widget can have the focus in
   * the application.
   */
  bool has_focus;
  /**
   * Visibility flag.
   */
  bool visible;
  /**
   * This is the implementation dependent area of the widget.
   */
  Curses::Window *area;

  bool update_area;
  /**
   * Parent widget.
   */
  Container *parent;
  /**
   * Color scheme.
   */
  char *color_scheme;

  virtual void ProceedUpdateArea();

  /**
   * Redraw() method is used by a widget to tell the CoreManager object that
   * the widget has been updated and that the screen should be redrawn.
   */
  virtual void Redraw();

  virtual void SetWishSize(int neww, int newh);
  virtual void SetWishWidth(int neww) { SetWishSize(neww, wish_height); }
  virtual void SetWishHeight(int newh) { SetWishSize(wish_width, newh); }

  /**
   * Convenient method that calls COLORSCHEME->GetColorPair(GetColorScheme(),
   * widget, property).
   */
  virtual int GetColorPair(const char *widget, const char *property) const;

  /**
   * @todo
   */
  virtual Container *GetTopContainer();

private:
  Widget(const Widget&);
  Widget& operator=(const Widget&);
};

} // namespace CppConsUI

#endif // __WIDGET_H__

/* vim: set tabstop=2 shiftwidth=2 textwidth=78 expandtab : */
