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
#include "Events.h"
#include "utils/macros.h"

event_type JackRecorder::memcpy_input(jack_default_audio_sample_t const* chunk)
{
  return m_recording_buffer.push(chunk) ? 0 : event_bit_stop_recording;
}

event_type JackRecorder::zero_input()
{
  return m_recording_buffer.push_zero() ? 0 : event_bit_stop_recording;
}

event_type JackRecorder::fill_output_buffer(int sequence_number)
{
  if (m_output_sequence_number == sequence_number)
    return 0;
  m_sequence_number = sequence_number;
  // api_output_provided_buffer requires we set m_chunk and m_chunk_size in fill_output_buffer.
  while (true)  // So we can use break and continue.
  {
    m_chunk = m_recording_buffer.read();
    if (AI_UNLIKELY(!m_chunk))
    {
      Dout(dc::notice, "JackRecorder::fill_output_buffer(" << sequence_number << "): at end of recording buffer.");
      // We reached the end. Reset the read pointer to the beginning of the buffer.
      reset_readptr();
      if (!m_repeat || m_recording_buffer.empty())
      {
        throw BrokenPipe(event_bit_stop_playback);
      }
      // We get here at most once because after reset_readptr() either read() will succeed or m_recording_buffer.empty() will return true.
      continue; // Try again.
    }
    m_chunk_size = m_recording_buffer.nframes();
    break;      // Done.
  }
  return handle_memcpys();
}
