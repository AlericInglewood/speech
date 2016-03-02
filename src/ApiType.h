/**
 * \file ApiType.h
 * \brief Declaration of api_type and constants.
 *
 * Copyright (C) 2016 Aleric Inglewood.
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

#ifndef API_TYPE_H
#define API_TYPE_H

// The type of the API features.
typedef int api_type;

// JackInput API

// None of the below api_input* API's.
api_type const api_input_uses_output_buffer = 0;

// virtual jack_default_audio_sample_t* provided_input_buffer() const
api_type const api_input_provided_buffer = 1;

// virtual void memcpy_input(jack_default_audio_sample_t const*)
// virtual void zero_input()
api_type const api_input_memcpy_zero = 2;       // This must be the largest api_input_* bit value.

// JackOutput API

// None of the below api_output* API's.
api_type const api_output_uses_allocated_or_input_buffer = 0;

// JackOutput::m_chunk is set in fill_output_buffer() and JackOutput::chunk_ptr() may be called to access it.
api_type const api_output_provided_buffer = 4;

// Combinations.

api_type const api_input_any = api_input_provided_buffer | api_input_memcpy_zero;
api_type const api_output_any = api_output_provided_buffer;

api_type const api_input_provided_buffer_memcpy_zero = api_input_provided_buffer | api_input_memcpy_zero;

// Easy access.

inline bool has_memcpy_input(api_type mask) { return (mask & api_input_memcpy_zero); }
inline bool has_zero_input(api_type mask) { return (mask & api_input_memcpy_zero); }
inline bool has_provided_input_buffer(api_type mask) { return (mask & api_input_provided_buffer); }
inline bool has_provided_output_buffer(api_type mask) { return (mask & api_output_provided_buffer); }

#endif // API_TYPE_H
