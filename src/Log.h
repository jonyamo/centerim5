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

#ifndef __LOG_H__
#define __LOG_H__

#include "CenterIM.h"

#include <cppconsui/TextView.h>
#include <cppconsui/Window.h>
#include <libpurple/purple.h>

#define LOG (Log::instance())

class Log
: public CppConsUI::Window
{
public:
  // levels are 1:1 mapped to glib levels
  enum Level {
    LEVEL_NONE,
    LEVEL_ERROR, // = fatal in libpurle
    LEVEL_CRITICAL, // = error in libpurple
    LEVEL_WARNING,
    LEVEL_MESSAGE, // no such level in libpurple
    LEVEL_INFO,
    LEVEL_DEBUG // = misc in libpurple
  };

  static Log *instance();

  // FreeWindow
  virtual void onScreenResized();

  void error(const char *fmt, ...) _attribute((format(printf, 2, 3)));
  void critical(const char *fmt, ...) _attribute((format(printf, 2, 3)));
  void warning(const char *fmt, ...) _attribute((format(printf, 2, 3)));
  void message(const char *fmt, ...) _attribute((format(printf, 2, 3)));
  void info(const char *fmt, ...) _attribute((format(printf, 2, 3)));
  void debug(const char *fmt, ...) _attribute((format(printf, 2, 3)));

protected:

private:
  enum Type {
    TYPE_CIM,
    TYPE_GLIB,
    TYPE_PURPLE
  };

  PurpleDebugUiOps centerim_debug_ui_ops;

  GIOChannel *logfile;

  CppConsUI::TextView *textview;

  guint default_handler;
  guint glib_handler;
  guint gmodule_handler;
  guint glib_gobject_handler;
  guint gthread_handler;
  guint cppconsui_handler;

  static Log *my_instance;

  Log();
  Log(const Log&);
  Log& operator=(const Log&);
  virtual ~Log();

  static void init();
  static void finalize();
  friend class CenterIM;

  // to catch libpurple's debug messages
  static void purple_print_(PurpleDebugLevel level, const char *category,
      const char *arg_s)
    { LOG->purple_print(level, category, arg_s); }
  static gboolean is_enabled_(PurpleDebugLevel level, const char *category)
    { return LOG->is_enabled(level, category); }

  void purple_print(PurpleDebugLevel level, const char *category,
      const char *arg_s);
  gboolean is_enabled(PurpleDebugLevel level, const char *category);

  // to catch default messages
  static void default_log_handler_(const char *domain, GLogLevelFlags flags,
      const char *msg, gpointer user_data)
    { reinterpret_cast<Log*>(user_data)->default_log_handler(domain, flags,
        msg); }
  void default_log_handler(const char *domain, GLogLevelFlags flags,
      const char *msg);

  // to catch glib's messages
  static void glib_log_handler_(const char *domain, GLogLevelFlags flags,
      const char *msg, gpointer user_data)
    { reinterpret_cast<Log*>(user_data)->glib_log_handler(domain, flags,
        msg); }
  void glib_log_handler(const char *domain, GLogLevelFlags flags,
      const char *msg);

  // to catch cppconsui messages
  static void cppconsui_log_handler_(const char *domain,
      GLogLevelFlags flags, const char *msg, gpointer user_data)
    { reinterpret_cast<Log*>(user_data)->cppconsui_log_handler(domain, flags,
        msg); }
  void cppconsui_log_handler(const char *domain, GLogLevelFlags flags,
      const char *msg);

  // called when log/debug pref is changed
  static void debug_change_(const char *name, PurplePrefType type,
      gconstpointer val, gpointer data)
    { reinterpret_cast<Log*>(data)->debug_change(name, type, val); }
  void debug_change(const char *name, PurplePrefType type,
      gconstpointer val);

  void shortenWindowText();
  void write(const char *text);
  void writeErrorToWindow(const char *fmt, ...);
  void writeToFile(const char *text);
  Level convertPurpleDebugLevel(PurpleDebugLevel purplelevel);
  Level convertGlibDebugLevel(GLogLevelFlags gliblevel);
  Level getLogLevel(const char *type);
};

#endif // __LOG_H__

/* vim: set tabstop=2 shiftwidth=2 textwidth=78 expandtab : */
