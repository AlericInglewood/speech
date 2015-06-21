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
class JackSwitch;
class RecorderJackProcessor;

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

    // Fill buffer.
    void fill(int sequence_number);

    // Accessors to underlaying buffer used by JackProcessor derived classes.
    inline jack_default_audio_sample_t* chunk_ptr() const;
    inline jack_nframes_t nframes() const;
    JackProcessor* owner() { return m_owner; }
    JackProcessor const* owner() const { return m_owner; }
    JackOutput const* connected_output() const { return m_connected_output; }
    inline bool provides_buffer() const;

  private:
    // Connect input to output.
    friend void operator<<(JackInput& input, JackOutput& output) { input.connect_to(output); }
    friend JackInput& operator<<(JackInput& input, JackProcessor& processor);
    friend JackInput& operator<<(JackProcessor& processor1, JackProcessor& processor2);
    friend void operator<<(JackProcessor& processor, JackOutput& output);
    friend void operator<<(JackInput& input, JackSwitch& jack_switch);
    void connect_to(JackOutput& output);

  private:
    friend class JackProcessor;  // Must not access private data above this line.
    friend class JackSwitch;     // Defines more input(s).
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

    // Should only be used by JackProcessors that provide the buffer dynamically.
    inline void update_buffer_ptr(jack_default_audio_sample_t* ptr);

    // Allocate a new buffer.
    void create_buffer();

    // Our output is written to input.
    void connect_to(JackInput* input);

    // Accessors to underlaying buffer used by JackProcessor derived classes.
    jack_default_audio_sample_t* chunk_ptr() const { ASSERT(m_out || m_connected_input); return m_connected_input ? m_connected_input->chunk_ptr() : m_out; }
    jack_nframes_t nframes() const { return m_connected_input ? m_connected_input->nframes() : m_chunk_size; }
    JackProcessor* owner() { return m_owner; }
    JackProcessor const* owner() const { return m_owner; }
    JackInput const* connected_input() const { return m_connected_input; }
    inline bool provides_buffer() const;

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
    int m_sequence_number;              // A sequence number that is incremented every time process is called.

  public:
    JackProcessor() : m_input(this), m_output(this), m_sequence_number(0) { }
    ~JackProcessor() { m_input.disown(); m_output.disown(); }

    // Connect processor to output.
    friend JackInput& operator<<(JackInput& input, JackProcessor& processor)
    { 
      input.connect_to(processor.m_output);
      return processor.m_input;
    }

    friend void operator<<(JackProcessor& processor, JackOutput& output)
    {
      processor.m_input.connect_to(output);
    }

    // Connect processor to processor.
    friend JackInput& operator<<(JackProcessor& processor1, JackProcessor& processor2)
    {
      processor1.m_input.connect_to(processor2.m_output);
      return processor2.m_input;
    }

    virtual bool provides_input_buffer() const { return false; }
    virtual bool provides_output_buffer() const { return false; }

    // Fill input buffer.
    virtual void fill_input_buffer(int sequence_number) { m_input.fill(sequence_number); }

    // Read input, process, write output.
    virtual void generate_output(int sequence_number) = 0;
};

bool JackInput::provides_buffer() const { return m_in || (m_owner && m_owner->provides_input_buffer()); }
bool JackOutput::provides_buffer() const { return m_out || (m_owner && m_owner->provides_output_buffer()); }
void JackOutput::update_buffer_ptr(jack_default_audio_sample_t* ptr) { ASSERT(m_owner && m_owner->provides_output_buffer()); m_out = ptr; }

#endif // JACK_PROCESSOR_H
