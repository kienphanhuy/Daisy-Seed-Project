// pedale.cpp
#include "pedale.h"
#include "daisysp.h"
#include "daisy_seed.h"

void pedale::UpdateFilter(float sample_rate, size_t numStages,float* piir_coeffs) {

    // Implementation of the function
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
    /* Generated with FFQ_2_DaisySeed.m matlab code */
    float freq[17] = {81.97, 561.02, 47.82, 15729.57, 106.39, 4825.61, 2351.75, 467.02, 186.93, 893.76, 766.36, 392.72, 292.25, 1713.81, 1168.52, 8173.25, 68.10}; //frequencies
    float gain[17] = {-12.37, -7.91, -10.39, -12.13, 11.23, 4.23, -8.80, -6.46, 6.96, -7.39, 5.14, 5.61, -3.54, -3.28, 3.67, 3.75, -2.54}; //Gains
    float q[17] = {1.00, 1.07, 2.10, 3.69, 3.94, 2.50, 12.97, 8.05, 9.17, 15.42, 9.17, 10.00, 5.95, 5.00, 7.71, 10.00, 5.95}; //Q-factor
    float typeIIR[17] = {2, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4}; //Type of filters [0-5] see above


    // float freq[1] = {1000.0}; // frequency
    // float gain[1] = {-6.0}; // Gain in dB
    // float q[1] = {0.701}; // Q factor
    // int typeIIR[1] = {3}; // Type of filter [0-5] see above

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
        float a0=0;float a1=0;float a2=0;float b0=0;float b1=0;float b2=0;

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
        piir_coeffs[i*5] = b0;
        piir_coeffs[i*5+1] = b1;
        piir_coeffs[i*5+2] = b2;
        piir_coeffs[i*5+3] = -a1;
        piir_coeffs[i*5+4] = -a2; // on envoie la valeur via le pointeur
    } 
}


void pedale::UpdateFilterHPF(float sample_rate, size_t numStages,float* piir_coeffs) {
    float freq[2] = {250,250}; //frequencies
    float q[2] = {0.707,0.707}; //Q-factor
    
    for(size_t i = 0; i < numStages; ++i) 
    {
        /* Intermediate Coefficients */ 

        float omega = 2.0f * M_PI * freq[i] / sample_rate; // pas d'entrée de samplerate
        float tsin  = sinf(omega);
        float tcos  = cosf(omega);
        float alpha = tsin / 2.0f / q[i];
        float a0=0;float a1=0;float a2=0;float b0=0;float b1=0;float b2=0;

            a0 =   1.0f + alpha;
            a1 =  (-2.0f*tcos)/a0;
            a2 =  (1.0f - alpha)/a0;
            b0 =  (1.0f + tcos)/2/a0;
            b1 =  -(1.0f + tcos)/a0;
            b2 =  (1.0f + tcos)/2/a0;
 
        /* Filter coefficients */
        //static float iir_coeffs[5] = {b0, b1, b2, -a1, -a2};
        //  the feedback coefficients a1 and a2 must be negated when used with the CMSIS DSP Library.
        piir_coeffs[i*5] = b0;
        piir_coeffs[i*5+1] = b1;
        piir_coeffs[i*5+2] = b2;
        piir_coeffs[i*5+3] = -a1;
        piir_coeffs[i*5+4] = -a2; // on envoie la valeur via le pointeur

        // butterworth MATLAB
        // piir_coeffs[i*5] = 0.9771;
        // piir_coeffs[i*5+1] = -1.9543;
        // piir_coeffs[i*5+2] = 0.9771;
        // piir_coeffs[i*5+3] = -1.9537;
        // piir_coeffs[i*5+4] = +0.9548; // on envoie la valeur via le pointeur        

    } 
}

void pedale::UpdateFilterLPF(float sample_rate, size_t numStages,float* piir_coeffs) {

    // Implementation of the function
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
    /* Generated with FFQ_2_DaisySeed.m matlab code */
    float freq[2] = {250,250}; //frequencies
    float q[2] = {0.707,0.707}; //Q-factor
    
    for(size_t i = 0; i < numStages; ++i) 
    {
        /* Intermediate Coefficients */ 
        //float A     = powf(10.0f, (gain[i] / 40.0f));
        float omega = 2.0f * M_PI * freq[i] / sample_rate; // pas d'entrée de samplerate
        float tsin  = sinf(omega);
        float tcos  = cosf(omega);
        float alpha = tsin / 2.0f / q[i];
        float a0=0;float a1=0;float a2=0;float b0=0;float b1=0;float b2=0;
            a0 =   1.0f + alpha;
            a1 =  (-2.0f*tcos)/a0;
            a2 =  (1.0f - alpha)/a0;
            b0 =  (1.0f - tcos)/2/a0;
            b1 =  (1.0f - tcos)/a0;
            b2 =  (1.0f - tcos)/2/a0;

        /* Filter coefficients */
        //static float iir_coeffs[5] = {b0, b1, b2, -a1, -a2};
        //  the feedback coefficients a1 and a2 must be negated when used with the CMSIS DSP Library.
        piir_coeffs[i*5] = b0;
        piir_coeffs[i*5+1] = b1;
        piir_coeffs[i*5+2] = b2;
        piir_coeffs[i*5+3] = -a1;
        piir_coeffs[i*5+4] = -a2; // on envoie la valeur via le pointeur

        // butterworth MATLAB
        // piir_coeffs[i*5] = 0.0002617f;
        // piir_coeffs[i*5+1] = 0.0005233f;
        // piir_coeffs[i*5+2] = 0.0002617f;
        // piir_coeffs[i*5+3] = +1.9537f;
        // piir_coeffs[i*5+4] = -0.9548f; // on envoie la valeur via le pointeur        
    } 
}

void pedale::UpdateFilterEQ(float* pfreq, float* pgain, float* pq, float* ptypeIIR,float sample_rate, size_t numStages,float* piir_coeffs) {

    // Implementation of the function
    /* coefficients formulae from Cookbook formulae for audio EQ biquad filter coefficients
by Robert Bristow-Johnson  <rbj@audioimagination.com>*/

    // float freq[1] = {1000.0}; // frequency
    // float gain[1] = {-6.0}; // Gain in dB
    // float q[1] = {0.701}; // Q factor
    // int typeIIR[1] = {3}; // Type of filter [0-5] see above

    /* Parameters for two EQ stages filters*/
    /* numStages must be set to 2 here*/
    // float freq[2] = {200, 1000}; // frequencies
    // float gain[2] = {+6, 0}; // Gains in dB
    // float q[2] = {0.701, 0.701}; // Q factor
    // int typeIIR[2] = {4,0}; // Type of filters [0-5] see above

    
    
    for(size_t i = 0; i < numStages; ++i) 
    {
        /* Intermediate Coefficients */ 
        float A     = powf(10.0f, (pgain[i] / 40.0f));
        float omega = 2.0f * M_PI * pfreq[i] / sample_rate; // pas d'entrée de samplerate
        float tsin  = sinf(omega);
        float tcos  = cosf(omega);
        float alpha = tsin / 2.0f / pq[i];
        float a0=0;float a1=0;float a2=0;float b0=0;float b1=0;float b2=0;

        // if(ptypeIIR[i] == 0) // Lowpass filter
        // {
        //     a0 =   1.0f + alpha;
        //     a1 =  (-2.0f*tcos)/a0;
        //     a2 =  (1.0f - alpha)/a0;
        //     b0 =  (1.0f - tcos)/2/a0;
        //     b1 =  (1.0f - tcos)/a0;
        //     b2 =  (1.0f - tcos)/2/a0;

        // }
        if(ptypeIIR[i] == 1) //Highpass filter
        {
            a0 =   1.0f + alpha;
            a1 =  (-2.0f*tcos)/a0;
            a2 =  (1.0f - alpha)/a0;
            b0 =  (1.0f + tcos)/2/a0;
            b1 =  -(1.0f + tcos)/a0;
            b2 =  (1.0f + tcos)/2/a0;
        }
        // if(ptypeIIR[i] == 2) //Low shelf
        // {
        //     float sqA = sqrtf(A);
        //     a0 =  (A+1.0f) + (A-1.0f)*tcos + 2.0f*sqA*alpha;
        //     b0 =  A*((A+1.0f) - (A-1.0f)*tcos + 2.0f*sqA*alpha )/a0;
        //     b1 =  2.0f*A*( (A-1.0f) - (A+1.0f)*tcos )/a0;
        //     b2 =  A*((A+1.0f) - (A-1.0f)*tcos - 2.0f*sqA*alpha )/a0;
        //     a1 =  -2.0f*( (A-1.0f) + (A+1.0f)*tcos )/a0;
        //     a2 =  ((A+1.0f) + (A-1.0f)*tcos - 2.0f*sqA*alpha)/a0;
        // }    
        if(ptypeIIR[i] == 3) //High shelf
        {
            float sqA = sqrtf(A);

            a0 =        (A+1) - (A-1)*tcos + 2*sqA*alpha;
            b0 =    A*( (A+1) + (A-1)*tcos + 2*sqA*alpha )/a0;
            b1 = -2*A*( (A-1) + (A+1)*tcos                   )/a0;
            b2 =    A*( (A+1) + (A-1)*tcos - 2*sqA*alpha )/a0;            
            a1 =    2*( (A-1) - (A+1)*tcos                   )/a0;
            a2 =       ((A+1) - (A-1)*tcos - 2*sqA*alpha)/a0;
        }    
        if(ptypeIIR[i] == 4)
        {
            a0 = (1.0f + alpha/A);
            a1 = (-2.0f * tcos)/a0;
            a2 = (1.0f - alpha/A)/a0;
                    
            b0 = (1.0f + alpha*A)/a0;
            b1 = (-2.0f * tcos)/a0;
            b2 = (1.0f - alpha*A)/a0;

        }
        // if(ptypeIIR[i] == 5)
        // {
        //     a0 =   1.0f + alpha;
        //     b0 =   1.0f/a0;
        //     b1 =  -2.0f*tcos/a0;
        //     b2 =   1.0f/a0;
        //     a1 =  -2.0f*tcos/a0;
        //     a2 =   (1.0f - alpha)/a0;
        // }
        /* Filter coefficients */
        //static float iir_coeffs[5] = {b0, b1, b2, -a1, -a2};
        //  the feedback coefficients a1 and a2 must be negated when used with the CMSIS DSP Library.
        piir_coeffs[i*5] = b0;
        piir_coeffs[i*5+1] = b1;
        piir_coeffs[i*5+2] = b2;
        piir_coeffs[i*5+3] = -a1;
        piir_coeffs[i*5+4] = -a2; // on envoie la valeur via le pointeur
    } 
}