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

GladeBuilder::GladeBuilder(std::string const& glade_path, char const* window_name) :
  m_refBuilder(create_from_file(glade_path, window_name))
{
}

Glib::RefPtr<Gtk::Builder> GladeBuilder::create_from_file(std::string const& glade_path, char const* window_name)
{
  Glib::RefPtr<Gtk::Builder> refBuilder;
  try
  {
    // Read and parse the glade file into a new GtkBuilder object.
    refBuilder = Gtk::Builder::create_from_file(glade_path.c_str(), window_name);
  }
  catch(Glib::FileError const& ex)
  {
    THROW_ALERT("While reading [GLADE_PATH], a Glib::FileError exception was thrown: [WHAT].",
	AIArgs("[GLADE_PATH]", glade_path)("[WHAT]", ex.what()));
  }
  catch(Glib::MarkupError const& ex)
  {
    THROW_ALERT("While reading [GLADE_PATH], a Glib::MarkupError exception was thrown: [WHAT].",
	AIArgs("[GLADE_PATH]", glade_path)("[WHAT]", ex.what()));
  }
  catch(Gtk::BuilderError const& ex)
  {
    THROW_ALERT("While reading [GLADE_PATH], a Glib::BuilderError exception was thrown: [WHAT].",
	AIArgs("[GLADE_PATH]", glade_path)("[WHAT]", ex.what()));
  }
  return refBuilder;
}

GtkWindow* GladeBuilder::get_window(std::string const& glade_path, char const* window_name)
{
  // Get and return a C-style window object (GtkWindow*) because otherwise we'll get
  // a warning when wrapping the GtkWindow* in the Gtk::Window base class of UIWindow,
  // saying that is already wrapped.
  GtkWindow* window = GTK_WINDOW(gtk_builder_get_object(m_refBuilder->gobj(), window_name));

  // Do not connect signals C-style, because we need them to be connected C++-style,
  // which is done in the constructor of UIWindow.

  // Return result, if any.
  if (!window)
  {
    THROW_ALERT("While reading [GLADE_PATH], could not find object '[WINDOW_NAME]'.",
	AIArgs("[GLADE_PATH]", glade_path)("[WINDOW_NAME]", window_name));
  }
  return window;
}

UIWindow::UIWindow(std::string const& glade_path, char const* window_name) :
  GladeBuilder(glade_path, window_name),				// Initialize the builder in the GladeBuilder base class.
  Gtk::Window(GladeBuilder::get_window(glade_path, window_name)) 	// Get the window from the builder and wrap it as the Gtk::Window base class.
{

  // Connect signals.

  // Clean up.
  m_refBuilder.reset();		// We're done with the builder.
}

UIWindow::~UIWindow()
{
}

void UIWindow::on_button_clicked(void)
{
  Dout(dc::notice, "Hello World");
}
