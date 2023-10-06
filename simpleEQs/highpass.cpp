#include "highpass.h"
#include <math.h>
#include "dsp.h"

using namespace daisysp;

void Highpass::Reset()
{
/*     float con   = cutoff_ * two_pi_d_sr_;
    float alpha = 1.0f - 2.0f * res_ * cosf(con) * cosf(con)
                  + res_ * res_ * cosf(2 * con);
    float beta  = 1.0f + cosf(con);
    float gamma = 1 + cosf(con);
    float m1    = alpha * gamma + beta * sinf(con);
    float m2    = alpha * gamma - beta * sinf(con);
    float den   = sqrtf(m1 * m1 + m2 * m2);

    b0_ = 1.5f * (alpha * alpha + beta * beta) / den;
    b1_ = b0_;
    b2_ = 0.0f;
    a0_ = 1.0f;
    a1_ = -2.0 * res_ * cosf(con);
    a2_ = res_ * res_; */  

   // float A     = powf(10.0f, (gain_ / 40.0f));
/*     float omega = 2.0f * M_PI * freq_ / sample_rate_; // pas d'entrée de samplerate
    float tsin  = sinf(omega);
    float tcos  = cosf(omega);
    //float beta  = sqrtf(A+A) / q_;
    float alpha = tsin / 2.0f / q_; 
    b0_ = (1.0f + tcos) / 2.0f;
    b1_ = -(1.0f + tcos);
    b2_ = (1.0f + tcos) / 2.0f;
    a0_ = (1.0f + alpha);
    a1_ = (-2.0f * tcos);
    a2_ = (1.0f - alpha); */

    float theta = 2.0f * M_PI * freq_ / sample_rate_; // pas d'entrée de samplerate
    float K  = tanf(theta/2);
    float W  = K * K;
    //float beta  = sqrtf(A+A) / q_;
    float alpha = 1.0f + K / q_ + W; 
    b0_ = 1.0f / alpha;
    b1_ = -2.0f / alpha;
    b2_ = b0_;

    // b0_ = W / alpha;
    // b1_ = 2.0f * W / alpha;
    // b2_ = b0_;

    a0_ = 1.0f;
    a1_ = 2.0f * (W - 1.0f) / alpha;
    a2_ = (1.0f - K / q_ + W) / alpha;
}

void Highpass::Init(float sample_rate)
{
    sample_rate_ = sample_rate;
    two_pi_d_sr_ = TWOPI_F / sample_rate_;

    //cutoff_ = 500;
    //res_    = 0.7;
    freq_ = 1000;
   // gain_ = 6;
    q_ = 0.7;
    Reset();

    xnm1_ = xnm2_ = ynm1_ = ynm2_ = 0.0f;
}

float Highpass::Process(float in)
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
