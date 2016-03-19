/**
 * \file Events.h
 * \brief Declaration of BrokenPipe, event_type and constants.
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

#ifndef EVENTS_H
#define EVENTS_H

#include <exception>

typedef int event_type;

event_type const event_bit_try_again = 0x1;
event_type const event_bit_stop_playback = 0x2;
event_type const event_bit_stop_recording = 0x4;

class BrokenPipe : public std::exception
{
  private:
    event_type m_event_mask;

  public:
    BrokenPipe(event_type mask) : m_event_mask(mask) { }

    event_type mask() const { return m_event_mask; }
};

#endif // EVENTS_H
