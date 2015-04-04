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
#include "JackFIFOBuffer.h"
#include "RecordingDeviceState.h"

#include <atomic>
#include <cassert>
#include <memory>

class FFTJackClient : public JackClient, public RecordingDeviceState
{
  protected:
    jack_nframes_t m_fft_buffer_size;
    int m_playback_state;
    std::unique_ptr<jack_default_audio_sample_t[]> m_silence_buffer;
    std::unique_ptr<jack_default_audio_sample_t[]> m_test_buffer;
    JackFIFOBuffer m_recording_buffer;

    // Helper variables used during crossfading.
    jack_nframes_t m_crossfade_frame;
    jack_nframes_t m_crossfade_nframes;
    float m_crossfade_frame_normalization;

  public:
    FFTJackClient(char const* name, double period);
    virtual ~FFTJackClient() { }

    void set_fft_buffer_size(jack_nframes_t nframes);

  protected:
    // Inherited from JackClient.
    /*virtual*/ void calculate_delay(jack_latency_range_t& range);
    /*virtual*/ int process(jack_default_audio_sample_t* in, jack_default_audio_sample_t* out, jack_nframes_t nframes);
    /*virtual*/ void buffer_size_changed();
    // Inherited from RecordingDeviceState.
    /*virtual*/ void output_source_changed();

  private:
    FFTJackClient(FFTJackClient const&);
    FFTJackClient(FFTJackClient&&);
};

#endif // FFT_JACK_CLIENT_H
