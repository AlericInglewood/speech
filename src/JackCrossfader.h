/**
 * \file JackCrossfader.h
 * \brief Declaration of JackCrossfader.
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

#ifndef JACK_CROSSFADER_H
#define JACK_CROSSFADER_H

#include "JackProcessor.h"

#include <jack/jack.h>
#include "debug.h"

// Crossfade between two inputs.
class JackCrossfader : public JackProcessor
{
  protected:
    JackInput m_previous_input;

  public:
    JackCrossfader() : m_previous_input(this) { }
    ~JackCrossfader() { m_previous_input.disown(); }

    // Connect crossfader to input.
    friend void operator<<(JackInput& input, JackCrossfader& crossfader)
    { 
      input.connect_to(crossfader.m_output);
    }

    // Read input, process, write output.
    virtual void process() = 0;
};

#endif // JACK_CROSSFADER_H
