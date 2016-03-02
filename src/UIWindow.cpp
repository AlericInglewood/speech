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
#include "utils/at_scope_end.h"
#include "RecordingDeviceState.h"

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

template<typename T>
void GladeBuilder::get_widget(char const* name, T& widget)
{
  widget = NULL;
  m_refBuilder->get_widget(name, widget);
  if (!widget)
  {
    THROW_ALERT("Failed to find widget \"[NAME]\"!", AIArgs("[NAME]", name));
  }
}

UIWindow::UIWindow(std::string const& glade_path, std::string const& css_path, char const* window_name, RecordingDeviceState& state) :
  GladeBuilder(glade_path, window_name),                                // Initialize the builder in the GladeBuilder base class.
  Gtk::Window(GladeBuilder::get_window(glade_path, window_name)),       // Get the window from the builder and wrap it as the Gtk::Window base class.
  m_state(state),
  m_internal_set_active(false),
  m_record_radio_buttons_state(RecordingDeviceState::record_input),
  m_stop_radio_buttons_state(RecordingDeviceState::passthrough),
  m_play_check_buttons_state(0)
{
  Gtk::Window* window;

  try
  {
    // Get all the widgets that we need.
    get_widget(window_name, window);
    get_widget("button_record", m_button_record);
    get_widget("button_play", m_button_play);
    get_widget("button_stop", m_button_stop);
    get_widget("repeat", m_checkbox_repeat);
    get_widget("playback_to_input", m_checkbox_playback_to_input);
    get_widget("input", m_radio_input);
    get_widget("test source", m_radio_test_source);
    get_widget("passthrough", m_radio_passthrough);
    get_widget("test output", m_radio_test_output);
    get_widget("mute", m_radio_mute);
  }
  catch(AIAlert::Error const& error)
  {
    THROW_ALERT("While reading [GLADE_PATH]", AIArgs("[GLADE_PATH]", glade_path), error);
  }

  // Load our css.
  Glib::RefPtr<Gtk::CssProvider> refProvider = Gtk::CssProvider::create();
  try
  {
    refProvider->load_from_path(css_path);
  }
  catch(Glib::Error const& error)
  {
    THROW_ALERT("While parsing [CSS_PATH] a Glib::Error exception occurred: \"[WHAT]\".",
        AIArgs("[CSS_PATH]", css_path)("[WHAT]", error.what()));
  }
  window->get_style_context()->add_provider_for_screen(Gdk::Screen::get_default(), refProvider, GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);

  // Clean up.
  m_refBuilder.reset();         // We're done with the builder.

  // Connect signals.
  m_button_record->signal_clicked().connect([this]{ on_button_record_clicked(); });
  m_button_play->signal_clicked().connect([this]{ on_button_play_clicked(); });
  m_button_stop->signal_clicked().connect([this]{ on_button_stop_clicked(); });
  m_checkbox_repeat->signal_toggled().connect([this]{ on_repeat_toggled(); });
  m_checkbox_playback_to_input->signal_toggled().connect([this]{ on_playback_to_input_toggled(); });
  m_radio_input->signal_toggled().connect([this]{ on_record_radio_toggled(RecordingDeviceState::record_input); });
  m_radio_test_source->signal_toggled().connect([this]{ on_record_radio_toggled(RecordingDeviceState::record_output); });
  m_radio_passthrough->signal_toggled().connect([this]{ on_stop_radio_toggled(RecordingDeviceState::passthrough); });
  m_radio_test_output->signal_toggled().connect([this]{ on_stop_radio_toggled(RecordingDeviceState::direct); });
  m_radio_mute->signal_toggled().connect([this]{ on_stop_radio_toggled(RecordingDeviceState::muted); });

  // Connect audio thread dispatcher.
  m_state_changed.connect([this]{ on_wakeup(); });
  m_state.connect([this]{ m_state_changed.emit(); });
}

UIWindow::~UIWindow()
{
}

void UIWindow::on_button_record_clicked()
{
  if (!m_button_record->get_active())
  {
    if (!m_internal_set_active)
    {
      // User clicked on active Record button. Undo this action.
      m_internal_set_active = true;
      auto&& reset_internal_set_active = at_scope_end([this]{ m_internal_set_active = false; });
      m_button_record->set_active(true);
      // But restart recording.
      m_state.clear_and_set(0, RecordingDeviceState::clear_buffer);
      reset_internal_set_active.now();
    }
    return;
  }
  else if (m_internal_set_active)
  {
    // We're just undoing a user click.
    return;
  }
  // Button was pressed by the user.
  stop_playback_if_any();
  // (Re)start recording.
  m_state.clear_and_set(RecordingDeviceState::record_mask, m_record_radio_buttons_state | RecordingDeviceState::clear_buffer);
}

void UIWindow::on_button_play_clicked()
{
  if (!m_button_play->get_active())
  {
    if (!m_internal_set_active)
    {
      // User clicked on active Play button. Undo this action.
      m_internal_set_active = true;
      auto&& reset_internal_set_active = at_scope_end([this]{ m_internal_set_active = false; });
      m_button_play->set_active(true);
      // But start from the beginning.
      m_state.clear_and_set(0, RecordingDeviceState::playback_reset);
      reset_internal_set_active.now();
    }
    return;
  }
  else if (m_internal_set_active)
  {
    // We're just undoing a user click.
    return;
  }
  // Button was pressed by the user.
  stop_recording_if_any();
  // Start the playback.
  m_state.set_playback_state(RecordingDeviceState::playback);
}

void UIWindow::stop_recording_if_any()
{
  m_state.set_recording_state(0);
  if (m_button_record->get_active())
  {
    m_internal_set_active = true;
    auto&& reset_internal_set_active = at_scope_end([this]{ m_internal_set_active = false; });
    m_button_record->set_active(false);
    reset_internal_set_active.now();
  }
}

void UIWindow::stop_playback_if_any()
{
  m_state.set_playback_state(m_stop_radio_buttons_state);
  if (m_button_play->get_active())
  {
    m_internal_set_active = true;
    auto&& reset_internal_set_active = at_scope_end([this]{ m_internal_set_active = false; });
    m_button_play->set_active(false);
    reset_internal_set_active.now();
  }
}

void UIWindow::on_button_stop_clicked()
{
  stop_recording_if_any();
  stop_playback_if_any();
}

void UIWindow::on_repeat_toggled()
{
  Dout(dc::notice, "Calling UIWindow::on_repeat_toggled(): " << m_checkbox_repeat->get_active());
  m_play_check_buttons_state = (m_play_check_buttons_state & ~RecordingDeviceState::playback_repeat) | (m_checkbox_repeat->get_active() ? RecordingDeviceState::playback_repeat : 0);
  m_state.clear_and_set(RecordingDeviceState::gui2jack_mask, m_play_check_buttons_state);
}

void UIWindow::on_playback_to_input_toggled()
{
  Dout(dc::notice, "Calling UIWindow::on_playback_to_input_toggled(): " << m_checkbox_playback_to_input->get_active());
  m_play_check_buttons_state = (m_play_check_buttons_state & ~RecordingDeviceState::playback_to_input) | (m_checkbox_playback_to_input->get_active() ? RecordingDeviceState::playback_to_input : 0);
  m_state.clear_and_set(RecordingDeviceState::gui2jack_mask, m_play_check_buttons_state);
}

void UIWindow::on_record_radio_toggled(int state)
{
  Gtk::RadioButton* radio_button;
  switch (state)
  {
    case RecordingDeviceState::record_input:
      radio_button = m_radio_input;
      break;
    default: // RecordingDeviceState::record_output
      assert(state == RecordingDeviceState::record_output);
      radio_button = m_radio_test_source;
      break;
  }
  if (radio_button->get_active())
    m_record_radio_buttons_state = state;
  else if (m_record_radio_buttons_state == state)
    m_record_radio_buttons_state = 0;
  Dout(dc::notice, "UIWindow::on_record_radio_toggled(" << state << "): " << radio_button->get_active() << "; m_record_radio_buttons_state = " << m_record_radio_buttons_state);
}

void UIWindow::on_stop_radio_toggled(int state)
{
  Gtk::RadioButton* radio_button;
  switch (state)
  {
    case RecordingDeviceState::passthrough:
      radio_button = m_radio_passthrough;
      break;
    case RecordingDeviceState::direct:
      radio_button = m_radio_test_output;
      break;
    default: // RecordingDeviceState::muted
      assert(state == RecordingDeviceState::muted);
      radio_button = m_radio_mute;
      break;
  }
  if (radio_button->get_active())
    m_stop_radio_buttons_state = state;
  else if (m_stop_radio_buttons_state == state)
    m_stop_radio_buttons_state = RecordingDeviceState::muted;
  Dout(dc::notice, "Calling UIWindow::on_stop_radio_toggled(" << state << "): " << radio_button->get_active() << "; m_stop_radio_buttons_state = " << m_stop_radio_buttons_state);
  if (!m_state.is_playing())
    m_state.set_playback_state(m_stop_radio_buttons_state);
}

void UIWindow::on_wakeup()
{
  Dout(dc::notice, "UIWindow::on_wakeup()");
  // Fix button states.
  if (!m_state.is_playing())
    stop_playback_if_any();
  if (!m_state.is_recording())
    stop_recording_if_any();
}
