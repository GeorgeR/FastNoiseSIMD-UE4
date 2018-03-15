#include "FastNoise_Fallback.h"

namespace FastNoise
{
#pragma region FMask
	FMask FMask::operator|(const FMask& InOther) { return Value | InOther; }
	FMask FMask::operator&(const FMask& InOther) { return Value & InOther; }
	bool FMask::operator!() { return !Value; }
	FSingle FMask::Add(const FSingle& InLeft, const FSingle& InRight, const FMask& InMask) { return InLeft + (InMask & InRight); }
	FSingle FMask::Subtract(const FSingle& InLeft, const FSingle& InRight, const FMask& InMask) { return InLeft - (InMask & InRight); }
#pragma endregion FMask

#pragma region FSingle
	void FSingle::Zero() { Value = 0.0f; }

	FSingle& FSingle::Store(TArray<float>& InValues) { Value = InValues[0]; }
	const TArray<float> FSingle::Load()	{ return { Value };	}

	FSingle& FSingle::operator=(const float& InValue) { Value = InValue; return *this; }
	FSingle& FSingle::operator=(const SINGLE_PRIMITIVE& InValue) { Value = InValue; return *this; }

	FSingle FSingle::operator+(const FSingle& InOther) { return Value + InOther; }
	FSingle FSingle::operator-(const FSingle& InOther) { return Value - InOther; }
	FSingle FSingle::operator*(const FSingle& InOther) { return Value * InOther; }
	FSingle FSingle::operator/(const FSingle& InOther) { return Value / InOther; }

	bool FSingle::operator<(const FSingle& InOther) { return Value < InOther; }
	bool FSingle::operator>(const FSingle& InOther) { return Value > InOther; }
	bool FSingle::operator<=(const FSingle& InOther) { return Value >= InOther; }
	bool FSingle::operator>=(const FSingle& InOther) { return Value <= InOther; }

	bool FSingle::operator&(const FSingle& InOther) { return FInteger::Cast(Value) & FInteger::Cast(InOther); }
	bool FSingle::operator^(const FSingle& InOther) { return FInteger(Value) ^ FInteger(InOther); }

	FSingle FSingle::Convert(const FInteger& InValue) { return (float)(InValue); }
	FSingle FSingle::Cast(const FInteger& InValue) { return (float)(InValue); }

	FSingle FSingle::Min(const FSingle& InLeft, const FSingle& InRight) { return FMath::Min(InLeft, InRight); }
	FSingle FSingle::Max(const FSingle& InLeft, const FSingle& InRight) { return FMath::Max(InLeft, InRight); }
	FSingle FSingle::InverseSqrt() { return FMath::InvSqrt(Value); }
	FSingle FSingle::Floor() { return FMath::FloorToFloat(Value); }
	FSingle FSingle::Abs() { return FMath::Abs(Value); }
	FSingle FSingle::Blend(const FSingle& InLeft, const FSingle& InRight, const FMask& InMask) { return InMask ? InRight : InLeft; }

	FSingle FSingle::MultiplyAdd(const FSingle& InMultiply, const FSingle& InAdd) { return (Value * InMultiply) + InAdd; }
	FSingle FSingle::MultiplySubtract(const FSingle& InMultiply, const FSingle& InSubtract) { return (Value * InMultiply) - InSubtract; }
	FSingle FSingle::MultiplyAddNegated(const FSingle& InMultiply, const FSingle& InAdd) { return InAdd - (Value * InMultiply); }

	FSingle FSingle::Gather() { return 1.0f; } // TODO
	FSingle FSingle::Permute() { return 1.0f; } // TODO
#pragma endregion FSingle

#pragma region FInteger
	FORCEINLINE void FInteger::Zero() { Value = 0; }

	FInteger& FInteger::operator=(const int32& InValue) { Value = InValue; return *this; }
	FInteger& FInteger::operator=(const INTEGER_PRIMITIVE& InValue) { Value = InValue; return *this; }

	FInteger FInteger::operator+(const FInteger& InOther) { return Value + InOther; }
	FInteger FInteger::operator-(const FInteger& InOther) { return Value - InOther; }
	FInteger FInteger::operator*(const FInteger& InOther) { return Value * InOther; }

	bool FInteger::operator<(const FInteger& InOther) { return Value < InOther; }
	bool FInteger::operator>(const FInteger& InOther) { return Value > InOther; }

	bool FInteger::operator&(const FInteger& InOther) { return Value & InOther; }
	bool FInteger::operator|(const FInteger& InOther) { return Value | InOther; }
	bool FInteger::operator^(const FInteger& InOther) { return Value ^ InOther; }
	bool FInteger::operator!() { return !Value; }
	bool FInteger::operator==(const FInteger& InOther) { return Value == InOther; }

	FInteger FInteger::operator>>(const FInteger& InOther) { return Value >> InOther; }
	FInteger FInteger::operator<<(const FInteger& InOther) { return Value << InOther; }

	FORCEINLINE static FInteger Convert(const FSingle& InValue) { return (int32)(InValue); }
	FORCEINLINE static FInteger Cast(const FSingle& InValue) { return (int32)(InValue); }
#pragma endregion FInteger
}