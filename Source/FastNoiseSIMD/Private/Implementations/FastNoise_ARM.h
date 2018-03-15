#pragma once

#if defined(__ARM__)

#include <arm_neon.h>

#define SINGLE_PRIMITIVE float32x4_t
#define INTEGER_PRIMITIVE int32x4_t

namespace FastNoise
{
	const int32 VectorSize = 4;
	const int32 MemoryAlignement = 16;

	struct FSingle
	{
	public:
		void Zero();
		FSingle() { Zero(); }
		FSingle(const float InValue) : Value(InValue) { }
		FSingle(const SINGLE_PRIMITIVE InValue)	: Value(InValue) { }

		FSingle& operator=(const float& InValue);
		FSingle& operator=(const SINGLE_PRIMITIVE& InValue) { Value = InValue; }
		operator SINGLE_PRIMITIVE() const { return Value; }

		FORCEINLINE FSingle& operator+(const FSingle& InValue);
		FORCEINLINE FSingle& operator-(const FSingle& InValue);
		FORCEINLINE FSingle& operator*(const FSingle& InValue);
		FORCEINLINE FSingle& operator/(const FSingle& InValue);

		FORCEINLINE bool operator==(const FSingle& InValue);
		FORCEINLINE bool operator!=(const FSingle& InValue);
		FORCEINLINE bool operator<(const FSingle& InValue);
		FORCEINLINE bool operator>(const FSingle& InValue);
		FORCEINLINE bool operator<=(const FSingle& InValue);
		FORCEINLINE bool operator>=(const FSingle& InValue);

		FORCEINLINE FSingle MultiplyAdd(const FSingle& InMultiply, const FSingle& InAdd);
		FORCEINLINE FSingle MultiplySubtract(const FSingle& InMultiply, const FSingle& InSubtract);

		static FSingle Blend(const FSingle& InLeft, const FSingle& InRight, const FMask& InMask);
		static FSingle Convert(const FInteger& InValue);
		static FSingle Cast(const FInteger& InValue);

	private:
		SINGLE_PRIMITIVE Value;
	};

	template<>
	static CONSTEXPR FORCEINLINE FSingle FMath::Min(const FSingle InLeft, const FSingle InRight) { return vminq_f32(InLeft, InRight); }

	template<>
	static CONSTEXPR FORCEINLINE FSingle FMath::Max(const FSingle InLeft, const FSingle InRight) { return vmaxq_f32(InLeft, InRight); }

	template<>
	static FORCEINLINE FSingle FMath::InvSqrt(const FSingle InValue) { return vrsqrteq_f32(InValue); }

	template<>
	static FORCEINLINE FSingle FMath::FloorToFloat(const FSingle InValue) { return vrndmq_f32(InValue); }

	template<>
	FORCEINLINE FSingle FMath::Lerp(const FSingle& InLeft, const FSingle& InRight, const FSingle& InAlpha) { return _mm512_mask_blend_ps(InAlpha, InLeft, InRight); }

	template<>
	FORCEINLINE FSingle FMath::Abs(const FSingle& InValue) { return InValue && (FSingle::Cast(FInteger_0)); }

	struct FInteger
	{
	public:
		void Zero();
		FInteger(const int32& InValue) : Value(InValue) { }
		FInteger(const INTEGER_PRIMITIVE& InValue) : Value(InValue) { }

		FInteger& operator=(const int32& InValue);
		FInteger& operator=(const INTEGER_PRIMITIVE& InValue);
		operator INTEGER_PRIMITIVE() const { return Value; }

		FORCEINLINE FInteger& operator+(const FInteger& InValue);
		FORCEINLINE FInteger& operator-(const FInteger& InValue);
		FORCEINLINE FInteger& operator*(const FInteger& InValue);

		FORCEINLINE FInteger& operator>>(const FInteger& InValue);
		FORCEINLINE FInteger& operator<<(const FInteger& InValue);

		FORCEINLINE bool operator==(const FInteger& InValue);
		FORCEINLINE bool operator!=(const FInteger& InValue);
		FORCEINLINE bool operator<(const FInteger& InValue);
		FORCEINLINE bool operator>(const FInteger& InValue);
		FORCEINLINE bool operator<=(const FInteger& InValue);
		FORCEINLINE bool operator>=(const FInteger& InValue);

		static FInteger Convert(const FSingle& InValue);
		static FInteger Cast(const FSingle& InValue);

	private:
		INTEGER_PRIMITIVE Value;
	};
}

#endif