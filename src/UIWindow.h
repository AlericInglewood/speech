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

#include "FFTJackClientStates.h"

#include <string>
#include <functional>

#include <gtkmm/window.h>
#include <gtkmm/application.h>
#include <gtkmm/builder.h>

// Helper class.
class GladeBuilder
{
  protected:
    Glib::RefPtr<Gtk::Builder> m_refBuilder;
    GladeBuilder(std::string const& glade_path, char const* window_name);
    Glib::RefPtr<Gtk::Builder> create_from_file(std::string const& glade_path, char const* window_name);
    GtkWindow* get_window(std::string const& glade_path, char const* window_name);
};

class UIWindow : private GladeBuilder, public Gtk::Window
{
    typedef std::function<void(int)> set_state_cb_type;

  public:
    UIWindow(std::string const& glade_path, std::string const& css_path, char const* window_name,
             set_state_cb_type const& set_playback_state_cb, set_state_cb_type const& set_record_state_cb);
    virtual ~UIWindow();

  protected:
    // Signal handlers.
    void on_button_record_clicked();
    void on_button_play_clicked();
    void on_button_stop_clicked();

  private:
    set_state_cb_type m_set_playback_state_cb;
    set_state_cb_type m_set_record_state_cb;
};

#endif // UI_WINDOW_H
