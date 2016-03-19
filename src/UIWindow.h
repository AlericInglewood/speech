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
#include <functional>

// Work around for compile warning in gtkmm 3.10.
#ifndef _GTKMM_STACK_H
#define _GTKMM_STACK_H
#define _GTKMM_STACKSWITCHER_H
#endif

#include <gtkmm/window.h>
#include <gtkmm/application.h>
#include <gtkmm/builder.h>
#include <gtkmm/radiobutton.h>
#include <glibmm/dispatcher.h>

// Forward declaration.
class RecordingDeviceState;

// Helper class.
class GladeBuilder
{
  protected:
    Glib::RefPtr<Gtk::Builder> m_refBuilder;
    GladeBuilder(std::string const& glade_path, char const* window_name);
    Glib::RefPtr<Gtk::Builder> create_from_file(std::string const& glade_path, char const* window_name);
    GtkWindow* get_window(std::string const& glade_path, char const* window_name);
    template<typename T> void get_widget(char const* name, T& widget);
};

class UIWindow : private GladeBuilder, public Gtk::Window
{
  public:
    UIWindow(std::string const& glade_path, std::string const& css_path, char const* window_name, RecordingDeviceState& state);
    virtual ~UIWindow();

  private:
    void stop_playback_if_any();
    void stop_recording_if_any();

  protected:
    // Signal handlers.
    void on_button_record_clicked();
    void on_button_play_clicked();
    void on_button_stop_clicked();
    void on_repeat_toggled();
    void on_playback_to_input_toggled();
    void on_record_radio_toggled(int state);
    void on_stop_radio_toggled(int state);
    void on_wakeup();

  private:
    Gtk::ToggleButton* m_button_record;
    Gtk::ToggleButton* m_button_play;
    Gtk::Button* m_button_stop;
    Gtk::CheckButton* m_checkbox_repeat;
    Gtk::CheckButton* m_checkbox_playback_to_input;
    Gtk::RadioButton* m_radio_input;
    Gtk::RadioButton* m_radio_test_source;
    Gtk::RadioButton* m_radio_passthrough;
    Gtk::RadioButton* m_radio_test_output;
    Gtk::RadioButton* m_radio_mute;

    RecordingDeviceState& m_state;
    Glib::Dispatcher m_state_changed;

    int m_internal_set_active;
    int m_record_radio_buttons_state;
    int m_stop_radio_buttons_state;
    int m_play_check_buttons_state;
};

#endif // UI_WINDOW_H
