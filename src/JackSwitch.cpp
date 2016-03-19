/**
 * /file JackSwitch.cpp
 * /brief Implementation of class JackSwitch.
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

#include "JackSwitch.h"
#include "utils/macros.h"
#include "debug.h"

void JackSwitch::stop_crossfading(JackOutput* current_source)
{
  ASSERT(is_crossfading());                                     // Don't call this when crossfading already stopped.
  ASSERT(m_crossfade_processor.active_inputs() == 0 &&          // m_crossfade_processor should already be entirely reset.
         !m_crossfade_processor.current_source());
  if (current_source)
    m_input.connect(*current_source);
  else
    m_input.disconnect();
}

void JackSwitch::connect(JackOutput& new_source)
{
  bool const crossfading = is_crossfading();
  JackOutput* prev_source = crossfading ? m_crossfade_processor.current_source() : m_input.connected_output();
  if (prev_source == &new_source)
    return;
  DoutEntering(dc::notice, "JackSwitch::connect(): [" << new_source.m_name << "] o<---o [" << m_input.m_name << " Switch]");
  Dout(dc::notice, "prev_source = " << (prev_source ? prev_source->m_name : "NULL"));
  if (crossfading)
  {
    // We're switching to a new output source while already crossfading!
    m_crossfade_processor.add(new_source);
  }
  else
  {
    m_input << m_crossfade_processor;
    m_crossfade_processor.begin(new_source, prev_source);
  }
}
