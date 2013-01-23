/*
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

#include "Notify.h"

#include <cppconsui/MessageDialog.h>
#include <string.h> // memset

Notify *Notify::instance = NULL;

Notify *Notify::Instance()
{
  return instance;
}

Notify::Notify()
{
  memset(&centerim_notify_ui_ops, 0, sizeof(centerim_notify_ui_ops));

  // set the purple notify callbacks
  centerim_notify_ui_ops.notify_message = notify_message_;
  //centerim_notify_ui_ops.notify_email = notify_email_;
  //centerim_notify_ui_ops.notify_emails = notify_emails_;
  //centerim_notify_ui_ops.notify_formatted = notify_formatted_;
  //centerim_notify_ui_ops.notify_searchresults = notify_searchresults_;
  //centerim_notify_ui_ops.notify_searchresults_new_rows =
  //  notify_searchresults_new_rows_;
  //centerim_notify_ui_ops.notify_userinfo = notify_userinfo_;
  //centerim_notify_ui_ops.notify_uri = notify_uri_;
  centerim_notify_ui_ops.close_notify = close_notify_;
  purple_notify_set_ui_ops(&centerim_notify_ui_ops);
}

Notify::~Notify()
{
  purple_notify_set_ui_ops(NULL);
}

void Notify::Init()
{
  g_assert(!instance);

  instance = new Notify;
}

void Notify::Finalize()
{
  g_assert(instance);

  delete instance;
  instance = NULL;
}

void *Notify::notify_message(PurpleNotifyMsgType /*type*/, const char *title,
    const char *primary, const char *secondary)
{
  char *text = g_strdup_printf("%s\n\n%s", primary, secondary);
  CppConsUI::MessageDialog *dialog = new CppConsUI::MessageDialog(title,
      text);
  g_free(text);
  dialog->Show();
  return dialog;
}

void Notify::close_notify(PurpleNotifyType type, void *ui_handle)
{
  // only message notifications are currently supported
  g_assert(type == PURPLE_NOTIFY_MESSAGE);

  CppConsUI::MessageDialog *dialog
    = reinterpret_cast<CppConsUI::MessageDialog*>(ui_handle);
  dialog->Close();
}

/* vim: set tabstop=2 shiftwidth=2 textwidth=78 expandtab : */
