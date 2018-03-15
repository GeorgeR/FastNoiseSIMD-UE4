#pragma once

#if defined(__AVX512F__)

#include <intrin.h>

namespace FastNoise 
{
	struct FSingle;
	struct FInteger;
	struct FMask;

	const int32 VectorSize = 16;
	const int32 MemoryAlignement = 32;

	struct FMask
	{
	public:

		FORCEINLINE bool operator|(const FMask& InValue) { return ((Value) | (InValue)); }
		FORCEINLINE bool operator&(const FMask& InValue) { return ((Value) & (InValue)); }
		FORCEINLINE bool operator~(const FMask& InValue) { return (~(Value) & (InValue)); }
		FORCEINLINE bool operator~() { return (~(Value)); }

		template <typename T>
		FORCEINLINE T Mask(const T& InValue);

		template <typename T>
		FORCEINLINE T MaskAdd(const T& InLeft, const T& InRight);

		template <typename T>
		FORCEINLINE T MaskSubtract(const T& InLeft, const T& InRight);

	private:
		__mask16 Value;
	};

	template <>
	FORCEINLINE FSingle FMask::Mask(const FSingle& InValue) { return _mm512_maskz_mov_ps(Value, InValue); }

	template <>
	FORCEINLINE FSingle FMask::MaskAdd(const FSingle& InLeft, const FSingle& InRight) { return _mm512_mask_add_ps(InLeft, Value, InLeft, InRight); }

	template <>
	FORCEINLINE FSingle FMask::MaskSubtract(const FSingle& InLeft, const FSingle& InRight) { return _mm512_mask_sub_ps(InLeft, Value, InLeft, InRight); }

	template <>
	FORCEINLINE FInteger FMask::MaskAdd(const FInteger& InLeft, const FInteger& InRight) { return  _mm512_mask_add_epi32(InLeft, Value, InLeft, InRight); }

	template <>
	FORCEINLINE FInteger FMask::MaskSubtract(const FInteger& InLeft, const FInteger& InRight) { return _mm512_mask_sub_epi32(InLeft, Value, InLeft, InRight); }

	struct FSingle
	{
	public:
		void Zero();

		FSingle& operator=(const _m512& InValue);
		operator _m512() const { return Value; }

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

		//FSingle& Gather()  _mm512_i32gather_ps(InRight, InLeft, 4)
		//FSingle& Permute() _mm512_permutexvar_ps(InLeft, InRight)

		FORCEINLINE FSingle MultiplyAdd(const FSingle& InMultiply, const FSingle& InAdd);
		FORCEINLINE FSingle MultiplySubtract(const FSingle& InMultiply, const FSingle& InSubtract);

		static FSingle Convert(const FInteger& InValue);
		static FSingle Cast(const FInteger& InValue);

	private:
		_m512 Value;
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

		FInteger& operator=(const _m512i& InValue);
		operator _m512i() const { return Value; }

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
		_m512i Value;
	};
}

#endif