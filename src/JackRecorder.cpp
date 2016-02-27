/**
 * /file JackRecorder.cpp
 * /brief Implementation of class JackRecorder.
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

#include "sys.h"

#include "JackRecorder.h"

api_type JackRecorder::type() const
{
  // We can only copy to the recorder.
  return api_input_memcpy_zero | api_output_memcpy;
}

void JackRecorder::memcpy_input(jack_default_audio_sample_t const* chunk)
{
  if (!m_recording_buffer.push(chunk))
  {
    throw RecorderFull();
  }
}

void JackRecorder::zero_input()
{
  if (!m_recording_buffer.push_zero())
  {
    throw RecorderFull();
  }
}

void JackRecorder::memcpy_output(jack_default_audio_sample_t* chunk) const
{
  std::memcpy(chunk, m_chunk, m_chunk_size * sizeof(jack_default_audio_sample_t));
}

void JackRecorder::fill_input_buffer(int sequence_number)
{
  if (m_input_sequence_number == sequence_number)
    return;
  m_input_sequence_number = sequence_number;

  ASSERT(m_connected_output);

  m_connected_output->fill_output_buffer(sequence_number);
  if (!m_recording_buffer.push(m_connected_output->chunk_ptr()))
  {
    throw RecorderFull();
  }
}

void JackRecorder::fill_output_buffer(int sequence_number)
{
  if (m_output_sequence_number == sequence_number)
    return;
  m_output_sequence_number = sequence_number;

  jack_default_audio_sample_t* ptr = m_recording_buffer.read();
  if (!ptr)
  {
    throw RecorderEmpty();
  }
  m_chunk = ptr;
  handle_memcpys();
}

