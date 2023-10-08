/**   @brief IIR Filter example
 *    @author Kien Phan Huy (acousticir.free.fr)
 *    @date October 2023
 */

#include "daisysp.h"
#include "daisy_seed.h"
#include <arm_math.h> // required for platform-optimized version

using namespace daisysp;
using namespace daisy;

static DaisySeed                  hw;

void AudioCallback(AudioHandle::InputBuffer  in,
                   AudioHandle::OutputBuffer out,
                   size_t                    size)
{    
    float gain=5; //linear voltage gain (not in dB)
    arm_scale_f32(const_cast<float*>(in[0]), gain, out[0], size);


}

int main(void)
{
    /* initialize seed hardware and daisysp modules */
    //float sample_rate;
    hw.Configure();
    hw.Init();
    hw.SetAudioBlockSize(48); // Number of samples per bloc

    hw.StartLog(false);

    /* start audio callback */
    hw.StartAudio(AudioCallback);

    while(1)
    {
    }
}
