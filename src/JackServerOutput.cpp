/**
 * /file JackServerOutput.cpp
 * /brief Implementation of class JackServerOutput.
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
#include "JackServerOutput.h"

event_type JackServerOutput::fill_output_buffer(int sequence_number)
{
  if (m_sequence_number == sequence_number)
    return 0;
  m_sequence_number = sequence_number;
  // api_output_provided_buffer requires we set m_chunk and m_chunk_size in fill_output_buffer(),
  // but those are already set by the call to initialize() when we get here.
  return handle_memcpys();
}
