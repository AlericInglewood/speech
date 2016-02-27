/**
 * /file JackProcessor.cpp
 * /brief Implementation of class JackProcessor.
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
#include "JackProcessor.h"

api_type JackProcessor::type() const
{
  // Assume as default that no buffers are provided by the processor.
  return api_input_memcpy_zero | api_output_memcpy;
}

void JackProcessor::memcpy_input(jack_default_audio_sample_t const* chunk)
{
  std::memcpy(m_connected_output->chunk_ptr(), chunk, m_connected_output->nframes() * sizeof(jack_default_audio_sample_t));
}

void JackProcessor::zero_input()
{
  std::memset(m_connected_output->chunk_ptr(), 0, m_connected_output->nframes() * sizeof(jack_default_audio_sample_t));
}

void JackProcessor::memcpy_output(jack_default_audio_sample_t* chunk) const
{
  std::memcpy(chunk, m_chunk, m_chunk_size * sizeof(jack_default_audio_sample_t));
}

void JackProcessor::fill_output_buffer(int sequence_number)
{
  if (m_sequence_number == sequence_number)
    return;
  m_sequence_number = sequence_number;

  generate_output();
  handle_memcpys();
}
