/**
 * /file speech.cpp
 * /brief Jack client for speech recognition.
 *
 * Copyright (C) 2014, 2015 Aleric Inglewood.
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

#include <iostream>
#include <cerrno>
#include <cstdlib>
#include <string>
#include <boost/filesystem.hpp>

#include "debug.h"
#include "FFTJackClient.h"
#include "UIWindow.h"
#include "Configuration.h"
#include "utils/debug_ostream_operators.h"
#include "utils/AIAlert.h"
#include "utils/GlobalObjectManager.h"

int main(int argc, char* argv[])
{
#ifdef DEBUGGLOBAL
  GlobalObjectManager::main_entered();
#endif
  Debug(debug::init());

  try
  {
    // Create and/or read the configuration file.
    char const* home = getenv("HOME");
    if (!home)
    {
      std::cerr << "HOME environment variable is not set." << std::endl;
      exit(1);
    }
    boost::filesystem::path config_path(home);
    config_path += "/.config/speech/";
    if (!boost::filesystem::exists(config_path))
    {
      boost::filesystem::create_directories(config_path);
    }
    config_path += "config.xml";
    Singleton<Configuration>::instance().set_path(config_path);

    // Find the UI glade file.
    char const* speech_src = getenv("SRCROOT"); // This works in our special env.source build environment.
    if (!speech_src)
    {
      THROW_ALERT("Environment variable SRCROOT not set. $SRCROOT/res/speechUI.glade must exist.");
    }
    std::string glade_path = std::string(speech_src) + "/res/speechUI.glade";
    if (!boost::filesystem::exists(glade_path))
    {
      THROW_ALERT("[GLADE_PATH]: No such file or directory. "
                  "Is your SRCROOT environment variable set correctly?",
                  AIArgs("[GLADE_PATH]", glade_path));
    }
    std::string css_path = std::string(speech_src) + "/res/speechUI.css";
    if (!boost::filesystem::exists(css_path))
    {
      THROW_ALERT("[CSS_PATH]: No such file or directory.",
                  AIArgs("[CSS_PATH]", css_path));
    }

    // Create the jack client.
    FFTJackClient jack_client("Speech", 10.0);

    // Create the UIWindow before activating the jack client, because it
    // creates a dispatcher that theoretically could be called from the jack client.
    Glib::RefPtr<Gtk::Application> refApp = Gtk::Application::create(argc, argv, "com.alinoe.speech");
    UIWindow* ui_window = new UIWindow(glade_path, css_path, "window1", jack_client);

    // Connect the ports. Note: you can't do this before the client is activated either,
    // because we can't allow connections to be made to clients that aren't running.
    jack_client.activate();
    jack_client.connect_ports();

    // Show the GUI.
    refApp->run(*ui_window);
    // Bug workaround for https://bugzilla.gnome.org/show_bug.cgi?id=744876
    // Let the mainloop run until all pending activity is handled.
    while (g_main_context_iteration(NULL, FALSE)) ;

#if 0 // For now, exit jack client when GUI window is closed.
    // Run until killed by the user.
    sleep(-1);
#endif
  }
  catch (AIAlert::ErrorCode const& error)
  {
    std::cerr << error << ": " << strerror(error.getCode()) << std::endl;
    return 1;
  }
  catch(AIAlert::Error const& error)
  {
    std::cerr << error << std::endl;
    return 1;
  }

  return 0;
}
