/**
 * \file FFTJackProcessor.h
 * \brief Declaration of FFTJackProcessor.
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

#ifndef FFT_JACK_PROCESSOR_H
#define FFT_JACK_PROCESSOR_H

#include "JackProcessor.h"
#include <complex>
#include <fftw3.h>

class FFTJackProcessor : public JackProcessor
{
  private:
    float* m_fftwf_real_array;
    union {
      fftwf_complex* m_fftwf_complex_array;
      std::complex<float>* m_complex_array;
    };
    fftwf_plan m_r2c_plan;
    fftwf_plan m_c2r_plan;

  public:
    FFTJackProcessor();

    // Read input, process, write output.
    /*virtual*/ void generate_output(int sequence_number);
};

#endif // FFT_JACK_PROCESSOR_H
