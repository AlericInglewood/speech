/**
 * /file RecorderJackProcessor.cpp
 * /brief Implementation of class RecorderJackProcessor.
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

#include "RecorderJackProcessor.h"

void RecorderJackProcessor::process(int sequence_number)
{
  if (m_sequence_number == sequence_number)
    return;
  m_sequence_number = sequence_number;

  // The output buffer is dynamically changed therefore everyone should point to us.
  ASSERT(m_output.connected_input());

  jack_default_audio_sample_t* ptr = m_recording_buffer.read();
  if (!ptr)
  {
    throw RecorderEmpty();
  }
  m_output.update_buffer_ptr(ptr);
}

void RecorderJackProcessor::buffer_size_changed(jack_nframes_t nframes)
{
  m_recording_buffer.buffer_size_changed(nframes);
}

void RecorderJackProcessor::clear()
{
  m_recording_buffer.clear();
}

void RecorderJackProcessor::reset_readptr()
{
  m_recording_buffer.reset_readptr();
}
