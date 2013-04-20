/*
 * Copyright (C) 2009-2013 by CenterIM developers
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
 * InputProcessor base class.
 *
 * @ingroup cppconsui
 */

#ifndef __INPUTPROCESSOR_H__
#define __INPUTPROCESSOR_H__

#include <sigc++/sigc++.h>
#include <sigc++/signal.h>

#include "libtermkey/termkey.h"

#include <map>
#include <string>

namespace CppConsUI
{

/**
 * Base class that takes care of input processing.
 *
 * It allows to define:
 * - key-action bindings,
 * - a chain of input processors (top to bottom).
 */
class InputProcessor
{
public:
  /**
   * Defines when a key binding will be processed comparing with the child
   * input processor, @see ProcessInput.
   */
  enum BindableType {
    /**
     * Key bindings will be processed after the child input processor.
     */
    BINDABLE_NORMAL,
    /**
     * Key bindings will be processed before the child input processor.
     */
    BINDABLE_OVERRIDE
  };

  InputProcessor();
  virtual ~InputProcessor() {}

  /**
   * There are 4 steps when processing input:
   * <ol>
   * <li>
   * <i>Overriding key combos</i><br>
   * Input is processed by checking for overriding key combinations. If
   * a match is found, the function for that combo is executed.
   * </li>
   * <li>
   * <i>Input child processing</i><br>
   * If an input child is assigned, processing is done recursively by this
   * child object.
   * </li>
   * <li>
   * <i>Other key combos</i><br>
   * Input is processed by checking for normal key combinations. If a match is
   * found, the signal for that combo is sent.
   * </li>
   * <li>
   * <i>Raw input processing</i><br>
   * Non key combo raw input processing by objects. Used for e.g. input
   * widgets.
   * </li>
   * </ol>
   *
   * @return True if the input was successfully processed, false otherwise.
   */
  virtual bool ProcessInput(const TermKeyKey& key);

protected:
  /**
   * Bindable struct holds a function and a bindable type that is associated
   * to some {context:action} pair.
   */
  class Bindable
  {
  public:
    Bindable() : type(BINDABLE_NORMAL) {}
    Bindable(const sigc::slot<void>& function_, BindableType type_)
      : function(function_), type(type_) {}
    //Bindable(const Bindable &other);
    //Bindable &operator=(const Bindable &other);
    virtual ~Bindable() {}

    sigc::slot<void> function;
    BindableType type;
  };

  /**
   * Holds all actions and Bindables for a context, {action: Bindable}.
   */
  typedef std::map<std::string, Bindable> BindableContext;
  /**
   * Holds all Key contexts for this class, {context: KeyContext}.
   */
  typedef std::map<std::string, BindableContext> Bindables;

  /**
   * The set of declared Bindables.
   */
  Bindables keybindings;

  /**
   * The child that will get to process the input.
   */
  InputProcessor *input_child;

  /**
   * Set the child object that must process input before this object.
   */
  virtual void SetInputChild(InputProcessor& child);
  virtual void ClearInputChild();
  virtual InputProcessor *GetInputChild() { return input_child; }

  /**
   * Binds a (context, action) pair with a function.
   *
   * The bind can be normal or override, depending on whether it needs to be
   * called after or before the @ref input_child.
   */
  virtual void DeclareBindable(const char *context, const char *action,
      const sigc::slot<void>& function, BindableType type);

  /**
   * Tries to match an appropriate bound action to the input and process it.
   * @return True if a match was found and processed.
   */
  virtual bool Process(BindableType type, const TermKeyKey& key);

  virtual bool ProcessInputText(const TermKeyKey& key);

private:
  InputProcessor(const InputProcessor&);
  InputProcessor& operator=(const InputProcessor&);
};

} // namespace CppConsUI

#endif // __INPUTPROCESSOR_H__

/* vim: set tabstop=2 shiftwidth=2 textwidth=78 expandtab : */
