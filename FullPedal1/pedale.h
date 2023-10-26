// pedale.h
#ifndef PEDALE_H
#define PEDALE_H
#include "daisysp.h"
#include "daisy_seed.h"

class pedale {
public:
    /* Conversion les paramètres des EQ en (an,bn) IIR*/
    void UpdateFilter(float sample_rate, size_t numStages,float* piir_coeffs);
    //void UpdateFilter2(float* pir_front);
    void UpdateFilterHPF(float sample_rate, size_t numStages,float* piir_coeffs);
    void UpdateFilterLPF(float sample_rate, size_t numStages,float* piir_coeffs);
    void UpdateFilterEQ(float* pfreq, float* pgain, float* pq, float* ptypeIIR,float sample_rate, size_t numStages,float* piir_coeffs);
};

#endif