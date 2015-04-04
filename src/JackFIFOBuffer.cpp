/**
 * /file JackFIFOBuffer.cpp.
 * /brief Implementation of class JackFIFOBuffer.
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

#include <cmath>

#include "JackFIFOBuffer.h"
#include "debug.h"

void JackFIFOBuffer::reallocate_buffer(int nchunks, jack_nframes_t nframes)
{
  m_nframes = nframes;
  m_capacity = nframes * nchunks;
  // The following is safe because the buffer isn't used at the moment.
  delete m_buffer;
  m_buffer = new jack_default_audio_sample_t[m_capacity];
  Dout(dc::notice, "Allocated buffer at " << m_buffer << " till " << &m_buffer[m_capacity]);
  m_head = m_buffer;
  clear();
}

JackFIFOBuffer::JackFIFOBuffer(jack_client_t* client, double period) : m_buffer(NULL)
{
  DoutEntering(dc::notice, "JackFIFOBuffer::JackFIFOBuffer(" << client << ", " << period << ")");
  jack_nframes_t sample_rate = jack_get_sample_rate(client);
  Dout(dc::notice, "sample_rate = " << sample_rate);
  jack_nframes_t nframes = jack_get_buffer_size(client);
  Dout(dc::notice, "nframes = " << nframes);
  intptr_t const required_samples = std::round(period * sample_rate);
  int const nchunks = (required_samples + nframes / 2) / nframes + 1;
  reallocate_buffer(nchunks, nframes);
}

void JackFIFOBuffer::buffer_size_changed(jack_nframes_t nframes)
{
  DoutEntering(dc::notice, "JackFIFOBuffer::buffer_size_changed(" << nframes << ")");
  if (nframes == m_nframes)
    return;     // No change.
  int const nchunks = m_capacity / m_nframes;   // Calculate nchunks from the previous number of frames.
  reallocate_buffer(nchunks, nframes);
}
