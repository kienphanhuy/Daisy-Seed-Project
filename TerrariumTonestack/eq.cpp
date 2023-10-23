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
Parameter freq;
Parameter gainB;
Parameter gainM;
Parameter gainT;
static IIR<IIRFILTER_USER_MEMORY> eq; // Tonestack EQ

bool  effectOn;
bool  muteOn;
Led led1, led2;

// /******************************************************************/
static constexpr size_t numStagesEQ    = 4;  /*< Number of EQ Stages */

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
        muteOn = !muteOn;
        led2.Set(muteOn ? 0.0f : 1.0f);
    }
    if (hw.switches[Terrarium::FOOTSWITCH_2].RisingEdge())
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
    else
    {
        memcpy(out, in, size * sizeof(float)); // limite l'autre canal
    }

}

int main(void)
{
    hw.Init();
    hw.SetAudioBlockSize(48);
    float sample_rate = hw.AudioSampleRate();
    InitLeds();

    effectOn  = true;
    muteOn  = false;

    /* INitialize pots */
    freq.Init(hw.knob[Terrarium::KNOB_1], 10.0f, 500.0f, Parameter::LOGARITHMIC);
    gainB.Init(hw.knob[Terrarium::KNOB_4], -12.0f, 12.0f, Parameter::LINEAR);
    gainM.Init(hw.knob[Terrarium::KNOB_5], -12.0f, 12.0f, Parameter::LINEAR);
    gainT.Init(hw.knob[Terrarium::KNOB_6], -12.0f, 12.0f, Parameter::LINEAR);

    /* INitialize IIR buffer */
    eq.SetStateBuffer(eq_state, DSY_COUNTOF(eq_state));
    /* INitialize IIR coefficients */
    /* typeIIR type of filter */
    // 0: lowpass, 1: highpass
    // 2: lowshelf, 3: highshelf
    // 4: Parametric EQ (peak), 5: Notch     
    float freqEQ[4] = {50,100,700,6500}; // Fishman ToneDeq/Mini style
    float gainEQ[4] = {1,0,0,0};
    float qEQ[4] = {0.707,1,1,0.707};
    float typeIIREQ[4] = {1,4,4,3}; // highpass,paraEQ,paraEQ,ShelfEQ 1,4,4,3   
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
        //gainEQ[0] = {gain.Process()}; // ok
        freqEQ[0] = freq.Process(); // ok
        gainEQ[1] = gainB.Process(); // ok
        gainEQ[2] = gainM.Process(); // ok
        gainEQ[3] = gainT.Process(); // ok
        myPedal.UpdateFilter(&freqEQ[0], &gainEQ[0], &qEQ[0], &typeIIREQ[0], sample_rate, numStagesEQ, &iir_coeffs_EQ[0]);
        eq.SetIIR(&iir_coeffs_EQ[0], numStagesEQ);
        /***********************************************************/
    }
}
