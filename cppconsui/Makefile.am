lib_LTLIBRARIES = libcppconsui.la

# When you add files here, also add them in po/POTFILES.in
libcppconsui_la_SOURCES = \
	AbstractDialog.cpp \
	AbstractDialog.h \
	AbstractLine.cpp \
	AbstractLine.h \
	AbstractListBox.cpp \
	AbstractListBox.h \
	Button.cpp \
	Button.h \
	CheckBox.cpp \
	CheckBox.h \
	ColorPicker.cpp \
	ColorPicker.h \
	ColorPickerPalette.cpp \
	ColorPickerPalette.h \
	ColorPickerComboBox.cpp \
	ColorPickerComboBox.h \
	ColorPickerDialog.cpp \
	ColorPickerDialog.h \
	ColorScheme.h \
	ColorScheme.cpp \
	ConsUICurses.cpp \
	ConsUICurses.h \
	Container.cpp \
	Container.h \
	ComboBox.cpp \
	ComboBox.h \
	CoreManager.cpp \
	CoreManager.h \
	CppConsUI.h \
	Dialog.cpp \
	Dialog.h \
	FreeWindow.cpp \
	FreeWindow.h \
	HorizontalLine.cpp \
	HorizontalLine.h \
	HorizontalListBox.cpp \
	HorizontalListBox.h \
	InputDialog.cpp \
	InputDialog.h \
	InputProcessor.cpp \
	InputProcessor.h \
	Label.cpp \
	Label.h \
	ListBox.cpp \
	ListBox.h \
	KeyConfig.cpp \
	KeyConfig.h \
	Keys.cpp \
	Keys.h \
	MenuWindow.cpp \
	MenuWindow.h \
	MessageDialog.cpp \
	MessageDialog.h \
	Panel.cpp \
	Panel.h \
	ScrollPane.cpp \
	ScrollPane.h \
	Spacer.cpp \
	Spacer.h \
	SplitDialog.cpp \
	SplitDialog.h \
	TextEdit.cpp \
	TextEdit.h \
	TextEntry.cpp \
	TextEntry.h \
	TextView.cpp \
	TextView.h \
	TreeView.cpp \
	TreeView.h \
	VerticalLine.cpp \
	VerticalLine.h \
	Widget.cpp \
	Widget.h \
	Window.cpp \
	Window.h \
	tree.hh \
	libtermkey/driver-csi.c \
	libtermkey/driver-ti.c \
	libtermkey/termkey-internal.h \
	libtermkey/termkey.c \
	libtermkey/termkey.h

libcppconsui_la_CPPFLAGS = \
	$(GLIB_CFLAGS) \
	$(SIGC_CFLAGS) \
	$(NCURSESW_CFLAGS) \
	-I$(top_srcdir) \
	-DG_LOG_DOMAIN=\"cppconsui\"

libcppconsui_la_LIBADD = \
	$(GLIB_LIBS) \
	$(SIGC_LIBS) \
	$(NCURSESW_LIBS)
