/**
 * /file JackOutput.cpp
 * /brief Implementation of class JackOutput.
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
#include "JackOutput.h"
#include "JackInput.h"
#include "JackChunkAllocator.h"

void JackOutput::create_allocated_buffer()
{
  DoutEntering(dc::notice, "JackOutput::create_allocated_buffer() with this = " << (void*)this << " [" << m_name << "].");

  // We should not try to allocate a buffer when we're already providing one.
  ASSERT(!has_provided_output_buffer(type()));
  // Makes no sense to allocate a buffer when we already have one.
  ASSERT(!m_allocated);

  m_chunk = static_cast<jack_default_audio_sample_t*>(JackChunkAllocator::instance().allocate());
  m_chunk_size = JackChunkAllocator::instance().chunk_size();
  m_allocated = true;
}

void JackOutput::release_allocated_buffer(void)
{
  DoutEntering(dc::notice, "JackOutput::release_allocated_buffer() with this = " << (void*)this << " [" << m_name << "].");

  if (m_allocated)
  {
    ASSERT(m_chunk);
    JackChunkAllocator::instance().release(m_chunk);
    m_chunk = NULL;
    m_chunk_size = 0;
    m_allocated = false;
  }
}

void JackOutput::connect(JackInput& input)
{
  DoutEntering(dc::notice, "JackOutput::connect([" << input.m_name << "]) with this = " << (void*)this << " [" << m_name << "].");
  JackOutput* connected_output = input.m_connected_output;
  if (connected_output == this)
  {
    Dout(dc::notice, "Already connected.");
    return;
  }
  else if (connected_output)
  {
    // An input can only be connected to one output at a time.
    connected_output->disconnect(input);
  }

  // Insert the new input in m_connected_inputs in decreasing order (sorted by input type)
  // by inserting it after the last element that has the same (or greater) value.
  api_type input_type = input.type() & api_input_any;
  auto pos = std::find_if(m_connected_inputs.begin(), m_connected_inputs.end(), [input_type] (std::pair<JackInput*, api_type> const& p) { return p.second < input_type; });
  m_connected_inputs.insert(pos, std::make_pair(&input, input_type));

  // Fix allocation if needed.
  bool need_allocation = !has_provided_output_buffer(type()) && !has_provided_input_buffer(m_connected_inputs[0].second);
  if (need_allocation != m_allocated)
  {
    if (m_allocated)
      release_allocated_buffer();
    else
      create_allocated_buffer();
  }

  // Update m_chunk if needed.
  if (!need_allocation)
  {
    if (has_provided_output_buffer(type()))
    {
      m_chunk = provided_output_buffer();
      m_chunk_size = nframes_provided_output_buffer();
    }
    else // has_provided_input_buffer(m_connected_inputs[0].second) must be true.
    {
      m_chunk = m_connected_inputs[0].first->provided_input_buffer();
      m_chunk_size = m_connected_inputs[0].first->nframes_provided_input_buffer();
    }
  }

  ASSERT(m_chunk);

  // Connect the input to this output.
  input.m_connected_output = this;
}

void JackOutput::disconnect(JackInput& input)
{
  DoutEntering(dc::notice, "JackOutput::disconnect([" << input.m_name << "]) with this = " << (void*)this << " [" << m_name << "].");
  ASSERT(input.m_connected_output == this);

  JackInput* const input_ptr = &input;
  auto pos = std::find_if(m_connected_inputs.begin(), m_connected_inputs.end(), [input_ptr] (std::pair<JackInput*, api_type> const& p) { return p.first == input_ptr; });
  ASSERT(pos != m_connected_inputs.end());
  m_connected_inputs.erase(pos);

  // Fix allocation if needed.
  bool need_allocation = !has_provided_output_buffer(type()) && !m_connected_inputs.empty() && !has_provided_input_buffer(m_connected_inputs[0].second);
  Dout(dc::notice, "m_connected_inputs.size() = " << m_connected_inputs.size() << "; need_allocation = " << need_allocation);
  if (need_allocation != m_allocated)
  {
    if (m_allocated)
      release_allocated_buffer();
    else
      create_allocated_buffer();
  }

  // Update m_chunk if needed.
  if (!need_allocation)
  {
    if (has_provided_output_buffer(type()))
    {
      m_chunk = provided_output_buffer();
      m_chunk_size = nframes_provided_output_buffer();
    }
    else if (!m_connected_inputs.empty())
    {
      // has_provided_input_buffer(m_connected_inputs[0].second)) must be true.
      m_chunk = m_connected_inputs[0].first->provided_input_buffer();
      m_chunk_size = m_connected_inputs[0].first->nframes_provided_input_buffer();
    }
  }

  ASSERT(m_chunk || m_connected_inputs.empty());

  // Disconnect the input from this output.
  input.m_connected_output = NULL;
}

void JackOutput::disconnect()
{
  DoutEntering(dc::notice, "JackOutput::disconnect()");
  for (auto input : m_connected_inputs)
    input.first->m_connected_output = NULL;
  m_connected_inputs.clear();
  release_allocated_buffer();
}

void JackOutput::handle_memcpys()
{
  jack_default_audio_sample_t* const from = chunk_ptr();
  for (auto input : m_connected_inputs)
  {
    if (!has_memcpy_input(input.second))
      break;
    if (has_provided_input_buffer(input.second) &&
        input.first->provided_input_buffer() == from)
      continue;
    input.first->memcpy_input(from);
  }
}
