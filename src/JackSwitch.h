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

#include "JackProcessor.h"

#include <jack/jack.h>
#include "debug.h"

// Crossfade between two inputs.
class JackSwitch
{
  protected:
    JackInput& m_input;         // A reference to the input that must be switched.

  public:
    JackSwitch(JackInput& input) : m_input(input) { }
    JackSwitch(JackProcessor& jack_processor) : m_input(jack_processor) { }

  public:
    bool is_crossfading() const { return false; }       // FIXME

    // Switch input to output of jack_processor.
    friend void operator<<(JackSwitch& jack_switch, JackProcessor& jack_processor)
    {
      jack_switch.m_input << jack_processor;
    }

    // Switch input to output.
    friend void operator<<(JackSwitch& jack_switch, JackOutput& output)
    {
      jack_switch.m_input << output;
    }

    void disconnect() { m_input.disconnect(); }
};

#endif // JACK_SWITCH_H
