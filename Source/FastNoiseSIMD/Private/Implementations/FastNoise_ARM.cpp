#include "FastNoise_neon.h"

#if defined(__ARM__)

namespace FastNoise
{
	void FSingle::Zero() { Value = vdupq_n_f32(0); }

	FSingle& FSingle::operator=(const float& InValue) { Value = vdupq_n_f32(InValue); }

	FastNoise::FSingle FSingle::Blend(const FSingle& InLeft, const FSingle& InRight, const FMask& InMask) { return vbslq_f32(vreinterpretq_u32_s32(InMask), InRight, InLeft); }

	void FInteger::Zero() { Value = vdupq_n_s32(0); }

	FInteger& FInteger::operator=(const int32& InValue) { Value = vdupq_n_s32(InValue); }
}

#endif