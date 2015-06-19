/**
 * \file FIFOBufferJackProcessor.h
 * \brief Declaration of FIFOBufferJackProcessor.
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

#ifndef FIFO_BUFFER_JACK_PROCESSOR_H
#define FIFO_BUFFER_JACK_PROCESSOR_H

#include "JackProcessor.h"

// This provides the output of our recording device.
class FIFOBufferJackProcessor : public JackProcessor
{
  public:
    // Read from the buffer and write to the output.
    virtual void process(int sequence_number) = 0;
};

#endif // FIFO_BUFFER_JACK_PROCESSOR_H
