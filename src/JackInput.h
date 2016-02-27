/**
 * \file JackInput.h
 * \brief Declaration of JackInput.
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

#ifndef JACK_INPUT_H
#define JACK_INPUT_H

#include "JackOutput.h"

// The input plug of a JackProcessor or JACK server.
class JackInput
{
  private:
    // Disallow copying.
    JackInput(JackInput const&);
    JackInput& operator=(JackInput const&);

  protected:
    friend class JackOutput;
    JackOutput* m_connected_output;             // A pointer to the JackOutput that we are connected to, if any; NULL when not connected.

#ifdef CWDEBUG
  public:
    std::string m_name;                         // A human readable string describing this object, used for debug output only.
#endif

  protected:
    // Construct a JackInput that is not connected.
    // The parameter \a name identifies what this object represents (for debugging purposes).
    JackInput(DEBUG_ONLY(std::string name)) :
        m_connected_output(NULL)
        COMMA_DEBUG_ONLY(m_name(name + " Input")) { }

    // The destructor makes sure we're not (still) connected, because the JackOutput keeps
    // pointers that point back to connected JackInput objects.
    virtual ~JackInput() { disconnect(); }

  public:
    // Fill buffer.
    void fill_input_buffer(int sequence_number)
    {
      // The output buffer of m_connected_output is our input buffer.
      m_connected_output->fill_output_buffer(sequence_number);
    }

    // Connect this input to output.
    void connect(JackOutput& output)
    {
      output.connect(*this);
    }

    // If connected, disconnect.
    void disconnect()
    {
      if (m_connected_output)
        m_connected_output->disconnect(*this);
    }

    // The underlaying buffer to use; used by JackProcessor derived classes to read data from.
    jack_default_audio_sample_t* chunk_ptr() const { return m_connected_output->chunk_ptr(); }

    // The size of the underlaying buffer.
    jack_nframes_t nframes() const { return m_connected_output->nframes(); }

    // Accessors.
    JackOutput* connected_output() const { return m_connected_output; }

    virtual api_type type() const
    {
      return api_input_uses_output_buffer;      // The default. The actual buffer used is the one returned by chunk_ptr().
    }

    virtual jack_default_audio_sample_t* provided_input_buffer() const
    {
      ASSERT(false);                            // This function should only be called when has_provided_input_buffer(type()) is true.
      return NULL;
    }

    virtual jack_nframes_t nframes_provided_input_buffer() const
    {
      ASSERT(false);                            // This function should only be called when has_provided_input_buffer(type()) is true.
      return 0;
    }

    virtual void memcpy_input(jack_default_audio_sample_t const*)
    {
      ASSERT(false);                            // This function should only be called when has_memcpy_input(type()) is true;
    };

    virtual void zero_input()
    {
      ASSERT(false);                            // This function should only be called when has_zero_input(type()) is true.
    };
};

#endif // JACK_INPUT_H
