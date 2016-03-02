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
#include <cmath>

#include "debug.h"
#include "FFTJackClient.h"
#include "JackProcessor.h"
#include "JackChunkAllocator.h"
#include "utils/macros.h"

FFTJackClient::FFTJackClient(char const* name, double period) : JackClient(name), RecordingDeviceState(passthrough),
  m_fft_buffer_size(0), m_playback_state(0), m_sequence_number(0),
  m_recorder(m_client, period),
  m_recording_switch(m_recorder), m_test_switch(m_fft_processor), m_output_switch(m_jack_server_input),
  m_crossfade_frame(-1)
{
  // Set the size of the FFT buffer, in samples.
  set_fft_buffer_size(256);
}

void FFTJackClient::output_source_changed()
{
  // The output source changed, initialize crossfading.
  constexpr float PI = 3.141592653589793f;      // Real precision is 3.1415927(410..)
  m_crossfade_frame = 0;
  m_crossfade_nframes = m_sample_rate / 50;     // Suppress output in 20 ms.
  m_crossfade_frame_normalization = PI / m_crossfade_nframes;
}

int FFTJackClient::process(jack_default_audio_sample_t* left, jack_default_audio_sample_t* right, jack_nframes_t nframes)
{
  DoutEntering(dc::notice, "FFTJackClient::process(" << left << ", " << right << ", " << nframes << ")");

  m_jack_server_input.initialize(right, nframes);      // right is the input from the jack server perspective.
  m_jack_server_output.initialize(left, nframes);

  // So we can jump back on a routing error.
  while (true)
  {
    // Starting a new process sequence
    ++m_sequence_number;

    // Read the state bits.
    int statebits = get_state();

    if (statebits != m_last_state)
    {
      if (AI_UNLIKELY(statebits & commands_mask))
      {
        if ((statebits & clear_buffer))
        {
          m_recorder.clear();
        }
        if ((statebits & playback_reset))
        {
          m_recorder.reset_readptr();
        }
        clear_and_set(commands_mask, 0);
      }
      m_recorder.set_repeat(is_repeat(statebits));

      bool const actually_playback_to_input = (statebits & (playback | playback_to_input)) == (playback | playback_to_input);
      bool const direct_or_playback_to_input = (statebits & direct) || actually_playback_to_input;

#if DEBUG_PROCESS
      Debug(if (!dc::notice.is_on()) dc::notice.on());
      assert(libcwd::channels::dc::notice.is_on());
#endif // DEBUG_PROCESS
      Dout(dc::notice, "-----------------------------------------------");
#if 0
      Dout(dc::notice, "record_input = " << (statebits & record_input) << ", record_output = " << (statebits & record_output) <<
                       ", playback_to_input = " << (statebits & playback_to_input) << ", direct_or_playback_to_input = " << (direct_or_playback_to_input) <<
                       " (direct = " << (statebits & direct) <<
                       "), playback = " << (statebits & playback) << ", passthrough = " << (statebits & passthrough));
#endif

      if ((statebits & record_input))
        m_recording_switch << m_jack_server_output;
      else if ((statebits & record_output))
        m_recording_switch << m_fft_processor;
      else
        m_recording_switch.disconnect();

      if (actually_playback_to_input)
        m_test_switch << m_recorder;
      else
        m_test_switch << m_jack_server_output;

      if (direct_or_playback_to_input)
        m_output_switch << m_fft_processor;
      else if ((statebits & playback))
        m_output_switch << m_recorder;
      else if ((statebits & passthrough))
        m_output_switch << m_jack_server_output;
      else
        m_output_switch << m_silence;

      m_last_state = statebits;
    }
#if DEBUG_PROCESS
    else
    {
      assert(!libcwd::channels::dc::notice.is_on());
    }
#endif // DEBUG_PROCESS

    try
    {
      // Fill recorder.
      if ((statebits & record_mask) || m_recording_switch.is_crossfading()) // (Still) recording?
      {
        m_recorder.fill_input_buffer(m_sequence_number);
      }

      // Fill JACK server.
      m_jack_server_input.fill_input_buffer(m_sequence_number);

      // Success.
      break;
    }
    catch (RoutingError const&)
    {
      // Stop recording/playback if needed.
      int const recorder_error = m_recorder.error();
      if ((recorder_error & recorder_empty))
      {
#if DEBUG_PROCESS
        Debug(if (!dc::notice.is_on()) dc::notice.on());
        Dout(dc::notice, "(Playing back to output -->) BUFFER EMPTY!");
#endif // DEBUG_PROCESS
        // Recording buffer is empty, mute the output.
        set_playback_state(statebits & (direct | passthrough));
      }
      if ((recorder_error & recorder_full))
      {
#if DEBUG_PROCESS
        Debug(if (!dc::notice.is_on()) dc::notice.on());
        Dout(dc::notice, "(RECORDING -->) BUFFER FULL!");
#endif // DEBUG_PROCESS
        // The recording buffer is full, stop recording.
        statebits &= ~record_mask;
        set_recording_state(0);
      }
      if (recorder_error)
        m_wakeup_gui();

      // Routing should be changed now.
      ASSERT(get_state() != m_last_state);
    }
  }

#if 0
  {
#if DEBUG_PROCESS
    Dout(dc::notice, "Crossfading from " << prev_source << " to " << current_source << "!");
#endif // DEBUG_PROCESS
    // Crossfade from prev_source to current_source.
    jack_nframes_t frame = 0;
    while (frame < nframes && m_crossfade_frame < m_crossfade_nframes)
    {
      float factor = 0.5f * (std::cos(m_crossfade_frame_normalization * m_crossfade_frame) + 1.0f);
      out[frame] = current_source[frame] + (prev_source[frame] - current_source[frame]) * factor;
      ++frame;
      ++m_crossfade_frame;
    }
    if (m_crossfade_frame == m_crossfade_nframes)       // Did the crossfading finish?
    {
#if DEBUG_PROCESS
      Debug(if (!dc::notice.is_on()) dc::notice.on());
      Dout(dc::notice, "(Crossfading from ... to ...! -->) Crossfading finished!");
#endif // DEBUG_PROCESS
      // Fill the remainder of the current output buffer if the crossfading finished before the end.
      if (frame < nframes)
      {
        std::memcpy(&out[frame], &current_source[frame], sizeof(jack_default_audio_sample_t) * (nframes - frame));
      }
      // Set the previous state bits to zero; this will cause prev_source to become NULL the next call.
      m_state.fetch_and(current_mask);
      m_crossfade_frame = -1;
    }
  }
#endif // 0

#if DEBUG_PROCESS
  Debug(if (dc::notice.is_on()) dc::notice.off());
  assert(!libcwd::channels::dc::notice.is_on());
#endif // DEBUG_PROCESS
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

void FFTJackClient::buffer_size_changed()
{
  // The FFT buffer must be at least as large as the input buffer.
  if (m_fft_buffer_size < m_input_buffer_size)
  {
    set_fft_buffer_size(m_input_buffer_size);
  }

  // Make sure that our internal buffers are large enough.
  m_recorder.buffer_size_changed(m_input_buffer_size);
  JackChunkAllocator::instance().buffer_size_changed(m_input_buffer_size);
  m_silence.buffer_size_changed(m_input_buffer_size);      // Must be called after JackChunkAllocator::buffer_size_changed.
}
