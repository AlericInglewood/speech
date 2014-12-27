/**
 * /file speech.cpp
 * /brief Jack client for speech recognition.
 *
 * Copyright (C) 2014 Aleric Inglewood.
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

#include <cerrno>
#include <cstdlib>
#include <cstring>

#include <unistd.h>
#include <jack/jack.h>

#include "debug.h"
#include "utils/debug_ostream_operators.h"
#include "utils/AIAlert.h"
#include "utils/GlobalObjectManager.h"

jack_port_t* input_port;
jack_port_t* output_port;

int process(jack_nframes_t nframes, void* UNUSED_ARG(arg))
{
  jack_default_audio_sample_t* out = (jack_default_audio_sample_t*)jack_port_get_buffer(output_port, nframes);
  jack_default_audio_sample_t* in = (jack_default_audio_sample_t*)jack_port_get_buffer(input_port, nframes);

  memcpy(out, in, sizeof(jack_default_audio_sample_t) * nframes);

  return 0;
}

void jack_shutdown(void* UNUSED_ARG(arg))
{
  exit(1);
}

int main(void)
{
#ifdef DEBUGGLOBAL
  GlobalObjectManager::main_entered();
#endif
  Debug(debug::init());

  try
  {
    jack_client_t* client;
    char const** ports;

    // Try to become a client of the JACK server.
    if ((client = jack_client_open("Speech", JackNoStartServer, NULL)) == NULL)
    {
      THROW_ALERT("Could not open jack client \"[NAME]\". Is the JACK server not running?",
	  AIArgs("[NAME]", "Speech"));
    }

    // Tell the JACK server to call `process()' whenever there is work to be done.
    jack_set_process_callback(client, process, 0);

    // Tell the JACK server to call `jack_shutdown()' if
    // it ever shuts down, either entirely, or if it
    // just decides to stop calling us.
    jack_on_shutdown(client, jack_shutdown, 0);

    // Display the current sample rate.
    std::cout << "Engine sample rate: " << jack_get_sample_rate(client) << " Hz." << std::endl;

    // Create two ports.
    input_port = jack_port_register(client, "input", JACK_DEFAULT_AUDIO_TYPE, JackPortIsInput, 0);
    output_port = jack_port_register(client, "output", JACK_DEFAULT_AUDIO_TYPE, JackPortIsOutput, 0);

    // Tell the JACK server that we are ready to roll.
    int err = jack_activate(client);
    if (err)
    {
      THROW_ALERTC(err, "Cannot activate client");
    }

    // Connect the ports. Note: you can't do this before
    // the client is activated, because we can't allow
    // connections to be made to clients that aren't
    // running.
    if ((ports = jack_get_ports(client, NULL, NULL, JackPortIsPhysical|JackPortIsOutput)) == NULL)
    {
      THROW_ALERT("Cannot find any physical capture ports.");
    }

    err = jack_connect(client, ports[0], jack_port_name(input_port));
    if (err == EEXIST)
    {
      std::cout << "Ports " << ports[0] << " and " << jack_port_name(input_port) << " are already connected!" << std::endl;
    }
    else if (err)
    {
      THROW_ALERTC(err, "jack_connect: Cannot connect input ports \"[PORT1]\" and \"[PORT2]\"",
	  AIArgs("[PORT1]", ports[0])("[PORT2]", jack_port_name(input_port)));
    }

    free(ports);

    if ((ports = jack_get_ports(client, NULL, NULL, JackPortIsPhysical|JackPortIsInput)) == NULL)
    {
      THROW_ALERT("Cannot find any physical playback ports.");
    }

    err = jack_connect(client, jack_port_name(output_port), ports[0]);
    if (err == EEXIST)
    {
      std::cout << "Ports " << ports[0] << " and " << jack_port_name(input_port) << " are already connected!" << std::endl;
    }
    else if (err)
    {
      THROW_ALERTC(err, "jack_connect: Cannot connect output ports \"[PORT1]\" and \"[PORT2]\"",
	  AIArgs("[PORT1]", jack_port_name(output_port))("[PORT2]", ports[0]));
    }

    free(ports);

    // Since this is just a toy, run for a few seconds, then finish.

    sleep(10);
    jack_client_close(client);

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
