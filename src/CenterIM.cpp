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

#include "CenterIM.h"

#include "Accounts.h"
#include "BuddyList.h"
#include "Connections.h"
#include "Conversations.h"
#include "Footer.h"
#include "Header.h"
#include "Log.h"
#include "Notify.h"
#include "Request.h"
#include "Transfers.h"

#include "AccountStatusMenu.h"
#include "GeneralMenu.h"

#include <cppconsui/ColorScheme.h>
#include <cppconsui/KeyConfig.h>
#include <errno.h>
#include <glib/gprintf.h>
#include <typeinfo>
#include "gettext.h"

CenterIM::LogBufferItems *CenterIM::logbuf = NULL;

const char *CenterIM::named_colors[] = {
  "default", /* -1 */
  "black",   /*  0 */
  "red",     /*  1 */
  "green",   /*  2 */
  "yellow",  /*  3 */
  "blue",    /*  4 */
  "magenta", /*  5 */
  "cyan",    /*  6 */
  "white"    /*  7 */
};

CenterIM *CenterIM::my_instance = NULL;

CenterIM *CenterIM::instance()
{
  return my_instance;
}

bool CenterIM::processInput(const TermKeyKey& key)
{
  if (idle_reporting_on_keyboard)
    purple_idle_touch();
  return InputProcessor::processInput(key);
}

int CenterIM::run(const char *config_path, bool ascii, bool offline)
{
  // ASCII mode
  if (ascii)
    CppConsUI::Curses::set_ascii_mode(ascii);

  // config path
  char *path;
  if (g_path_is_absolute(config_path)) {
    // absolute path
    path = g_strdup(config_path);
  }
  else
    path = g_build_path(G_DIR_SEPARATOR_S, purple_home_dir(), config_path,
        NULL);

  if (purpleInit(path))
    return 1;

  g_free(path);

  prefsInit();

  // initialize Log component
  Log::init();
  if (logbuf) {
    for (LogBufferItems::iterator i = logbuf->begin();
        i != logbuf->end(); i++) {
      purple_debug(i->level, i->category, "%s", i->arg_s);
      g_free(i->category);
      g_free(i->arg_s);
    }

    delete logbuf;
    logbuf = NULL;
  }

  /* Init colorschemes and keybinds after the Log is initialized so the user
   * can see if there is any error in the configs. */
  loadColorSchemeConfig();
  loadKeyConfig();

  Footer::init();

  Accounts::init();
  Connections::init();
  Notify::init();
  Request::init();

  // initialize UI
  Conversations::init();
  Header::init();
  // init BuddyList last so it takes the focus
  BuddyList::init();

  const char *key = KEYCONFIG->getKeyBind("centerim", "generalmenu");
  LOG->info(_("Welcome to CenterIM 5. Press %s to display main menu."), key);

  // restore last know status on all accounts
  ACCOUNTS->restoreStatuses(offline);

  mngr->setTopInputProcessor(*this);
  mngr->enableResizing();
  mngr->startMainLoop();

  purple_prefs_disconnect_by_handle(this);

  resize_conn.disconnect();
  top_window_change_conn.disconnect();

  Conversations::finalize();
  Header::finalize();
  BuddyList::finalize();

  Accounts::finalize();
  Connections::finalize();
  Notify::finalize();
  Request::finalize();

  Footer::finalize();

  Log::finalize();

  purpleFinalize();

  return 0;
}

void CenterIM::quit()
{
  mngr->quitMainLoop();
}

CppConsUI::Rect CenterIM::getScreenArea(ScreenArea area)
{
  return areas[area];
}

CppConsUI::Rect CenterIM::getScreenAreaCentered(ScreenArea area)
{
  CppConsUI::Rect s = areas[WHOLE_AREA];
  CppConsUI::Rect r = areas[area];
  int x = (s.width - r.width) / 2;
  int y = (s.height - r.height) / 2;
  return CppConsUI::Rect(x, y, r.width, r.height);
}

bool CenterIM::loadColorSchemeConfig()
{
  xmlnode *root = purple_util_read_xml_from_file("colorschemes.xml",
      _("color schemes"));

  if (!root) {
    // read error, first time run?
    loadDefaultColorSchemeConfig();
    if (saveColorSchemeConfig())
      return true;
    return false;
  }

  COLORSCHEME->clear();
  bool res = false;

  for (xmlnode *scheme = xmlnode_get_child(root, "scheme"); scheme;
      scheme = xmlnode_get_next_twin(scheme)) {
    const char *name = xmlnode_get_attrib(scheme, "name");
    if (!name) {
      LOG->error(_("Missing 'name' attribute in the scheme definition."));
      goto out;
    }

    for (xmlnode *color = xmlnode_get_child(scheme, "color"); color;
        color = xmlnode_get_next_twin(color)) {
      const char *widget = xmlnode_get_attrib(color, "widget");
      if (!widget) {
        LOG->error(
            _("Missing 'widget' attribute in the color definition."));
        goto out;
      }

      const char *property = xmlnode_get_attrib(color, "property");
      if (!property) {
        LOG->error(
            _("Missing 'property' attribute in the color definition."));
        goto out;
      }

      const char *fgs = xmlnode_get_attrib(color, "foreground");
      const char *bgs = xmlnode_get_attrib(color, "background");
      const char *attrs = xmlnode_get_attrib(color, "attributes");

      int fg = CppConsUI::Curses::Color::DEFAULT;
      int bg = CppConsUI::Curses::Color::DEFAULT;
      int attr = 0;

      if (fgs && !stringToColor(fgs, &fg)) {
        LOG->error(_("Unrecognized color '%s'."), fgs);
        goto out;
      }

      if (bgs && !stringToColor(bgs, &bg)) {
        LOG->error(_("Unrecognized color '%s'."), bgs);
        goto out;
      }

      if (attrs && !stringToColorAttributes(attrs, &attr)) {
        LOG->error(_("Unrecognized attributes '%s'."), attrs);
        goto out;
      }

      COLORSCHEME->setColorPair(name, widget, property, fg, bg, attr);
    }
  }

  res = true;

out:
  if (!res) {
    LOG->error(_("Error parsing 'colorschemes.xml', "
          "loading default color scheme."));
    loadDefaultColorSchemeConfig();
  }

  xmlnode_free(root);

  return res;
}

bool CenterIM::loadKeyConfig()
{
  xmlnode *root = purple_util_read_xml_from_file("binds.xml",
      _("key bindings"));

  if (!root) {
    // read error, first time run?
    loadDefaultKeyConfig();
    if (saveKeyConfig())
      return true;
    return false;
  }

  KEYCONFIG->clear();
  bool res = false;

  for (xmlnode *bind = xmlnode_get_child(root, "bind"); bind;
      bind = xmlnode_get_next_twin(bind)) {
    const char *context = xmlnode_get_attrib(bind, "context");
    if (!context) {
        LOG->error(_("Missing 'context' attribute in the bind definition."));
        goto out;
    }
    const char *action = xmlnode_get_attrib(bind, "action");
    if (!action) {
        LOG->error(_("Missing 'action' attribute in the bind definition."));
        goto out;
    }
    const char *key = xmlnode_get_attrib(bind, "key");
    if (!key) {
        LOG->error(_("Missing 'key' attribute in the bind definition."));
        goto out;
    }

    if (!KEYCONFIG->bindKey(context, action, key)) {
      LOG->error(_("Unrecognized key '%s'."), key);
      goto out;
    }
  }

  res = true;

out:
  if (!res) {
    LOG->error(_("Error parsing 'binds.xml', loading default keys."));
    loadDefaultKeyConfig();
  }

  xmlnode_free(root);

  return res;
}

CenterIM::CenterIM()
: convs_expanded(false), idle_reporting_on_keyboard(false)
{
  mngr = CppConsUI::CoreManager::instance();
  resize_conn = mngr->signal_resize.connect(sigc::mem_fun(this,
        &CenterIM::onScreenResized));
  top_window_change_conn = mngr->signal_top_window_change.connect(
      sigc::mem_fun(this, &CenterIM::onTopWindowChanged));

  memset(&centerim_core_ui_ops, 0, sizeof(centerim_core_ui_ops));
  memset(&logbuf_debug_ui_ops, 0, sizeof(logbuf_debug_ui_ops));
  memset(&centerim_glib_eventloops, 0, sizeof(centerim_glib_eventloops));

  declareBindables();
}

void CenterIM::init()
{
  g_assert(!my_instance);

  my_instance = new CenterIM;
}

void CenterIM::finalize()
{
  g_assert(my_instance);

  delete my_instance;
  my_instance = NULL;
}

int CenterIM::purpleInit(const char *config_path)
{
  g_assert(config_path);
  g_assert(g_path_is_absolute(config_path));

  purple_util_set_user_dir(config_path);

  /* This does not disable debugging, but rather it disables printing to
   * stdout. Don't change this to TRUE or things will get messy. */
  purple_debug_set_enabled(FALSE);

  /* This catches and buffers libpurple debug messages until the Log object
   * can be instantiated. */
  logbuf_debug_ui_ops.print = tmp_purple_print;
  logbuf_debug_ui_ops.is_enabled = tmp_is_enabled;
  purple_debug_set_ui_ops(&logbuf_debug_ui_ops);

  centerim_core_ui_ops.get_ui_info = get_ui_info;
  purple_core_set_ui_ops(&centerim_core_ui_ops);

  // set the uiops for the eventloop
  centerim_glib_eventloops.timeout_add = timeout_add;
  centerim_glib_eventloops.timeout_remove = timeout_remove;
  centerim_glib_eventloops.input_add = input_add;
  centerim_glib_eventloops.input_remove = input_remove;
  purple_eventloop_set_ui_ops(&centerim_glib_eventloops);

  // search user-specific plugins
  char *path = g_build_filename(purple_user_dir(), "plugins", NULL);
  purple_plugins_add_search_path(path);
  g_free(path);

  // search centerim-specific plugins
  purple_plugins_add_search_path(PKGLIBDIR);

  if (!purple_core_init(PACKAGE_NAME)) {
    // can't do much without libpurple
    fprintf(stderr, _("Libpurple initialization failed."));
    return 1;
  }

  purple_prefs_add_none(CONF_PREFIX);
  purple_prefs_add_none(CONF_PLUGINS_PREF);

  // load the desired plugins
  if (purple_prefs_exists(CONF_PLUGINS_SAVE_PREF))
    purple_plugins_load_saved(CONF_PLUGINS_SAVE_PREF);

  return 0;
}

void CenterIM::purpleFinalize()
{
  purple_plugins_save_loaded(CONF_PLUGINS_SAVE_PREF);

  purple_core_set_ui_ops(NULL);
  //purple_eventloop_set_ui_ops(NULL);
  purple_core_quit();
}

void CenterIM::prefsInit()
{
  // remove someday...
  if (purple_prefs_exists("/centerim"))
    purple_prefs_rename("/centerim", CONF_PREFIX);

  // init prefs
  purple_prefs_add_none(CONF_PREFIX "/dimensions");
  purple_prefs_add_int(CONF_PREFIX "/dimensions/buddylist_width", 20);
  purple_prefs_add_int(CONF_PREFIX "/dimensions/log_height", 25);
  purple_prefs_add_bool(CONF_PREFIX "/dimensions/show_header", true);
  purple_prefs_add_bool(CONF_PREFIX "/dimensions/show_footer", true);
  purple_prefs_connect_callback(this, CONF_PREFIX "/dimensions",
      dimensions_change_, this);

  purple_prefs_connect_callback(this, "/purple/away/idle_reporting",
      idle_reporting_change_, this);
  /* Proceed the callback. Note: This potentially triggers other callbacks
   * inside libpurple. */
  purple_prefs_trigger_callback("/purple/away/idle_reporting");
}

void CenterIM::onScreenResized()
{
  CppConsUI::Rect size;

  int screen_width = CppConsUI::Curses::getmaxx();
  int screen_height = CppConsUI::Curses::getmaxy();

  int buddylist_width;
  int log_height;
  if (convs_expanded) {
    buddylist_width = 0;
    log_height = 0;
  }
  else {
    buddylist_width = purple_prefs_get_int(CONF_PREFIX
        "/dimensions/buddylist_width");
    buddylist_width = CLAMP(buddylist_width, 0, 50);
    log_height = purple_prefs_get_int(CONF_PREFIX "/dimensions/log_height");
    log_height = CLAMP(log_height, 0, 50);
  }

  bool show_header = purple_prefs_get_bool(CONF_PREFIX
      "/dimensions/show_header");
  int header_height;
  if (show_header)
    header_height = 1;
  else
    header_height = 0;
  bool show_footer = purple_prefs_get_bool(CONF_PREFIX
      "/dimensions/show_footer");
  int footer_height;
  if (show_footer)
    footer_height = 1;
  else
    footer_height = 0;

  size.x = 0;
  size.y = header_height;
  size.width = screen_width / 100.0 * buddylist_width;
  size.height = screen_height - header_height - footer_height;
  areas[BUDDY_LIST_AREA] = size;

  size.x = areas[BUDDY_LIST_AREA].width;
  size.width = screen_width - size.x;
  size.height = screen_height / 100.0 * log_height;
  size.y = screen_height - size.height - footer_height;
  areas[LOG_AREA] = size;

  size.x = areas[BUDDY_LIST_AREA].width;
  size.y = header_height;
  size.width = screen_width - size.x;
  size.height = screen_height - size.y - areas[LOG_AREA].height
    - footer_height;
  if (convs_expanded) {
    size.x -= 2;
    size.width += 4;
  }
  areas[CHAT_AREA] = size;

  size.x = 0;
  size.y = 0;
  size.width = screen_width;
  size.height = header_height;
  areas[HEADER_AREA] = size;

  size.x = 0;
  size.y = screen_height - 1;
  size.width = screen_width;
  size.height = footer_height;
  areas[FOOTER_AREA] = size;

  size.x = 0;
  size.y = 0;
  size.width = screen_width;
  size.height = screen_height;
  areas[WHOLE_AREA] = size;
}

void CenterIM::onTopWindowChanged()
{
  if (!convs_expanded)
    return;

  CppConsUI::FreeWindow *top = mngr->getTopWindow();
  if (top && typeid(Conversation) != typeid(*top)) {
    convs_expanded = false;
    CONVERSATIONS->setExpandedConversations(convs_expanded);
    mngr->onScreenResized();
  }
}

GHashTable *CenterIM::get_ui_info()
{
  static GHashTable *ui_info = NULL;

  if (!ui_info) {
    ui_info = g_hash_table_new(g_str_hash, g_str_equal);

    /* Note: the C-style casts are used below because otherwise we would need
     * to use the const_cast and reinterpret_cast together (which is too much
     * typing). */
    g_hash_table_insert(ui_info, (void*)"name", (void*)PACKAGE_NAME);
    g_hash_table_insert(ui_info, (void*)"version", (void*)version);
    g_hash_table_insert(ui_info, (void*)"website", (void*)PACKAGE_URL);

    g_hash_table_insert(ui_info, (void*)"dev_website",
        (void*)PACKAGE_BUGREPORT);
    g_hash_table_insert(ui_info, (void*)"client_type", (void*)"pc");
  }

  return ui_info;
}

guint CenterIM::timeout_add(guint interval, GSourceFunc function,
    gpointer data)
{
  return g_timeout_add(interval, function, data);
}

gboolean CenterIM::timeout_remove(guint handle)
{
  return g_source_remove(handle);
}

#define PURPLE_GLIB_READ_COND  (G_IO_IN | G_IO_HUP | G_IO_ERR)
#define PURPLE_GLIB_WRITE_COND (G_IO_OUT | G_IO_HUP | G_IO_ERR | G_IO_NVAL)

guint CenterIM::input_add(int fd, PurpleInputCondition condition,
  PurpleInputFunction function, gpointer data)
{
  IOClosure *closure = new IOClosure;
  GIOChannel *channel;
  int cond = 0;

  closure->function = function;
  closure->data = data;

  if (condition & PURPLE_INPUT_READ)
    cond |= PURPLE_GLIB_READ_COND;
  if (condition & PURPLE_INPUT_WRITE)
    cond |= PURPLE_GLIB_WRITE_COND;

  channel = g_io_channel_unix_new(fd);
  closure->result = g_io_add_watch_full(channel, G_PRIORITY_DEFAULT,
      static_cast<GIOCondition>(cond), purple_glib_io_input, closure,
      purple_glib_io_destroy);

  g_io_channel_unref(channel);
  return closure->result;
}

gboolean CenterIM::input_remove(guint handle)
{
  return g_source_remove(handle);
}

gboolean CenterIM::purple_glib_io_input(GIOChannel *source,
    GIOCondition condition, gpointer data)
{
  IOClosure *closure = static_cast<IOClosure*>(data);
  int purple_cond = 0;

  if (condition & PURPLE_GLIB_READ_COND)
    purple_cond |= PURPLE_INPUT_READ;
  if (condition & PURPLE_GLIB_WRITE_COND)
    purple_cond |= PURPLE_INPUT_WRITE;

  closure->function(closure->data, g_io_channel_unix_get_fd(source),
      static_cast<PurpleInputCondition>(purple_cond));

  return TRUE;
}

void CenterIM::purple_glib_io_destroy(gpointer data)
{
  delete static_cast<IOClosure*>(data);
}

void CenterIM::tmp_purple_print(PurpleDebugLevel level, const char *category,
    const char *arg_s)
{
  if (!logbuf)
    logbuf = new std::vector<LogBufferItem>;

  LogBufferItem item;
  item.level = level;
  item.category = g_strdup(category);
  item.arg_s = g_strdup(arg_s);
  logbuf->push_back(item);
}

void CenterIM::dimensions_change(const char * /*name*/,
    PurplePrefType /*type*/, gconstpointer /*val*/)
{
  mngr->onScreenResized();
}

void CenterIM::idle_reporting_change(const char * /*name*/,
    PurplePrefType type, gconstpointer val)
{
  g_return_if_fail(type == PURPLE_PREF_STRING);

  const char *value = static_cast<const char*>(val);
  if (!strcmp(value, "system"))
    idle_reporting_on_keyboard = true;
  else
    idle_reporting_on_keyboard = false;
}

void CenterIM::loadDefaultColorSchemeConfig()
{
  COLORSCHEME->clear();

  // default colors init
  COLORSCHEME->setColorPair("accountstatusmenu", "panel", "line",
      CppConsUI::Curses::Color::CYAN, CppConsUI::Curses::Color::DEFAULT);
  COLORSCHEME->setColorPair("accountstatusmenu", "horizontalline", "line",
      CppConsUI::Curses::Color::CYAN, CppConsUI::Curses::Color::DEFAULT);
  COLORSCHEME->setColorPair("accountstatusmenu", "button", "normal",
      CppConsUI::Curses::Color::CYAN, CppConsUI::Curses::Color::DEFAULT);

  COLORSCHEME->setColorPair("buddylist", "treeview", "line",
      CppConsUI::Curses::Color::GREEN, CppConsUI::Curses::Color::DEFAULT);
  COLORSCHEME->setColorPair("buddylist", "panel", "line",
      CppConsUI::Curses::Color::BLUE, CppConsUI::Curses::Color::DEFAULT,
      CppConsUI::Curses::Attr::BOLD);
  COLORSCHEME->setColorPair("buddylist", "button", "normal",
      CppConsUI::Curses::Color::GREEN, CppConsUI::Curses::Color::DEFAULT);
  COLORSCHEME->setColorPair("buddylistgroup", "button", "normal",
      CppConsUI::Curses::Color::MAGENTA, CppConsUI::Curses::Color::DEFAULT);

  COLORSCHEME->setColorPair("buddylistbuddy_offline", "button", "normal",
      CppConsUI::Curses::Color::RED, CppConsUI::Curses::Color::DEFAULT);
  COLORSCHEME->setColorPair("buddylistbuddy_online", "button", "normal",
      CppConsUI::Curses::Color::GREEN, CppConsUI::Curses::Color::DEFAULT);
  COLORSCHEME->setColorPair("buddylistbuddy_na", "button", "normal",
      CppConsUI::Curses::Color::MAGENTA, CppConsUI::Curses::Color::DEFAULT);
  COLORSCHEME->setColorPair("buddylistbuddy_away", "button", "normal",
      CppConsUI::Curses::Color::BLUE, CppConsUI::Curses::Color::DEFAULT);

  COLORSCHEME->setColorPair("buddylistcontact_offline", "button", "normal",
      CppConsUI::Curses::Color::RED, CppConsUI::Curses::Color::DEFAULT);
  COLORSCHEME->setColorPair("buddylistcontact_online", "button", "normal",
      CppConsUI::Curses::Color::GREEN, CppConsUI::Curses::Color::DEFAULT);
  COLORSCHEME->setColorPair("buddylistcontact_na", "button", "normal",
      CppConsUI::Curses::Color::MAGENTA, CppConsUI::Curses::Color::DEFAULT);
  COLORSCHEME->setColorPair("buddylistcontact_away", "button", "normal",
      CppConsUI::Curses::Color::BLUE, CppConsUI::Curses::Color::DEFAULT);

  COLORSCHEME->setColorPair("conversation", "textview", "text",
      CppConsUI::Curses::Color::MAGENTA, CppConsUI::Curses::Color::DEFAULT);
  COLORSCHEME->setColorPair("conversation", "textview", "color1",
      CppConsUI::Curses::Color::CYAN, CppConsUI::Curses::Color::DEFAULT);
  COLORSCHEME->setColorPair("conversation", "textview", "color2",
      CppConsUI::Curses::Color::MAGENTA, CppConsUI::Curses::Color::DEFAULT);
  COLORSCHEME->setColorPair("conversation", "panel", "line",
      CppConsUI::Curses::Color::BLUE, CppConsUI::Curses::Color::DEFAULT,
      CppConsUI::Curses::Attr::BOLD);
  COLORSCHEME->setColorPair("conversation", "horizontalline", "line",
      CppConsUI::Curses::Color::BLUE, CppConsUI::Curses::Color::DEFAULT,
      CppConsUI::Curses::Attr::BOLD);
  COLORSCHEME->setColorPair("conversation", "textedit", "text",
      CppConsUI::Curses::Color::CYAN, CppConsUI::Curses::Color::DEFAULT);

  COLORSCHEME->setColorPair("conversation", "label", "text",
      CppConsUI::Curses::Color::CYAN, CppConsUI::Curses::Color::DEFAULT);
  COLORSCHEME->setColorPair("conversation-active", "label", "text",
      CppConsUI::Curses::Color::MAGENTA, CppConsUI::Curses::Color::DEFAULT);
  COLORSCHEME->setColorPair("conversation-new", "label", "text",
      CppConsUI::Curses::Color::CYAN, CppConsUI::Curses::Color::DEFAULT,
      CppConsUI::Curses::Attr::BOLD);

  COLORSCHEME->setColorPair("footer", "label", "text",
      CppConsUI::Curses::Color::BLACK, CppConsUI::Curses::Color::WHITE);
  COLORSCHEME->setColorPair("footer", "container", "background",
      CppConsUI::Curses::Color::BLACK, CppConsUI::Curses::Color::WHITE);

  COLORSCHEME->setColorPair("generalmenu", "panel", "line",
      CppConsUI::Curses::Color::CYAN, CppConsUI::Curses::Color::DEFAULT);
  COLORSCHEME->setColorPair("generalmenu", "horizontalline", "line",
      CppConsUI::Curses::Color::CYAN, CppConsUI::Curses::Color::DEFAULT);
  COLORSCHEME->setColorPair("generalmenu", "button", "normal",
      CppConsUI::Curses::Color::CYAN, CppConsUI::Curses::Color::DEFAULT);

  COLORSCHEME->setColorPair("generalwindow", "panel", "line",
      CppConsUI::Curses::Color::CYAN, CppConsUI::Curses::Color::DEFAULT);
  COLORSCHEME->setColorPair("generalwindow", "horizontalline", "line",
      CppConsUI::Curses::Color::CYAN, CppConsUI::Curses::Color::DEFAULT);
  COLORSCHEME->setColorPair("generalwindow", "verticalline", "line",
      CppConsUI::Curses::Color::CYAN, CppConsUI::Curses::Color::DEFAULT);

  COLORSCHEME->setColorPair("log", "panel", "line",
      CppConsUI::Curses::Color::BLUE, CppConsUI::Curses::Color::DEFAULT,
      CppConsUI::Curses::Attr::BOLD);
  COLORSCHEME->setColorPair("log", "textview", "text",
      CppConsUI::Curses::Color::CYAN, CppConsUI::Curses::Color::DEFAULT);

  COLORSCHEME->setColorPair("header", "label", "text",
      CppConsUI::Curses::Color::BLACK, CppConsUI::Curses::Color::WHITE);
  COLORSCHEME->setColorPair("header", "container", "background",
      CppConsUI::Curses::Color::BLACK, CppConsUI::Curses::Color::WHITE);
  COLORSCHEME->setColorPair("header-request", "label", "text",
      CppConsUI::Curses::Color::RED, CppConsUI::Curses::Color::WHITE);
}

bool CenterIM::saveColorSchemeConfig()
{
  xmlnode *root = xmlnode_new("colorscheme");
  xmlnode_set_attrib(root, "version", "1.0");

  for (CppConsUI::ColorScheme::Schemes::const_iterator
      si = COLORSCHEME->getSchemes().begin();
      si != COLORSCHEME->getSchemes().end(); si++) {
    xmlnode *scheme = xmlnode_new("scheme");
    xmlnode_set_attrib(scheme, "name", si->first.c_str());
    xmlnode_insert_child(root, scheme);

    for (CppConsUI::ColorScheme::Widgets::const_iterator
        wi = si->second.begin();
        wi != si->second.end(); wi++) {
      for (CppConsUI::ColorScheme::Properties::const_iterator
          pi = wi->second.begin();
          pi != wi->second.end(); pi++) {
        xmlnode *color = xmlnode_new("color");
        xmlnode_set_attrib(color, "widget", wi->first.c_str());
        xmlnode_set_attrib(color, "property", pi->first.c_str());
        xmlnode_insert_child(scheme, color);
        char *str;

        if (pi->second.foreground != CppConsUI::Curses::Color::DEFAULT) {
          str = colorToString(pi->second.foreground);
          xmlnode_set_attrib(color, "foreground", str);
          g_free(str);
        }

        if (pi->second.background != CppConsUI::Curses::Color::DEFAULT) {
          str = colorToString(pi->second.background);
          xmlnode_set_attrib(color, "background", str);
          g_free(str);
        }

        if ((str = colorAttributesToString(pi->second.attrs))) {
          xmlnode_set_attrib(color, "attributes", str);
          g_free(str);
        }
      }
    }
  }

  char *data = xmlnode_to_formatted_str(root, NULL);
  bool res = true;
  if (!purple_util_write_data_to_file("colorschemes.xml", data, -1)) {
    LOG->error(_("Error saving 'colorschemes.xml'."));
    res = false;
  }
  g_free(data);
  xmlnode_free(root);
  return res;
}

char *CenterIM::colorToString(int color)
{
  if (color >= -1 && color < static_cast<int>(G_N_ELEMENTS(named_colors) - 1))
    return g_strdup(named_colors[color + 1]);
  return g_strdup_printf("%d", color);
}

bool CenterIM::stringToColor(const char *str, int *color)
{
  g_assert(str);
  g_assert(color);

  *color = 0;

  if (g_ascii_isdigit(str[0]) || str[0] == '-') {
    // numeric colors
    long i = strtol(str, NULL, 10);
    if (errno == ERANGE || i > INT_MAX || i < -1)
      return false;
    *color = i;
    return true;
  }

  // symbolic colors
  for (int i = -1; i < static_cast<int>(G_N_ELEMENTS(named_colors) - 1); i++)
    if (!strcmp(str, named_colors[i + 1])) {
      *color = i;
      return true;
    }

  return false;
}

char *CenterIM::colorAttributesToString(int attrs)
{
#define APPEND(str) do { \
    if (s.size()) \
      s.append("|"); \
    s.append(str); \
  } while (0)

  std::string s;

  if (attrs == CppConsUI::Curses::Attr::NORMAL)
    return NULL;

  if (attrs & CppConsUI::Curses::Attr::STANDOUT)
    APPEND("standout");
  if (attrs & CppConsUI::Curses::Attr::REVERSE)
    APPEND("reverse");
  if (attrs & CppConsUI::Curses::Attr::BLINK)
    APPEND("blink");
  if (attrs & CppConsUI::Curses::Attr::DIM)
    APPEND("dim");
  if (attrs & CppConsUI::Curses::Attr::BOLD)
    APPEND("bold");

  return g_strdup(s.c_str());

#undef APPEND
}

bool CenterIM::stringToColorAttributes(const char *str, int *attrs)
{
  g_assert(str);
  g_assert(attrs);

  gchar **tokens = g_strsplit(str, "|", 0);
  *attrs = 0;

  bool valid = true;
  for (size_t i = 0; tokens[i]; i++) {
    if (!strcmp("normal", tokens[i])) {
      *attrs |= CppConsUI::Curses::Attr::NORMAL;
      continue;
    }
    if (!strcmp("standout", tokens[i])) {
      *attrs |= CppConsUI::Curses::Attr::STANDOUT;
      continue;
    }
    if (!strcmp("reverse", tokens[i])) {
      *attrs |= CppConsUI::Curses::Attr::REVERSE;
      continue;
    }
    if (!strcmp("blink", tokens[i])) {
      *attrs |= CppConsUI::Curses::Attr::BLINK;
      continue;
    }
    if (!strcmp("dim", tokens[i])) {
      *attrs |= CppConsUI::Curses::Attr::DIM;
      continue;
    }
    if (!strcmp("bold", tokens[i])) {
      *attrs |= CppConsUI::Curses::Attr::BOLD;
      continue;
    }
    // unrecognized attribute
    valid = false;
    break;
  }

  g_strfreev(tokens);

  return valid;
}

void CenterIM::loadDefaultKeyConfig()
{
  // clear current bindings and load default ones
  KEYCONFIG->loadDefaultKeyConfig();

  KEYCONFIG->bindKey("centerim", "quit", "Ctrl-q");
  KEYCONFIG->bindKey("centerim", "buddylist", "F1");
  KEYCONFIG->bindKey("centerim", "conversation-active", "F2");
  KEYCONFIG->bindKey("centerim", "accountstatusmenu", "F3");
  KEYCONFIG->bindKey("centerim", "generalmenu", "F4");
  KEYCONFIG->bindKey("centerim", "generalmenu", "Ctrl-g");
  KEYCONFIG->bindKey("centerim", "buddylist-toggle-offline", "F5");
  KEYCONFIG->bindKey("centerim", "conversation-expand", "F6");

  KEYCONFIG->bindKey("centerim", "conversation-prev", "Ctrl-p");
  KEYCONFIG->bindKey("centerim", "conversation-next", "Ctrl-n");
  KEYCONFIG->bindKey("centerim", "conversation-number1", "Alt-1");
  KEYCONFIG->bindKey("centerim", "conversation-number2", "Alt-2");
  KEYCONFIG->bindKey("centerim", "conversation-number3", "Alt-3");
  KEYCONFIG->bindKey("centerim", "conversation-number4", "Alt-4");
  KEYCONFIG->bindKey("centerim", "conversation-number5", "Alt-5");
  KEYCONFIG->bindKey("centerim", "conversation-number6", "Alt-6");
  KEYCONFIG->bindKey("centerim", "conversation-number7", "Alt-7");
  KEYCONFIG->bindKey("centerim", "conversation-number8", "Alt-8");
  KEYCONFIG->bindKey("centerim", "conversation-number9", "Alt-9");
  KEYCONFIG->bindKey("centerim", "conversation-number10", "Alt-0");
  KEYCONFIG->bindKey("centerim", "conversation-number11", "Alt-q");
  KEYCONFIG->bindKey("centerim", "conversation-number12", "Alt-w");
  KEYCONFIG->bindKey("centerim", "conversation-number13", "Alt-e");
  KEYCONFIG->bindKey("centerim", "conversation-number14", "Alt-r");
  KEYCONFIG->bindKey("centerim", "conversation-number15", "Alt-t");
  KEYCONFIG->bindKey("centerim", "conversation-number16", "Alt-y");
  KEYCONFIG->bindKey("centerim", "conversation-number17", "Alt-u");
  KEYCONFIG->bindKey("centerim", "conversation-number18", "Alt-i");
  KEYCONFIG->bindKey("centerim", "conversation-number19", "Alt-o");
  KEYCONFIG->bindKey("centerim", "conversation-number20", "Alt-p");

  KEYCONFIG->bindKey("buddylist", "contextmenu", "Ctrl-d");
  KEYCONFIG->bindKey("buddylist", "filter", "/");

  KEYCONFIG->bindKey("conversation", "send", "Ctrl-x");
}

bool CenterIM::saveKeyConfig()
{
  xmlnode *root = xmlnode_new("keyconfig");
  xmlnode_set_attrib(root, "version", "1.0");

  const CppConsUI::KeyConfig::KeyBinds *binds = KEYCONFIG->getKeyBinds();
  for (CppConsUI::KeyConfig::KeyBinds::const_iterator bi = binds->begin();
      bi != binds->end(); bi++) {
    /* Invert the map because the output should be sorted by context+action,
     * not by context+key. */
    typedef std::multimap<std::string, TermKeyKey> InvertedMap;
    InvertedMap inverted;
    for (CppConsUI::KeyConfig::KeyBindContext::const_iterator
        ci = bi->second.begin();
        ci != bi->second.end(); ci++)
      inverted.insert(std::make_pair(ci->second, ci->first));

    for (InvertedMap::iterator ci = inverted.begin(); ci != inverted.end();
        ci++) {
      xmlnode *bind = xmlnode_new("bind");
      xmlnode_set_attrib(bind, "context", bi->first.c_str());
      xmlnode_set_attrib(bind, "action", ci->first.c_str());
      char *key;
      if ((key = KEYCONFIG->termKeyToString(ci->second))) {
        xmlnode_set_attrib(bind, "key", key);
        g_free(key);
      }

      xmlnode_insert_child(root, bind);
    }
  }

  char *data = xmlnode_to_formatted_str(root, NULL);
  bool res = true;
  if (!purple_util_write_data_to_file("binds.xml", data, -1)) {
    LOG->error(_("Error saving 'binds.xml'."));
    res = false;
  }
  g_free(data);
  xmlnode_free(root);
  return res;
}

void CenterIM::actionFocusBuddyList()
{
  BUDDYLIST->show();
}

void CenterIM::actionFocusActiveConversation()
{
  CONVERSATIONS->focusActiveConversation();
}

void CenterIM::actionOpenAccountStatusMenu()
{
  /* Don't allow to open the account status menu if there is any 'top' window
   * (except general menu, we can close that). */
  CppConsUI::FreeWindow *top = mngr->getTopWindow();
  if (top) {
    if (dynamic_cast<GeneralMenu*>(top))
      top->close();
    else if (top->getType() == CppConsUI::FreeWindow::TYPE_TOP)
      return;
  }

  AccountStatusMenu *menu = new AccountStatusMenu;
  menu->show();
}

void CenterIM::actionOpenGeneralMenu()
{
  /* Don't allow to open the general menu if there is any 'top' window (except
   * account status menu, we can close that). */
  CppConsUI::FreeWindow *top = mngr->getTopWindow();
  if (top) {
    if (dynamic_cast<AccountStatusMenu*>(top))
      top->close();
    else if (top->getType() == CppConsUI::FreeWindow::TYPE_TOP)
      return;
  }

  GeneralMenu *menu = new GeneralMenu;
  menu->show();
}

void CenterIM::actionBuddyListToggleOffline()
{
  gboolean cur = purple_prefs_get_bool(CONF_PREFIX
      "/blist/show_offline_buddies");
  purple_prefs_set_bool(CONF_PREFIX "/blist/show_offline_buddies", !cur);
}

void CenterIM::actionFocusPrevConversation()
{
  CONVERSATIONS->focusPrevConversation();
}

void CenterIM::actionFocusNextConversation()
{
  CONVERSATIONS->focusNextConversation();
}

void CenterIM::actionFocusConversation(int i)
{
  CONVERSATIONS->focusConversation(i);
}

void CenterIM::actionExpandConversation()
{
  CppConsUI::FreeWindow *top = mngr->getTopWindow();
  if (top && top->getType() == CppConsUI::FreeWindow::TYPE_TOP)
    return;

  if (!convs_expanded) {
    CONVERSATIONS->focusActiveConversation();
    top = mngr->getTopWindow();
    if (!top || typeid(Conversation) != typeid(*top))
      return;
  }

  convs_expanded = !convs_expanded;
  CONVERSATIONS->setExpandedConversations(convs_expanded);
  mngr->onScreenResized();
}

void CenterIM::declareBindables()
{
  declareBindable("centerim", "quit",
      sigc::mem_fun(this, &CenterIM::quit),
      InputProcessor::BINDABLE_OVERRIDE);
  declareBindable("centerim", "buddylist",
      sigc::mem_fun(this, &CenterIM::actionFocusBuddyList),
      InputProcessor::BINDABLE_OVERRIDE);
  declareBindable("centerim", "conversation-active",
      sigc::mem_fun(this, &CenterIM::actionFocusActiveConversation),
      InputProcessor::BINDABLE_OVERRIDE);
  declareBindable("centerim", "accountstatusmenu",
      sigc::mem_fun(this, &CenterIM::actionOpenAccountStatusMenu),
      InputProcessor::BINDABLE_OVERRIDE);
  declareBindable("centerim", "generalmenu",
      sigc::mem_fun(this, &CenterIM::actionOpenGeneralMenu),
      InputProcessor::BINDABLE_OVERRIDE);
  declareBindable("centerim", "buddylist-toggle-offline",
      sigc::mem_fun(this, &CenterIM::actionBuddyListToggleOffline),
      InputProcessor::BINDABLE_OVERRIDE);
  declareBindable("centerim", "conversation-prev",
      sigc::mem_fun(this, &CenterIM::actionFocusPrevConversation),
      InputProcessor::BINDABLE_OVERRIDE);
  declareBindable("centerim", "conversation-next",
      sigc::mem_fun(this, &CenterIM::actionFocusNextConversation),
      InputProcessor::BINDABLE_OVERRIDE);
  char action[] = "conversation-numberXX";
  for (int i = 1; i <= 20; i++) {
    g_sprintf(action + sizeof(action) - 3, "%d", i);
    declareBindable("centerim", action, sigc::bind(sigc::mem_fun(this,
            &CenterIM::actionFocusConversation), i),
        InputProcessor::BINDABLE_OVERRIDE);
  }
  declareBindable("centerim", "conversation-expand",
      sigc::mem_fun(this, &CenterIM::actionExpandConversation),
      InputProcessor::BINDABLE_OVERRIDE);
}

/* vim: set tabstop=2 shiftwidth=2 textwidth=78 expandtab : */
