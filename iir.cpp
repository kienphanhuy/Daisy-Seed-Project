/**   @brief IIR Filter example
 *    @author Kien Phan Huy (acousticir.free.fr)
 *  BAsed on ext_FIR.cpp from Alexander Petrov-Savchenko (axp@soft-amp.com)
 *    @date October 2023
 */

#include "daisysp.h"
#include "daisy_seed.h"

using namespace daisysp;
using namespace daisy;

static DaisySeed                  hw;
static IIR<IIRFILTER_USER_MEMORY> flt;

/*All information available on the CMSIS-DSP librairy at 
https://www.keil.com/pack/doc/CMSIS/DSP/html/group__BiquadCascadeDF1.html*/
static constexpr size_t numStages    = 1;  /*< Number of EQ Stages */
static float iir_coeffs[numStages*5]; /*The pCoeffs array contains a total of 5*numStages values. {b0, b1, b2, -a1, -a2} */
static float flt_state[numStages*4];       /*The state variables are arranged in the pState array (Buffer)*/

/* EQ biquad filters */
static void UpdateFilter(float sample_rate)
{
/* coefficients formulae from Cookbook formulae for audio EQ biquad filter coefficients
by Robert Bristow-Johnson  <rbj@audioimagination.com>*/

    /* Parameters for one EQ filter*/
    /* typeIIR type of filter */
    // 0 lowpass
    // 1 highpass
    // 2 lowshelf
    // 3 highshelf
    // 4 Parametric EQ (peak)
    // 5 Notch

    float freq[1] = {1000.0}; // frequency
    float gain[1] = {-6.0}; // Gain in dB
    float q[1] = {0.701}; // Q factor
    int typeIIR[1] = {3}; // Type of filter [0-5] see above

    /* Parameters for two EQ stages filters*/
    /* numStages must be set to 2 here*/
    // float freq[2] = {200, 1000}; // frequencies
    // float gain[2] = {+6, 0}; // Gains in dB
    // float q[2] = {0.701, 0.701}; // Q factor
    // int typeIIR[2] = {4,0}; // Type of filters [0-5] see above

    
    
    for(size_t i = 0; i < numStages; ++i) 
    {
        /* Intermediate Coefficients */ 
        float A     = powf(10.0f, (gain[i] / 40.0f));
        float omega = 2.0f * M_PI * freq[i] / sample_rate; // pas d'entrée de samplerate
        float tsin  = sinf(omega);
        float tcos  = cosf(omega);
        float alpha = tsin / 2.0f / q[i];
        float a0, a1, a2, b0, b1, b2;

        if(typeIIR[i] == 0) // Lowpass filter
        {
            a0 =   1.0f + alpha;
            a1 =  (-2.0f*tcos)/a0;
            a2 =  (1.0f - alpha)/a0;
            b0 =  (1.0f - tcos)/2/a0;
            b1 =  (1.0f - tcos)/a0;
            b2 =  (1.0f - tcos)/2/a0;

        }
        if(typeIIR[i] == 1) //Highpass filter
        {
            a0 =   1.0f + alpha;
            a1 =  (-2.0f*tcos)/a0;
            a2 =  (1.0f - alpha)/a0;
            b0 =  (1.0f + tcos)/2/a0;
            b1 =  -(1.0f + tcos)/a0;
            b2 =  (1.0f + tcos)/2/a0;
        }
        if(typeIIR[i] == 2) //Low shelf
        {
            float sqA = sqrtf(A);
            a0 =  (A+1.0f) + (A-1.0f)*tcos + 2.0f*sqA*alpha;
            b0 =  A*((A+1.0f) - (A-1.0f)*tcos + 2.0f*sqA*alpha )/a0;
            b1 =  2.0f*A*( (A-1.0f) - (A+1.0f)*tcos )/a0;
            b2 =  A*((A+1.0f) - (A-1.0f)*tcos - 2.0f*sqA*alpha )/a0;
            a1 =  -2.0f*( (A-1.0f) + (A+1.0f)*tcos )/a0;
            a2 =  ((A+1.0f) + (A-1.0f)*tcos - 2.0f*sqA*alpha)/a0;
        }    
        if(typeIIR[i] == 3) //High shelf
        {
            float sqA = sqrtf(A);

            a0 =        (A+1) - (A-1)*tcos + 2*sqA*alpha;
            b0 =    A*( (A+1) + (A-1)*tcos + 2*sqA*alpha )/a0;
            b1 = -2*A*( (A-1) + (A+1)*tcos                   )/a0;
            b2 =    A*( (A+1) + (A-1)*tcos - 2*sqA*alpha )/a0;            
            a1 =    2*( (A-1) - (A+1)*tcos                   )/a0;
            a2 =       ((A+1) - (A-1)*tcos - 2*sqA*alpha)/a0;
        }    
        if(typeIIR[i] == 4)
        {
            a0 = (1.0f + alpha/A);
            a1 = (-2.0f * tcos)/a0;
            a2 = (1.0f - alpha/A)/a0;
                    
            b0 = (1.0f + alpha*A)/a0;
            b1 = (-2.0f * tcos)/a0;
            b2 = (1.0f - alpha*A)/a0;

        }
        if(typeIIR[i] == 5)
        {
            a0 =   1.0f + alpha;
            b0 =   1.0f/a0;
            b1 =  -2.0f*tcos/a0;
            b2 =   1.0f/a0;
            a1 =  -2.0f*tcos/a0;
            a2 =   (1.0f - alpha)/a0;
        }
        /* Filter coefficients */
        //static float iir_coeffs[5] = {b0, b1, b2, -a1, -a2};
        //  the feedback coefficients a1 and a2 must be negated when used with the CMSIS DSP Library.
        iir_coeffs[i*5] = b0;
        iir_coeffs[i*5+1] = b1;
        iir_coeffs[i*5+2] = b2;
        iir_coeffs[i*5+3] = -a1;
        iir_coeffs[i*5+4] = -a2;
    } 





    


    
}

void AudioCallback(AudioHandle::InputBuffer  in,
                   AudioHandle::OutputBuffer out,
                   size_t                    size)
{    

    flt.ProcessBlock(const_cast<float*>(in[0]), out[0], size);   

}

int main(void)
{
    /* initialize seed hardware and daisysp modules */
    float sample_rate;
    hw.Configure();
    hw.Init();
    hw.SetAudioBlockSize(48); // Number of samples per bloc
    sample_rate = hw.AudioSampleRate();

    hw.StartLog(false);


    /** initialize IIR biquad filter and set parameters      */
    flt.SetStateBuffer(flt_state, DSY_COUNTOF(flt_state));

    //InitWindow();
    UpdateFilter(sample_rate);    
    flt.SetIIR(&iir_coeffs[0], numStages);

    /* start audio callback */
    hw.StartAudio(AudioCallback);

    while(1)
    {
    }
}
