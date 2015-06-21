/**
 * /file FFTJackProcessor.cpp
 * /brief Implementation of class FFTJackProcessor.
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

#include "FFTJackProcessor.h"

FFTJackProcessor::FFTJackProcessor()
{
  // Prepare FFTW.
  m_fftwf_real_array = fftwf_alloc_real(256);
  m_fftwf_complex_array = fftwf_alloc_complex(256);
  Dout(dc::notice, "Calling fftwf_plan_dft_r2c_1d()");
  m_r2c_plan = fftwf_plan_dft_r2c_1d(256, m_fftwf_real_array, m_fftwf_complex_array, FFTW_PATIENT);
  Dout(dc::notice, "Calling fftwf_plan_dft_c2r_1d()");
  m_c2r_plan = fftwf_plan_dft_c2r_1d(256, m_fftwf_complex_array, m_fftwf_real_array, FFTW_PATIENT | FFTW_DESTROY_INPUT);
  Dout(dc::notice, "Done()");
}

void FFTJackProcessor::generate_output(int sequence_number)
{
  if (m_sequence_number == sequence_number)
    return;
  m_sequence_number = sequence_number;

  jack_default_audio_sample_t const* test_in = m_input.chunk_ptr();
  jack_default_audio_sample_t* test_out = m_output.chunk_ptr();
  jack_nframes_t const nframes = m_input.nframes();
  ASSERT(nframes == m_output.nframes());

  // Make sure the arrays are properly aligned.
  assert(fftwf_alignment_of(const_cast<jack_default_audio_sample_t*>(test_in)) == fftwf_alignment_of(m_fftwf_real_array));
  assert(fftwf_alignment_of(test_out) == fftwf_alignment_of(m_fftwf_real_array));

  // Perform test operation. The const_cast is allowed because we are not using FFTW_DESTROY_INPUT for m_r2c_plan.
  fftwf_execute_dft_r2c(m_r2c_plan, const_cast<jack_default_audio_sample_t*>(test_in), m_fftwf_complex_array);
  for (jack_nframes_t freq = 0; freq <= nframes / 2; ++freq)
  {
    m_complex_array[freq] = std::abs(m_complex_array[freq]);
  }
  fftwf_execute_dft_c2r(m_c2r_plan, m_fftwf_complex_array, test_out);

  // Normalize.
  for (jack_nframes_t frame = 0; frame < nframes; ++frame)
  {
    test_out[frame] /= nframes;
  }
}
