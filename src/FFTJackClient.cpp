/**
 * /file FFTJackClient.cpp
 * /brief Implementation of class FFTJackClient.
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

#include <iostream>

#include "debug.h"
#include "FFTJackClient.h"

FFTJackClient::FFTJackClient(char const* name) : JackClient(name), m_fft_buffer_size(0)
{
  // Set the size of the FFT buffer, in samples.
  set_fft_buffer_size(256);
}

int FFTJackClient::process(jack_default_audio_sample_t* in, jack_default_audio_sample_t* out, jack_nframes_t nframes)
{
  //std::cout << "Copying " << nframes << " frames, each of " << sizeof(jack_default_audio_sample_t) << " bytes." << std::endl;
  std::memcpy(out, in, sizeof(jack_default_audio_sample_t) * nframes);
  return 0;
}

void FFTJackClient::calculate_delay(jack_latency_range_t& range)
{
  range.min = range.max = m_fft_buffer_size - m_input_buffer_size;
}

void FFTJackClient::set_fft_buffer_size(jack_nframes_t nframes)
{
  m_fft_buffer_size = std::max(m_input_buffer_size, nframes);
  Dout(dc::notice, "FFT buffer size: " << m_fft_buffer_size << " samples.");
}

int FFTJackClient::buffer_size_changed(void)
{
  if (m_fft_buffer_size < m_input_buffer_size)
  {
    set_fft_buffer_size(m_input_buffer_size);
  }
  return 0;
}

