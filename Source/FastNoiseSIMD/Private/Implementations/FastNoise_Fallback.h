#pragma once

#include "CoreMinimal.h"

#define SINGLE_PRIMITIVE float
#define INTEGER_PRIMITIVE int32

namespace FastNoise
{
	struct FMask;
	struct FSingle;
	struct FInteger;

	struct FMaskVector;
	struct FSingleVector;
	struct FIntegerVector;
	
	const int32 VectorSize = 16;
	const int32 MemoryAlignement = 32;

#define MASK_PRIMITIVE FInteger

	struct FMask
	{
	public:
		FMask(const MASK_PRIMITIVE InValue) : Value(InValue) { }

		FMask& operator=(const MASK_PRIMITIVE& InValue);
		operator MASK_PRIMITIVE() const { return Value; }
		operator bool() const { return Value > 0; }

		FORCEINLINE FMask operator|(const FMask& InOther);
		FORCEINLINE FMask operator&(const FMask& InOther);
		FORCEINLINE bool operator!();

		FORCEINLINE static FSingle Mask(const FSingle& InLeft, const FSingle& InRight, const FMask& InMask);
		FORCEINLINE static FSingle Add(const FSingle& InLeft, const FSingle& InRight, const FMask& InMask);
		FORCEINLINE static FSingle Subtract(const FSingle& InLeft, const FSingle& InRight, const FMask& InMask);

	private:
		MASK_PRIMITIVE Value;
	};

	struct FSingle
	{
	public:
		FSingle() { Zero(); }
		FSingle(const float InValue) : Value(InValue) { } // Set
		FSingle(const SINGLE_PRIMITIVE InValue)	: Value(InValue) { } // Set
		
		FORCEINLINE void Zero(); // Set Zero, Undefined

		FORCEINLINE FSingle& Store(TArray<float>& InValues); // Store
		FORCEINLINE const TArray<float> Load(); // Load

		FORCEINLINE FSingle& operator=(const float& InValue);
		FORCEINLINE FSingle& operator=(const SINGLE_PRIMITIVE& InValue);
		FORCEINLINE operator SINGLE_PRIMITIVE() const { return Value; }

		FORCEINLINE FSingle operator+(const FSingle& InOther);
		FORCEINLINE FSingle operator-(const FSingle& InOther);
		FORCEINLINE FSingle operator*(const FSingle& InOther);
		FORCEINLINE FSingle operator/(const FSingle& InOther);

		FORCEINLINE bool operator<(const FSingle& InOther);
		FORCEINLINE bool operator>(const FSingle& InOther);
		FORCEINLINE bool operator<=(const FSingle& InOther);
		FORCEINLINE bool operator>=(const FSingle& InOther);

		FORCEINLINE bool operator&(const FSingle& InOther);
		FORCEINLINE bool operator^(const FSingle& InOther);

		FORCEINLINE static FSingle Convert(const FInteger& InValue); // Convert
		FORCEINLINE static FSingle Cast(const FInteger& InValue); // Cast

		FORCEINLINE static FSingle Min(const FSingle& InLeft, const FSingle& InRight);
		FORCEINLINE static FSingle Max(const FSingle& InLeft, const FSingle& InRight);
		FORCEINLINE FSingle InverseSqrt();
		FORCEINLINE FSingle Floor();
		FORCEINLINE FSingle Abs();
		FORCEINLINE static FSingle Blend(const FSingle& InLeft, const FSingle& InRight, const FMask& InMask);

		FORCEINLINE FSingle MultiplyAdd(const FSingle& InMultiply, const FSingle& InAdd);
		FORCEINLINE FSingle MultiplySubtract(const FSingle& InMultiply, const FSingle& InSubtract);
		FORCEINLINE FSingle MultiplyAddNegated(const FSingle& InMultiply, const FSingle& InAdd);

		FORCEINLINE FSingle Gather();
		FORCEINLINE FSingle Permute();

	private:
		SINGLE_PRIMITIVE Value;
	};

	struct FInteger
	{
	public:
		FORCEINLINE FInteger() { Zero(); }
		FORCEINLINE FInteger(const int32& InValue) : Value(InValue) { } // Set
		FORCEINLINE FInteger(const INTEGER_PRIMITIVE& InValue) : Value(InValue) { }

		FORCEINLINE void Zero(); // Set Zero, Undefined

		FORCEINLINE FInteger& operator=(const int32& InValue); // Set
		FORCEINLINE FInteger& operator=(const INTEGER_PRIMITIVE& InValue);
		FORCEINLINE operator INTEGER_PRIMITIVE() const { return Value; }

		FORCEINLINE FInteger operator+(const FInteger& InOther);
		FORCEINLINE FInteger operator-(const FInteger& InOther);
		FORCEINLINE FInteger operator*(const FInteger& InOther);

		FORCEINLINE bool operator<(const FInteger& InOther);
		FORCEINLINE bool operator>(const FInteger& InOther);

		FORCEINLINE bool operator&(const FInteger& InOther);
		FORCEINLINE bool operator|(const FInteger& InOther);
		FORCEINLINE bool operator^(const FInteger& InOther);
		FORCEINLINE bool operator!();
		FORCEINLINE bool operator==(const FInteger& InOther);

		FORCEINLINE FInteger operator>>(const FInteger& InOther);
		FORCEINLINE FInteger operator<<(const FInteger& InOther);

		FORCEINLINE static FInteger Convert(const FSingle& InValue);
		FORCEINLINE static FInteger Cast(const FSingle& InValue);

	private:
		INTEGER_PRIMITIVE Value;
	};

	static FSingle FSingle_1 = 1.0f;
	static FInteger FInteger_0 = 0;

	struct FMaskVector
	{
	public:
		FMask X;
		FMask Y;
		FMask Z;

		FSingleVector(const FMask InX, const FMask InY, const FMask InZ)
			: X(InX),
			Y(InY),
			Z(InZ) { }
	};

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