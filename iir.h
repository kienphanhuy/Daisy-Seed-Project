#pragma once
#ifndef DSY_IIRFILTER_H
#define DSY_IIRFILTER_H

#include <cstdint>
#include <cstring> // for memset
#include <cassert>
#include <utility>


#ifdef USE_ARM_DSP
#include <arm_math.h> // required for platform-optimized version
#endif

/**   @brief IIR Biquad Filter implementation, generic and ARM CMSIS DSP based
 *    @author Kien Phan Huy (acousticir.free.fr)
 *  Based on fir.h from Alexander Petrov-Savchenko (axp@soft-amp.com)
 *    @date October 2023
 */

namespace daisysp
{

/* use this as a template parameter to indicate user-provided memory storage */
#define IIRFILTER_USER_MEMORY 0, 0

/** Helper class that defines the memory model - internal or user-provided 
 * \param max_size - maximal filter length
 * \param max_block - maximal length of the block processing
 * if both parameters are 0, does NOT allocate any memory and instead
 * requires user-provided memory blocks to be passed as parameters.
 *
 * Not intended to be used directly, so constructor is not exposed
 */
template <size_t max_size, size_t max_block>

//struct FIRMemory
struct IIRMemory
{
    /* Public part of the API to be passed through to the FIR users */
  public:
    /* Reset the internal filter state (but not the coefficients) */
    void Reset() { memset(state_, 0, state_size_ * sizeof(state_[0])); }


  protected:
  //  FIRMemory() : state_{0}, coefs_{0}, size_(0) {}
    IIRMemory() : state_{0}, coefs_{0}, size_(0) {}   
    /* Expression for the maximum block size */
    static constexpr size_t MaxBlock() { return max_block; }

    /** Configure the filter coefficients
     * \param coefs - pointer to coefficients (tail-first order)
     * \param size - number of coefficients pointed by coefs (filter length)
     * \param reverse - flag to perform reversing of the filter
     * \return true if all conditions are met and the filter is configured
     */
//    bool SetCoefs(const float coefs[], size_t size, bool reverse)
    bool SetCoefs(const float coefs[], size_t size)
    {
        /* truncate silently */
        size_ = DSY_MIN(size, max_size);

            /* just copy as is */
            memcpy(coefs_, coefs, size_ * sizeof(coefs[0]));

        return true;
    }

    static constexpr size_t state_size_ = max_size + max_block - 1u;
    float                   state_[state_size_]; /*< Internal state buffer */
    float                   coefs_[max_size];    /*< Filter coefficients */
    size_t                  size_; /*< Active filter length (<= max_size) */
};

/* Specialization for user-provided memory */
template <>
struct IIRMemory<IIRFILTER_USER_MEMORY>
{
    /* Public part of the API to be passed through to the FIRFilter user */
  public:
    /** Set user-provided state buffer
     * \param state - pointer to the allocated memory block
     * \param length - length of the provided memory block (in elements)
     * The length should be determined as follows 
     * length >= max_filter_size + max_processing_block - 1
     */
    void SetStateBuffer(float state[], size_t length)
    {
        state_      = state;
        state_size_ = length;
    }
    /* Reset the internal filter state (but not the coefficients) */
    void Reset()
    {
        assert(nullptr != state_);
        assert(0 != state_size_);
        if(nullptr != state_)
        {
            memset(state_, 0, state_size_ * sizeof(state_[0]));
        }
    }

  protected:
    IIRMemory() : state_(nullptr), coefs_(nullptr), size_(0), state_size_(0) {}

    /* Expression for the maximum processing block size currently supported */
    size_t MaxBlock() const
    {
        return state_size_ + 1u > size_ ? state_size_ + 1u - size_ : 0;
    }

    /** Configure the filter coefficients
     * \param coefs - pointer to coefficients (tail-first order)
     * \param size - number of filter stages 
     * \param reverse - flag to perform reversing of the filter
     * \return true if all conditions are met and the filter is configured
     */
    //bool SetCoefs(const float coefs[], size_t size, bool reverse)
    bool SetCoefs(const float coefs[], size_t size)
    {
	assert(nullptr != coefs || 0 == size);							 
            coefs_ = coefs;
            size_  = size;

        return false;
    }

    /* Internal member variables */

    float*       state_;      /*< Internal state buffer */
    const float* coefs_;      /*< Filter coefficients */
    size_t       size_;       /*< number of filter coefficients */
    size_t       state_size_; /*< length of the state buffer */
};







#if(defined(USE_ARM_DSP) && defined(__arm__))
/** ARM-specific FIR implementation, expose only on __arm__ platforms
 * \param max_size - maximal filter length
 * \param max_block - maximal block size for ProcessBlock()
 * if both parameters are 0 (via FIRFILTER_USER_MEMORY macro)
 * Assumes the user will provide own memory buffers
 * Otherwise statically allocates the necessary buffers
 */
template <size_t max_size, size_t max_block>
class IIRFilterImplARM : public IIRMemory<max_size, max_block>
{
  private:
    using IIRMem = IIRMemory<max_size, max_block>; // just a shorthand

  public:
    /* Default constructor */
    IIRFilterImplARM() : iir_{0} {}

    /* Reset filter state (but not the coefficients) */
    //using IIRMem::Reset;

    /* FIR Latency is always 0, but API is unified with FFT and FastConv */
    static constexpr size_t GetLatency() { return 0; }

    /* Process one sample at a time */
    float Process(float in)
    {
        //float out(0);
        //arm_fir_f32(&fir_, &in, &out, 1);
        //arm_biquad_cascade_df1_f32(&iir_, &in, &out, 1);
        float out = in; // ça marche presque (haute frequence bug)
        return out;
    }

    /* Process a block of data */
    
    void ProcessBlock(float* pSrc, float* pDst, size_t block)
    //void ProcessBlock(const float* pSrc, float* pDst, size_t block)
    {
        //assert(block <= IIRMem::MaxBlock());
        //arm_fir_f32(&fir_, pSrc, pDst, block);
        //arm_biquad_cascade_df1_f32 (const arm_biquad_casd_df1_inst_f32 *S, const float32_t *pSrc, float32_t *pDst, uint32_t blockSize)
        
        arm_biquad_cascade_df1_f32(&iir_, pSrc, pDst, block);
        
        //memcpy(pDst, pSrc, block* sizeof(float));
    }

    /** Set filter coefficients (aka Impulse Response)
     * Coefficients need to be in reversed order (tail-first)
     * If internal storage is used, makes a local copy
     * and allows reversing the impulse response
     */
    bool SetIIR(const float* ir, size_t numStages)
    {
        /* Function order is important */
        const bool result = IIRMem::SetCoefs(ir, numStages);

        //arm_fir_init_f32(&fir_, len, (float*)coefs_, state_, max_block);
        //arm_biquad_cascade_df1_init_f32 (arm_biquad_casd_df1_inst_f32 *S, uint8_t numStages, const float32_t *pCoeffs, float32_t *pState)
        arm_biquad_cascade_df1_init_f32(&iir_, numStages, (float*)coefs_, state_);
        return result;
    }

    /* Create an alias to comply with DaisySP API conventions */
    template <typename... Args>
    inline auto Init(Args&&... args)
        -> decltype(SetIIR(std::forward<Args>(args)...))
    {
        return SetIR(std::forward<Args>(args)...);
    }

  protected:
    arm_biquad_casd_df1_inst_f32 iir_; /*< ARM CMSIS DSP library IIR filter instance */ 
    //arm_fir_instance_f32 fir_; /*< ARM CMSIS DSP library FIR filter instance */
    using IIRMem::coefs_;      /*< IIR coefficients buffer or pointer */
    using IIRMem::size_;       /*< IIR length*/
    using IIRMem::state_;      /*< IIR state buffer or pointer */
};


/* default to ARM implementation */
template <size_t max_size, size_t max_block>
using IIR = IIRFilterImplARM<max_size, max_block>;


//  #else // USE_ARM_DSP

// /* default to generic implementation */
// template <size_t max_size, size_t max_block>
// using IIR = IIRFilterImplGeneric<max_size, max_block>;

#endif // USE_ARM_DSP 


} // namespace daisysp

#endif // DSY_IIRFILTER_H
