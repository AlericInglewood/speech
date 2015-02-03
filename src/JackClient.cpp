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

  // Tell the JACK server to call `port_connect_cb' whenever a connection is made.
  jack_set_port_connect_callback(m_client, &JackClient::port_connect_cb, this);

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

// Connect input and output port.
void JackClient::connect(void)
{
  bool needs_update = false;
  JackPorts ports(m_client);
  // First connect output then input.
  for (int connect_output = 1; connect_output >= 0; --connect_output)
  {
    // Do some magic to define the correct values to source_port_name and target_port_name.
    std::set<std::string> other_port_names =
      connect_output ? Singleton<Configuration>::instance().get_playback_ports()
                     : Singleton<Configuration>::instance().get_capture_ports();
    if (other_port_names.empty())
    {
      other_port_names.insert(ports.get(JackPortIsPhysical | (connect_output ? JackPortIsInput : JackPortIsOutput)));
      if (connect_output)
	Singleton<Configuration>::instance().set_playback_ports(other_port_names);
      else
	Singleton<Configuration>::instance().set_capture_ports(other_port_names);
      needs_update = true;
    }
    std::string our_port = jack_port_name(connect_output ? m_output_port : m_input_port);
    for (std::set<std::string>::iterator iter = other_port_names.begin(); iter != other_port_names.end(); ++iter)
    {
      std::string source_port_name = *iter;
      std::string target_port_name = our_port;
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
  }
  if (needs_update)
    Singleton<Configuration>::instance().update();
}

//static
void JackClient::port_connect_cb(jack_port_id_t a, jack_port_id_t b, int yn, void* self)
{
  JackClient* client = static_cast<JackClient*>(self);
  client->port_connect(a, b, yn);
}

void JackClient::port_connect(jack_port_id_t a, jack_port_id_t b, int)
{
  jack_port_t const* port_a = jack_port_by_id(m_client, a);
  jack_port_t const* port_b = jack_port_by_id(m_client, b);
  if (jack_port_is_mine(m_client, port_a))
  {
    assert(strcmp(jack_port_short_name(port_a), "output") == 0);
    assert(jack_port_flags(port_a) == JackPortIsOutput);
    char const** array = jack_port_get_connections(port_a);
    std::set<std::string> playback_ports;
    for (char const** ptr = array; *ptr; ++ptr) playback_ports.insert(*ptr);
    jack_free(array);
    Singleton<Configuration>::instance().set_playback_ports(playback_ports);
  }
  if (jack_port_is_mine(m_client, port_b))
  {
    assert(strcmp(jack_port_short_name(port_b), "input") == 0);
    assert(jack_port_flags(port_b) == JackPortIsInput);
    char const** array = jack_port_get_connections(port_b);
    std::set<std::string> capture_ports;
    for (char const** ptr = array; *ptr; ++ptr) capture_ports.insert(*ptr);
    jack_free(array);
    Singleton<Configuration>::instance().set_capture_ports(capture_ports);
  }
  Singleton<Configuration>::instance().update();
}
