/**
 * /file UIWindow.cpp
 * /brief Implementation of class UIWindow.
 *
 * Copyright (C) 2015 Aleric Inglewood.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Affero General Public License as published
 * by the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Affero General Public License for more details.
 *
 * You should have received a copy of the GNU Affero General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "sys.h"

#include "UIWindow.h"
#include "utils/AIAlert.h"
#include "debug.h"

#include <gtkmm.h>

//static
GtkWindow* UIWindow::read_glade(std::string const& glade_path, char const* window_name)
{
  // This block is C code because we need a plain GtkWindow instead of a Gtk::Window.
  // Otherwise we get a warning when wrapping the GtkWindow* in a Gtk::Window below,
  // saying that is already wrapped.

  GtkBuilder* builder = gtk_builder_new_from_file(glade_path.c_str());
  GtkWindow* window = GTK_WINDOW(gtk_builder_get_object(builder, window_name));
  // Do not connect signals C-style though, because we need them to be connected
  // C++-style, which is done in the constructor of UIWindow.
  g_object_unref(G_OBJECT(builder));	// Delete the builder.
  if (!window)
  {
    THROW_ALERT("While reading [GLADE_PATH], could not find object '[WINDOW_NAME]'.",
	AIArgs("[GLADE_PATH]", glade_path)("[WINDOW_NAME]", window_name));
  }
  return window;
}

UIWindow::UIWindow(Glib::RefPtr<Gtk::Application> const& refApp, std::string const& glade_path, char const* window_name) :
    Gtk::Window(read_glade(glade_path, window_name)), m_refApp(refApp)
{
  // Do not leave main loop until the destructor is called.
  m_refApp->hold();

  // Connect signals.
}

UIWindow::~UIWindow()
{
  // Do not terminate the application immediately because otherwise
  // the window is not removed from the screen and we get an annoying
  // pop-up from kwin saying "". g_application_run
  m_refApp->set_inactivity_timeout(1000);
  m_refApp->release();
}

void UIWindow::on_button_clicked(void)
{
  Dout(dc::notice, "Hello World");
}
