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

class JackFIFOBuffer
{
  private:
    jack_nframes_t m_nframes;                           //!< Number of frames per jack buffer (one frame is one sample because this is mono).
    intptr_t m_capacity;                                //!< Total size of m_buffer in jack_default_audio_sample_t's (nframes * nchunks).
    jack_default_audio_sample_t* m_buffer;              //!< Buffer start.
    std::atomic<jack_default_audio_sample_t*> m_head;   //!< Write position in circular buffer.
    jack_default_audio_sample_t* m_readptr;             //!< Non-destructive read position in circular buffer.
    std::atomic<jack_default_audio_sample_t*> m_tail;   //!< Read position in circular buffer.

  private:
    jack_default_audio_sample_t* increment(jack_default_audio_sample_t* ptr) const { return ((ptr - m_buffer + m_nframes) % m_capacity) + m_buffer; }
    void reallocate_buffer(int nchunks, jack_nframes_t nframes);

  public:
    //! Construct a buffer for \a client with a duration of \a period seconds.
    JackFIFOBuffer(jack_client_t* client, double period);

    //! Construct a buffer of \a nchunks chunks, each of \a nframes frames.
    JackFIFOBuffer(int nchunks, jack_nframes_t nframes) : m_buffer(NULL) { reallocate_buffer(nchunks + 1, nframes); }

    //! Destructor.
    virtual ~JackFIFOBuffer();

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

    // Same as the above but writes zero's.
    bool push_zero()
    {
      auto const current_head = m_head.load(std::memory_order_relaxed);
      auto const next_head = increment(current_head);

      if (next_head != m_tail.load(std::memory_order_acquire))  // Otherwise the buffer would appear empty after the m_head.store below,
                                                                // and we'd be writing over data that possibly still needs to be read
                                                                // by the consumer (that was returned by pop()).
      {
        std::memset(current_head, 0, m_nframes * sizeof(jack_default_audio_sample_t));
        m_head.store(next_head, std::memory_order_release);
        return true;
      }

      return false; // Full queue.
    }

    //-------------------------------------------------------------------------
    // Consumer thread.

    //! Return m_tail and advance it, possibly also advancing m_readptr. Returns NULL if the buffer is empty.
    jack_default_audio_sample_t* pop()
    {
      auto const current_tail = m_tail.load(std::memory_order_relaxed);
      if (current_tail == m_head.load(std::memory_order_acquire))
        return NULL; // Empty queue.

      auto const next_tail = increment(current_tail);
      if (current_tail == m_readptr)
        m_readptr = next_tail;
      m_tail.store(next_tail, std::memory_order_release);
      return current_tail;
    }

    //! Return m_readptr and advance it. Returns NULL if the read pointer is at the end of the buffer.
    jack_default_audio_sample_t* read()
    {
      auto current_ptr = m_readptr;
      if (current_ptr == m_head.load(std::memory_order_acquire))
        return NULL; // At end.

      m_readptr = increment(current_ptr);
      return current_ptr;
    }

    //! Reset the read pointer to the beginning of the recorded data in the buffer.
    void reset_readptr()
    {
      m_readptr = m_tail.load(std::memory_order_relaxed);
    }

    //! Clear the buffer.
    //
    // There is no way to clear the buffer in a thread-safe way from the producer (or any other thread).
    // However, any thread may call this function to clear the buffer if it is known that
    // the consumer thread is not using the buffer at that moment.
    void clear()
    {
      auto const current_head = m_head.load(std::memory_order_relaxed);
      m_readptr = current_head;
      m_tail.store(current_head, std::memory_order_release);
    }

    //-------------------------------------------------------------------------

    // Return value    :  true                                false
    // Producer thread :  Is empty/at_end.                    Is, or was recently, non-empty.
    // Consumer thread :  Is, or was recently, empty/at_end.  Is non-empty.
    // Other threads   :             meaningless (don't call this)
    bool empty() const
    {
      auto const current_head = m_head.load(std::memory_order_relaxed);
      return current_head == m_tail.load(std::memory_order_relaxed);
    }

    // Return value    :  true                                false
    // Producer thread :  Is at end.                          Is, or was recently, not at end.
    // Consumer thread :  Is, or was recently, at end.        Is not at end.
    // Other threads   :             meaningless (don't call this)
    bool at_end() const
    {
      auto const current_head = m_head.load(std::memory_order_relaxed);
      return current_head == m_readptr;
    }

    // Return value    :  true                                false
    // Producer thread :  Is, or was recently, full.          Is not full.
    // Consumer thread :  Is full.                            Is, or was recently, not full.
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

    // Accessor.
    jack_nframes_t nframes() const { return m_nframes; }

  private:
    // Disallow copying.
    JackFIFOBuffer(JackFIFOBuffer const&);
};

#endif // JACK_FIFO_BUFFER_H
