/**
 * /file JackFIFOBuffer.h
 * /brief A Jack FIFO buffer.
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

#ifndef JACK_FIFO_BUFFER_H
#define JACK_FIFO_BUFFER_H

#include <jack/jack.h>
#include <atomic>
#include <cstdint>
#include <cstring>

class JackFIFOBuffer {
  private:
    jack_nframes_t m_nframes;                           //!< Number of frames per jack buffer (one frame is one sample because this is mono).
    intptr_t m_capacity;                                        //!< Total size of m_buffer in jack_default_audio_sample_t's (nframes * nchunks).
    jack_default_audio_sample_t* m_buffer;              //!< Buffer start.
    std::atomic<jack_default_audio_sample_t*> m_head;   //!< Write position in circular buffer.
    std::atomic<jack_default_audio_sample_t*> m_tail;   //!< Read position in circular buffer.

  private:
    jack_default_audio_sample_t* increment(jack_default_audio_sample_t* ptr) const { return ((ptr - m_buffer + m_nframes) % m_capacity) + m_buffer; }

  public:
    //! Construct a buffer for \a client with a duration of \a period seconds.
    JackFIFOBuffer(jack_client_t* client, double period);

    //! Construct a buffer of \a nchunks chunks, each of \a nframes frames.
    JackFIFOBuffer(jack_nframes_t nframes, int nchunks) : m_nframes(nframes), m_capacity(nframes * (nchunks + 1)), m_buffer(new jack_default_audio_sample_t[m_capacity]), m_head(m_buffer), m_tail(m_buffer) { }

    //! Destructor.
    virtual ~JackFIFOBuffer() { delete [] m_buffer; }

    //-------------------------------------------------------------------------
    // Producer thread.

    //! Copy m_nframes frames from \a in to m_head and advance head. Returns true if the operation succeeded, false if the buffer is full.
    bool push(jack_default_audio_sample_t const* in)
    {
      auto const current_head = m_head.load(std::memory_order_relaxed);
      auto const next_head = increment(current_head);

      if (next_head != m_tail.load(std::memory_order_acquire))  // Otherwise the buffer would appear empty after the m_head.store below,
                                                                // and we'd be writing over data that possibly still needs to be read
                                                                // by the consumer (that was returned by pop()).
      {
        std::memcpy(current_head, in, m_nframes * sizeof(jack_default_audio_sample_t));
        m_head.store(next_head, std::memory_order_release);
        return true;
      }

      return false; // Full queue.
    }

    //-------------------------------------------------------------------------
    // Consumer thread.

    //! Return m_tail and advance it. Returns NULL if the buffer is empty.
    jack_default_audio_sample_t* pop()
    {
      auto const current_tail = m_tail.load(std::memory_order_relaxed);
      if (current_tail == m_head.load(std::memory_order_acquire))
        return NULL; // Empty queue.

      m_tail.store(increment(current_tail), std::memory_order_release);
      return current_tail;
    }

    //! Clear the buffer.
    //
    // There is no way to clear the buffer in a thread-safe way from the producer (or any other thread).
    // However, any thread may call this function to clear the buffer if it is known that
    // the consumer thread is not using the buffer at that moment.
    void clear()
    {
      auto const current_head = m_head.load(std::memory_order_relaxed);
      m_tail.store(current_head, std::memory_order_release);
    }

    //-------------------------------------------------------------------------

    // Return value    :  true                          false
    // Producer thread :  Is empty.                             Is, or was recently, non-empty.
    // Consumer thread :  Is, or was recently, empty.   Is non-empty.
    // Other threads   :             meaningless (don't call this)
    bool empty() const
    {
      auto const current_head = m_head.load(std::memory_order_relaxed);
      return current_head == m_tail.load(std::memory_order_relaxed);
    }

    // Return value    :  true                          false
    // Producer thread :  Is, or was recently, full.    Is not full.
    // Consumer thread :  Is full.                              Is, or was recently, not full.
    // Other threads   :             meaningless (don't call this)
    bool full() const
    {
      auto const current_tail = m_tail.load(std::memory_order_relaxed);
      auto const next_head = increment(m_head.load(std::memory_order_relaxed));
      return next_head == current_tail;
    }

    bool is_lock_free() const { return m_head.is_lock_free() && m_tail.is_lock_free(); }

  public:
    virtual void buffer_size_changed(jack_nframes_t nframes);

  private:
    // Disallow copying.
    JackFIFOBuffer(JackFIFOBuffer const&);
};

#endif // JACK_FIFO_BUFFER_H
