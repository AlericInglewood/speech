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
#include "Events.h"
#include "utils/macros.h"

FFTJackClient::FFTJackClient(char const* name, double period) : JackClient(name), RecordingDeviceState(passthrough),
  m_fft_buffer_size(0), m_playback_state(0), m_sequence_number(0),
  m_recorder(m_client, period),
  m_recording_switch(m_recorder), m_test_switch(m_fft_processor), m_output_switch(m_jack_server_input)
{
  // Set the size of the FFT buffer, in samples.
  set_fft_buffer_size(256);

  // Initialize the switches.
  sample_rate_changed(m_sample_rate);
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

    // Attempt to fill the input buffers that we have.
    event_type events = 0;
    try
    {
      // Fill recorder.
      if ((statebits & record_mask) || m_recording_switch.is_crossfading()) // (Still) recording?
      {
        events |= m_recorder.fill_input_buffer(m_sequence_number);
      }

      // Fill JACK server.
      events |= m_jack_server_input.fill_input_buffer(m_sequence_number);
    }
    catch (BrokenPipe const& error)
    {
      Dout(dc::notice, "FFTJackClient::process: caught BrokenPipe");
      // With the current position of the switches we cannot create the necessary output!
      events |= event_bit_try_again;    // If the pipe broke then the code below should change the routing,
      events |= error.mask();           // using these events, after which we need to try again.
    }

    if (AI_UNLIKELY(events))
    {
      // Stop recording/playback if needed.
      if ((events & event_bit_stop_playback))
      {
#if DEBUG_PROCESS
        Debug(if (!dc::notice.is_on()) dc::notice.on());
        Dout(dc::notice, "(Playing back to output -->) BUFFER EMPTY!");
#endif // DEBUG_PROCESS
        // Recording buffer is empty, mute the output.
        set_playback_state(statebits & (direct | passthrough));
      }
      if ((events & event_bit_stop_recording))
      {
#if DEBUG_PROCESS
        Debug(if (!dc::notice.is_on()) dc::notice.on());
        Dout(dc::notice, "(RECORDING -->) BUFFER FULL!");
#endif // DEBUG_PROCESS
        // The recording buffer is full, stop recording.
        statebits &= ~record_mask;
        set_recording_state(0);
      }
      if ((events & (event_bit_stop_playback | event_bit_stop_recording)))
        m_wakeup_gui();

      if ((events & event_bit_try_again))
      {
        // Routing should be changed now.
        ASSERT(get_state() != m_last_state);
        continue;                       // Retry filling the output buffer.
      }
    }
    break;
  }

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

int FFTJackClient::sample_rate_changed(jack_nframes_t sample_rate)
{
  m_recording_switch.sample_rate_changed(sample_rate);
  m_test_switch.sample_rate_changed(sample_rate);
  m_output_switch.sample_rate_changed(sample_rate);
  return 0;
}
