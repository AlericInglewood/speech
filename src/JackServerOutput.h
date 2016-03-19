/**
 * \file JackServerOutput.h
 * \brief Declaration of JackServerOutput.
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

#ifndef JACK_SERVER_OUTPUT_H
#define JACK_SERVER_OUTPUT_H

#include "JackOutput.h"

class JackServerOutput : public JackOutput
{
  public:
    // Construct a JackOutput that represents the input to the JACK server (it being the output of the JACK server).
    JackServerOutput() : JackOutput(DEBUG_ONLY("Jack Server")) { }

    // Set the external buffer to read data from.
    void initialize(jack_default_audio_sample_t* chunk, jack_nframes_t nframes)
    {
      // Setting an external buffer for a JackOutput that was previously allocated a buffer is weird.
      ASSERT(!m_allocated);
      m_chunk = chunk;
      m_chunk_size = nframes;
    }

    /*virtual*/ api_type type() const { return api_output_provided_buffer; }

    // JackOutput
    /*virtual*/ event_type fill_output_buffer(int sequence_number);
};

#endif // JACK_SERVER_OUTPUT_H
