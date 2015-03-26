/**
 * /file FFTJackClientStates.h
 * /brief The states of FFTJackClient.
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

#ifndef FFT_JACK_CLIENT_STATES_H
#define FFT_JACK_CLIENT_STATES_H

namespace FFTJackClientStates
{

constexpr int muted = 0x0;
constexpr int playback_to_output = 0x1;
constexpr int playback_to_input = 0x2;
constexpr int direct = 0x3;
constexpr int passthrough = 0x4;
constexpr int playback_mask = 0x7;

constexpr int none = 0x0;
constexpr int record_output = 0x8;
constexpr int record_input = 0x10;
constexpr int record_mask = 0x18;

constexpr int current_mask = 0x1f;
constexpr int prev_mask_shift = 8;      // Larger or equal the number of bits in current_mask.

// This makes use of the fact that 'x & playback_to_input' is
// only true for x = direct and x = playback_to_input.
inline bool is_direct_or_playback_to_input(int statebits) { return statebits & playback_to_input; }

} // namespace FFTJackClientStates

#endif // FFT_JACK_CLIENT_STATES_H
