/**
 * /file RecordingDeviceState.cpp
 * /brief Implementation of class RecordingDeviceState.
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

#include "debug.h"
#include "RecordingDeviceState.h"

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
//              |   direct    TEST   `--------------'                passthrough      o----------o out
//              `---------------->----------------------------------------------o
//                                                                            muted
//                                                                            none o
//
// The playback_mask bits determine the state of the TEST and OUTPUT switches:
//
// (statebits & playback_mask)    TEST               OUTPUT
// ---------------------------------------------------------------------------
//                      muted  :  direct             muted
//         playback_to_output  :  direct             playback_to_output
//          playback_to_input  :  playback_to_input  direct/playback_to_input
//                     direct  :  direct             direct/playback_to_input
//                passthrough  :  direct             passthrough
//

void RecordingDeviceState::set_playback_state(int playback_state)
{
  playback_state &= playback_mask;
  constexpr int mask = playback_mask | (playback_mask << prev_mask_shift);
  int oldstate = m_state.load(std::memory_order_relaxed);
  while (playback_state != (oldstate & playback_mask) &&                                                                        // If the playback state changed then
         !m_state.compare_exchange_weak(oldstate,                                                                               // atomically replace oldstate with
                                        (oldstate & ~mask) | ((oldstate & playback_mask) << prev_mask_shift) | playback_state,  // this, where the old playback state is moved the previous playback state
                                                                                                                                // and the current playback state is replaced with playback_state.
                                        std::memory_order_relaxed, std::memory_order_relaxed));                                 // No synchronization with other memory is needed.
}
