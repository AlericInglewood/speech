/**
 * /file JackChunkAllocator.cpp.
 * /brief Implementation of class JackChunkAllocator.
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

#include "JackChunkAllocator.h"
#include "fftw3.h"
#include "debug.h"

//                                                         .--- m_start + m_initial_size
//                                                         V
// m_start --> [next ...][ ... ][ ... ][ ... ][next][ ... ]     (first allocated block)
//               |                            ^ |
//             .-' m_free_chunk-.  .----------' `------------.
//             v                v  |                         |
//             [next ...][ ... ][next][ ... ][ ... ][ ... ]  |
//               |                                         ^_|__ next + m_increment_size
//             .-'                                   .-------'
//             V                                     V
// m_begin --> [next ...][ ... ][ ... ][ ... ][ ... ][next]     (last allocated block)
//               |                                     |   ^
//               v                                     v   `--- m_end
//              NULL                                  NULL
// or
//                                     V
// m_begin --> [next ...][ ... ][ ... ][next ][unused[unused]   (last allocated block)
//               |                       |                   ^
//               v                       v                   `--- m_end
//              NULL                    NULL

// Allocate a new block (size is a multiple of m_chunk_size * sizeof(jack_default_audio_sample_t)).
// Begin and end of this block are stored in m_begin and m_end respectively.
inline JackChunkAllocator::chunk* JackChunkAllocator::allocate_new_block(size_t size)
{
  m_begin = reinterpret_cast<chunk*>(fftwf_malloc(size));
  m_end = &m_begin->data[0] + size;
  return m_begin;
}

JackChunkAllocator::JackChunkAllocator(jack_nframes_t nframes, int increment_chunks, int initial_chunks) :
  m_chunk_size(nframes),
  m_initial_size(nframes * sizeof(jack_default_audio_sample_t) * initial_chunks),
  m_increment_size(nframes * sizeof(jack_default_audio_sample_t) * increment_chunks),
  m_begin(allocate_new_block(m_initial_size)),
  m_start(m_begin)
{
  ASSERT(increment_chunks > 1 && initial_chunks > 1);                           // Otherwise increment(m_begin) would already be outside the block
                                                                                // (seriously though, these two sizes should be MUCH larger than 1).
  ASSERT(nframes * sizeof(jack_default_audio_sample_t) >= sizeof(chunk));       // struct chunk must fit inside it.
  ASSERT((nframes * sizeof(jack_default_audio_sample_t)) % 16 == 0);            // Must be a multiple of 16.
  m_begin->meta.next = NULL;                            // No next block yet.
  find_free_chunk_after(m_begin);
}

JackChunkAllocator::~JackChunkAllocator()
{
  // Free all blocks.
  chunk const* block = m_start;
  do
  {
    chunk const* ptr = block;
    block = block->meta.next;
    fftwf_free(const_cast<chunk*>(ptr));
  }
  while(block);
}

// This function set m_free_chunk to point to the next, never used before, chunk after last_chunk.
void JackChunkAllocator::find_free_chunk_after(chunk* last_chunk)
{
  ASSERT(last_chunk->meta.next == NULL);
  // A value of NULL in `next' means the next chunk in the block (this is so that we can
  // avoid having to initialize all of the next values at once).
  m_free_chunk = increment(last_chunk);
  // However, if this was the last chunk in the allocated block then we need to allocate more memory.
  if (AI_UNLIKELY(m_free_chunk == m_end))
  {
    // Store reference to the next pointer in m_begin because the call to allocate_new_block changes the value of m_begin.
    chunk*& next_block = m_begin->meta.next;
    assert(next_block == NULL);
    next_block = allocate_new_block(m_increment_size);  // Updates m_begin and m_end for a newly allocated block and return m_begin.
    ASSERT(next_block == m_begin);
    m_begin->meta.next = NULL;            // No next block yet.
    m_free_chunk = increment(m_begin);
  }
  // Initialize the new chunk.
  m_free_chunk->meta.next = NULL;
}
