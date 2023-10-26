/**   @brief IIR Filter example
 *    @author Kien Phan Huy (acousticir.free.fr)
 *  BAsed on ext_FIR.cpp from Alexander Petrov-Savchenko (axp@soft-amp.com)
 *    @date October 2023
 */

#include "daisysp.h"
#include "daisy_seed.h"
#include "pedale.h"
#include <math.h>
using namespace daisysp;
using namespace daisy;
pedale myPedal;


static DaisySeed                  hw;
static IIR<IIRFILTER_USER_MEMORY> flt;
static FIR<FIRFILTER_USER_MEMORY> flt2; // QUackless IR
static FIR<FIRFILTER_USER_MEMORY> flt3; // Miclike IR
static IIR<IIRFILTER_USER_MEMORY> HPF;
static IIR<IIRFILTER_USER_MEMORY> LPF;
IIR<IIRFILTER_USER_MEMORY> EQ; // Tonestack EQ

/******************************************************************/
bool  passThruOn; // When true 'bypass' is on
//Configure and initialize button
Switch FSW1; // Footswitches are D25-26
Switch T1; // Mono input / Dual source
Switch T2; // Toogle switches are D7-10 IR
Switch T3; // Toogle switches are D7-10 Shape EQ
bool IR = false;
bool EQshape = false;
bool DS = false;
// Create an LED - Terrarium
//GPIO led1;
GPIO led1;
/******************************************************************/
//Setting Struct containing parameters we want to load/save to flash
struct Settings {
	float p1[2048]; // IR1
	float p2[1024]; // IR2
    /* Generated with AutoIR_load_txt2.m matlab code */

	//Overloading the != operator
	//This is necessary as this operator is used in the PersistentStorage source code
	bool operator!=(const Settings& a) const {
        return !(a.p1==p1 && a.p2==p2);
    }
};
//Persistent Storage Declaration. Using type Settings and passed the devices qspi handle
//PersistentStorage<Settings> SavedSettings(hw.seed.qspi);
PersistentStorage<Settings> SavedSettings(hw.qspi);

float ir_front[1024]={0};
float ir_front0[2048]={0};
bool use_preset = false;
/*************************************************************************/
void Load() {

	//Reference to local copy of settings stored in flash
	Settings &LocalSettings = SavedSettings.GetSettings();
	memcpy(&ir_front0, &LocalSettings.p1, 2048 * sizeof(float));
    memcpy(&ir_front, &LocalSettings.p2, 1024 * sizeof(float));
	//state1 = LocalSettings.p1;

	use_preset = true;
}
/******************************************************************/
/*All information available on the CMSIS-DSP librairy at 
https://www.keil.com/pack/doc/CMSIS/DSP/html/group__BiquadCascadeDF1.html*/
//static constexpr size_t numStages    = 2;  /*< Number of EQ Stages */
//static constexpr size_t numStages    = 1;  /*< Number of EQ Stages */
static constexpr size_t numStages    = 17;  /*< Number of EQ Stages */
static float iir_coeffs[numStages*5]; /*The pCoeffs array contains a total of 5*numStages values. {b0, b1, b2, -a1, -a2} */
static float flt_state[numStages*4];       /*The state variables are arranged in the pState array (Buffer)*/
/******************************************************************/
static constexpr size_t numStagesLPF    = 2;  /*< Number of EQ Stages */
static float iir_LPF[numStagesLPF*5]; /*The pCoeffs array contains a total of 5*numStages values. {b0, b1, b2, -a1, -a2} */
static float LPF_state[numStagesLPF*4];       /*The state variables are arranged in the pState array (Buffer)*/
/******************************************************************/
static constexpr size_t numStagesHPF    = 2;  /*< Number of EQ Stages */
static float iir_HPF[numStagesHPF*5]; /*The pCoeffs array contains a total of 5*numStages values. {b0, b1, b2, -a1, -a2} */
static float HPF_state[numStagesHPF*4];       /*The state variables are arranged in the pState array (Buffer)*/
/******************************************************************/
// Tonestack EQ stage
static constexpr size_t numStagesEQ    = 4;  /*< Number of EQ Stages */
static float iir_EQ[numStagesEQ*5]; /*The pCoeffs array contains a total of 5*numStages values. {b0, b1, b2, -a1, -a2} */
static float EQ_state[numStagesEQ*4];       /*The state variables are arranged in the pState array (Buffer)*/
float freqEQ[4] = {50,100,700,6500}; // Fishman ToneDeq/Mini style
float gainEQ[4] = {1,0,0,0};
float qEQ[4] = {0.707,1,1,0.707};
float typeIIREQ[4] = {1,4,4,3}; // highpass,paraEQ,paraEQ,ShelfEQ 1,4,4,3   
/******************************************************************/
// FIR filter
static constexpr size_t flt_size    = 1024;  /*< FIR filter length */
static constexpr size_t flt_size3    = 2048;  /*< FIR filter length */
//static float ir_front[flt_size]    = {0};   /*< Active FIR coefficients */
static float flt_state2[flt_size + 48-1];       /*< Impl-specific storage */
static float flt_state3[flt_size3 + 48-1];       /*< Impl-specific storage */
/******************************************************************/
float* pbuffer = new float[48];
float* pbuffer0 = new float[48];
float* pbuffer1 = new float[48];
float* pbuffer2 = new float[48];
float* pbuffer3 = new float[48];
/******************************************************************/
float* pbufBLD1 = new float[48];
float* pbufBLD2 = new float[48];
float* pbufBLDo = new float[48];
float* pbufHPF = new float[48];
float* pbufLPF = new float[48];
float* pbufEQ = new float[48];
float* pbuf0 = new float[48];
/******************************************************************/
// Debounce pots
bool updated_;
bool last_update_;
/******************************************************************/
void ProcessControls() {

        FSW1.Debounce(); // Debounce FSW1
    if (FSW1.RisingEdge())
    {
        passThruOn = !passThruOn; // Toggle the FX
        led1.Toggle(); // Toggle the LED2
    }

        

}


void AudioCallback(AudioHandle::InputBuffer  in,
                   AudioHandle::OutputBuffer out,
                   size_t                    size)
{    
    ProcessControls(); //This makes sure all the moveable parameters are up to date (knobs, switches, leds, etc)
    if(passThruOn){
        memcpy(out, in, size * sizeof(float)); // bypass
        //memcpy(out[0], in[0], size * sizeof(float)); // bypass
        //arm_scale_f32(const_cast<float*>(in[0]), 1, out[0], size);   // balanced outpyt
        //arm_scale_f32(const_cast<float*>(in[0]), -1, out[1], size);   // balanced outpyt
    }
    else
    {
        // /******************************************************************/
        /* EQ Shape (only for DRY) */
        if (EQshape==true)
        {
            flt.ProcessBlock(const_cast<float*>(in[0]), pbuffer, size); // EQ
        }else
        {
            memcpy(pbuffer, in[0], size * sizeof(float)); // limite l'autre canal
        }
        /* Choose IR (true is miclike / false is quackless) */
        if (IR==true)
        {
            float val=0.3;
            arm_scale_f32(pbuffer, val, pbuf0, size);  // weight IR OK
            flt3.ProcessBlock(pbuf0, pbuffer3, size); //FIR miclike
            //flt3.ProcessBlock(pbuffer, pbuffer3, size); //FIR miclike
        }else
        {
            flt2.ProcessBlock(pbuffer, pbuffer3, size); //FIR quackless
        }
        
        //flt3.ProcessBlock(pbuffer, pbuffer3, size); //FIR mic-like

        float value2 = hw.adc.GetFloat(0); // get Blend pot value
        arm_scale_f32(const_cast<float*>(in[0]), 1-value2, pbuffer1, size);   // weight Pickup OK
        arm_scale_f32(pbuffer3, value2, pbuffer2, size);  // weight IR OK
        //arm_add_f32(pbuffer1,pbuffer2,out[0], size); // Blend both OK                
        arm_add_f32(pbuffer1,pbuffer2,pbuffer0, size); // Blend both OK                
        /*********************************************************************/
        if (DS==true)
        {
            /********************************************************/
            // ABOVE 250 HZ
            float value = hw.adc.GetFloat(1); // get Blend pot value
            float gain=2.86; //2.36
            arm_scale_f32(pbuffer0, (1-value), pbufBLD1, size);   // weight Pickup OK
            arm_scale_f32(const_cast<float*>(in[1]), value*gain, pbufBLD2, size);  // weight Mic OK
            //arm_scale_f32(pbuffer0, -(1-value), pbufBLD1, size);   // weight Pickup OK PHASE INVERSE
            //arm_scale_f32(const_cast<float*>(in[1]), -value*gain, pbufBLD2, size);  // weight Mic OK PHASE INVERSE
            arm_add_f32(pbufBLD1,pbufBLD2,pbufBLDo, size); // Blend both OK                
            HPF.ProcessBlock(pbufBLDo, pbufHPF, size); //HPF 250Hz Mic+Pickup
            //HPF.ProcessBlock(const_cast<float*>(in[0]), out[0], size); //HPF 250Hz Mic+Pickup
            /********************************************************/
            // BELOW 250 HZ
            LPF.ProcessBlock(pbuffer0, pbufLPF, size); //HPF 250Hz Mic+Pickup
            ////LPF.ProcessBlock(const_cast<float*>(in[0]), out[0], size); //HPF 250Hz Mic+Pickup
            /********************************************************/
            arm_add_f32(pbufHPF,pbufLPF,pbufEQ, size); // Xover   
        }else
        {
            memcpy(pbufEQ, pbuffer0, size * sizeof(float)); // limite l'autre canal    
        }
        EQ.ProcessBlock(pbufEQ, out[0], size); //EQ
        memcpy(out[1], in[1], size * sizeof(float)); // limite l'autre canal


    }
}

int main(void)
{
    /* initialize seed hardware and daisysp modules */
    float sample_rate;
    hw.Configure();
    hw.Init();
    hw.SetAudioBlockSize(8); // Number of samples per bloc
    sample_rate = hw.AudioSampleRate();

    /******************************************************/
    // Led 1 and footwitch 1
    // Initialize led1 to pin D22 as an OUTPUT
    led1.Init(daisy::seed::D22, daisy::GPIO::Mode::OUTPUT);
    
    /******************************************************/
    //Set button to pin D26, to be updated at a 1kHz  samplerate
    FSW1.Init(daisy::seed::D26, 1000,daisy::Switch::TYPE_MOMENTARY,daisy::Switch::POLARITY_INVERTED,daisy::Switch::PULL_NONE);
    passThruOn = true;// This starts the pedal in the 'off' 
    // Set the pin LOW
    led1.Write(false);
    /******************************************************/
    T1.Init(daisy::seed::D10, 1000,daisy::Switch::TYPE_TOGGLE,daisy::Switch::POLARITY_NORMAL,daisy::Switch::PULL_NONE);
    T2.Init(daisy::seed::D9, 1000,daisy::Switch::TYPE_TOGGLE,daisy::Switch::POLARITY_NORMAL,daisy::Switch::PULL_NONE);
    T3.Init(daisy::seed::D8, 1000,daisy::Switch::TYPE_TOGGLE,daisy::Switch::POLARITY_NORMAL,daisy::Switch::PULL_NONE);
    /******************************************************/
    //This is our ADC configuration
    AdcChannelConfig adcConfig[6];
    //Configure pin A1-A6 as an ADC input. This is where we'll read the knob.
    // Les 3 potards du haut (de G->D): A1,A2,A3
    // Les 3 potards du bas (de G->D): A4,A5,A6
    //adcConfig.InitSingle(daisy::seed::A1);
    adcConfig[0].InitSingle(daisy::seed::A2); // Blend IR
    adcConfig[1].InitSingle(daisy::seed::A3); // Blend Mic
    adcConfig[2].InitSingle(daisy::seed::A1); // HPF
    adcConfig[3].InitSingle(daisy::seed::A4); // Bass
    adcConfig[4].InitSingle(daisy::seed::A5); // Mid
    adcConfig[5].InitSingle(daisy::seed::A6); // Treble
    //Initialize the adc with the config we just made
    //hw.adc.Init(&adcConfig, 1);
    hw.adc.Init(adcConfig, 6);
    //Start reading values
    hw.adc.Start();
    /******************************************************/

    hw.StartLog(false);

    /******************************************************/
    /** initialize IIR biquad filter and set parameters      */
    flt.SetStateBuffer(flt_state, DSY_COUNTOF(flt_state));
    myPedal.UpdateFilter(sample_rate, numStages, &iir_coeffs[0]);
    flt.SetIIR(&iir_coeffs[0], numStages);
    /*****************************************************************/
    //Initilize the PersistentStorage Object with default values.
	//Defaults will be the first values stored in flash when the device is first turned on. They can also be restored at a later date using the RestoreDefaults method
	Settings DefaultSettings = {0.0f, 0.0f};
	SavedSettings.Init(DefaultSettings);
    //ir_front[1023]=0.95;
    Load(); // load IRs form QSPI flash
    /******************************************************/
    /** initialize FIR filter and set parameters      */
    flt2.SetStateBuffer(flt_state2, DSY_COUNTOF(flt_state2)); // FIR quackless 1024
    flt3.SetStateBuffer(flt_state3, DSY_COUNTOF(flt_state3)); // FIR mic-like 2048
    //myPedal.UpdateFilter2(&ir_front[0]);
    flt2.SetIR(&ir_front[0], flt_size, false); // FIR quackless 1024
    flt3.SetIR(&ir_front0[0], flt_size3, false); // FIR mic-like 2048
    /******************************************************/
    /** initialize LPF IIR      */
    LPF.SetStateBuffer(LPF_state, DSY_COUNTOF(LPF_state));
    myPedal.UpdateFilterLPF(sample_rate, numStagesHPF, &iir_LPF[0]);
    LPF.SetIIR(&iir_LPF[0], numStagesLPF);
    /** initialize HPF IIR      */
    HPF.SetStateBuffer(HPF_state, DSY_COUNTOF(HPF_state));
    myPedal.UpdateFilterHPF(sample_rate, numStagesLPF, &iir_HPF[0]);
    HPF.SetIIR(&iir_HPF[0], numStagesHPF);
    /*****************************************************************/
    EQ.SetStateBuffer(EQ_state, DSY_COUNTOF(EQ_state));
    myPedal.UpdateFilterEQ(&freqEQ[0], &gainEQ[0], &qEQ[0], &typeIIREQ[0], sample_rate, numStagesEQ, &iir_EQ[0]);
    EQ.SetIIR(&iir_EQ[0], numStagesEQ);
    /*****************************************************************/
    /* start audio callback */
    hw.StartAudio(AudioCallback);
    float valf = 50;
    float gainB=0;
    float gainM=0;
    float gainT=0;
    while(1)
    {
        
        hw.DelayMs(100); // wait for 100ms
        /*************************************************************************/
        // Potentiometers
        valf = (hw.adc.GetFloat(2)*hw.adc.GetFloat(2) * (500.0f - 10.0f)) + 10.0f;
        gainB= -12.0f+hw.adc.GetFloat(3)*24.0f;
        gainM= -12.0f+hw.adc.GetFloat(4)*24.0f;
        gainT= -12.0f+hw.adc.GetFloat(5)*24.0f;
        if ((abs((valf - freqEQ[0]) / freqEQ[0]) > 0.05) ||
        (abs((gainB - gainEQ[1]) / gainEQ[1]) > 0.05) ||
        (abs((gainM - gainEQ[2]) / gainEQ[2]) > 0.05) ||
        (abs((gainT - gainEQ[3]) / gainEQ[3]) > 0.05))
        {
            freqEQ[0]=valf;
            gainEQ[1]=-12.0f+hw.adc.GetFloat(3)*24.0f;
            gainEQ[2]=-12.0f+hw.adc.GetFloat(4)*24.0f;
            gainEQ[3]=-12.0f+hw.adc.GetFloat(5)*24.0f;
            myPedal.UpdateFilterEQ(&freqEQ[0], &gainEQ[0], &qEQ[0], &typeIIREQ[0], 48000.0f, numStagesEQ, &iir_EQ[0]);
            EQ.SetIIR(&iir_EQ[0], numStagesEQ);
        }
        /*************************************************************************/
        // Toggle Switches
        T1.Debounce(); // Debounce T1
        T2.Debounce(); // Debounce T2
        T3.Debounce(); // Debounce T3
        if (T1.Pressed())
        {
            DS=true; // DUal source
        }
        else
        {
            DS=false; // MOno source
        }
        if (T2.Pressed())
        {
            IR=true; // Mic-like
        }
        else
        {
            IR=false; // Quackless 
        }
         if (T3.Pressed())
        {
            EQshape=true; //EQ ON
        }
        else
        {
            EQshape=false; // EQshape off
        }
    }
}
