/**
 * /file JackSilenceOutput.cpp
 * /brief Implementation of class JackSilenceOutput.
 *
 * Copyright (C) 2015, 2016 Aleric Inglewood.
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
#include "JackSilenceOutput.h"
#include "JackChunkAllocator.h"
#include "JackInput.h"

JackSilenceOutput::JackSilenceOutput() : JackOutput(DEBUG_ONLY("Silence"))
{
  // At this point buffer_size_changed() wasn't called yet, which means
  // that JackChunkAllocator doesn't know the chunk size and we can't
  // allocate anything yet.
}

JackSilenceOutput::~JackSilenceOutput()
{
  if (m_chunk)
    JackChunkAllocator::instance().release(m_chunk);
}

void JackSilenceOutput::buffer_size_changed(jack_nframes_t nframes)
{
  // m_empty_chunk was already invalidated by calling JackChunkAllocator::buffer_size_changed.
  ASSERT(JackChunkAllocator::instance().chunk_size() == nframes);
  m_chunk = static_cast<jack_default_audio_sample_t*>(JackChunkAllocator::instance().allocate());
  m_chunk_size = nframes;
}

event_type JackSilenceOutput::fill_output_buffer(int sequence_number)
{
  if (m_sequence_number == sequence_number)
    return 0;
  m_sequence_number = sequence_number;

  // api_output_provided_buffer requires we set m_chunk and m_chunk_size in fill_output_buffer(),
  // but those already set by the call to buffer_size_changed() when we get here.

  event_type events = 0;

  for (auto input : m_connected_inputs)
  {
    if (!has_zero_input(input.second))
      break;
    events |= input.first->zero_input();
  }

  return events;
}
