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

#include "AccountWindow.h"

#include "Log.h"

#include <errno.h>
#include "gettext.h"

AccountWindow::AccountWindow()
: SplitDialog(0, 0, 80, 24, _("Accounts"))
{
  SetColorScheme("generalwindow");

  accounts = new CppConsUI::TreeView(AUTOSIZE, AUTOSIZE);
  SetContainer(*accounts);

  Populate();

  buttons->AppendItem(_("Add"), sigc::mem_fun(this,
        &AccountWindow::AddAccount));
  buttons->AppendSeparator();
  buttons->AppendItem(_("Done"), sigc::hide(sigc::mem_fun(this,
          &AccountWindow::Close)));
}

void AccountWindow::OnScreenResized()
{
  MoveResizeRect(CENTERIM->GetScreenAreaSize(CenterIM::CHAT_AREA));
}

AccountWindow::BoolOption::BoolOption(PurpleAccount *account,
    PurpleAccountOption *option)
: account(account), option(option), type(TYPE_PURPLE)
{
  g_assert(account);
  g_assert(option);

  SetText(purple_account_option_get_text(option));
  SetState(purple_account_get_bool(account,
        purple_account_option_get_setting(option),
        purple_account_option_get_default_bool(option)));

  signal_toggle.connect(sigc::mem_fun(this, &BoolOption::OnToggle));
}

AccountWindow::BoolOption::BoolOption(PurpleAccount *account,
    Type type)
: account(account), option(NULL), type(type)
{
  g_assert(account);

  if (type == TYPE_REMEMBER_PASSWORD) {
    SetText(_("Remember password"));
    SetState(purple_account_get_remember_password(account));
  }
  else if (type == TYPE_ENABLE_ACCOUNT) {
    SetText(_("Account enabled"));
    SetState(purple_account_get_enabled(account, PACKAGE_NAME));
  }

  signal_toggle.connect(sigc::mem_fun(this, &BoolOption::OnToggle));
}

void AccountWindow::BoolOption::OnToggle(CheckBox& /*activator*/,
    bool new_state)
{
  if (type == TYPE_REMEMBER_PASSWORD)
    purple_account_set_remember_password(account, new_state);
  else if (type == TYPE_ENABLE_ACCOUNT)
    purple_account_set_enabled(account, PACKAGE_NAME, new_state);
  else
    purple_account_set_bool(account,
        purple_account_option_get_setting(option), new_state);
}

AccountWindow::StringOption::StringOption(PurpleAccount *account,
    PurpleAccountOption *option)
: Button(FLAG_VALUE), account(account), option(option), type(TYPE_PURPLE)
{
  g_assert(account);
  g_assert(option);

  Initialize();
}

AccountWindow::StringOption::StringOption(PurpleAccount *account, Type type)
: Button(FLAG_VALUE), account(account), option(NULL), type(type)
{
  g_assert(account);

  Initialize();
}

void AccountWindow::StringOption::Initialize()
{
  if (type == TYPE_PASSWORD)
    SetText(_("Password"));
  else if (type == TYPE_ALIAS)
    SetText(_("Alias"));
  else
    SetText(purple_account_option_get_text(option));

  UpdateValue();
  signal_activate.connect(sigc::mem_fun(this, &StringOption::OnActivate));
}

void AccountWindow::StringOption::UpdateValue()
{
  if (type == TYPE_PASSWORD) {
    SetMasked(true);
    SetValue(purple_account_get_password(account));
  }
  else if (type == TYPE_ALIAS)
    SetValue(purple_account_get_alias(account));
  else
    SetValue(purple_account_get_string(account,
          purple_account_option_get_setting(option),
          purple_account_option_get_default_string(option)));
}

void AccountWindow::StringOption::OnActivate(Button& /*activator*/)
{
  CppConsUI::InputDialog *dialog = new CppConsUI::InputDialog(GetText(),
      GetValue());
  dialog->SetMasked(GetMasked());
  dialog->signal_response.connect(sigc::mem_fun(this,
        &StringOption::ResponseHandler));
  dialog->Show();
}

void AccountWindow::StringOption::ResponseHandler(
    CppConsUI::InputDialog& activator,
    AbstractDialog::ResponseType response)
{
  switch (response) {
    case AbstractDialog::RESPONSE_OK:
      if (type == TYPE_PASSWORD)
        purple_account_set_password(account, activator.GetText());
      else if (type == TYPE_ALIAS)
        purple_account_set_alias(account, activator.GetText());
      else
        purple_account_set_string(account,
            purple_account_option_get_setting(option), activator.GetText());

      UpdateValue();
      break;
    default:
      break;
  }
}

AccountWindow::IntOption::IntOption(PurpleAccount *account,
    PurpleAccountOption *option)
: Button(FLAG_VALUE), account(account), option(option)
{
  g_assert(account);
  g_assert(option);

  SetText(purple_account_option_get_text(option));
  UpdateValue();
  signal_activate.connect(sigc::mem_fun(this, &IntOption::OnActivate));
}

void AccountWindow::IntOption::UpdateValue()
{
  SetValue(purple_account_get_int(account,
        purple_account_option_get_setting(option),
        purple_account_option_get_default_int(option)));
}

void AccountWindow::IntOption::OnActivate(Button& /*activator*/)
{
  CppConsUI::InputDialog *dialog = new CppConsUI::InputDialog(GetText(),
      GetValue());
  dialog->SetFlags(CppConsUI::TextEntry::FLAG_NUMERIC);
  dialog->signal_response.connect(sigc::mem_fun(this,
        &IntOption::ResponseHandler));
  dialog->Show();
}

void AccountWindow::IntOption::ResponseHandler(
    CppConsUI::InputDialog& activator,
    CppConsUI::AbstractDialog::ResponseType response)
{
  const char *text;
  long int i;

  switch (response) {
    case AbstractDialog::RESPONSE_OK:
      text = activator.GetText();
      errno = 0;
      i = strtol(text, NULL, 10);
      if (errno == ERANGE || i > INT_MAX || i < INT_MIN)
        LOG->Warning(_("Value is out of range."));
      purple_account_set_int(account,
          purple_account_option_get_setting(option),
          CLAMP(i, INT_MIN, INT_MAX));

      UpdateValue();
      break;
    default:
      break;
  }
}

AccountWindow::StringListOption::StringListOption(PurpleAccount *account,
    PurpleAccountOption *option)
: account(account), option(option)
{
  g_assert(account);
  g_assert(option);

  SetText(purple_account_option_get_text(option));

  const char *def = purple_account_get_string(account,
      purple_account_option_get_setting(option),
      purple_account_option_get_default_list_value(option));
  for (GList *l = purple_account_option_get_list(option); l; l = l->next)
    if (l->data) {
      PurpleKeyValuePair *kvp = reinterpret_cast<PurpleKeyValuePair*>(
          l->data);
      AddOptionPtr(kvp->key, kvp->value);
      if (kvp->value && def
          && !strcmp(def, reinterpret_cast<const char*>(kvp->value)))
        SetSelectedByDataPtr(kvp->value);
    }

  signal_selection_changed.connect(sigc::mem_fun(this,
        &StringListOption::OnSelectionChanged));
}

void AccountWindow::StringListOption::OnSelectionChanged(
    ComboBox& /*activator*/, int /*new_entry*/, const char* /*title*/,
    intptr_t data)
{
  purple_account_set_string(account,
      purple_account_option_get_setting(option),
      reinterpret_cast<const char*>(data));
}

AccountWindow::SplitOption::SplitOption(PurpleAccount *account,
    PurpleAccountUserSplit *split, AccountEntry *account_entry)
: Button(FLAG_VALUE), account(account), split(split)
, account_entry(account_entry)
{
  g_assert(account);

  if (split)
    SetText(purple_account_user_split_get_text(split));
  else
    SetText(_("Username"));

  signal_activate.connect(sigc::mem_fun(this, &SplitOption::OnActivate));
}

void AccountWindow::SplitOption::UpdateSplits()
{
  SplitWidgets *split_widgets = &account_entry->split_widgets;
  SplitWidgets::iterator split_widget = split_widgets->begin();
  SplitOption *widget = *split_widget;
  const char *val = widget->GetValue();
  split_widget++;

  GString *username = g_string_new(val);
  PurplePluginProtocolInfo *prplinfo = PURPLE_PLUGIN_PROTOCOL_INFO(
      purple_find_prpl(purple_account_get_protocol_id(account)));

  for (GList *iter = prplinfo->user_splits;
      iter && split_widget != split_widgets->end();
      iter = iter->next, split_widget++) {
    PurpleAccountUserSplit *user_split
      = reinterpret_cast<PurpleAccountUserSplit*>(iter->data);
    widget = *split_widget;

    val = widget->GetValue();
    if (!val || !*val)
      val = purple_account_user_split_get_default_value(user_split);
    g_string_append_printf(username, "%c%s",
        purple_account_user_split_get_separator(user_split), val);
  }

  purple_account_set_username(account, username->str);
  g_string_free(username, TRUE);
}

void AccountWindow::SplitOption::OnActivate(Button& /*activator*/)
{
  CppConsUI::InputDialog *dialog = new CppConsUI::InputDialog(text, value);
  dialog->signal_response.connect(sigc::mem_fun(this,
        &SplitOption::ResponseHandler));
  dialog->Show();
}

void AccountWindow::SplitOption::ResponseHandler(
    CppConsUI::InputDialog& activator,
    CppConsUI::AbstractDialog::ResponseType response)
{
  switch (response) {
    case AbstractDialog::RESPONSE_OK:
      SetValue(activator.GetText());
      UpdateSplits();
      break;
    default:
      break;
  }
}

AccountWindow::ProtocolOption::ProtocolOption(PurpleAccount *account,
    AccountWindow &account_window)
: account_window(&account_window), account(account)
{
  g_assert(account);

  SetText(_("Protocol"));

  for (GList *i = purple_plugins_get_protocols(); i; i = i->next)
    AddOptionPtr(purple_plugin_get_name(
          reinterpret_cast<PurplePlugin*>(i->data)), i->data);

  const char *proto_id = purple_account_get_protocol_id(account);
  PurplePlugin *plugin = purple_plugins_find_with_id(proto_id);
  SetSelectedByDataPtr(plugin);

  signal_selection_changed.connect(sigc::mem_fun(this,
        &ProtocolOption::OnProtocolChanged));
}

void AccountWindow::ProtocolOption::OnProtocolChanged(ComboBox& /*activator*/,
    size_t /*new_entry*/, const char* /*title*/, intptr_t data)
{
  purple_account_set_protocol_id(account, purple_plugin_get_id(
        reinterpret_cast<PurplePlugin*>(data)));

  // this deletes us so don't touch any instance variable after
  account_window->PopulateAccount(account);
}

AccountWindow::ColorOption::ColorOption(PurpleAccount *account)
: ColorPicker(CppConsUI::Curses::Color::DEFAULT,
    CppConsUI::Curses::Color::DEFAULT, _("Buddylist Color:"), true)
, account(account)
{
  g_assert(account);

  int fg = purple_account_get_ui_int(account, "centerim5",
      "buddylist-foreground-color", CppConsUI::Curses::Color::DEFAULT);
  int bg = purple_account_get_ui_int(account, "centerim5",
      "buddylist-background-color", CppConsUI::Curses::Color::DEFAULT);
  SetColorPair(fg, bg);
  signal_colorpair_selected.connect(sigc::mem_fun(this,
        &ColorOption::OnColorChanged));
}

void AccountWindow::ColorOption::OnColorChanged(
    CppConsUI::ColorPicker& /*activator*/, int new_fg, int new_bg)
{
  purple_account_set_ui_int(account, "centerim5",
      "buddylist-foreground-color", new_fg);
  purple_account_set_ui_int(account, "centerim5",
      "buddylist-background-color", new_bg);
}

bool AccountWindow::ClearAccount(PurpleAccount *account, bool full)
{
  AccountEntry *account_entry = &account_entries[account];

  // move focus
  account_entry->parent->GrabFocus();

  // remove the nodes from the tree
  accounts->DeleteNodeChildren(account_entry->parent_reference, false);
  if (full) {
    accounts->DeleteNode(account_entry->parent_reference, false);
    account_entries.erase(account);
  }

  if (account_entries.empty())
    buttons->GrabFocus();

  return false;
}

void AccountWindow::Populate()
{
  for (GList *i = purple_accounts_get_all(); i; i = i->next) {
    PurpleAccount *account = reinterpret_cast<PurpleAccount*>(i->data);
    PopulateAccount(account);
  }
}

void AccountWindow::PopulateAccount(PurpleAccount *account)
{
  if (account_entries.find(account) == account_entries.end()) {
    // no entry for this account, so add one
    AccountEntry entry;
    entry.parent = NULL;
    account_entries[account] = entry;
  }
  else {
    // the account exists, so clear all data
    ClearAccount(account, false);
  }

  AccountEntry *account_entry = &account_entries[account];

  if (!account_entry->parent) {
    CppConsUI::TreeView::ToggleCollapseButton *button
      = new CppConsUI::TreeView::ToggleCollapseButton;
    CppConsUI::TreeView::NodeReference parent_reference
      = accounts->AppendNode(accounts->GetRootNode(), *button);
    accounts->SetCollapsed(parent_reference, true);
    account_entry->parent = button;
    account_entry->parent_reference = parent_reference;
  }

  char *label = g_strdup_printf("[%s] %s",
      purple_account_get_protocol_name(account),
      purple_account_get_username(account));
  account_entry->parent->SetText(label);
  g_free(label);

  const char *protocol_id = purple_account_get_protocol_id(account);
  PurplePlugin *prpl = purple_find_prpl(protocol_id);

  if (!prpl) {
    // we cannot change the settings of an unknown account
    CppConsUI::Label *label = new CppConsUI::Label(
        _("Invalid account or protocol plugin not loaded"));
    accounts->AppendNode(account_entry->parent_reference, *label);
  }
  else {
    PurplePluginProtocolInfo *prplinfo = PURPLE_PLUGIN_PROTOCOL_INFO(prpl);

    // protocols combobox
    ProtocolOption *combobox = new ProtocolOption(account, *this);
    CppConsUI::TreeView::NodeReference protocol_node
      = accounts->AppendNode(account_entry->parent_reference, *combobox);
    combobox->GrabFocus();

    /* The username must be treated in a special way because it can contain
     * multiple values (e.g. user@server:port/resource). */
    char *username = g_strdup(purple_account_get_username(account));

    for (GList *iter = g_list_last(prplinfo->user_splits); iter;
        iter = iter->prev) {
      PurpleAccountUserSplit *split
        = reinterpret_cast<PurpleAccountUserSplit*>(iter->data);

      char *s;
      if (purple_account_user_split_get_reverse(split))
        s = strrchr(username, purple_account_user_split_get_separator(split));
      else
        s = strchr(username, purple_account_user_split_get_separator(split));

      const char *value;
      if (s) {
        *s = '\0';
        value = s + 1;
      }
      else
        value = purple_account_user_split_get_default_value(split);

      // create widget for the username split and remember
      SplitOption *widget_split = new SplitOption(account, split,
          account_entry);
      widget_split->SetValue(value);
      account_entry->split_widgets.push_front(widget_split);

      accounts->AppendNode(account_entry->parent_reference, *widget_split);
    }

    SplitOption *widget_split = new SplitOption(account, NULL, account_entry);
    widget_split->SetValue(username);
    account_entry->split_widgets.push_front(widget_split);
    accounts->InsertNodeAfter(protocol_node, *widget_split);
    g_free(username);

    // password
    Widget *widget = new StringOption(account, StringOption::TYPE_PASSWORD);
    accounts->AppendNode(account_entry->parent_reference, *widget);

    // remember password
    widget = new BoolOption(account, BoolOption::TYPE_REMEMBER_PASSWORD);
    accounts->AppendNode(account_entry->parent_reference, *widget);

    // alias
    widget = new StringOption(account, StringOption::TYPE_ALIAS);
    accounts->AppendNode(account_entry->parent_reference, *widget);

    for (GList *pref = prplinfo->protocol_options; pref; pref = pref->next) {
      PurpleAccountOption *option
        = reinterpret_cast<PurpleAccountOption*>(pref->data);
      PurplePrefType type = purple_account_option_get_type(option);

      switch (type) {
        case PURPLE_PREF_STRING:
          widget = new StringOption(account, option);
          accounts->AppendNode(account_entry->parent_reference, *widget);
          break;
        case PURPLE_PREF_INT:
          widget = new IntOption(account, option);
          accounts->AppendNode(account_entry->parent_reference, *widget);
          break;
        case PURPLE_PREF_BOOLEAN:
          widget = new BoolOption(account, option);
          accounts->AppendNode(account_entry->parent_reference, *widget);
          break;
        case PURPLE_PREF_STRING_LIST:
          widget = new StringListOption(account, option);
          accounts->AppendNode(account_entry->parent_reference, *widget);
          break;
        default:
          LOG->Error(_("Invalid Account Option pref type (%d)."), type);
          break;
      }
    }

    // enable/disable account
    widget = new BoolOption(account, BoolOption::TYPE_ENABLE_ACCOUNT);
    accounts->AppendNode(account_entry->parent_reference, *widget);

    // coloring
    widget = new ColorOption(account);
    accounts->AppendNode(account_entry->parent_reference, *widget);
  }

  // drop account
  CppConsUI::Button *drop_button = new CppConsUI::Button(_("Drop account"));
  drop_button->signal_activate.connect(sigc::bind(sigc::mem_fun(this,
          &AccountWindow::DropAccount), account));
  accounts->AppendNode(account_entry->parent_reference, *drop_button);
}

void AccountWindow::AddAccount(CppConsUI::Button& /*activator*/)
{
  GList *i = purple_plugins_get_protocols();
  PurpleAccount *account = purple_account_new(_("Username"),
      purple_plugin_get_id(reinterpret_cast<PurplePlugin*>(i->data)));

  /* Stop here if libpurple returned an already created account. This happens
   * when user pressed Add button twice in a row. In that case there is
   * already one "empty" account that user can edit. */
  if (account_entries.find(account) == account_entries.end()) {
    purple_accounts_add(account);

    PopulateAccount(account);
  }
  account_entries[account].parent->GrabFocus();
}

void AccountWindow::DropAccount(CppConsUI::Button& /*activator*/,
    PurpleAccount *account)
{
  CppConsUI::MessageDialog *dialog = new CppConsUI::MessageDialog(
      _("Account deletion"),
      _("Are you sure you want to delete this account?"));
  dialog->signal_response.connect(sigc::bind(sigc::mem_fun(this,
          &AccountWindow::DropAccountResponseHandler), account));
  dialog->Show();
}

void AccountWindow::DropAccountResponseHandler(
    CppConsUI::MessageDialog& /*activator*/,
    CppConsUI::AbstractDialog::ResponseType response, PurpleAccount *account)
{
  switch (response) {
    case AbstractDialog::RESPONSE_OK:
      purple_accounts_remove(account);
      ClearAccount(account, true);
      break;
    default:
      break;
  }
}

/* vim: set tabstop=2 shiftwidth=2 textwidth=78 expandtab : */
