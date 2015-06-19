/**
 * /file RecordingDeviceState.h
 * /brief The state bits of the recording device of the FFTJackClient.
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

#ifndef RECORDING_DEVICE_STATE_H
#define RECORDING_DEVICE_STATE_H

#include <atomic>
#include <memory>
#include <functional>

class RecordingDeviceState
{
  public:
    static constexpr int muted = 0x0;
    static constexpr int playback = 0x1;
    static constexpr int direct = 0x2;
    static constexpr int passthrough = 0x4;
    static constexpr int playback_mask = playback | direct | passthrough;

    static constexpr int record_output = 0x8;
    static constexpr int record_input = 0x10;
    static constexpr int record_mask = record_output | record_input;

    static constexpr int playback_repeat = 0x20;
    static constexpr int playback_to_input = 0x40;
    static constexpr int gui2jack_mask = playback_repeat | playback_to_input;

    static constexpr int clear_buffer = 0x80;
    static constexpr int playback_reset = 0x100;
    static constexpr int commands_mask = clear_buffer | playback_reset;

    static constexpr int current_mask = playback_mask | record_mask | gui2jack_mask | commands_mask;
    static constexpr int prev_mask_shift = 16;      // Larger or equal than the number of bits in current_mask but no larger than 16.

  protected:
    std::atomic<int> m_state;
    int m_last_state;
    std::function<void()> m_wakeup_gui;

  public:
    RecordingDeviceState(int initial_state) : m_state(initial_state), m_last_state(-1) { }
    virtual ~RecordingDeviceState() { }

    void connect(std::function<void()> const& wakeup_gui) { m_wakeup_gui = wakeup_gui; }
    int get_state() const { return m_state.load(std::memory_order_relaxed); }
    void clear_and_set(int clearbits, int setbits)
    {
      int oldstate = m_state.load(std::memory_order_relaxed);
      while (!m_state.compare_exchange_weak(oldstate,                                               // Atomically replace oldstate with
                                            (oldstate & ~clearbits) | setbits,                      // this.
                                            std::memory_order_relaxed, std::memory_order_relaxed));
    }
    void set_recording_state(int record_state) { clear_and_set(record_mask, record_state & record_mask); }
    void set_playback_state(int playback_state);

    // Returns true when statebits & record_mask is record_output or record_input.
    bool is_recording() { return get_state() & record_mask; }

    // Returns true when playing back from the recording buffer.
    bool is_playing() { return get_state() & playback; }

  protected:
    // Return true if we must play from the start when reaching the end of the playback buffer.
    static bool is_repeat(int statebits) { return statebits & playback_repeat; }

  private:
    virtual void output_source_changed() = 0;
};

#endif // RECORDING_DEVICE_STATE_H
