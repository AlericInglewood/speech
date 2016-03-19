/**
 * /file JackServerInput.cpp
 * /brief Implementation of class JackServerInput.
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
#include "JackServerInput.h"

jack_default_audio_sample_t* JackServerInput::provided_input_buffer() const
{
  return m_chunk;
}

jack_nframes_t JackServerInput::nframes_provided_input_buffer() const
{
  return m_chunk_size;
}

event_type JackServerInput::memcpy_input(jack_default_audio_sample_t const* chunk)
{
  std::memcpy(m_chunk, chunk, m_chunk_size * sizeof(jack_default_audio_sample_t));
  return 0;
}

event_type JackServerInput::zero_input()
{
  std::memset(m_chunk, 0, m_chunk_size * sizeof(jack_default_audio_sample_t));
  return 0;
}
