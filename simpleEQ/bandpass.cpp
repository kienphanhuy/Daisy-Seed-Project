#include "lowshelf.h"
#include <math.h>
#include "dsp.h"

using namespace daisysp;

void Lowshelf::Reset()
{
    float A     = powf(10.0f, (gain_ / 40.0f));
    float omega = 2.0f * M_PI * freq_ / sample_rate_; // pas d'entrée de samplerate
    float tsin  = sinf(omega);
    float tcos  = cosf(omega);
    float sqA = sqrtf(A);
    //float beta  = sqrtf(A+A) / q_;
    float alpha = tsin / 2.0f / q_; 
   
    b0_ =  A*((A+1.0f) - (A-1.0f)*tcos + 2*sqA*alpha );
    b1_ =  2.0f*A*( (A-1) - (A+1.0f)*tcos );
    b2_ =  A*((A+1.0f) - (A-1)*tcos - 2.0f*sqA*alpha );
    a0_ =  (A+1.0f) + (A-1)*tcos + 2.0f*sqA*alpha;
    a1_ =  -2.0f*( (A-1.0f) + (A+1)*tcos );
    a2_ =  (A+1.0f) + (A-1)*tcos - 2.0f*sqA*alpha;
}

void Lowshelf::Init(float sample_rate)
{
    sample_rate_ = sample_rate;
    two_pi_d_sr_ = TWOPI_F / sample_rate_;

    //cutoff_ = 500;
    //res_    = 0.7;
    freq_ = 1000;
    gain_ = 6;
    q_ = 0.7;
    Reset();

    xnm1_ = xnm2_ = ynm1_ = ynm2_ = 0.0f;
}

float Lowshelf::Process(float in)
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
