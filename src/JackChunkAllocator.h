/**
 * \file JackChunkAllocator.h
 * \brief Declaration of JackChunkAllocator.
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

#ifndef JACK_CHUNK_ALLOCATOR_H
#define JACK_CHUNK_ALLOCATOR_H

#include "utils/macros.h"
#include "utils/Singleton.h"
#include <cstddef>
#include <jack/jack.h>

class JackChunkAllocator : public Singleton<JackChunkAllocator>
{
    friend_Instance;

  private:
    struct chunk
    {
      union
      {
        jack_default_audio_sample_t data[16];   // Audio data - the real size being m_chunk_size, not 16.
        struct
        {
          chunk* next;                          // Points to next free chunk in the chain, or NULL (call find_free_chunk_after(this)).
        } meta;                                 // Only valid for free (unused) chunks.
      };
    };

    size_t m_chunk_size;        // Size of one chunk in samples. Must be a multiple of 16 (is likely a power of 2 anyway).
    size_t m_initial_size;      // Size of the first allocated block in bytes.
    size_t m_increment_size;    // Size of subsequent blocks in bytes.
    chunk* m_free_chunk;        // Pointer to the first chunk in the free list.
    chunk* m_begin;             // Start of last allocated block.
    void const* m_end;          // One past the end of last allocated block.
    chunk const* m_start;       // Start of first allocated block.

    static int s_increment_chunks;
    static int s_initial_chunks;

  private:
    // Return the next chunk in the block (or a pointer that points one past the end of it).
    chunk* increment(chunk* ptr) const { return reinterpret_cast<chunk*>(&ptr->data[m_chunk_size]); }
    chunk const* increment(chunk const* ptr) const { return reinterpret_cast<chunk const*>(&ptr->data[m_chunk_size]); }

    // Allocate a new block of size / (m_chunk_size * sizeof(jack_default_audio_sample_t) chunks and return a pointer to the first chunk.
    // size is either m_initial_size or m_increment_size (a multiple of m_chunk_size * sizeof(jack_default_audio_sample_t)).
    chunk* allocate_new_block(size_t size);

    // Initialize m_free_chunk.
    void find_free_chunk_after(chunk* last_chunk);

  private:
    JackChunkAllocator();
    ~JackChunkAllocator();

  public:
    // Return a pointer to fftwf_malloc aligned memory of m_chunk_size samples in O(1) time.
    // It can happen occassionally that no more blocks are available, in that case
    // a call to fftwf_malloc() happens which is slow -- the only way to avoid that
    // is to increase initial_chunks passed to the constructor.
    void* allocate()
    {
      chunk* current_chunk = m_free_chunk;
      m_free_chunk = current_chunk->meta.next;
      if (AI_UNLIKELY(!m_free_chunk))
        find_free_chunk_after(current_chunk);
      return current_chunk;
    }

    // Release a memory chunk so it can be returned again by allocate(), always in O(1) time.
    void release(void const* ptr)
    {
      chunk* free_chunk = reinterpret_cast<chunk*>(const_cast<void*>(ptr));
      free_chunk->meta.next = m_free_chunk;
      m_free_chunk = free_chunk;
    }

    // Return current chunk size.
    jack_nframes_t chunk_size() const { return m_chunk_size; }

    // Re-initialize this object for a new chuck size.
    // This invalidates ALL pointers previously returned by allocate()!
    // You may not even call release() on them anymore.
    void buffer_size_changed(jack_nframes_t nframes);
};

#endif // JACK_CHUNK_ALLOCATOR_H
