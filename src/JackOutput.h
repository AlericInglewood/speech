/**
 * \file JackOutput.h
 * \brief Declaration of JackOutput.
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

#ifndef JACK_OUTPUT_H
#define JACK_OUTPUT_H

#include "ApiType.h"
#include <jack/jack.h>
#include <string>
#include <vector>
#include <utility>
#include "debug.h"

// Forward declaration.
class JackInput;

class JackOutput
{
    typedef std::vector<std::pair<JackInput*, api_type>> connected_inputs_type; // The type of m_connected_inputs.

  protected:
    jack_default_audio_sample_t* m_chunk;       // The buffer to use.
    jack_nframes_t m_chunk_size;                // The buffer size, in frames.
    bool m_allocated;                           // Set if m_chunk was allocated (by us).
    int m_sequence_number;                      // sequence_number of the last call to fill_output_buffer.
    connected_inputs_type m_connected_inputs;   // A list of connected JackInput pointers and their api type.

#ifdef CWDEBUG
  public:
    std::string m_name;                         // A human readable string describing this object, used for debug output only.
#endif

  private:
    // Disconnect from all connected inputs (if any).
    void disconnect();

    // Allocate a new buffer.
    void create_allocated_buffer();

    // Release allocated buffer (if any).
    void release_allocated_buffer(void);

    // Disallow copying.
    JackOutput(JackOutput const&);
    JackOutput& operator=(JackOutput const&);

  protected:
    // Construct a JackOutput that is not connected nor associated with any buffer.
    JackOutput(DEBUG_ONLY(std::string processor_name)) :
      m_chunk(NULL), m_chunk_size(0), m_allocated(false),
      m_sequence_number(-1)
      COMMA_DEBUG_ONLY(m_name(processor_name + " Output")) { }

    // Construct a JackOutput as wrapper around a jack buffer (chunk).
    JackOutput(jack_default_audio_sample_t* chunk, jack_nframes_t nframes COMMA_DEBUG_ONLY(std::string processor_name)) :
        m_chunk(chunk), m_chunk_size(nframes), m_allocated(false),
        m_sequence_number(-1)
        COMMA_DEBUG_ONLY(m_name(processor_name + " Output")) { }

    // The destructor makes sure we're not (still) connected, because the JackInputs keep
    // pointers that point back to the connected JackOutput object.
    // Also free allocated memory, if any.
    virtual ~JackOutput() { disconnect(); release_allocated_buffer(); }

    // Copy generated output to inputs that provide their own buffer, if any.
    void handle_memcpys();

  public:
    // Connect an input to this output; does nothing when already connected to this output.
    // First calls disconnect when already connected to another output.
    void connect(JackInput& input);

    // Disconnect a previously, to this output, connected input.
    void disconnect(JackInput& input);

    // The underlaying buffer to use; used by JackProcessor derived classes to write data to.
    jack_default_audio_sample_t* chunk_ptr() const { ASSERT(m_chunk); return m_chunk; }

    // The size of the underlaying buffer.
    jack_nframes_t nframes() const { return m_chunk_size; }

  public:
    // Does all the work, so all connected inputs are ready to be read from after this call.
    // This function should first check if it wasn't called before with sequence_number,
    // and if it wasn't, generate the output (to m_chunk) and finally call handle_memcpys().
    virtual void fill_output_buffer(int sequence_number) = 0;

    virtual api_type type() const
    {
      return api_output_uses_allocated_or_input_buffer; // The default. The actual buffer used (returned by chunk_ptr())
                                                        // is allocated (m_allocated is set) or is provided by a connected
                                                        // input that has api_provided_input_buffer.
    }

    virtual jack_default_audio_sample_t* provided_output_buffer() const
    {
      ASSERT(false);                                    // This function should only be called when has_provided_output_buffer(type()) is true.
      return NULL;
    }

    virtual jack_nframes_t nframes_provided_output_buffer() const
    {
      ASSERT(false);                                    // This function should only be called when has_provided_output_buffer(type()) is true.
      return 0;
    }

    virtual void memcpy_output(jack_default_audio_sample_t*) const
    {
      ASSERT(false);                                    // This function should only be called when has_memcpy_output(type()) is true;
    };
};

// Inline functions.

inline void operator<<(JackInput& input, JackOutput& output) { output.connect(input); }

#endif // JACK_OUTPUT_H
