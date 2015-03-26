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
#include "utils/macros.h"

using namespace FFTJackClientStates;

FFTJackClient::FFTJackClient(char const* name, double period) : JackClient(name), m_fft_buffer_size(0), m_state(passthrough), m_playback_state(0), m_recording_buffer(m_client, period), m_crossfade_frame(-1)
{
  // Set the size of the FFT buffer, in samples.
  set_fft_buffer_size(256);
}

void FFTJackClient::set_recording_state(int record_state)
{
  record_state &= record_mask;
  int oldstate = m_state.load(std::memory_order_relaxed);
  while (!m_state.compare_exchange_weak(oldstate,                                               // Atomically replace oldstate with
                                        (oldstate & ~record_mask) | record_state,               // this.
                                        std::memory_order_relaxed, std::memory_order_relaxed));
}

void FFTJackClient::set_playback_state(int playback_state)
{
  playback_state &= playback_mask;
  constexpr int mask = playback_mask | (playback_mask << prev_mask_shift);
  bool changed;
  int oldstate = m_state.load(std::memory_order_relaxed);
  while ((changed = playback_state != (oldstate & playback_mask)) &&                                                            // If the playback state changed then
         !m_state.compare_exchange_weak(oldstate,                                                                               // atomically replace oldstate with
                                        (oldstate & ~mask) | ((oldstate & playback_mask) << prev_mask_shift) | playback_state,  // this, where the old playback state is moved the previous playback state
                                                                                                                                // and the current playback state is replaced with playback_state.
                                        std::memory_order_relaxed, std::memory_order_relaxed));                                 // No synchronization with other memory is needed.
  if (!changed)
    return;

  // The output source changed, initialize crossfading.
  constexpr float PI = 3.141592653589793f;      // Real precision is 3.1415927(410..)
  m_crossfade_frame = 0;
  m_crossfade_nframes = m_sample_rate / 50;     // Suppress output in 20 ms.
  m_crossfade_frame_normalization = PI / m_crossfade_nframes;
}

//              none o
//                    \  RECORDING  .---------------.
// record_input .--o   o--------->--| Record buffer |---.
//              |                   `---------------'   |
// record_output|  .-o                                  |
//              |  `-------------<--------------------. |
//              |        .-------<--------------------|-+        playback_to_output
//              |        `-o playback_to_input        | `--------------------------o
//              |           \        .--------------. | direct/playback_to_input    \                .
//        in o--+--------o   o--->---|     Test     |-+-------------------------o    \ OUTPUT
//              |   direct    TEST   `--------------'                      muted      o----------o out
//              |                                              m_silence >------o
//              |                                                       passthrough
//              `---------------->-------------------------------------------------o
//
// The playback_mask bits detemine the state of the TEST and OUTPUT switches:
//
// (statebits & playback_mask)    TEST               OUTPUT
// ---------------------------------------------------------------------------
//                      muted  :  direct             muted
//         playback_to_output  :  direct             playback_to_output
//          playback_to_input  :  playback_to_input  direct/playback_to_input
//                     direct  :  direct             direct/playback_to_input
//                passthrough  :  direct             passthrough
//

int FFTJackClient::process(jack_default_audio_sample_t* in, jack_default_audio_sample_t* out, jack_nframes_t nframes)
{
  // Read the state bits.
  int statebits = get_state();
  int playback_state = statebits & playback_mask;
  bool const perform_test = is_direct_or_playback_to_input(statebits);  // Whether or not we need to perform the test.
  bool const crossfading = m_crossfade_frame != jack_nframes_t(-1);
  bool const test_is_buffered = perform_test && crossfading;            // The output of test must be written to the output and we're in the processes of crossfading.
  bool perform_recording = (statebits & record_input);                  // Start with the recording stage?

  static int last_statebits = 0;
  static bool last_crossfading = false;
  if (statebits != last_statebits || crossfading != last_crossfading)
  {
    Debug(if (!dc::notice.is_on()) dc::notice.on());
    assert(libcwd::channels::dc::notice.is_on());
  }
  else
  {
    assert(!libcwd::channels::dc::notice.is_on());
  }
  last_statebits = statebits;
  last_crossfading = crossfading;

  DoutEntering(dc::notice, "FFTJackClient::process(" << in << ", " << out << ", " << nframes << ")");
  assert(nframes == m_input_buffer_size);
  Dout(dc::notice, "statebits = " << (void*)(long)statebits);
  Dout(dc::notice, "playback_state = " << (void*)(long)playback_state << "; perform_test = " << perform_test << "; test_is_buffered = " << test_is_buffered << "; perform_recording = " << perform_recording << "; crossfading = " << crossfading);

  jack_default_audio_sample_t* test_out;                // The buffer that the test output is to be written to (either 'out' or a temporary buffer).

  // Perform the recording and test operation stages.
  // The order of those two is determined by the initial value of perform_recording.
  for (int stage = 0; stage < 2; ++stage)
  {
    if (perform_recording)
    {
      if ((statebits & record_mask))                    // Are we recording?
      {
        Dout(dc::notice, "RECORDING");
        // Write the appropriate data to the recording buffer.
        if (AI_UNLIKELY(!m_recording_buffer.push((statebits & record_input) ? in : test_out)))
        {
          Debug(if (!dc::notice.is_on()) dc::notice.on());
          Dout(dc::notice, "(RECORDING -->) BUFFER FULL!");
          // The recording buffer is full, stop recording.
          statebits &= ~record_mask;
          m_state.fetch_and(~record_mask);
        }
        else
          Dout(dc::notice, "Wrote " << ((statebits & record_input) ? in : test_out) << " to recording buffer.");
      }
    }
    else if (perform_test)                              // Are we testing?
    {
      Dout(dc::notice, "PERFORMING TEST");
      // Determine the test source.
      jack_default_audio_sample_t* test_in = (playback_state == playback_to_input) ? m_recording_buffer.pop() : in;
      assert(test_in);
      if (playback_state == playback_to_input)
        Dout(dc::notice, "Read " << test_in << " from recording buffer.");
      // Determine the test output.
      // Note that m_test_buffer cannot change at this point, after activation (otherwise we wouldn't be here)
      // buffer_size_changed() is guaranteed to only be called by the same thread as this one.
      test_out = test_is_buffered ? m_test_buffer.get() : out;
      // Perform test operation.
      for (jack_nframes_t frame = 0; frame < nframes; ++frame)
      {
        test_out[frame] = test_in[frame] * 0.1f;
      }
    }
    // Go to the other task.
    perform_recording = !perform_recording;
  }

  // If test isn't buffered then it was written to out and we're done.
  if (perform_test && !test_is_buffered)
  {
    Dout(dc::notice, "DONE!");
    Debug(if (dc::notice.is_on()) dc::notice.off());
    assert(!libcwd::channels::dc::notice.is_on());
    return 0;
  }

  jack_default_audio_sample_t* current_source;
  jack_default_audio_sample_t* prev_source = NULL;      // The default if we're not crossfading.
  jack_default_audio_sample_t** source = &current_source;

  for (int stage = 0; stage < 2; ++stage)
  {
    switch(playback_state)
    {
      case muted:
        *source = crossfading ? m_silence_buffer.get() : NULL; // No need for the silence buffer when we're not crossfading.
        break;
      case playback_to_output:
        Dout(dc::notice, "Playing back to output");
        *source = m_recording_buffer.pop();
        if (AI_UNLIKELY(!*source))
        {
          Debug(if (!dc::notice.is_on()) dc::notice.on());
          Dout(dc::notice, "(Playing back to output -->) BUFFER EMPTY!");
          // Recording buffer is empty, mute the output.
          *source = m_silence_buffer.get();
          m_state.fetch_and(~(playback_mask << ((source == &current_source) ? 0 : prev_mask_shift)));
        }
        else
          Dout(dc::notice, "Read " << *source << " from recording buffer.");
        break;
      case playback_to_input:
      case direct:
        *source = test_out;
        break;
      case passthrough:
        *source = in;
        break;
    }
    Dout(dc::notice, "Stage " << stage << ": source = " << *source);
    if (AI_LIKELY(!crossfading))        // Leave prev_source at NULL when we're not crossfading.
      break;
    // Next, determine the previous source.
    source = &prev_source;
    statebits >>= prev_mask_shift;
    playback_state = statebits & playback_mask;
  }

  // Write source(s) to the output.

  // If there is only one source then prev_source is NULL.
  if (AI_LIKELY(current_source && !prev_source))
  {
    Dout(dc::notice, "Copying " << current_source << " to out.");
    // Pass through.
    std::memcpy(out, current_source, sizeof(jack_default_audio_sample_t) * nframes);
  }
  else if (!current_source)
  {
    Dout(dc::notice, "Writing zeroes to out.");
    // Muted.
    std::memset(out, 0, sizeof(jack_default_audio_sample_t) * nframes);
  }
  else
  {
    Dout(dc::notice, "Crossfading from " << prev_source << " to " << current_source << "!");
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
      Debug(if (!dc::notice.is_on()) dc::notice.on());
      Dout(dc::notice, "(Crossfading from ... to ...! -->) Crossfading finished!");
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

  Debug(if (dc::notice.is_on()) dc::notice.off());
  assert(!libcwd::channels::dc::notice.is_on());
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
  m_silence_buffer.reset(new jack_default_audio_sample_t[m_input_buffer_size]);
  m_test_buffer.reset(new jack_default_audio_sample_t[m_input_buffer_size]);
  // Clear the silence buffer with zeroes.
  std::memset(&m_silence_buffer[0], 0, sizeof(jack_default_audio_sample_t) * m_input_buffer_size);

  m_recording_buffer.buffer_size_changed(m_input_buffer_size);
}
