/**
 * \file JackServerInput.h
 * \brief Declaration of JackServerInput.
 *
 * Copyright (C) 2015, 2016 Aleric Inglewood.
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

#ifndef JACK_SERVER_INPUT_H
#define JACK_SERVER_INPUT_H

#include "JackInput.h"

class JackServerInput : public JackInput
{
  private:
    jack_default_audio_sample_t* m_chunk;       // Externally provided buffer.
    jack_nframes_t m_chunk_size;                // The buffer size, in frames.

  public:
    // Construct a JackInput that represents the output to the JACK server (it being the input of the JACK server).
    JackServerInput() : JackInput(DEBUG_ONLY("Jack Server")) { }

    // Set the external buffer to write data to.
    void initialize(jack_default_audio_sample_t* chunk, jack_nframes_t nframes)
    {
      m_chunk = chunk;
      m_chunk_size = nframes;
    }

    /*virtual*/ api_type type() const { return api_input_provided_buffer_memcpy_zero; }

    /*virtual*/ jack_default_audio_sample_t* provided_input_buffer() const;
    /*virtual*/ jack_nframes_t nframes_provided_input_buffer() const;
    /*virtual*/ void memcpy_input(jack_default_audio_sample_t const* chunk);
    /*virtual*/ void zero_input();
};

#endif // JACK_SERVER_INPUT_H
