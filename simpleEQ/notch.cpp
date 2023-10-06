#include "notch.h"
#include <math.h>
#include "dsp.h"

using namespace daisysp;

void Notch::Reset()
{
    float omega = 2.0f * M_PI * freq_ / sample_rate_; // pas d'entrée de samplerate
    float tcos  = cosf(omega);
    float tsin  = sinf(omega);
    float q = 10.0f;
    float alpha = tsin / 2.0f / q; 
   
    b0_ =   1.0f;
    b1_ =  -2.0f*tcos;
    b2_ =   1.0f;
    a0_ =   1.0f + alpha;
    a1_ =  -2.0f*tcos;
    a2_ =   1.0f - alpha;    
}

void Notch::Init(float sample_rate)
{
    sample_rate_ = sample_rate;
    two_pi_d_sr_ = TWOPI_F / sample_rate_;

    freq_ = 1000;

    Reset();

    xnm1_ = xnm2_ = ynm1_ = ynm2_ = 0.0f;
}

float Notch::Process(float in)
{
    float xn, yn;
    float a0 = a0_, a1 = a1_, a2 = a2_;
    float b0 = b0_, b1 = b1_, b2 = b2_;
    float xnm1 = xnm1_, xnm2 = xnm2_, ynm1 = ynm1_, ynm2 = ynm2_;

    xn   = in;
    yn   = (b0 * xn + b1 * xnm1 + b2 * xnm2 - a1 * ynm1 - a2 * ynm2) / a0;
    xnm2 = xnm1;
    xnm1 = xn;
    ynm2 = ynm1;
    ynm1 = yn;

    xnm1_ = xnm1;
    xnm2_ = xnm2;
    ynm1_ = ynm1;
    ynm2_ = ynm2;

    return yn;
}
