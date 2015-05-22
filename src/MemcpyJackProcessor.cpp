/**
 * /file MemcpyJackProcessor.cpp
 * /brief Implementation of class MemcpyJackProcessor.
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

#include "MemcpyJackProcessor.h"

void MemcpyJackProcessor::process()
{
  jack_default_audio_sample_t const* in = m_input.chunk_ptr();
  jack_default_audio_sample_t* out = m_output.chunk_ptr();
  jack_nframes_t const nframes = m_input.nframes();
  ASSERT(nframes == m_output.nframes());

  std::memcpy(out, in, nframes * sizeof(jack_default_audio_sample_t));
}
