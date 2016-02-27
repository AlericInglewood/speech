/**
 * /file JackSilenceOutput.cpp
 * /brief Implementation of class JackSilenceOutput.
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
#include "JackSilenceOutput.h"
#include "JackInput.h"

void JackSilenceOutput::fill_output_buffer(int sequence_number)
{
  if (m_sequence_number == sequence_number)
    return;
  m_sequence_number = sequence_number;

  std::memset(m_chunk, 0, m_chunk_size * sizeof(jack_default_audio_sample_t));
  for (auto input : m_connected_inputs)
  {
    if (uses_external_input_buffer(input.second))
      break;
    if (has_provided_input_buffer(input.second) &&
        input.first->provided_input_buffer() == m_chunk)
      break;
    input.first->zero_input();
  }
}

api_type JackSilenceOutput::type() const
{
  return api_output_memcpy;
}

void JackSilenceOutput::memcpy_output(jack_default_audio_sample_t* chunk) const
{
  std::memset(chunk, 0, m_chunk_size * sizeof(jack_default_audio_sample_t));
}
