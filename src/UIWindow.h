/**
 * /file UIWindow.h
 * /brief Declaration of class UIWindow.
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

#ifndef UI_WINDOW_H
#define UI_WINDOW_H

#include <string>

#include <gtkmm/window.h>
#include <gtkmm/application.h>

class UIWindow : public Gtk::Window
{
  private:
    Glib::RefPtr<Gtk::Application> m_refApp;

    static GtkWindow* read_glade(std::string const& glade_path, char const* window_name);

  public:
    UIWindow(Glib::RefPtr<Gtk::Application> const& refApp, std::string const& glade_path, char const* window_name);
    virtual ~UIWindow();

  protected:
    // Signal handlers.
    void on_button_clicked(void);
    bool on_delete(GdkEventAny* event);
    void on_hide(void);
};

#endif // UI_WINDOW_H
