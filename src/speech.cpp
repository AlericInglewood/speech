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
#include "JackClient.h"
#include "Configuration.h"
#include "utils/debug_ostream_operators.h"
#include "utils/AIAlert.h"
#include "utils/GlobalObjectManager.h"

int main(void)
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
    Configuration config(config_path);

    // Create the jack client.
    JackClient jack_client("Speech");

    // Connect the ports. Note: you can't do this before
    // the client is activated, because we can't allow
    // connections to be made to clients that aren't
    // running.
    jack_client.activate();
    jack_client.connect(config);

    // Since this is just a toy, run for a few seconds, then finish.
    sleep(10);
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
