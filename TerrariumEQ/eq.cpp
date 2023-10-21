#include "daisy_petal.h"
#include "daisysp.h"
#include "terrarium.h"
#include "pedale.h"
#include <arm_math.h> // required for platform-optimized version

using namespace daisy;
using namespace daisysp;
using namespace terrarium;

DaisyPetal hw;
Chorus     ch;
pedale myPedal;
Parameter gain;
static IIR<IIRFILTER_USER_MEMORY> eq; // Tonestack EQ

bool  effectOn;
Led led1, led2;

// /******************************************************************/
static constexpr size_t numStagesEQ    = 1;  /*< Number of EQ Stages */

static float iir_coeffs_EQ[numStagesEQ*5]; /*The pCoeffs array contains a total of 5*numStages values. {b0, b1, b2, -a1, -a2} */
static float eq_state[numStagesEQ*4];       /*The state variables are arranged in the pState array (Buffer)*/
/******************************************************************/
float sample_rate=48000;

void Controls()
{
    hw.ProcessAllControls();

    //footswitch
    // effectOn ^= hw.switches[0].RisingEdge();
    if (hw.switches[Terrarium::FOOTSWITCH_1].RisingEdge())
    {
        effectOn = !effectOn;
        led1.Set(effectOn ? 0.0f : 1.0f);
    }


}
void InitLeds(void)
{
    led1.Init(hw.seed.GetPin(Terrarium::LED_1),true);
    led2.Init(hw.seed.GetPin(Terrarium::LED_2),false);
}

void AudioCallback(AudioHandle::InputBuffer  in,
                   AudioHandle::OutputBuffer out,
                   size_t                    size)
{

    led1.Update();
    led2.Update();
    Controls();

    if(effectOn)
    {

        eq.ProcessBlock(const_cast<float*>(in[0]), out[0], size); 
        memcpy(out[1], in[1], size * sizeof(float)); // limite l'autre canal
    }

}

int main(void)
{
    hw.Init();
    hw.SetAudioBlockSize(48);
    float sample_rate = hw.AudioSampleRate();
    InitLeds();

    effectOn  = true;

    /* INitialize pots */
    //gain.Init(hw.knob[Terrarium::KNOB_2], -6.0f, 6.0f, Parameter::LOGARITHMIC);
    gain.Init(hw.knob[Terrarium::KNOB_2], -12.0f, 12.0f, Parameter::LINEAR);

    /* INitialize IIR buffer */
    eq.SetStateBuffer(eq_state, DSY_COUNTOF(eq_state));
    /* INitialize IIR coefficients */
    float freqEQ[1] = {700}; // Fishman ToneDeq/Mini style
    float gainEQ[1] = {6};
    float qEQ[1] = {0.707};
    float typeIIREQ[1] = {4}; // highpass,paraEQ,paraEQ,ShelfEQ    
    myPedal.UpdateFilter(&freqEQ[0], &gainEQ[0], &qEQ[0], &typeIIREQ[0], sample_rate, numStagesEQ, &iir_coeffs_EQ[0]);
    eq.SetIIR(&iir_coeffs_EQ[0], numStagesEQ);




    hw.StartAdc();
    hw.StartAudio(AudioCallback);
    while(1)
    {
        /* Pots update every 100ms*/
        hw.DelayMs(100); 

        /***********************************************************/
        /* Update IIR filter coefficients*/
        gainEQ[0] = {gain.Process()}; // ok
        myPedal.UpdateFilter(&freqEQ[0], &gainEQ[0], &qEQ[0], &typeIIREQ[0], sample_rate, numStagesEQ, &iir_coeffs_EQ[0]);
        eq.SetIIR(&iir_coeffs_EQ[0], numStagesEQ);
        /***********************************************************/
    }
}
