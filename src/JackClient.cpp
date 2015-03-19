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
#include <cassert>

#include "debug.h"
#include "JackClient.h"
#include "JackPorts.h"
#include "Configuration.h"
#include "utils/AIAlert.h"

//static
void JackClient::thread_init_cb(void* self)
{
  Debug(debug::init_thread());
  JackClient* client = static_cast<JackClient*>(self);
  client->thread_init();
}

//static
void JackClient::shutdown_cb(void* self)
{
  JackClient* client = static_cast<JackClient*>(self);
  client->shutdown();
  exit(1);
}

//static
int JackClient::process_cb(jack_nframes_t nframes, void* self)
{
  JackClient* client = static_cast<JackClient*>(self);
  jack_default_audio_sample_t* in = (jack_default_audio_sample_t*)jack_port_get_buffer(client->m_input_port, nframes);
  jack_default_audio_sample_t* out = (jack_default_audio_sample_t*)jack_port_get_buffer(client->m_output_port, nframes);
  return client->process(in, out, nframes);
}

JackClient::JackClient(char const* name) : m_sample_rate(0), m_input_buffer_size(0)
{
  // Try to become a client of the JACK server.
  m_client = jack_client_open(name, JackNoStartServer, NULL);
  if (!m_client)
  {
    THROW_ALERT("Could not open jack client \"[NAME]\". Is the JACK server not running?",
	AIArgs("[NAME]", name));
  }

  // Tell JACK to call thread_init_cb once just after the creation of the thread in which all
  // other callbacks will be handled.
  jack_set_thread_init_callback(m_client, &JackClient::thread_init_cb, this);

  // Tell the Jack server to call sample_rate_cb whenever the system sample rate changes.
  jack_set_sample_rate_callback(m_client, &JackClient::sample_rate_cb, this);

  // Tell JACK to call buffer_size_cb whenever the size of the the buffer that will be
  // passed to the process_callback is about to change.
  jack_set_buffer_size_callback(m_client, &JackClient::buffer_size_cb, this);

  // Tell the JACK server to call process_cb whenever there is work to be done.
  jack_set_process_callback(m_client, &JackClient::process_cb, this);

  // Tell the JACK server to call latency_cb whenever it is necessary to recompute
  // the latencies for some or all Jack ports.
  jack_set_latency_callback(m_client, &JackClient::latency_cb, this);

  // Tell the JACK server to call shutdown_cb if it ever shuts down, either entirely,
  // or if it just decides to stop calling us.
  jack_on_shutdown(m_client, &JackClient::shutdown_cb, this);

  // Tell the JACK server to call port_connect_cb whenever a connection is made.
  jack_set_port_connect_callback(m_client, &JackClient::port_connect_cb, this);

  // Create two ports.
  m_input_port = jack_port_register(m_client, "input", JACK_DEFAULT_AUDIO_TYPE, JackPortIsInput, 0);
  m_output_port = jack_port_register(m_client, "output", JACK_DEFAULT_AUDIO_TYPE, JackPortIsOutput, 0);
}

//static
int JackClient::sample_rate_cb(jack_nframes_t nframes, void* self)
{
  JackClient* client = static_cast<JackClient*>(self);
  client->m_sample_rate = nframes;
  Dout(dc::notice, "Engine sample rate: " << nframes << " Hz.");
  return client->sample_rate_changed();
}

//static
int JackClient::buffer_size_cb(jack_nframes_t buffer_size, void* self)
{
  JackClient* client = static_cast<JackClient*>(self);
  client->m_input_buffer_size = buffer_size;
  Dout(dc::notice, "Input buffer size: " << buffer_size << " samples.");
  return client->buffer_size_changed();
}

JackClient::~JackClient()
{
  jack_client_close(m_client);
}

void JackClient::activate()
{
  // Tell the JACK server that we are ready to roll.
  int err = jack_activate(m_client);
  if (err)
  {
    THROW_ALERTC(err, "Cannot activate client");
  }
}

// Connect input and output port.
void JackClient::connect()
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

void JackClient::port_connect(jack_port_id_t a, jack_port_id_t b, int
#ifdef CWDEBUG
    what
#endif
    )
{
  jack_port_t const* port_a = jack_port_by_id(m_client, a);
  jack_port_t const* port_b = jack_port_by_id(m_client, b);
  DoutEntering(dc::notice, "JackClient::port_connect(\"" << jack_port_name(port_a) << "\", \"" << jack_port_name(port_b) << "\", " << what << ")");

  if (jack_port_is_mine(m_client, port_a))
  {
    assert(strcmp(jack_port_short_name(port_a), "output") == 0);
    assert(jack_port_flags(port_a) == JackPortIsOutput);
    char const** array = jack_port_get_connections(port_a);
    std::set<std::string> playback_ports;
    if (array)
    {
      for (char const** ptr = array; *ptr; ++ptr) playback_ports.insert(*ptr);
      jack_free(array);
    }
    Singleton<Configuration>::instance().set_playback_ports(playback_ports);
  }
  if (jack_port_is_mine(m_client, port_b))
  {
    assert(strcmp(jack_port_short_name(port_b), "input") == 0);
    assert(jack_port_flags(port_b) == JackPortIsInput);
    char const** array = jack_port_get_connections(port_b);
    std::set<std::string> capture_ports;
    if (array)
    {
      for (char const** ptr = array; *ptr; ++ptr) capture_ports.insert(*ptr);
      jack_free(array);
    }
    Singleton<Configuration>::instance().set_capture_ports(capture_ports);
  }
  Singleton<Configuration>::instance().update();
}

//static
void JackClient::latency_cb(jack_latency_callback_mode_t mode, void* self)
{
  JackClient* client = static_cast<JackClient*>(self);
  client->latency(mode);
}

void JackClient::latency(jack_latency_callback_mode_t mode)
{
  DoutEntering(dc::notice, "JackClient::latency(" << mode << ")");
  jack_latency_range_t range;
  range.min = 1000000;
  range.max = 0;
  jack_port_t* our_port = (mode == JackPlaybackLatency) ? m_input_port : m_output_port;
  char const** connected_ports = jack_port_get_connections(our_port);
  if (connected_ports)
  {
    char const** ptr = connected_ports;
    assert(*ptr);	// Otherwise start with the while and don't call jack_port_set_latency_range when range is still 1000000, 0.
    do
    {
      jack_port_t* port = jack_port_by_name(m_client, *ptr);
      jack_latency_range_t port_latency_range;
      jack_port_get_latency_range(port, mode, &port_latency_range);
      range.min = std::min(range.min, port_latency_range.min);
      range.max = std::max(range.max, port_latency_range.max);
    }
    while(*++ptr);
    jack_free(connected_ports);
    // Since we only have one input and one output port, we're free to add all delay on the output port.
    if (mode == JackCaptureLatency)
    {
      // Calculate our delay.
      jack_latency_range_t delay;
      calculate_delay(delay);
      // Update range.
      range.min += delay.min;
      range.max += delay.max;
    }
    Dout(dc::notice, "Calling jack_port_set_latency_range(" <<
	 ((mode == JackPlaybackLatency) ? "m_input_port, JackPlaybackLatency" : "m_output_port, JackCaptureLatency") <<
	 ", {" << range.min << ", " << range.max << "})");
    jack_port_set_latency_range(our_port, mode, &range);
  }
}
