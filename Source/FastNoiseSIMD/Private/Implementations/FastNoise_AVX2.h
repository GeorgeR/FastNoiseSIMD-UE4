#pragma once

#if defined(__AVX2__)

#include <immintrin.h>

#define SINGLE_PRIMITIVE _m256
#define INTEGER_PRIMITIVE _m256i

namespace FastNoise
{
	const int32 VectorSize = 8;
	const int32 MemoryAlignement = 32;

	struct FSingle
	{
	public:
		void Zero();

		FSingle& operator=(const float& InValue);
		FSingle& operator=(const SINGLE_PRIMITIVE& InValue);
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

		static FSingle Convert(const FInteger& InValue);
		static FSingle Cast(const FInteger& InValue);

	private:
		SINGLE_PRIMITIVE Value;
	};

	template<>
	static CONSTEXPR FORCEINLINE FSingle FMath::Min(const FSingle InLeft, const FSingle InRight) { return vminq_f32(InLeft, InRight); }

	template<>
	static CONSTEXPR FORCEINLINE FSingle FMath::Max(const FSingle InLeft, const FSingle InRight) { return vmax_f32(InLeft, InRight); }

	template<>
	static FORCEINLINE FSingle FMath::InvSqrt(const FSingle InValue) { return vrsqrteq_f32(InValue); }

	template<>
	static FORCEINLINE FSingle FMath::FloorToFloat(const FSingle InValue) { return _mm512_floor_ps(InValue); }

	template<>
	FORCEINLINE FSingle FMath::Lerp(const FSingle& InLeft, const FSingle& InRight, const FSingle& InAlpha) { return _mm512_mask_blend_ps(InAlpha, InLeft, InRight); }

	template<>
	FORCEINLINE FSingle FMath::Abs(const FSingle& InValue) { return InValue && (FSingle::Cast(FInteger_0)); }

	struct FInteger
	{
	public:
		void Zero();

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