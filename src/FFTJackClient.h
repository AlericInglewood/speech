/**
 * /file FFTJackClient.h
 * /brief Jack client for speech recognition.
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

#ifndef FFT_JACK_CLIENT_H
#define FFT_JACK_CLIENT_H

#include "JackClient.h"

class FFTJackClient : public JackClient {
  protected:
    jack_nframes_t m_fft_buffer_size;

  public:
    FFTJackClient(char const* name);
    void set_fft_buffer_size(jack_nframes_t nframes);

  protected:
    /*virtual*/ void calculate_delay(jack_latency_range_t& range);
    /*virtual*/ int process(jack_default_audio_sample_t* in, jack_default_audio_sample_t* out, jack_nframes_t nframes);
    /*virtual*/ int buffer_size_changed(void);
};

#endif // FFT_JACK_CLIENT_H
