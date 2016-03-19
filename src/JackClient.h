/**
 * /file JackClient.h
 * /brief Jack client for speech recognition.
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

#ifndef JACK_CLIENT_H
#define JACK_CLIENT_H

#include <jack/jack.h>

class Configuration;

class JackClient
{
  protected:
    jack_client_t* m_client;
    jack_nframes_t m_input_buffer_size;

    jack_port_t* m_input_port;
    jack_port_t* m_output_port;

    jack_nframes_t m_sample_rate;

  public:
    JackClient(char const* name);
    virtual ~JackClient();
    void activate();
    void connect_ports();

  private:
    static void thread_init_cb(void* self);
    static void shutdown_cb(void* self);
    static int sample_rate_cb(jack_nframes_t nframes, void* self);
    static int buffer_size_cb(jack_nframes_t buffer_size, void* self);
    static int process_cb(jack_nframes_t nframes, void* self);
    static void port_connect_cb(jack_port_id_t a, jack_port_id_t b, int yn, void* self);
    static void latency_cb(jack_latency_callback_mode_t mode, void* self);

  protected:
    virtual void thread_init() { }
    virtual void shutdown() { }
    virtual int sample_rate_changed(jack_nframes_t) { return 0; }
    virtual void buffer_size_changed() { }
    virtual void port_connect(jack_port_id_t a, jack_port_id_t b, int yn);
    virtual void latency(jack_latency_callback_mode_t mode);
    virtual void calculate_delay(jack_latency_range_t& range) { range.min = range.max = 0; }

    virtual int process(jack_default_audio_sample_t* in, jack_default_audio_sample_t* out, jack_nframes_t nframes);
};

#endif // JACK_CLIENT_H
