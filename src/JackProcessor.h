/**
 * \file JackProcessor.h
 * \brief Declaration of JackProcessorInput, JackProcessorOutput and JackProcessor.
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

#ifndef JACK_PROCESSOR_H
#define JACK_PROCESSOR_H

#include "JackOutput.h"
#include "JackInput.h"

// Each JackProcessor has one JackInput and one JackOutput.
// The JackServerOutput is a JackOutput (generating data that we read from).
// The JackServerInput is a JackInput (sinking the data that we generate).
// A JackInput can be connected to at most one JackOutput.
// A JackOutput can be connected to zero or more JackInputs.
//
//                                 Audio direction =========>
//
// ,---,------------.   ,-----------------------------------------------.    ,-----------------------------------------------.   ,-----------.---.
// |   | JackOutput |   | JackInput |    JackProcessor     | JackOutput |    | JackInput |    JackProcessor     | JackOutput |   | JackInput |   |
// |   |            +---+           |                      |            +--.-+           |                      |            +---+           |   |
// |   |            |   |           |                      |            |  | |           |                      |            |   |           |   |
// |   `------------|   |-----------'                      `------------|  | |-----------'                      `------------|   |-----------'   |
// |JackServerOutput|   |                                               |  | |                                               |   |JackServerInput|
// `----------------'   |                                               |  | |                                               |   `---------------'
//                      |                                               |  | |                                               |
//                      `-----------------------------------------------'  | `-----------------------------------------------'
//                                                                         |
//                                                                         | ,----------------...
//                                                                         | | JackInput |
//                                                                         +-+           |
//                                                                         | |           |
//                                                                         . |-----------'
//                                                                         . |
//                                                                         . |
//                                                                           |
//                                                                           `----------------...
// Base class for objects with a single input and output.
class JackProcessor : public JackInput, public JackOutput
{
  protected:
#ifdef CWDEBUG
    std::string m_name;
#endif

  public:
    JackProcessor(DEBUG_ONLY(std::string name)) : JackInput(DEBUG_ONLY(name)), JackOutput(DEBUG_ONLY(name)) COMMA_DEBUG_ONLY(m_name(name)) { }

    // Read input, process, write output.
    virtual void generate_output() = 0;

    /*virtual*/ api_type type() const;

    // JackInput
    /*virtual*/ void memcpy_input(jack_default_audio_sample_t const* chunk);
    /*virtual*/ void zero_input();

    // JackOutput
    /*virtual*/ void fill_output_buffer(int sequence_number);
    /*virtual*/ void memcpy_output(jack_default_audio_sample_t* chunk) const;
};

#endif // JACK_PROCESSOR_H
