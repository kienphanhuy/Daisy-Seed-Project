# Daisy-Seed-Project
Programs for Electrosmith Daisy Seed from Kien Phan Huy
-----------------------------------------------------------------------
EQ for Daisy Seed processed by CMSIS-DSP biquad using bloc processing
Thanks to 
arm_biquad_cascade_df1_f32
arm_biquad_cascade_df1_init_f32

The code is based on Alexander Petrov-Savchenko original fir filter code
and uses formulae from Robert Bristow-Johnson Cookbook
-----------------------------------------------------------------------
numStages: number of EQ stages
typeIIR:
* 0 lowpass
* 1 highpass
* 2 lowshelf
* 3 highshelf
* 4 Parametric EQ (peak)
* 5 Notch
    
For each stages you can set 
* frequency (Hz)
* Q-factor
* Gain (dB)
* typeIIR (see above)

