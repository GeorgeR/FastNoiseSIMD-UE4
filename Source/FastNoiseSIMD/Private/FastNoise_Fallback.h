#pragma once

#include "CoreMinimal.h"

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
		FSingle& operator=(const FInteger& InValue);
		operator FInteger() const { return Value; }

	private:
		FInteger Value;
	};

	struct FSingle
	{
	public:
		FSingle() { Zero(); }
		FSingle(const float InValue)
			: Value(InValue) { }
		
		void Zero();

		FSingle& operator=(const float& InValue);
		operator float() const { return Value; }

		FSingle& operator+(const FSingle& InValue);
		FSingle& operator-(const FSingle& InValue);
		FSingle& operator*(const FSingle& InValue);
		FSingle& operator/(const FSingle& InValue);

		//FSingle& Gather()  _mm512_i32gather_ps(InRight, InLeft, 4)
		//FSingle& Permute() _mm512_permutexvar_ps(InLeft, InRight)

		static FSingle Convert(const FInteger& InValue);
		static FSingle Cast(const FInteger& InValue);

	private:
		float Value;
	};

	template<>
	static CONSTEXPR FORCEINLINE FSingle FMath::Min(const FSingle InLeft, const FSingle InRight) { return FMath::Min(InLeft, InRight); }

	template<>
	static CONSTEXPR FORCEINLINE FSingle FMath::Max(const FSingle InLeft, const FSingle InRight) { return FMath::Max(InLeft, InRight); }

	template<>
	static FORCEINLINE FSingle FMath::InvSqrt(const FSingle InValue) { return FMath::InvSqrt(InValue); }

	template<>
	static FORCEINLINE FSingle FMath::FloorToFloat(const FSingle InValue) { return FMath::FloorToFloat(InValue); }

	template<>
	FORCEINLINE FSingle FMath::Lerp(const FSingle& InLeft, const FSingle& InRight, const FSingle& InAlpha) { return FMath::Lerp(InLeft, InRight, InAlpha); }

	struct FInteger
	{
	public:
		FInteger() { Zero(); }
		FInteger(const int32 InValue)
			: Value(InValue) { }

		void Zero();

		FInteger& operator=(const int32& InValue);
		operator int32() const { return Value; }

		FInteger& operator+(const FInteger& InValue);
		FInteger& operator-(const FInteger& InValue);
		FInteger& operator*(const FInteger& InValue);

		static FInteger Convert(const FSingle& InValue);
		static FInteger Cast(const FSingle& InValue);

	private:
		int32 Value;
	};

	static FSingle FSingle_1 = 1.0f;
	static FInteger FInteger_0 = 0;


	struct FSingleVector
	{
	public:
		FSingle X;
		FSingle Y;
		FSingle Z;

		FSingleVector(const FSingle InX, const FSingle InY, const FSingle InZ)
			: X(InX), 
			Y(InY),
			Z(InZ) { }

		static FSingleVector Convert(const FIntegerVector& InValue);
		static FSingleVector Cast(const FIntegerVector& InValue);
	};

	struct FIntegerVector
	{
	public:
		FInteger X;
		FInteger Y;
		FInteger Z;

		FIntegerVector(const FInteger InX, const FInteger InY, const FInteger InZ)
			: X(InX),
			Y(InY),
			Z(InZ) { }

		static FIntegerVector Convert(const FSingleVector& InValue);
		static FIntegerVector Cast(const FSingleVector& InValue);
	};
}