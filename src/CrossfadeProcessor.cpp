/**
 * /file CrossfadeProcessor.cpp
 * /brief Implementation of class CrossfadeProcessor.
 *
 * Copyright (C) 2016 Aleric Inglewood.
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

#include "JackSwitch.h"
#include "JackRecorder.h"
#include "Events.h"
#include "utils/macros.h"
#include <cmath>

CrossfadeProcessor::CrossfadeProcessor(JackSwitch& owner) :
    JackProcessor(DEBUG_ONLY(owner.input().m_name + " \e[48;5;14mCrossfadeProcessor\e[0m")),
    m_switch(owner), m_active_inputs(0), m_sample_rate(0)
{
}

void CrossfadeProcessor::sample_rate_changed(jack_nframes_t sample_rate)
{
  m_sample_rate = sample_rate;
  m_crossfade_nframes = m_sample_rate / 50 * 100;     // Suppress output in 20 ms.
  m_crossfade_frame_normalization = 1.0f / m_crossfade_nframes;

  Dout(dc::notice, "m_crossfade_nframes set to " << m_crossfade_nframes);
  // sample_rate_changed() should have been called before this point.
  ASSERT(m_crossfade_nframes != 0);
}

void CrossfadeProcessor::begin(JackOutput& new_source, JackOutput* prev_source)
{
  Dout(dc::notice, "Entering CrossfadeProcessor::begin([" << new_source.m_name << "], [" << (prev_source ? prev_source->m_name : "<null>") << "])");
  Debug(libcw_do.push_marker());
  Debug(libcw_do.marker().assign(": | "));

  ASSERT(m_active_inputs == 0);
  // Add the new source.
  ASSERT(!current_source());
  m_sources[0].m_crossfade_frame = 0;
  m_sources[0].m_direction = 1;
  m_active_inputs = 1;
  new_source.connect(m_sources[0]);
  // Add the previous source, if any.
  if (prev_source)
  {
    m_sources[1].m_crossfade_frame = m_crossfade_nframes;
    m_sources[1].m_direction = -1;
    ++m_active_inputs;
    prev_source->connect(m_sources[1]);
  }

  Debug(libcw_do.marker().assign(": ` "));
  Dout(dc::notice, "Leaving CrossfadeProcessor::begin.");
  Debug(libcw_do.pop_marker());
}

void CrossfadeProcessor::add(JackOutput& new_source)
{
  Dout(dc::notice, "Entering CrossfadeProcessor::add([" << new_source.m_name << "])");
  Debug(libcw_do.push_marker());
  Debug(libcw_do.marker().assign(": | "));

  int index = -1;                                               // new_source wasn't found yet and no place to insert it was determined yet.
  jack_nframes_t volume = m_crossfade_nframes + 1;              // Larger than any real volume.
  for (int i = s_max_sources - 1; i >= 0; --i)
  {
    int direction = m_sources[i].m_direction;
    // Set the current input that is fade-in to fade-out.
    if (direction == 1 || m_sources[i].m_crossfade_frame > 0)
    {
      m_active_inputs += 1 - std::abs(direction);
      direction = m_sources[i].m_direction = -1;                // Because we do this first, direction == 0 now means that the input is unused.
    }
    if (index >= -1)     // new_source wasn't found (yet)?
    {
      // Look for a place to insert new_source.
      if (direction == 0 || m_sources[i].m_crossfade_frame < volume)
      {
        index = i;                                              // Remember where we found it.
        volume = m_sources[i].m_crossfade_frame;                // Possibly overwrite an input that has the lowest volume.
        ASSERT(direction != 0 || volume == 0);                  // If direction == 0 then volume must be zero to prevent us from overwriting index because we find a lower volume.
      }
      // Check if new_source is already connected.
      if (m_sources[i].connected_output() == &new_source)
      {
        index = -2;                                             // Mark that we found new_source to be already connected.
        int const not_max_volume = m_sources[i].m_crossfade_frame < m_crossfade_nframes ? 1 : 0;
        m_sources[i].m_direction = not_max_volume;              // Start fading it in, if not already at the maximum volume.
        m_active_inputs += not_max_volume - std::abs(direction);
      }
    }
  }
  // The first pass through the loop already sets index.
  ASSERT(index != -1);
  // Connect the new source when it was really new.
  if (index >= 0)
  {
    ASSERT(!current_source());                                  // All sources should be fading down now.
    m_sources[index].m_crossfade_frame = 0;
    m_active_inputs += 1 - std::abs(m_sources[index].m_direction);
    m_sources[index].m_direction = 1;
    new_source.connect(m_sources[index]);                       // If m_sources[index] is already in use then this will disconnect it first.
  }

  Debug(libcw_do.marker().assign(": ` "));
  Dout(dc::notice, "Leaving CrossfadeProcessor::add.");
  Debug(libcw_do.pop_marker());
}

void CrossfadeProcessor::stop_crossfading()
{
  Dout(dc::notice, "Entering CrossfadeProcessor::stop_crossfading()");
  Debug(libcw_do.push_marker());
  Debug(libcw_do.marker().assign(": | "));
  JackOutput* current_source = NULL;
  for (int i = 0; i < s_max_sources; ++i)
  {
    if (m_sources[i].m_crossfade_frame > 0)
    {
      ASSERT(m_sources[i].m_direction == 0 && m_sources[i].m_crossfade_frame == m_crossfade_nframes);
      current_source = m_sources[i].connected_output();
      m_sources[i].disconnect();
      m_sources[i].m_crossfade_frame = 0;
      break;
    }
  }
  m_switch.stop_crossfading(current_source);
  Debug(libcw_do.marker().assign(": ` "));
  Dout(dc::notice, "Leaving CrossfadeProcessor::stop_crossfading.");
  Debug(libcw_do.pop_marker());
}

event_type CrossfadeProcessor::fill_output_buffer(int sequence_number)
{
  if (m_sequence_number == sequence_number)
    return 0;
  m_sequence_number = sequence_number;

  event_type events = 0;

  // We shouldn't get here when there aren't any active inputs remaining.
  ASSERT(m_active_inputs > 0);

  // Try to fill the input buffer of the active inputs.
  for (int i = 0; i < s_max_sources; ++i)
  {
    int const direction = m_sources[i].m_direction;
    if (direction == 0 && m_sources[i].m_crossfade_frame == 0)
      continue;
    try
    {
      events |= m_sources[i].fill_input_buffer(sequence_number);
    }
    catch (BrokenPipe const& error)
    {
      Dout(dc::notice, "CrossfadeProcessor::fill_output_buffer: caught BrokenPipe for input \"" << m_sources[i].m_name << "\".");
      // Disconnect the failed input and mark it as unused.
      m_sources[i].disconnect();
      m_active_inputs -= std::abs(direction);
      m_sources[i].m_direction = 0;
      m_sources[i].m_crossfade_frame = 0;
      // An input failed; if this was the last input then stop.
      if (m_active_inputs == 0)
      {
        Dout(dc::notice, "No active inputs left: abort crossfading and rethrow.");
        stop_crossfading();
        throw;
      }
      events |= error.mask();
    }
  }

  generate_output();
  events |= handle_memcpys();

  return events;
}

void CrossfadeProcessor::generate_output()
{
  jack_default_audio_sample_t* out = JackOutput::chunk_ptr();
  jack_nframes_t nframes = JackOutput::nframes();

#if DEBUG_PROCESS
#ifdef CWDEBUG
  JackOutput* new_source = NULL;
  Dout(dc::notice|continued_cf, "\e[48;5;5mCrossfading from\e[0m ");
  int count = 0;
  for (int i = 0; i < s_max_sources; ++i)
  {
    if (m_sources[i].m_direction == 0 && m_sources[i].m_crossfade_frame == 0)
    {
      // An input that has m_direction and m_crossfade_frame set to zero is disconnected.
      ASSERT(!m_sources[i].connected_output());
    }
    else if (m_sources[i].m_direction == -1)
    {
      JackOutput* prev_source = m_sources[i].connected_output();
      // An input that is being faded down must be connected.
      ASSERT(prev_source);
      Dout(dc::continued, "[" << prev_source->m_name  << "] ");
      ++count;
    }
    else
    {
      ASSERT(m_sources[i].m_direction == 1 || (m_sources[i].m_direction == 0 && m_sources[i].m_crossfade_frame == m_crossfade_nframes));
      // Only one source can be faded up at a time.
      ASSERT(!new_source);
      new_source = m_sources[i].connected_output();
      // An input that is being faded up must be connected.
      ASSERT(new_source);
    }
  }
  if (count == 0)
    Dout(dc::continued, "NULL ");
  Dout(dc::finish, "to [" << (new_source ? new_source->m_name : "<silence>") << "] ==> [" << m_connected_inputs[0].first->m_name << "]");
  debug::Indent debug_indent(2);
  bool debug_on = LIBCWD_DEBUGCHANNELS::dc::notice.is_on();
  if (!debug_on) LIBCWD_DEBUGCHANNELS::dc::notice.on();
#endif
#endif // DEBUG_PROCESS

  // Crossfade. Written slightly hackish because here is where the CPU counts.
  jack_nframes_t end[3] = { m_crossfade_nframes, 0, 0 };

  // For each frame;
  jack_nframes_t frame = 0;
  while (frame < nframes)
  {
    // Calculate the output sample from,
    jack_default_audio_sample_t sample = 0;
    // all input sources...
    CrossfadeInput* sourcep = m_sources;
    for (int i = 0; i < s_max_sources; ++i, ++sourcep)
    {
      // Cache m_sources[i].
      jack_nframes_t crossfade_frame = sourcep->m_crossfade_frame;
      int const direction = sourcep->m_direction;

      // ...that are relevant.
      if (crossfade_frame == 0 && direction == 0)
        continue;

      sample += sourcep->chunk_ptr()[frame] * crossfade_frame;
      crossfade_frame += direction;

      // Note that for a fully faded-in current input, direction == 0 and m_crossfade_frame == m_crossfade_nframes
      // so that the following boolean expression will be false because end[1] is 0 != m_crossfade_nframes.
      if (AI_UNLIKELY(crossfade_frame == end[1 - direction]))
      {
        ASSERT(direction != 0);
        m_sources[i].m_direction = 0;
        --m_active_inputs;
        if (direction == -1)
          m_sources[i].disconnect();
        Dout(dc::notice, m_active_inputs << " active inputs left.");
      }

      // Store value back.
      m_sources[i].m_crossfade_frame = crossfade_frame;
    }
    // Store the result.
    out[frame] = sample * m_crossfade_frame_normalization;
    ++frame;
  }

  if (m_active_inputs == 0)  // Did the crossfading finish?
  {
#if DEBUG_PROCESS
    Dout(dc::notice, "Crossfading finished!");
#endif // DEBUG_PROCES
    stop_crossfading();
  }
#if DEBUG_PROCESS
#ifdef CWDEBUG
  if (!debug_on) LIBCWD_DEBUGCHANNELS::dc::notice.off();
#endif
#endif
}
