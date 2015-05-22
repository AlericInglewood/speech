/**
 * \file JackProcessor.h
 * \brief Declaration of JackInput, JackOutput and JackProcessor.
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

#ifndef JACK_PROCESSOR_H
#define JACK_PROCESSOR_H

#include "JackChunkAllocator.h"

#include <jack/jack.h>
#include "debug.h"

class JackInput;
class JackOutput;
class JackProcessor;
class JackCrossfader;

// This is what JackProcessor gets data FROM.
class JackInput
{
  private:
    jack_default_audio_sample_t* m_in;
    jack_nframes_t m_chunk_size;
    JackOutput* m_connected_output;
    JackProcessor* m_owner;
    JackProcessor* m_connected_owner;

  public:
    // Construct a JackInput that is not associated with any buffer.
    JackInput() : m_in(NULL), m_chunk_size(0), m_connected_output(NULL), m_owner(NULL), m_connected_owner(NULL) { }
    // Construct a JackInput as wrapper around a jack input buffer (chunk).
    JackInput(jack_default_audio_sample_t* chunk_in, jack_nframes_t nframes) : m_in(chunk_in), m_chunk_size(nframes), m_connected_output(NULL), m_owner(NULL), m_connected_owner(NULL) { }
    // The destructor makes sure we're not (still) owned and frees allocated memory, if any.
    virtual ~JackInput() { ASSERT(!m_owner); if (m_in) JackChunkAllocator::instance().release(m_in); }

    // Assign external chunk `in' to read data from, to this JackInput.
    void assign_external_buffer(jack_default_audio_sample_t* in, jack_nframes_t nframes)
    {
      // This function is only called for the jack server output object which has no owner.
      // If ever this is called for a JackProcessor owned obect then m_in should not be set or we would leak memory.
      ASSERT(!m_owner || !m_in);
      // We should only set m_connected_output when we don't have a buffer.
      ASSERT(!m_connected_output);
      m_in = in;
      m_chunk_size = nframes;
    }

    // Process the whole chain up till and including this output.
    void process();

    // Accessors to underlaying buffer used by JackProcessor derived classes.
    inline jack_default_audio_sample_t* chunk_ptr() const;
    inline jack_nframes_t nframes() const;
    JackProcessor* owner() { return m_owner; }
    bool has_buffer() const { return m_in; }

  private:
    // Connect input to output.
    friend void operator<<(JackInput& input, JackOutput& output) { input.connect_to(output); }
    friend JackInput& operator<<(JackInput& input, JackProcessor& processor);
    friend void operator<<(JackInput& input, JackCrossfader& crossfader);
    void connect_to(JackOutput& output);

  private:
    friend class JackProcessor;  // Must not access private data above this line.
    friend class JackCrossfader; // Defines more input(s).
    // Used by the constructor of JackProcessor.
    JackInput(JackProcessor* owner) : m_in(NULL), m_chunk_size(0), m_connected_output(NULL), m_owner(owner), m_connected_owner(NULL) { }
    // Used by the destructor of JackProcessor.
    void disown() { ASSERT(m_owner); m_owner = NULL; m_in = NULL; m_chunk_size = 0; }
};

// This is where JackProcessor writes data TO.
class JackOutput
{
  private:
    jack_default_audio_sample_t* m_out; // An (optional) chunk buffer.
    jack_nframes_t m_chunk_size;        // The buffer size, if any.
    JackInput* m_connected_input;       // A pointer to the JackInput that we are connected to.
    JackProcessor* m_owner;

  public:
    // Construct a JackOutput that is not associated with any buffer.
    JackOutput() : m_out(NULL), m_chunk_size(0), m_connected_input(NULL), m_owner(NULL) { }
    // Construct a JackOutput as wrapper around a jack output buffer (chunk).
    JackOutput(jack_default_audio_sample_t* out, jack_nframes_t nframes) : m_out(out), m_chunk_size(nframes), m_connected_input(NULL), m_owner(NULL) { }
    // The destructor makes sure we're not (still) owned and frees allocated memory, if any.
    virtual ~JackOutput() { ASSERT(!m_owner); if (m_out) JackChunkAllocator::instance().release(m_out); }

    // Assign external chunk `out' to write data to from this JackOutput.
    void assign_external_buffer(jack_default_audio_sample_t* out, jack_nframes_t nframes)
    {
      // Setting m_out while owning this object would leak memory.
      ASSERT(!m_owner || !m_out);
      // We should only set m_connected_input when we don't have a buffer.
      ASSERT(!m_connected_input);
      m_out = out;
      m_chunk_size = nframes;
    }

    // Allocate a new buffer.
    void create_buffer();

    // Our output is written to input.
    void connect_to(JackInput* input);

    // Accessors to underlaying buffer used by JackProcessor derived classes.
    jack_default_audio_sample_t* chunk_ptr() const { ASSERT(m_out || m_connected_input); return m_connected_input ? m_connected_input->chunk_ptr() : m_out; }
    jack_nframes_t nframes() const { return m_connected_input ? m_connected_input->nframes() : m_chunk_size; }
    JackProcessor* owner() { return m_owner; }
    bool has_buffer() const { return m_out; }

  private:
    friend class JackProcessor; // Must not access private data above this line.
    JackOutput(JackProcessor* owner) : m_out(NULL), m_chunk_size(0), m_connected_input(NULL), m_owner(owner) { }
    void disown() { ASSERT(m_owner); m_owner = NULL; m_out = NULL; m_chunk_size = 0; }
};

jack_default_audio_sample_t* JackInput::chunk_ptr() const
{
  ASSERT(m_connected_output || m_in);
  return m_connected_output ? m_connected_output->chunk_ptr() : m_in;
}

jack_nframes_t JackInput::nframes() const
{
  return m_connected_output ? m_connected_output->nframes() : m_chunk_size;
}

// Base class for objects with a single input and output.
class JackProcessor
{
  protected:
    JackInput m_input;
    JackOutput m_output;

  public:
    JackProcessor() : m_input(this), m_output(this) { }
    ~JackProcessor() { m_input.disown(); m_output.disown(); }

    // Connect processor to output.
    friend JackInput& operator<<(JackInput& input, JackProcessor& processor)
    { 
      input.connect_to(processor.m_output);
      return processor.m_input;
    }

    // Read input, process, write output.
    virtual void process() = 0;
};

#endif // JACK_PROCESSOR_H
