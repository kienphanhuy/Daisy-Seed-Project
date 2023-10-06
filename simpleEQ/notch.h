#pragma once
#ifndef DSY_NOTCH_H
#define DSY_NOTCH_H

#include <stdint.h>
#ifdef __cplusplus

namespace daisysp
{
/** Two pole recursive filter

    Original author(s) : Hans Mikelson
    Modified for Audio Equalizer: Kien PHan Huy
    Year: 1998 - 2023

    Ported from soundpipe by Ben Sergentanis, May 2020
*/
class Notch
{
  public:
    Notch() {}
    ~Notch() {}
    /** Initializes the biquad module.
        \param sample_rate - The sample rate of the audio engine being run. 
    */
    void Init(float sample_rate);


    /** Filters the input signal
        \return filtered output
    */
    float Process(float in);


    /** Sets filter frequency in Hz
        \param freq : Set filter freq.
    */
    inline void SetFreq(float freq)
    {
        freq_ = freq;
        Reset();
    }


  private:
    float sample_rate_, freq_, b0_, b1_, b2_, a0_, a1_, a2_,
        two_pi_d_sr_, xnm1_, xnm2_, ynm1_, ynm2_;
    void Reset();
};
} // namespace daisysp
#endif
#endif
