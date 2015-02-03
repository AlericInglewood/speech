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

class JackClient {
  private:
    jack_client_t* m_client;

    jack_port_t* m_input_port;
    jack_port_t* m_output_port;

  public:
    JackClient(char const* name);
    ~JackClient();
    void activate(void);
    void connect(void);

  private:
    static void shutdown_cb(void* self);
    static int process_cb(jack_nframes_t nframes, void* self);

  protected:
    int process(jack_nframes_t nframes);
};

#endif // JACK_CLIENT_H
