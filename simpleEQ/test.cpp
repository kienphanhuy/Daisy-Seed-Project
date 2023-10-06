#include "daisysp.h"
#include "daisy_seed.h"

using namespace daisysp;
using namespace daisy;

static DaisySeed  hw;
static Lowshelf     flt;
//static Oscillator osc, lfo;

static void AudioCallback(AudioHandle::InterleavingInputBuffer  in,
                          AudioHandle::InterleavingOutputBuffer out,
                          size_t                                size)
{
    //float saw, freq, output;
    float freq, output;
    for(size_t i = 0; i < size; i += 2)
    {
        freq = 1000; // + (lfo.Process() * 10000);
        flt.SetFreq(freq);
        flt.SetQ(0.707);
        flt.SetGain(-6);
        float input = in[i];
        output = flt.Process(input);

        // left out
        out[i] = output;

        // right out
        out[i + 1] = output;
    }
}

int main(void)
{
    // initialize seed hardware and daisysp modules
    float sample_rate;
    hw.Configure();
    hw.Init();
    hw.SetAudioBlockSize(4);
    sample_rate = hw.AudioSampleRate();

    // initialize Biquad and set parameters
    flt.Init(sample_rate);
    //flt.SetRes(0.7);
    // flt.SetGain(10);
    // flt.SetFreq(1000);
    // flt.SetQ(0.7);


    // start callback
    hw.StartAudio(AudioCallback);


    while(1) {}
}
