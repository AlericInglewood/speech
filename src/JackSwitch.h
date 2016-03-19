/**
 * \file JackSwitch.h
 * \brief Declaration of JackSwitch.
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

#ifndef JACK_SWITCH_H
#define JACK_SWITCH_H

#include "CrossfadeProcessor.h"
#include <jack/jack.h>
#include "debug.h"

// When an input is not crossfading, it is connected to a single output:
//
// JackOutput1 <--- JackInput::m_connected_output
//
// While it is crossfading, it is connected to a CrossfadeProcessor,
// which is connected to one or more output sources.
//
//                 .--------------------.
// JackOutput1 <---| CrossfadeProcessor |<--- JackInput::m_connected_output
// JackOutput2 <---|                    |
//   .             |                    | 
//   .             |                    |
// JackOutputN <---|                    |
//                 `--------------------'
//
// As such we have at most one CrossfadeProcessor per JackInput that
// is connected to a JackSwitch. Therefore the CrossfadeProcessor is
// an element of the switch.
//
// Crossfade between two inputs.
class JackSwitch
{
  protected:
    JackInput& m_input;                         // A reference to the input that must be switched.
    CrossfadeProcessor m_crossfade_processor;   // The input of the crossfade processor is NOT connected/used!
                                                // The new output source (fade-in) as well as the previous output
                                                // source(s) (fade-out) are connected to JackInput member variables
                                                // in the array CrossfadeProcessor::m_sources.

  public:
    JackSwitch(JackInput& input) : m_input(input), m_crossfade_processor(*this) { }
    JackSwitch(JackProcessor& jack_processor) : m_input(jack_processor), m_crossfade_processor(*this) { }

  public:
    bool is_crossfading() const { return m_input.connected_output() == &m_crossfade_processor; }

    // Accessor.
    JackInput const& input() const { return m_input; }

    // Switch from current output source (m_input.connected_output() when not crossfading,
    // or m_crossfade_processor.current_source() when already crossfading) to new_source.
    void connect(JackOutput& new_source);

    // Called by m_crossfade_processor when the crossfading finished.
    void stop_crossfading(JackOutput* current_source);

    // Switch m_input to output.
    friend void operator<<(JackSwitch& jack_switch, JackOutput& output) { jack_switch.connect(output); }

    void disconnect() { m_input.disconnect(); }
    void sample_rate_changed(jack_nframes_t sample_rate) { m_crossfade_processor.sample_rate_changed(sample_rate); }
};

#endif // JACK_SWITCH_H
