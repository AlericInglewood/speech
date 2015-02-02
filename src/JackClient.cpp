/**
 * /file JackClient.cpp
 * /brief Implementation of class JackClient.
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

#include <cstdlib>
#include <cstring>
#include <utility>
#include <iostream>

#include "JackClient.h"
#include "JackPorts.h"
#include "Configuration.h"
#include "utils/AIAlert.h"

//static
void JackClient::shutdown_cb(void* UNUSED_ARG(self))
{
  exit(1);
}

//static
int JackClient::process_cb(jack_nframes_t nframes, void* self)
{
  JackClient* client = static_cast<JackClient*>(self);
  return client->process(nframes);
}

int JackClient::process(jack_nframes_t nframes)
{
  jack_default_audio_sample_t* out = (jack_default_audio_sample_t*)jack_port_get_buffer(m_output_port, nframes);
  jack_default_audio_sample_t* in = (jack_default_audio_sample_t*)jack_port_get_buffer(m_input_port, nframes);

  std::memcpy(out, in, sizeof(jack_default_audio_sample_t) * nframes);

  return 0;
}

JackClient::JackClient(char const* name)
{
  // Try to become a client of the JACK server.
  m_client = jack_client_open(name, JackNoStartServer, NULL);
  if (!m_client)
  {
    THROW_ALERT("Could not open jack client \"[NAME]\". Is the JACK server not running?",
	AIArgs("[NAME]", name));
  }

  // Tell the JACK server to call `process()' whenever there is work to be done.
  jack_set_process_callback(m_client, &JackClient::process_cb, this);

  // Tell the JACK server to call `jack_shutdown()' if
  // it ever shuts down, either entirely, or if it
  // just decides to stop calling us.
  jack_on_shutdown(m_client, &JackClient::shutdown_cb, this);

  // Display the current sample rate.
  std::cout << "Engine sample rate: " << jack_get_sample_rate(m_client) << " Hz." << std::endl;

  // Create two ports.
  m_input_port = jack_port_register(m_client, "input", JACK_DEFAULT_AUDIO_TYPE, JackPortIsInput, 0);
  m_output_port = jack_port_register(m_client, "output", JACK_DEFAULT_AUDIO_TYPE, JackPortIsOutput, 0);
}

JackClient::~JackClient()
{
  jack_client_close(m_client);
}

void JackClient::activate(void)
{
  // Tell the JACK server that we are ready to roll.
  int err = jack_activate(m_client);
  if (err)
  {
    THROW_ALERTC(err, "Cannot activate client");
  }
}

// Connect input or output port.
void JackClient::connect(Configuration& config)
{
  bool needs_update = false;
  JackPorts ports(m_client);
  // First connect output then input.
  for (int connect_output = 1; connect_output >= 0; --connect_output)
  {
    // Do some magic to define the correct values to source_port_name and target_port_name.
    std::string source_port_name = connect_output ? config.get_playback_port() : config.get_capture_port();
    if (source_port_name.empty())
    {
      source_port_name = ports.get(JackPortIsPhysical | (connect_output ? JackPortIsInput : JackPortIsOutput));
      if (connect_output)
	config.set_playback_port(source_port_name);
      else
	config.set_capture_port(source_port_name);
      needs_update = true;
    }
    std::string target_port_name = jack_port_name(connect_output ? m_output_port : m_input_port);
    if (connect_output) std::swap(source_port_name, target_port_name);

    // Connect port source_port_name to target_port_name.
    int err = jack_connect(m_client, source_port_name.c_str(), target_port_name.c_str());
    if (err == EEXIST)
    {
      std::cout << "Ports " << source_port_name << " and " << target_port_name << " are already connected!" << std::endl;
    }
    else if (err)
    {
      THROW_ALERTC(err, "jack_connect: Cannot connect port \"[PORT1]\" to \"[PORT2]\"",
	  AIArgs("[PORT1]", source_port_name)("[PORT2]", target_port_name));
    }
  }
  if (needs_update)
    config.update();
}
