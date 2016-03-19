/**
 * \file JackRecorder.h
 * \brief Declaration of JackRecorder.
 *
 * Copyright (C) 2016 Aleric Inglewood.
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

#ifndef JACK_RECORDER_H
#define JACK_RECORDER_H

#include "JackInput.h"
#include "JackOutput.h"
#include "JackFIFOBuffer.h"

class JackRecorder : public JackInput, public JackOutput
{
  private:
    JackFIFOBuffer m_recording_buffer;
    int m_input_sequence_number;              // sequence_number of the last call to fill_input_buffer.
    int m_output_sequence_number;             // sequence_number of the last call to fill_output_buffer.
    bool m_repeat;

  public:
    JackRecorder(jack_client_t* client, double period) :
        DEBUG_ONLY(JackInput("JackRecorder"), JackOutput("JackRecorder"),)
        m_recording_buffer(client, period), m_input_sequence_number(-1), m_repeat(false) { }

    void buffer_size_changed(jack_nframes_t nframes)
    {
      m_recording_buffer.buffer_size_changed(nframes);
      m_chunk_size = nframes;
    }

    void set_repeat(bool repeat)
    {
      m_repeat = repeat;
    }

    void clear()
    {
      m_recording_buffer.clear();
      m_input_sequence_number = m_output_sequence_number = -1;
    }

    void reset_readptr()
    {
      m_recording_buffer.reset_readptr();
      m_input_sequence_number = m_output_sequence_number = -1;
    }

  public:
    // We can only copy to the recorder.
    /*virtual*/ api_type type() const { return api_input_memcpy_zero | api_output_provided_buffer; }

    // JackInput
    /*virtual*/ event_type memcpy_input(jack_default_audio_sample_t const* chunk);
    /*virtual*/ event_type zero_input();

    // JackOutput
    /*virtual*/ event_type fill_output_buffer(int sequence_number);
};

#endif // JACK_RECORDER_H
