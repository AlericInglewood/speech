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
class JackSwitch : public JackProcessor
{
  protected:
    JackInput m_previous_input;

  public:
    JackSwitch() : m_previous_input(this) { }
    ~JackSwitch() { m_previous_input.disown(); }

    // Connect switch to input.
    friend void operator<<(JackInput& input, JackSwitch& jack_switch)
    { 
      input.connect_to(jack_switch.m_output);
    }

    // Read input, process, write output.
    /*virtual*/ void process(int sequence_number);
};

#endif // JACK_SWITCH_H
