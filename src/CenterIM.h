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

#ifndef __CENTERIM_H__
#define __CENTERIM_H__

#ifdef __GNUC__
#define _attribute(x) __attribute__(x)
#else
#define _attribute(x)
#endif

#include <cppconsui/CoreManager.h>
#include <libpurple/purple.h>
#include <vector>

#define CONF_PREFIX "/centerim5"
#define CONF_PLUGINS_PREF CONF_PREFIX "/plugins"
#define CONF_PLUGINS_SAVE_PREF CONF_PLUGINS_PREF "/loaded"

#define CENTERIM (CenterIM::instance())

class CenterIM
: public CppConsUI::InputProcessor
{
public:
  enum ScreenArea {
    BUDDY_LIST_AREA,
    CHAT_AREA,
    FOOTER_AREA,
    HEADER_AREA,
    LOG_AREA,
    WHOLE_AREA,
    AREAS_NUM
  };

  static CenterIM *instance();

  // InputProcessor
  virtual bool processInput(const TermKeyKey& key);

  int run(const char *config_path, bool ascii, bool offline);
  void quit();

  // returns a position and size of a selected area
  CppConsUI::Rect getScreenArea(ScreenArea area);
  CppConsUI::Rect getScreenAreaCentered(ScreenArea area);

  static const char * const version;

  bool loadColorSchemeConfig();
  bool loadKeyConfig();

  bool getExpandedConversations() const { return convs_expanded; }

protected:

private:
  struct IOClosure
  {
    PurpleInputFunction function;
    guint result;
    gpointer data;

    IOClosure() : function(NULL), result(0), data(NULL) {}
  };

  struct LogBufferItem
  {
    PurpleDebugLevel level;
    char *category;
    char *arg_s;
  };

  typedef std::vector<LogBufferItem> LogBufferItems;

  static LogBufferItems *logbuf;

  CppConsUI::CoreManager *mngr;
  sigc::connection resize_conn;
  sigc::connection top_window_change_conn;
  // flag to indicate if the conversation full-screen mode is activated
  bool convs_expanded;
  // flag to indicate if idle reporting is based on keyboard presses
  bool idle_reporting_on_keyboard;

  PurpleCoreUiOps centerim_core_ui_ops;
  PurpleDebugUiOps logbuf_debug_ui_ops;
  PurpleEventLoopUiOps centerim_glib_eventloops;

  CppConsUI::Rect areas[AREAS_NUM];

  static const char *named_colors[];

  static CenterIM *my_instance;

  CenterIM();
  CenterIM(const CenterIM&);
  CenterIM& operator=(const CenterIM&);
  virtual ~CenterIM() {}

  static void init();
  static void finalize();
  friend int main(int argc, char *argv[]);

  int purpleInit(const char *config_path);
  void purpleFinalize();
  void prefsInit();

  // recalculates area sizes to fit into current screen size
  void onScreenResized();

  void onTopWindowChanged();

  // PurpleCoreUiOps callbacks
  // returns information about CenterIM such as name, website etc.
  static GHashTable *get_ui_info();

  // PurpleEventLoopUiOps callbacks
  // adds timeout to glib main loop context
  static guint timeout_add(guint interval, GSourceFunc function, gpointer data);
  // removes timeout from glib main loop context
  static gboolean timeout_remove(guint handle);
  // adds IO watch to glib main loop context
  static guint input_add(int fd, PurpleInputCondition condition,
      PurpleInputFunction function, gpointer data);
  // removes input from glib main loop context
  static gboolean input_remove(guint handle);

  // helper function for input_add
  // process IO input to purple callback
  static gboolean purple_glib_io_input(GIOChannel *source,
      GIOCondition condition, gpointer data);
  // destroyes libpurple io input callback internal data
  static void purple_glib_io_destroy(gpointer data);

  // PurpleDebugUiOps callbacks
  // helper function to catch debug messages during libpurple initialization
  /* Catches and buffers libpurple debug messages until the Log object can be
   * instantiated. */
  static void tmp_purple_print(PurpleDebugLevel level, const char *category,
      const char *arg_s);
  static gboolean tmp_is_enabled(PurpleDebugLevel /*level*/, const char* /*category*/)
    { return TRUE; }

  // called when CONF_PREFIX/dimensions prefs are changed
  static void dimensions_change_(const char *name, PurplePrefType type,
      gconstpointer val, gpointer data)
    { reinterpret_cast<CenterIM*>(data)->dimensions_change(name, type, val); }
  void dimensions_change(const char *name, PurplePrefType type,
      gconstpointer val);

  // called when /libpurple/away/idle_reporting pref is changed
  static void idle_reporting_change_(const char *name, PurplePrefType type,
      gconstpointer val, gpointer data)
    { reinterpret_cast<CenterIM*>(data)->idle_reporting_change(name, type,
        val); }
  void idle_reporting_change(const char *name, PurplePrefType type,
      gconstpointer val);

  // config handling
  void loadDefaultColorSchemeConfig();
  bool saveColorSchemeConfig();
  char *colorToString(int color);
  bool stringToColor(const char *str, int *color);
  char *colorAttributesToString(int attrs);
  bool stringToColorAttributes(const char *str, int *attrs);
  void loadDefaultKeyConfig();
  bool saveKeyConfig();

  void actionFocusBuddyList();
  void actionFocusActiveConversation();
  void actionOpenAccountStatusMenu();
  void actionOpenGeneralMenu();
  void actionBuddyListToggleOffline();
  void actionFocusPrevConversation();
  void actionFocusNextConversation();
  void actionFocusConversation(int i);
  void actionExpandConversation();

  void declareBindables();
};

#endif // __CENTERIM_H__

/* vim: set tabstop=2 shiftwidth=2 textwidth=78 expandtab : */
