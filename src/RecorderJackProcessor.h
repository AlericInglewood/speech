/**
 * \file RecorderJackProcessor.h
 * \brief Declaration of RecorderJackProcessor.
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

#ifndef RECORDER_JACK_PROCESSOR_H
#define RECORDER_JACK_PROCESSOR_H

#include "JackProcessor.h"
#include "JackFIFOBuffer.h"
#include <exception>

class RecorderEmpty : public std::exception
{
};

class RecorderJackProcessor : public JackProcessor
{
  private:
    JackFIFOBuffer m_recording_buffer;

  public:
    RecorderJackProcessor(jack_client_t* client, double period) : m_recording_buffer(client, period) { }

    void buffer_size_changed(jack_nframes_t nframes);
    void clear();
    void reset_readptr();

  public:
    // This object provides the buffers in process().
    /*virtual*/ bool provides_input_buffer() const { return true; }
    /*virtual*/ bool provides_output_buffer() const { return true; }

    /*virtual*/ void fill_input_buffer(int sequence_number);

    // Read input, process, write output.
    /*virtual*/ void generate_output(int sequence_number);
};

#endif // RECORDER_JACK_PROCESSOR_H
