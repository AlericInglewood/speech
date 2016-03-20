/**
 * \file CrossfadeProcessor.h
 * \brief Declaration of CrossfadeProcessor.
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

#ifndef CROSSFADE_PROCESSOR_H
#define CROSSFADE_PROCESSOR_H

#include "JackProcessor.h"
#include "JackSilenceOutput.h"

class JackSwitch;
class CrossfadeProcessor;

class CrossfadeInput : public JackInput
{
  private:
    friend class CrossfadeProcessor;
    jack_nframes_t m_crossfade_frame;                   // Volume fraction of previous source (m_crossfade_frame / m_crossfade_nframes), runs from m_crossfade_nframes to 0.
    int m_direction;                                    // Plus or minus one for respectively fading in or out (zero when we reached the limit or when the input is disconnected).

  public:
    // Construct a JackInput that represents the input of CrossfadeProcessor.
    CrossfadeInput() : JackInput(DEBUG_ONLY("\e[48;5;14mCrossfadeProcessor\e[0m")), m_crossfade_frame(0), m_direction(0) { }
};

class CrossfadeProcessor : public JackProcessor
{
  private:
    static int const s_max_sources = 4;                 // Maximum number of simultaneous inputs.
    JackSwitch& m_switch;                               // Reference to the switch that owns this crossfader.
    CrossfadeInput m_sources[s_max_sources];            // Array with old inputs that we crossfade away from.
    int m_active_inputs;                                // The number of inputs that are fading up or down (still changing volume).
                                                        // Should be equal at all times with the sum of the absolute values of m_direction of all s_max_sources inputs in m_sources.
    jack_nframes_t m_sample_rate;                       // Copy of the sample rate.
    jack_nframes_t m_crossfade_nframes;                 // Number of frames used to crossfade between 0% and 100%.
    jack_default_audio_sample_t m_crossfade_frame_normalization;        // Precalculated normalization factor.

  private:
    void stop_crossfading();

  public:
    CrossfadeProcessor(JackSwitch& owner);

    void sample_rate_changed(jack_nframes_t sample_rate);

    // Accessor.
    int active_inputs() const { return m_active_inputs; }

    JackOutput* current_source()
    {
      for (int i = 0; i < s_max_sources; ++i)
        if (m_sources[i].m_direction == 1 ||
            (m_sources[i].m_direction == 0 && m_sources[i].m_crossfade_frame == m_crossfade_nframes))
          return m_sources[i].connected_output();
      return NULL;
    }

    void begin(JackOutput& new_source, JackOutput* prev_source);
    void add(JackOutput& new_source);

    // Read input, process, write output.
    /*virtual*/ event_type fill_output_buffer(int sequence_number);
    /*virtual*/ void generate_output();
};

#endif // CROSSFADE_PROCESSOR_H
