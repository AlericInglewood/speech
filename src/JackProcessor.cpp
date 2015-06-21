/**
 * /file JackProcessor.cpp
 * /brief Implementation of class JackProcessor.
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
#include "JackProcessor.h"
#include "JackChunkAllocator.h"
#include "RecorderJackProcessor.h"

JackProcessor** connected_owner_ptr;

void JackInput::connect_to(JackOutput& output)
{
#if DEBUG_PROCESS
  DoutEntering(dc::notice, "JackInput::connect_to(" << (void*)&output << ") with this = " << (void*)this);
#endif

  // I don't think that this needs to be supported. We add buffers on outputs except on the terminal JackProcessor.
  ASSERT(!provides_buffer() || !output.provides_buffer());

  if (!provides_buffer() && !output.provides_buffer())
  {
    output.create_buffer();
  }

  if (provides_buffer())
  {
    output.connect_to(this);
    if (!m_owner)
    {
      connected_owner_ptr = &m_connected_owner;
      m_connected_owner = output.owner();
      // Everthing but jack server input and output is a JackProcess,
      // and we never connect the jack server input and output directly.
      // This must be a jack server input being connected to a JackProcess.
      ASSERT(m_connected_owner);
    }
  }
  else
  {
    m_connected_output = &output;
  }
}

void JackInput::fill(int sequence_number)
{
#if DEBUG_PROCESS
  DoutEntering(dc::notice, "JackInput::fill(" << sequence_number << ") with this = " << (void*)this);
#endif

  // We are going to write to this buffer.
  ASSERT(provides_buffer());

  ASSERT(m_connected_owner || !m_owner);

  if (m_connected_owner)
    m_connected_owner->generate_output(sequence_number);
}

void JackOutput::create_buffer()
{
  // All outputs are owned by JackProcess except the jack server output which already has a buffer.
  ASSERT(m_owner);

  m_out = static_cast<jack_default_audio_sample_t*>(JackChunkAllocator::instance().allocate());
  m_chunk_size = JackChunkAllocator::instance().chunk_size();
}

void JackOutput::connect_to(JackInput* input)
{
  m_connected_input = input;
}
