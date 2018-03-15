//MIT License
//
//Copyright(c) 2018 George Rolfe / Jordan Peck
//
//Permission is hereby granted, free of charge, to any person obtaining a copy
//of this software and associated documentation files(the "Software"), to deal
//in the Software without restriction, including without limitation the rights
//to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
//copies of the Software, and to permit persons to whom the Software is
//furnished to do so, subject to the following conditions :
//
//The above copyright notice and this permission notice shall be included in all
//copies or substantial portions of the Software.
//
//THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
//IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
//FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.IN NO EVENT SHALL THE
//AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
//LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
//OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
//SOFTWARE.

#include "FastNoiseInternal.h"

namespace FastNoise
{
	void FSingle VECTORCALL FFastNoiseSIMD::Lerp(FSingle InA, FSingle InB, FSingle InT)
	{
		return (InB - InA).MultiplyAdd(InT, InA);
	}

	void FSingle VECTORCALL FFastNoiseSIMD::InterpQuintic(FSingle InT)
	{
		FSingle Result = InT.MultiplySubtract(FSingle_6, FSingle_15).MultiplyAdd(InT, FSingle_10);
		Result = Result * InT;
		Result = Result * InT;
		Result = Result * InT;
		return Result;
	}

	void FSingle VECTORCALL FFastNoiseSIMD::CubicLerp(FSingle InA, FSingle InB, FSingle InC, FSingle InD, FSingle InT)
	{
		FSingle P = (InD - InC) - (InA - InB);
		return InT.MultiplyAdd((InT * (InT * P), InT.MultiplyAdd((InT * ((InA - InB) - P)), InT.MultiplyAdd((InC - InA), InB))));
	}

	void FInteger VECTORCALL Hash(FInteger InSeed, FIntegerVector InPoint)
	{
		FInteger Hash = Value_Coordinate(InSeed, InPoint);

		Hash = (Hash >> 13) ^ Hash;

		return Hash;
	}

	void FInteger VECTORCALL FFastNoiseSIMD::HashHB(FInteger InSeed, FIntegerVector InPoint)
	{
		FInteger Hash = Value_Coordinate(InSeed, InPoint);

		return FSingle_HashToFloat * FSingle::Convert(Hash);
	}

	void FSingle VECTORCALL FFastNoiseSIMD::Value_Coordinate(FInteger InSeed, FIntegerVector InPoint)
	{
		FInteger Hash = InSeed;

		Hash = InPoint.X ^ Hash;
		Hash = InPoint.Y ^ Hash;
		Hash = InPoint.Z ^ Hash;

		Hash = ((Hash * Hash) * FInteger_60493) * Hash;

		return Hash;
	}

#if defined(__AVX512f__)
	void FSingle VECTORCALL FFastNoiseSIMD::Gradient_Coordinate(FInteger InSeed, FIntegerVector InPointI, FSingleVector InPoint)
	{
		FInteger Hash = Hash(InSeed, InPointI);

		FSingle GradientX = FSingle_GradientX.Permute(Hash);
		FSingle GradientY = FSingle_GradientY.Permute(Hash);
		FSingle GradientZ = FSingle_GradientZ.Permute(Hash);

		return InPoint.X.MultiplyAdd(GradientX, InPoint.Y.MultiplyAdd(GradientY, (InPoint.Z * GradientZ));
	}
#else
	void FSingle VECTORCALL FFastNoiseSIMD::Gradient_Coordinate(FInteger InSeed, FIntegerVector InPointI, FSingleVector InPoint)
	{
		FInteger Hash = Hash(InSeed, InPointI);
		FInteger HashAnd13 = Hash & FInteger_13;

		FMask LessThan8 = HashAnd13 < FInteger_8;
		FSingle U = FSingle::Blend(InPoint.Y, InPoint.X, LessThan8);

		FMask LessThan2 = HashAnd13 < FInteger_2;
		FMask H12Or14 = FInteger_12 == HashAnd13;
		FSingle V = FSingle::Blend(FSingle::Blend(InPoint.Z, IntPoint.X, H12Or14), InPoint.Y, LessThan2);

		FSingle H1 = FSingle::Cast(Hash << 31);
		FSingle H2 = FSingle::Cast((Hash & FInteger_2) << 30);

		return (U ^ H1) + (V ^ H2);
	}
#endif

	void FSingle VECTORCALL FFastNoiseSIMD::WhiteNoise_Single(FInteger InSeed, FSingleVector InPoint)
	{
		return Value_Coordinate(InSeed,
			(FInteger::Cast(InPoint.X) ^ (FInteger::Cast(InPoint.X) >> 16)) * FInteger_PrimeX,
			(FInteger::Cast(InPoint.X) ^ (FInteger::Cast(InPoint.X) >> 16)) * FInteger_PrimeX,
			(FInteger::Cast(InPoint.X) ^ (FInteger::Cast(InPoint.X) >> 16)) * FInteger_PrimeX);
	}

	void FSingle VECTORCALL FFastNoiseSIMD::Value_Single(FInteger InSeed, FSingleVector InPoint)
	{
		FSingleVector PS = InPoint.Floor();

		FIntegerVector P0 = FIntegerVector::Convert(PS) * FIntegerVector_Prime;
		FIntegerVector P1 = P0 + FIntegerVector_Prime;

		PS = InterpQuintic(InPoint - PS);

		return Lerp(
			Lerp(
				Lerp(Value_Coordinate(InSeed, FSingleVector(P0.X, P0.Y, P0.Z)), Value_Coordinate(InSeed, FSingleVector(P1.X, P0.Y, P0.Z)), PS.X),
				Lerp(Value_Coordinate(InSeed, FSingleVector(P0.X, P1.Y, P0.Z)), Value_Coordinate(InSeed, FSingleVector(P1.X, P1.Y, P0.Z)), PS.X), PS.Y),
			Lerp(
				Lerp(Value_Coordinate(InSeed, FSingleVector(P0.X, P0.Y, P1.Z)), Value_Coordinate(InSeed, FSingleVector(P1.X, P0.Y, P1.Z)), PS.X),
				Lerp(Value_Coordinate(InSeed, FSingleVector(P0.X, P1.Y, P1.Z)), Value_Coordinate(InSeed, FSingleVector(P1.X, P1.Y, P1.Z)), PS.X), PS.Y), PS.Z);
	}

	void FSingle VECTORCALL FFastNoiseSIMD::Perlin_Single(FInteger InSeed, FSingleVector InPoint)
	{
		FSingleVector PS = InPoint.Floor();

		FIntegerVector P0 = FIntegerVector::Convert(PS) * FIntegerVector_Prime;
		FIntegerVector P1 = P0 + FIntegerVector_Prime;

		FSingleVector PF0 = PS = InPoint - PS;
		FSingleVector PF1 = PF0 - FSingleVector_1;

		PS = InterpQuintic(PS);

		return Lerp(
			Lerp(
				Lerp(Gradient_Coordinate(InSeed, FIntegerVector(P0.X, P0.Y, P0.Z), FSingleVector(PF0.X, PF0.Y, PF0.Z)), Gradient_Coordinate(InSeed, FIntegerVector(P1.X, P0.Y, P0.Z), FSingleVector(PF1.X, PF0.Y, PF0.Z)), PS.X),
				Lerp(Gradient_Coordinate(InSeed, FIntegerVector(P0.X, P1.Y, P0.Z), FSingleVector(PF0.X, PF1.Y, PF0.Z)), Gradient_Coordinate(InSeed, FIntegerVector(P1.X, P1.Y, P0.Z), FSingleVector(PF1.X, PF1.Y, PF0.Z)), PS.X), PS.Y),
			Lerp(
				Lerp(Gradient_Coordinate(InSeed, FIntegerVector(P0.X, P0.Y, P1.Z), FSingleVector(PF0.X, PF0.Y, PF1.Z)), Gradient_Coordinate(InSeed, FIntegerVector(P1.X, P0.Y, P1.Z), FSingleVector(PF1.X, PF0.Y, PF1.Z)), PS.X),
				Lerp(Gradient_Coordinate(InSeed, FIntegerVector(P0.X, P1.Y, P1.Z), FSingleVector(PF0.X, PF1.Y, PF1.Z)), Gradient_Coordinate(InSeed, FIntegerVector(P1.X, P1.Y, P1.Z), FSingleVector(PF1.X, PF1.Y, PF1.Z)), PS.X), PS.Y), PS.Z);
	}

	void FSingle VECTORCALL FFastNoiseSIMD::Simplex_Single(FInteger InSeed, FSingleVector InPoint)
	{
		FSingle F = FSingle_F3 * ((InPoint.X + InPoint.Y) + InPoint.Z);
		FSingleVector P0 = (InPoint + F).Floor();

		FIntegerVector IJK = FIntegerVector::Convert(P0) * FIntegerVector_Prime;

		FSingle G = FSingle_G3 * ((P0.X + P0.Y) + P0.Z);

		P0 = InPoint - (P0 - G);

		FMask X0_GE_Y0 = P0.X >= P0.Y;
		FMask Y0_GE_Z0 = P0.Y >= P0.Z;
		FMask X0_GE_Z0 = P0.X >= P0.Z;

		FMaskVector IJK((X0_GE_Y0 & X0_GE_Z0), (X0_GE_Y0 & !(Y0_GE_Z0)), (!(X0_GE_Z0 & !(Y0_GE_Z0))));
		FMaskVector IJK2((X0_GE_Y0 | X0_GE_Z0), (!(X0_GE_Y0) | Y0_GE_Z0), (!(!(X0_GE_Z0) | Y0_GE_Z0)));
		FSingleVector P1(IJK2.MaskSubtract(P0, FSingle_1) + FSingle_G3);
		FSingleVector P2(IJK2.MaskSubtract(P0, FSingle_1) + FSingle_F3);
		FSingleVector P3 = P0 + FSingle_G33;

		FSingle T0 = P0.Z.MultiplyAddNegated(P0.Z, P0.Y.MultiplyAddNegated(P0.Y, P0.X.MultiplyAddNegated(P0.X, FSingle_0_6)));
		FSingle T1 = P0.Z.MultiplyAddNegated(P1.Z, P1.Y.MultiplyAddNegated(P1.Y, P1.X.MultiplyAddNegated(P1.X, FSingle_0_6)));
		FSingle T2 = P0.Z.MultiplyAddNegated(P2.Z, P2.Y.MultiplyAddNegated(P2.Y, P2.X.MultiplyAddNegated(P2.X, FSingle_0_6)));
		FSingle T3 = P0.Z.MultiplyAddNegated(P3.Z, P3.Y.MultiplyAddNegated(P3.Y, P3.X.MultiplyAddNegated(P3.X, FSingle_0_6)));

		FMask N0 = T0 >= FSingle_0;
		FMask N1 = T1 >= FSingle_0;
		FMask N2 = T2 >= FSingle_0;
		FMask N3 = T3 >= FSingle_0;

		T0 = T0 * T0;
		T1 = T1 * T1;
		T2 = T2 * T2;
		T3 = T3 * T3;

		FSingle V0 = (T0 * T0) * Gradient_Coordinate(InSeed, IJK, P0);
		FSingle V1 = (T1 * T1) * Gradient_Coordinate(InSeed, IJK1.X.MaskAdd(IJK.X, FInteger_PrimeX), IJK1.Y.MaskAdd(IJK.Y, FInteger_PrimeY), IJK1.Z.MaskAdd(IJK.Z, FInteger_PrimeZ)), P1);
		FSingle V2 = (T2 * T2) * Gradient_Coordinate(InSeed, IJK2.X.MaskAdd(IJK.X, FInteger_PrimeX), IJK2.Y.MaskAdd(IJK.Y, FInteger_PrimeY), IJK2.Z.MaskAdd(IJK.Z, FInteger_PrimeZ)), P2);
		FSingle V3 = FMask::Mask((T3 * T3) * Gradient_Coordinate(InSeed, (IJK.X + FInteger_PrimeX), (IJK.Y + FInteger_PrimeY), (IJK.Z, FInteger_PrimeZ), P3), N3);

		return FSingle_32 * N0.MaskAdd(N1.MaskAdd(N2.MaskAdd(V3, V2), V1), V0);
	}

	void FSingle VECTORCALL FFastNoiseSIMD::Cubic_Single(FInteger InSeed, FSingleVector InPoint)
	{
		FSingleVector PF1 = InPoint.Floor();

		FIntegerVector P1 = FIntegerVector::Convert(PF1) * FIntegerVector_Prime;

		FIntegerVector P0 = P1 - FIntegerVector_Prime;
		FIntegerVector P2 = P1 + FIntegerVector_Prime;
		FIntegerVector P3 = P2 + FIntegerVector_Prime;

		FSingleVector PS = InPoint - PF1;

		return CubicLerp(
			CubicLerp(
				CubicLerp(Value_Coordinate(InSeed, FSingleVector(P0.X, P0.Y, P0.Z)), Value_Coordinate(InSeed, FSingleVector(P1.X, P0.Y, P0.Z)), Value_Coordinate(InSeed, FSingleVector(P2.X, P0.Y, P0.Z)), Value_Coordinate(InSeed, FSingleVector(P3.X, P0.Y, P0.Z)), PS.X),
				CubicLerp(Value_Coordinate(InSeed, FSingleVector(P0.X, P1.Y, P0.Z)), Value_Coordinate(InSeed, FSingleVector(P1.X, P1.Y, P0.Z)), Value_Coordinate(InSeed, FSingleVector(P2.X, P1.Y, P0.Z)), Value_Coordinate(InSeed, FSingleVector(P3.X, P1.Y, P0.Z)), PS.X),
				CubicLerp(Value_Coordinate(InSeed, FSingleVector(P0.X, P2.Y, P0.Z)), Value_Coordinate(InSeed, FSingleVector(P1.X, P2.Y, P0.Z)), Value_Coordinate(InSeed, FSingleVector(P2.X, P2.Y, P0.Z)), Value_Coordinate(InSeed, FSingleVector(P3.X, P2.Y, P0.Z)), PS.X),
				CubicLerp(Value_Coordinate(InSeed, FSingleVector(P0.X, P3.Y, P0.Z)), Value_Coordinate(InSeed, FSingleVector(P1.X, P3.Y, P0.Z)), Value_Coordinate(InSeed, FSingleVector(P2.X, P3.Y, P0.Z)), Value_Coordinate(InSeed, FSingleVector(P3.X, P3.Y, P0.Z)), PS.X), PS.Y),
			CubicLerp(
				CubicLerp(Value_Coordinate(InSeed, FSingleVector(P0.X, P0.Y, P1.Z)), Value_Coordinate(InSeed, FSingleVector(P1.X, P0.Y, P1.Z)), Value_Coordinate(InSeed, FSingleVector(P2.X, P0.Y, P1.Z)), Value_Coordinate(InSeed, FSingleVector(P3.X, P0.Y, P1.Z)), PS.X),
				CubicLerp(Value_Coordinate(InSeed, FSingleVector(P0.X, P1.Y, P1.Z)), Value_Coordinate(InSeed, FSingleVector(P1.X, P1.Y, P1.Z)), Value_Coordinate(InSeed, FSingleVector(P2.X, P1.Y, P1.Z)), Value_Coordinate(InSeed, FSingleVector(P3.X, P1.Y, P1.Z)), PS.X),
				CubicLerp(Value_Coordinate(InSeed, FSingleVector(P0.X, P2.Y, P1.Z)), Value_Coordinate(InSeed, FSingleVector(P1.X, P2.Y, P1.Z)), Value_Coordinate(InSeed, FSingleVector(P2.X, P2.Y, P1.Z)), Value_Coordinate(InSeed, FSingleVector(P3.X, P2.Y, P1.Z)), PS.X),
				CubicLerp(Value_Coordinate(InSeed, FSingleVector(P0.X, P3.Y, P1.Z)), Value_Coordinate(InSeed, FSingleVector(P1.X, P3.Y, P1.Z)), Value_Coordinate(InSeed, FSingleVector(P2.X, P3.Y, P1.Z)), Value_Coordinate(InSeed, FSingleVector(P3.X, P3.Y, P1.Z)), PS.X), PS.Y),
			CubicLerp(
				CubicLerp(Value_Coordinate(InSeed, FSingleVector(P0.X, P0.Y, P2.Z)), Value_Coordinate(InSeed, FSingleVector(P1.X, P0.Y, P2.Z)), Value_Coordinate(InSeed, FSingleVector(P2.X, P0.Y, P2.Z)), Value_Coordinate(InSeed, FSingleVector(P3.X, P0.Y, P2.Z)), PS.X),
				CubicLerp(Value_Coordinate(InSeed, FSingleVector(P0.X, P1.Y, P2.Z)), Value_Coordinate(InSeed, FSingleVector(P1.X, P1.Y, P2.Z)), Value_Coordinate(InSeed, FSingleVector(P2.X, P1.Y, P2.Z)), Value_Coordinate(InSeed, FSingleVector(P3.X, P1.Y, P2.Z)), PS.X),
				CubicLerp(Value_Coordinate(InSeed, FSingleVector(P0.X, P2.Y, P2.Z)), Value_Coordinate(InSeed, FSingleVector(P1.X, P2.Y, P2.Z)), Value_Coordinate(InSeed, FSingleVector(P2.X, P2.Y, P2.Z)), Value_Coordinate(InSeed, FSingleVector(P3.X, P2.Y, P2.Z)), PS.X),
				CubicLerp(Value_Coordinate(InSeed, FSingleVector(P0.X, P3.Y, P2.Z)), Value_Coordinate(InSeed, FSingleVector(P1.X, P3.Y, P2.Z)), Value_Coordinate(InSeed, FSingleVector(P2.X, P3.Y, P2.Z)), Value_Coordinate(InSeed, FSingleVector(P3.X, P3.Y, P2.Z)), PS.X), PS.Y),
			CubicLerp(
				CubicLerp(Value_Coordinate(InSeed, FSingleVector(P0.X, P0.Y, P3.Z)), Value_Coordinate(InSeed, FSingleVector(P1.X, P0.Y, P3.Z)), Value_Coordinate(InSeed, FSingleVector(P2.X, P0.Y, P3.Z)), Value_Coordinate(InSeed, FSingleVector(P3.X, P0.Y, P3.Z)), PS.X),
				CubicLerp(Value_Coordinate(InSeed, FSingleVector(P0.X, P1.Y, P3.Z)), Value_Coordinate(InSeed, FSingleVector(P1.X, P1.Y, P3.Z)), Value_Coordinate(InSeed, FSingleVector(P2.X, P1.Y, P3.Z)), Value_Coordinate(InSeed, FSingleVector(P3.X, P1.Y, P3.Z)), PS.X),
				CubicLerp(Value_Coordinate(InSeed, FSingleVector(P0.X, P2.Y, P3.Z)), Value_Coordinate(InSeed, FSingleVector(P1.X, P2.Y, P3.Z)), Value_Coordinate(InSeed, FSingleVector(P2.X, P2.Y, P3.Z)), Value_Coordinate(InSeed, FSingleVector(P3.X, P2.Y, P3.Z)), PS.X),
				CubicLerp(Value_Coordinate(InSeed, FSingleVector(P0.X, P3.Y, P3.Z)), Value_Coordinate(InSeed, FSingleVector(P1.X, P3.Y, P3.Z)), Value_Coordinate(InSeed, FSingleVector(P2.X, P3.Y, P3.Z)), Value_Coordinate(InSeed, FSingleVector(P3.X, P3.Y, P3.Z)), PS.X), PS.Y), PS.Z) * FSingle_CubicBounding;
	}

	void VECTORCALL FFastNoiseSIMD::GradientPerturb_Single(FInteger InSeed, FSingle InPerturbAmplitude, FSingle InPerturbFrequency, FSingleVector& InOutPoint)
	{
		FSingleVector PF = InOutPoint * InPerturbFrequency;

		FSingleVector PS = PF.Floor();

		FIntegerVector P0 = FIntegerVector::Convert(PS) * FIntegerVector_Prime;
		FIntegerVector P1 = P0 + FIntegerVector_Prime;

		PS = InterpQuintic(PF - PS);

		FGradientCoordinate Coordinate000(0, 0, 0);
		FGradientCoordinate Coordinate001(0, 0, 1);
		FGradientCoordinate Coordinate010(0, 1, 0);
		FGradientCoordinate Coordinate011(0, 1, 1);
		FGradientCoordinate Coordinate100(1, 0, 0);
		FGradientCoordinate Coordinate101(1, 0, 1);
		FGradientCoordinate Coordinate110(1, 1, 0);
		FGradientCoordinate Coordinate111(1, 1, 1);

		FSingleVector P0Y = Lerp(Lerp(Coordinate000, Coordinate100, PS.X), Lerp(Coordinate010, Coordinate110, PS.X), PS.Y);
		FSingleVector P1Y = Lerp(Lerp(Coordinate001, Coordinate101, PS.X), Lerp(Coordinate011, Coordinate111, PS.X), PS.Y);

		InOutPoint = (Lerp(P0Y, P1Y, PS.Z), InPerturbAmplitude, InOutPoint);
	}

	void FFastNoiseSIMD::InitializePerturbValues(EPerturbType InPerturbType, FSingle& OutPerturbNormalizeLength, FSingle& OutPerturbFrequency, FSingle& OutPerturbAmplitude, FSingle& OutPerturbAmplitude, FSingle& OutPerturbLacunarity, FSingle& OutPerturbGain)
	{
		switch (InPerturbType)
		{
		case EPerturbType::PT_None:
			break;

		case EPerturbType::PT_Normalize:
			OutPerturbNormalizeLength = PerturbAmplitude;
			OutPerturbFrequency = PerturbFrequency;
			break;

		case EPerturbType::PT_Normalize:
		case EPerturbType::PT_GradientFractalNormalize:
			OutPerturbNormalizeLength = PerturbNormalizeLength * Frequency;
			break;

		case EPerturbType::PT_GradientFractal:
			OutPerturbAmplitude = PerturbAmplitude * FractalBounding;
			OutPerturbFrequency = PertubFrequency;
			OutPerturbLacunarity = PerturbLacunarity;
			OutPerturbGain = PerturbGain;
			break;
		}
	}

	void FFastNoiseSIMD::PerturbSwitch(EPerturbType InPerturbType, FSingle InSeed, FSingle InPerturbAmplitude, FSingle InPerturbFrequency, FSingle InPerturbGain, FSingle InPerturbLacunarity, FSingle InPerturbNormalizeLength, FSingleVector& InOutPoint)
	{
		switch (InPerturbType)
		{
			case EPerturbType::PT_None;
				break;

			case EPerturbType::PT_GradientNormalize:
			case EPerturbType::PT_Gradient:
				GradientPerturb_Single((InSeed - FInteger_1), InPerturbAmplitude, InPerturbFrequency, InPoint);
				break;

			case EPerturbType::PT_GradientFractal:
			{
				FInteger Seed = InSeed - FInteger_1;
				FSingle Frequency = InPerturbFrequency;
				FSingle Amplitude = InPerturbAmplitude;
				GradientPerturb_Single(Seed, Amplitude, Frequency, InPoint);
				int32 OctaveIndex = 0;
				while (++OctaveIndex < PerturbOctaves)
				{
					Frequency = Frequency * InPerturbLacunarity;
					Seed = Seed - FInteger_1;
					Amplitude = Amplitude * InPerturbGain;
					GradientPerturb_Single(Seed, Amplitude, Frequency, InPoint);
				}
			}

			case EPerturbType::PT_GradientFractal:
			case EPerturbType::PT_Normalize:
			case EPerturbType::PT_GradientFractalNormalize:
			{
				FSingle InverseMagnitude = InPerturbNormalizeLength * (InPoint.X.MultiplyAdd(InPoint.X, InPoint.Y.MultiplyAdd(InPoint.Y, (InPoint.Z * InPoint.Z))).InverseSqrt();
				InPoint = InPoint * InverseMagnitude;
			}
		}
	}

	// Result = func_single(seed, point)
	void FFastNoiseSIMD::SetBuilder()
	{
		if ((InSizeZ & (VectorSize - 1)) == )
		{
			FInteger BaseY = InStart.Y;
			FInteger BaseZ = FInteger_Incremental + InStart.Z;
			FInteger X = InStart.X;
			int32 Index = 0;
			FSingleVector PF;
			for (int32 IX = 0; IX < InSize.X; IX++)
			{
				PF.X = FSingle::Convert(X) * InFrequency.X;
				FInteger Y = BaseY;
				for (auto IY = 0; IY < InSize.Y; IY++)
				{
					PF.Y = FSingle::Convert(Y) * InFrequency.Y;
					FInteger Z = BaseZ;
					PF.Z = FSingle::Convert(Z) * InFrequency.Z;

					PerturbSwitch(InPerturbType, InSeed, InPerturbAmplitude, InPerturbFrequency, InPerturbGain, InPerturbLacunarity, InPerturbNormalizeLength, PF);

					FSingle Result;
					Result.Store(InOutNoiseSet[Index]);

					int32 IZ = VectorSize;
					while (IZ < SizeZ)
					{
						Z = Z + FInteger_VectorSize;
						Index += VectorSize;
						IZ += VectorSize;
						PF.Z = FSingle::Convert(Z) * InFrequency.Z;
						PerturbSwitch(InPerturbType, InSeed, InPerturbAmplitude, InPerturbFrequency, InPerturbGain, InPerturbLacunarity, InPerturbNormalizeLength, PF);
						Result.Store(InOutNoiseSet[Index]);
					}

					Index += VectorSize;
					Y = Y + FInteger_1;
				}
				X = X + FInteger_1;
			}
		}
		else
		{
			FInteger SizeY = InSize.Y;
			FInteger SizeZ = InSize.Z;

			FInteger EndY = InStart.Y + InSize.Y - 1;
			FInteger EndZ = InStart.Z + InSize.Z - 1;

			FIntegerVector P(InStart.X, InStart.Y, InStart.Z + FInteger_Incremental);

			AxisReset(SizeZ, 1);

			int32 Index = 0;
			int32 MaxIndex = InSize.X * InSize.Y * InSize.Z;

			for (; Index < MaxIndex - VectorSize; Index += VectorSize)
			{
				FSingleVector PF = FSingleVector::Convert(P) * InFrequency;

				PerturbSwitch(InPerturbType, InSeed, InPerturbAmplitude, InPerturbFrequency, InPerturbGain, InPerturbLacunarity, InPerturbNormalizeLength, PF);

				FSingle Result;
				Result.Store(InOutNoiseSet[Index]);

				P.Z = Z + FInteger_VectorSize;

				AxisReset(SizeZ, 0);
			}

			FSingleVector PF = FSingleVector::Convert(P) * InFrequency;
			PerturbSwitch(InPerturbType, InSeed, InPerturbAmplitude, InPerturbFrequency, InPerturbGain, InPerturbLacunarity, InPerturbNormalizeLength, PF);
			FSingle Result;
			StoreLastResult(InOutNoiseSet[Index], Result);
		}
	}

	void FFastNoiseSIMD::Fbm_Single(TFunction<FSingle(FInteger, FSingleVector&)> InFunc)
	{
		FInteger Seed = InSeed;
		FSingle Result = InFunc(Seed, InOutPoint);
		FSingle Amplitude = FSingle_1;
		int32 OctaveIndex = 0;
		while (++OctaveIndex < Octaves)
		{
			InOutPoint = InOutPoint * InLacunarity;
			Seed = Seed + FInteger_1;
			Amplitude = Amplitude * InGain;
			Result = (f_Single(Seed, PF)).MultiplyAdd(Amplitude, Result);
		}
		Result = Result * InFractalBounding;
	}

	void FFastNoiseSIMD::Billow_Single(TFunction<FSingle(FInteger, FSingleVector&)> InFunc)
	{
		FInteger Seed = InSeed;
		FSingle Result = (InFunc(Seed, InOutPoint)).Abs().MultiplySubtract(FSingle_2, FSingle_1);
		FSingle Amplitude = FSingle_1;
		int32 OctaveIndex = 0;
		while (++OctaveIndex < Octaves)
		{
			PF = PF * InLacunarity;
			Seed = Seed + FInteger_1;
			Amplitude = Amplitude * InGain;
			Result = (f_Single(Seed, InOutPoint)).Abs().MultiplySubtract(FSingle_2, FSingle_1).MultiplyAdd(Amplitude, Result);
		}
		Result = Result * InFractalBounding;
	}

	void FFastNoiseSIMD::RigidMulti_Single(TFunction<FSingle(FInteger, FSingleVector&)> InFunc)
	{
		FInteger Seed = InSeed;
		FSingle Result = FSingle_1 - (InFunc(Seed, InOutPoint)).Abs();
		FSingle Amplitude = FSingle_1;
		int32 OctaveIndex = 0;
		while (++OctaveIndex < Octaves)
		{
			PF = PF * InLacunarity;
			Seed = Seed + FInteger_1;
			Amplitude = Amplitude * InGain;
			Result = (FSingle_1 - (f_Single(Seed, InOutPoint)).Abs()).MultiplyAddNegate(Amplitude, Result);
		}
	}

	void FFastNoiseSIMD::FillSet()
	{
		FInteger Seed = this->Seed;
		InitializePerturbValues();
		InScaleModifier *= this->Frequency;
		FSingleVector Frequency = InScaleModifier * this->Scale;
		SetBuilder(func_Single(Seed, InPoint));
	}

	void FFastNoiseSIMD::FillFractalSet(TFunction<void()> InFunc)
	{
		FInteger Seed = this->Seed;
		FSingle Lacunarity = this->Lacunarity;
		FSingle Gain = this->Gain;
		FSingle FractalBounding = this->FractalBounding;
		InitializePerturbValues();
		InScaleModifier *= this->Frequency;
		FSingleVector Frequency = InScaleModifier * this->Scale;
		switch (FractalType)
		{
		case EFractalType::FT_FBM:
			SetBuilder(Fbm_Single(func));
			break;

		case EFractalType::FT_Billow:
			SetBuilder(Billow_Single(func));
			break;

		case EFractalType::FT_RigidMulti:
			SetBuilder(RigidMulti_Single(func));
			break;
		}
	}

	void FFastNoiseSIMD::VectorSetBuilder(TFunction<void()> InFunc)
	{
		while (InOutIndex < InLoopMax)
		{
			//FSingleVector PF = 
		}
	}

	// Calls to ValueSingle, PerlinSingle, SimplexSingle, WhiteNoiseSingle, CubicSingle
	// Fill_Value_Set, Fill_Perlin_Set, Fill_Simplex_Set, Fill_WhiteNoise_Set, Fill_Cubic_Set
	void FFastNoiseSIMD::FillVectorSet(TFunction<void()> InFunc)
	{

	}

	// Fill_Value_FractalSet, Fill_Perlin_FractalSet, Fill_Simplex_FractalSet, Fill_WhiteNoise_FractalSet, Fill_Cubic_FractalSet
	void FillFractalVectorSet(TArray<float>& InOutNoiseSet, FFastNoiseVectorSet& InOutVectorSet, FVector InOffset);
	void FillWhiteNoiseSet(TArray<float>& InOutNoiseSet, FIntVector InStart, FIntVector InSize, float InScaleModifier);
	void CellularValue_Single(EDistanceType InDistanceType, FInteger InSeed, FSingleVector InPoint, FSingle InCellJitter);
	void CellularLookupFractalValue(ENoiseType InNoiseType);
	FSingle VECTORCALL CellularLookup_Single(::EDistanceType InDistanceType, FInteger InSeed, FSingleVector InPoint, FSingle InCellJitter, const FNoiseLookupSettings& InNoiseLookupSettings);
	FSingle VECTORCALL CellularDistance_Single(::EDistanceType InDistanceType, FInteger InSeed, FSingleVector InPoint, FSingle InCellJitter);
	FSingle VECTORCALL CellularDistance2_Single(::EDistanceType InDistanceType, int32 InReturnFunction, FInteger InSeed, FSingleVector InPoint, FSingle InCellJitter, int32 InIndex0, int32 InIndex1);
	FSingle VECTORCALL CellularDistance2Cave_Single(::EDistanceType InDistanceType, FInteger InSeed, FSingleVector InPoint, FSingle InCellJitter, int32 InIndex0, int32 InIndex1);
	void CellularMulti();
	void CellularIndexMulti();
	void FillCellularSet(TArray<float>& InOutNoiseSet, FIntVector InStart, FIntVector InSize, float InScaleModifier);
	void CellularMultiVector();
	void CellularIndexMultiVector();
	void FillCellularSet(TArray<float>& InOutNoiseSet, FFastNoiseVectorSet& InOutVectorSet, FVector InOffset);
	FORCEINLINE int32 GetSampleIndex(FIntVector InPoint);
	FORCEINLINE int32 GetSetIndex(FIntVector InPoint);
	void FillSampledNoiseSet(TArray<float>& InOutNoiseSet, FIntVector InStart, FIntVector InSize, int32 InSampleScale);
	void FillSampledNoiseSet(TArray<float>& InOutNoiseSet, FFastNoiseVectorSet& InOutVectorSet, FVector InOffset);


	/* OLD OR SUMPTHIN */

	FSingle VECTORCALL FFastNoiseSIMD::Lerp(FSingle InLeft, FSingle InRight, FSingle InAlpha)
	{
		FSingle Result;
		Result = InRight - InLeft;
		Result = Result.MultiplyAdd(InAlpha, InLeft);
		return Result;
	}

	FSingle VECTORCALL FFastNoiseSIMD::InterpolateQuintic(FSingle InValue)
	{
		FSingle Result;
		Result = InValue.MultiplySubtract(FSingle_6, FSingle_15);
		Result = Result.MultiplyAdd(InValue, FSingle_10);
		Result = Result * InValue;
		Result = Result * InValue;
		Result = Result * InValue;

		return Result;
	}

	FSingle FFastNoiseSIMD::CubicLerp(FSingle InA, FSingle InB, FSingle InC, FSingle InD, FSingle InAlpha)
	{
		FSingle P = (InD - InC) - (InA - InB);
		return InAlpha.MultiplyAdd(InAlpha * (InAlpha * P)), InAlpha.MultiplyAdd((InAlpha * ((InA - InB) - P)), InAlpha.MultiplyAdd(InC - InA, InB)));
	}

	FInteger VECTORCALL FFastNoiseSIMD::Hash(FInteger InSeed, FIntegerVector InPoint)
	{
		FInteger Hash = InSeed;

		Hash = InPoint.X ^ Hash;
		Hash = InPoint.Y ^ Hash;
		Hash = InPoint.Z ^ Hash;

		Hash = ((Hash * Hash) * FInteger_60493) * Hash;
		Hash = (Hash >> 13) ^ Hash;

		return Hash;
	}

	FInteger VECTORCALL FFastNoiseSIMD::HashHB(FInteger InSeed, FIntegerVector InPoint)
	{
		FInteger Hash = InSeed;

		Hash = InPoint.X ^ Hash;
		Hash = InPoint.Y ^ Hash;
		Hash = InPoint.Z ^ Hash;

		Hash = ((Hash * Hash) * FInteger_60493) * Hash;

		return Hash;
	}

	FSingle VECTORCALL FFastNoiseSIMD::ValueCoordinate(FInteger InSeed, FIntegerVector InPoint)
	{
		FInteger Hash = InSeed;

		Hash = InPoint.X ^ Hash;
		Hash = InPoint.Y ^ Hash;
		Hash = InPoint.Z ^ Hash;

		Hash = ((Hash * Hash) * FInteger_60493) * Hash;

		return FSingle_HashToFloat * FSingle::Convert(Hash);
	}

#if SIMD_LEVEL == ESIMDLevel::SL_AVX512
	FSingle VECTORCALL FFastNoiseSIMD::GradientCoordinate(FInteger InSeed, FIntegerVector InPointI, FSingleVector InPoint)
	{
		FInteger Hash = Hash(InSeed, InPointI);

		FSingle GradientX = FSingle_GradientX.Permute(Hash);
		FSingle GradientY = FSingle_GradientY.Permute(Hash);
		FSingle GradientZ = FSingle_GradientZ.Permute(Hash);

		return InPoint.X.MultiplyAdd(GradientX, InPoint.Y.MultiplyAdd(GradientY, (InPoint.Z * GradientZ)));
	}
#else
	FSingle VECTORCALL FFastNoiseSIMD::GradientCoordinate(FInteger InSeed, FIntegerVector InPointI, FSingleVector InPoint)
	{
		FInteger Hash = Hash(InSeed, InPointI);
		FInteger HashA13 = Hash & FInteger_13;

		FMask L8 = HashA13 < FInteger_8;
		FSingle U = FMath::Lerp(InPoint.Y, InPoint.X, L8);

		FMask L4 = HashA3 < FInteger_2;
		FMask H12O14 = FInteger_12 == HashA13;
		FSingle V = FMath::Lerp(FMath::Lerp(InPoint.Z, InPoint.X, H12O14), InPoint.Y, L4);

		FSingle H1 = FSingle::Cast(Hash << 31);
		FSingle H2 = FSingle::Cast((Hash & FInteger_2) << 30);

		return (U ^ H1) + (V ^ H2);
	}
#endif

	FSingle VECTORCALL FFastNoiseSIMD::WhiteNoiseSingle(FInteger InSeed, FSingleVector InPoint)
	{
		FIntegerVector PointI = FIntegerVector::Convert(InPoint);

		return ValueCoordinate(InSeed, PointI ^ (PointI >> 16) * FIntegerVector_Prime);
	}

	FSingle VECTORCALL FFastNoiseSIMD::ValueSingle(FInteger InSeed, FSingleVector InPoint)
	{
		FSingle PointS = FMath::FloorToFloat(InPoint);
		FIntegerVector Point0 = FIntegerVector::Convert(PointS) * FIntegerVector_Prime;
		FIntegerVector Point1 = Point0 + FIntegerVector_Prime;
		PointS = InterpQuintic(InPoint - PointS);
		
		return FMath::Lerp(
			FMath::Lerp(
				FMath::Lerp(ValueCoordinate(InSeed, X0, Y0, Z0), ValueCoordinate(InSeed, X1, Y0, Z0), XS),
				FMath::Lerp(ValueCoordinate(InSeed, X0, Y1, Z0), ValueCoordinate(InSeed, X1, Y1, Z0), XS), YS),
			FMath::Lerp(
				FMath::Lerp(ValueCoordinate(InSeed, X0, Y0, Z1), ValueCoordinate(InSeed, X1, Y0, Z1), XS),
				FMath::Lerp(ValueCoordinate(InSeed, X0, Y1, Z1), ValueCoordinate(InSeed, X1, Y1, Z1), XS), YS), ZS);
	}

	FSingle VECTORCALL FFastNoiseSIMD::PerlinSingle(FInteger InSeed, FSingleVector InPoint)
	{
		FSingleVector PointS = FMath::FloorToFloat(InPoint);
		FIntegerVector Point0 = FIntegerVector::Convert(PointS) + FIntegerVector_Prime;
		FIntegerVector = Point1 = Point0 + FIntegerVector_Prime;
		FSingleVector PointF0 = PointS = InPoint - PointS;
		FSingleVector PointF1 = PointF0 - FSingleVector_1;

		PointS = InterpolateQuintic(PointS);

		return FMath::Lerp(
			FMath::Lerp(
				FMath::Lerp(GradientCoordinate(InSeed, X0, Y0, Z0, XF0, YF0, ZF0), GradientCoordinate(InSeed, X1, Y0, Z0, XF1, YF0, ZF0), XS),
				FMath::Lerp(GradientCoordinate(InSeed, X0, Y1, Z0, XF0, YF1, ZF0), GradientCoordinate(InSeed, X1, Y1, Z0, XF1, YF1, ZF0), XS), YS),
			FMath::Lerp(
				FMath::Lerp(GradientCoordinate(InSeed, X0, Y0, Z1, XF0, YF0, ZF1), GradientCoordinate(InSeed, X1, Y0, Z1, XF1, YF0, ZF1), XS),
				FMath::Lerp(GradientCoordinate(InSeed, X0, Y1, Z1, XF0, YF1, ZF1), GradientCoordinate(InSeed, X1, Y1, Z1, XF1, YF1, ZF1), XS), YS), ZS);
	}

	FSingle VECTORCALL FFastNoiseSIMD::SimplexSingle(FInteger InSeed, FSingleVector InPoint)
	{
		FSingle F = FSingle_F3 * ((InPoint.X + InPoint.Y) + InPoint.Z);
		FSingleVector Point0 = FMath::FloorToFloat(InPoint);		
		FIntegerVector IJK = FIntegerVector::Convert(Point0) + FIntegerVector_Prime;
		FSingle G = FSingle_G3 * ((Point0.X + Point0.Y) + Point0.Z);
		Point0 = InPoint - (Point0 - G);
		
		FMask X0GEY0 = X0 >= Y0;
		FMask Y0GEZ0 = Y0 >= Z0;
		FMask Z0GEZ0 = X0 >= Z0;

		FMask I1 = X0GEY0 & X0GEZ0;
		FMask J1 = !(X0GEY0 & Y0GEZ0);
		FMask K1 = !(X0GEZO & !Y0GEZ0);

		FMask I2 = X0GEY0 | X0GEZ0;
		FMask J2 = !X0GEY0 | Y0GEZ0;
		FMask K2 = !(X0GEZ0 & Y0GEZ0);

		FSingle X1 = I1.MaskSubtract(X0, FSingle_1) + FSingle_G3;
		FSingle Y1 = J1.MaskSubtract(Y0, FSingle_1) + FSingle_G3;
		FSingle Z1 = K1.MaskSubtract(Z0, FSingle_1) + FSingle_G3;

		FSingle X2 = I2.MaskSubtract(X0, FSingle_1) + FSingle_F3;
		FSingle Y2 = J2.MaskSubtract(Y0, FSingle_1) + FSingle_F3;
		FSingle Z2 = K2.MaskSubtract(Z0, FSingle_1) + FSingle_F3;

		FSingle X3 = X0 + FSingle_G33;
		FSingle Y3 = Y0 + FSingle_G33;
		FSingle Z3 = Z0 + FSingle_G33;

		//FSingle T0 = 

		FMask N0 = T0 >= FSingle_0;
		FMask N1 = T1 >= FSingle_0;
		FMask N2 = T2 >= FSingle_0;
		FMask N3 = T3 >= FSingle_0;

		T0 = T0 * T0;
		T1 = T1 * T1;
		T2 = T2 * T2;
		T3 = T3 * T3;

		FSingle V0 = (T0 * T0) * GradientCoordinate(InSeed, I, J, K, X0, Y0, Z0);
		FSingle V1 = (T1 * T1) * GradientCoordinate(InSeed, I1.MaskAdd(I, FInteger_PrimeX), J1.MaskAdd(J, FInteger_PrimeY), K1.MaskAdd(K, FInteger_PrimeZ), X1, Y1, Z1);
		FSingle V2 = (T2 * T2) * GradientCoordinate(InSeed, I2.MaskAdd(I, FInteger_PrimeX), J2.MaskAdd(J, FInteger_PrimeY), K2.MaskAdd(K, FInteger_PrimeZ), X2, Y2, Z2);
		FSingle V3 = N3.Mask((T3 * T3) * GradientCoordinate(InSeed, I + FInteger_PrimeX, J + FInteger_PrimeY, K + FInteger_PrimeZ), X3, Y3, Z3);

		return FSingle_32 * N0.MaskAdd(N1.MaskAdd(N2.MaskAdd(V3, V2), V1), V0);
	}

	FSingle VECTORCALL FFastNoiseSIMD::CubicSingle(FInteger InSeed, FSingleVector InPoint)
	{
		FSingleVector PointF1 = FMath::FloorToFloat(InPoint);
		FIntegerVector Point1 = FIntegerVector::Convert(PointF1) * FIntegerVector_Prime;
		
		FIntegerVector Point0 = Point1 - FIntegerVector_Prime;
		FIntegerVector Point2 = Point1 + FIntegerVector_Prime;
		FIntegerVector Point3 = Point2 + FIntegerVector_Prime;
		
		FSingleVector PointS = InPoint - PointF1;
		
		return (CubicLerp(
			CubicLerp(
				CubicLerp(ValueCoordinate(InSeed, X0, Y0, Z0), ValueCoordinate(InSeed, X1, Y0, Z0), ValueCoordinate(InSeed, X2, Y0, Z0), ValueCoordinate(InSeed, X3, Y0, Z0), XS),
				CubicLerp(ValueCoordinate(InSeed, X0, Y1, Z0), ValueCoordinate(InSeed, X1, Y1, Z0), ValueCoordinate(InSeed, X2, Y1, Z0), ValueCoordinate(InSeed, X3, Y1, Z0), XS),
				CubicLerp(ValueCoordinate(InSeed, X0, Y2, Z0), ValueCoordinate(InSeed, X1, Y2, Z0), ValueCoordinate(InSeed, X2, Y2, Z0), ValueCoordinate(InSeed, X3, Y2, Z0), XS),
				CubicLerp(ValueCoordinate(InSeed, X0, Y3, Z0), ValueCoordinate(InSeed, X1, Y3, Z0), ValueCoordinate(InSeed, X2, Y3, Z0), ValueCoordinate(InSeed, X3, Y3, Z0), XS),
				YS),
			CubicLerp(
				CubicLerp(ValueCoordinate(InSeed, X0, Y0, Z1), ValueCoordinate(InSeed, X1, Y0, Z1), ValueCoordinate(InSeed, X2, Y0, Z1), ValueCoordinate(InSeed, X3, Y0, Z1), XS),
				CubicLerp(ValueCoordinate(InSeed, X0, Y1, Z1), ValueCoordinate(InSeed, X1, Y1, Z1), ValueCoordinate(InSeed, X2, Y1, Z1), ValueCoordinate(InSeed, X3, Y1, Z1), XS),
				CubicLerp(ValueCoordinate(InSeed, X0, Y2, Z1), ValueCoordinate(InSeed, X1, Y2, Z1), ValueCoordinate(InSeed, X2, Y2, Z1), ValueCoordinate(InSeed, X3, Y2, Z1), XS),
				CubicLerp(ValueCoordinate(InSeed, X0, Y3, Z1), ValueCoordinate(InSeed, X1, Y3, Z1), ValueCoordinate(InSeed, X2, Y3, Z1), ValueCoordinate(InSeed, X3, Y3, Z1), XS),
				YS),
			CubicLerp(
				CubicLerp(ValueCoordinate(InSeed, X0, Y0, Z2), ValueCoordinate(InSeed, X1, Y0, Z2), ValueCoordinate(InSeed, X2, Y0, Z2), ValueCoordinate(InSeed, X3, Y0, Z2), XS),
				CubicLerp(ValueCoordinate(InSeed, X0, Y1, Z2), ValueCoordinate(InSeed, X1, Y1, Z2), ValueCoordinate(InSeed, X2, Y1, Z2), ValueCoordinate(InSeed, X3, Y1, Z2), XS),
				CubicLerp(ValueCoordinate(InSeed, X0, Y2, Z2), ValueCoordinate(InSeed, X1, Y2, Z2), ValueCoordinate(InSeed, X2, Y2, Z2), ValueCoordinate(InSeed, X3, Y2, Z2), XS),
				CubicLerp(ValueCoordinate(InSeed, X0, Y3, Z2), ValueCoordinate(InSeed, X1, Y3, Z2), ValueCoordinate(InSeed, X2, Y3, Z2), ValueCoordinate(InSeed, X3, Y3, Z2), XS),
				YS),
			CubicLerp(
				CubicLerp(ValueCoordinate(InSeed, X0, Y0, Z3), ValueCoordinate(InSeed, X1, Y0, Z3), ValueCoordinate(InSeed, X2, Y0, Z3), ValueCoordinate(InSeed, X3, Y0, Z3), XS),
				CubicLerp(ValueCoordinate(InSeed, X0, Y1, Z3), ValueCoordinate(InSeed, X1, Y1, Z3), ValueCoordinate(InSeed, X2, Y1, Z3), ValueCoordinate(InSeed, X3, Y1, Z3), XS),
				CubicLerp(ValueCoordinate(InSeed, X0, Y2, Z3), ValueCoordinate(InSeed, X1, Y2, Z3), ValueCoordinate(InSeed, X2, Y2, Z3), ValueCoordinate(InSeed, X3, Y2, Z3), XS),
				CubicLerp(ValueCoordinate(InSeed, X0, Y3, Z3), ValueCoordinate(InSeed, X1, Y3, Z3), ValueCoordinate(InSeed, X2, Y3, Z3), ValueCoordinate(InSeed, X3, Y3, Z3), XS),
				YS),
			ZS) * FSingle_CubicBounding);
	}

	struct FGradientCoordinate
	{
	public:
		FInteger Hash;
		FSingle X;
		FSingle Y;
		FSingle Z;

		FGradientCoordinate(FSingle InX, FSingle InY, FSingle InZ)
		{
			Hash = HashHB(InX, InY, InZ);

			X = FSingle::Convert(Hash & FInteger_Bit10Mask) - FSingle_511_5;
			Y = FSingle::Convert((Hash >> 10) & FInteger_Bit10Mask) - FSingle_511_5;
			Z = FSingle::Convert((Hash >> 20) & FInteger_Bit10Mask) - FSingle_511_5;
		}
	};

	void VECTORCALL FFastNoiseSIMD::GradientPerturbSingle(FInteger InSeed, FSingle InPerturbAmplitude, FSingle InPerturbFrequency, FSingleVector& InOutPoint)
	{
		FSingle XF = InOutX * InPerturbFrequency;
		FSingle YF = InOutY * InPerturbFrequency;
		FSingle ZF = InOutZ * InPerturbFrequency;

		FSingle XS = FMath::Floor(XF);
		FSingle YS = FMath::Floor(YF);
		FSingle ZS = FMath::Floor(ZF);

		FInteger X0 = FInteger::Convert(XS) * FInteger_PrimeX;
		FInteger Y0 = FInteger::Convert(YS) * FInteger_PrimeY;
		FInteger Z0 = FInteger::Convert(ZS) * FInteger_PrimeZ;

		FInteger X1 = X0 + FInteger_PrimeX;
		FInteger Y1 = Y1 + FInteger_PrimeY;
		FInteger Z1 = Z1 + FInteger_PrimeZ;

		XS = InterpQuintic(XF - XS);
		YS = InterpQuintic(YF - YS);
		ZS = InterpQuintic(ZF - ZS);

		FGradientCoordinate G000(0, 0, 0);
		FGradientCoordinate G001(0, 0, 1);
		FGradientCoordinate G010(0, 1, 0);
		FGradientCoordinate G011(0, 1, 1);
		FGradientCoordinate G100(1, 0, 0);
		FGradientCoordinate G101(1, 0, 1);
		FGradientCoordinate G110(1, 1, 0);
		FGradientCoordinate G111(1, 1, 1);

		FSingle X0X = FMath::Lerp(FMath::Lerp(G000.X, G100.X, XS), FMath::Lerp(G010.X, G110.X, XS), YS);
		FSingle Y0Y = FMath::Lerp(FMath::Lerp(G000.Y, G100.Y, XS), FMath::Lerp(G010.Y, G110.Y, XS), YS);
		FSingle Z0Y = FMath::Lerp(FMath::Lerp(G000.Z, G100.Z, XS), FMath::Lerp(G010.Z, G110.Z, XS), YS);

		FSingle X1X = FMath::Lerp(FMath::Lerp(G001.X, G101.X, XS), FMath::Lerp(G011.X, G111.X, XS), YS);
		FSingle Y1Y = FMath::Lerp(FMath::Lerp(G001.Y, G101.Y, XS), FMath::Lerp(G011.Y, G111.Y, XS), YS);
		FSingle Z1Y = FMath::Lerp(FMath::Lerp(G001.Z, G101.Z, XS), FMath::Lerp(G011.Z, G111.Z, XS), YS);

		InOutX = FMath::Lerp(X0Y, X1Y, ZS).MultiplyAdd(InPerturbAmplitude, InOutX);
		InOutY = FMath::Lerp(Y0Y, Y1Y, ZS).MultiplyAdd(InPerturbAmplitude, InOutY);
		InOutZ = FMath::Lerp(Z0Z, Z1Y, ZS).MultiplyAdd(InPerturbAmplitude, InOutZ);
	}
}

// NOTE: Everything below here is old

#if defined(SIMD_LEVEL) || defined(FN_COMPILE_NO_SIMD_FALLBACK)

#ifndef SIMD_LEVEL
#define SIMD_LEVEL FN_NO_SIMD_FALLBACK
#define SIMD_LEVEL_H FN_NO_SIMD_FALLBACK
#include "FastNoiseInternal.h"
#define FN_ALIGNED_SETS
#endif

#define L_TYPE(T, L) T##_L##l
#define FSIMDFloat L_TYPE(SIMDf,  SIMD_LEVEL)
#define FSIMDInt L_TYPE(SIMi, SIMD_LEVEL)

#define L_uSIMD2(S) u##S
#define uSIMDf L_uSIMD(SIMDf)
#define uSIMDi L_uSIMD(SIMDi)

// ie. FSIMDFloat_1
#define L_SIMD_NUM(N, S) S##_##N
#define SIMDf_NUM(N) L_SIMD_NUM(N, SIMDf)
#define SIMDi_NUM(N) L_SIMD_NUM(N, SIMDi)

#define L_VAR(X, Y) L##Y##_##X
#define VAR(X) L_VAR(X, SIMD_LEVEL)
#define FUNC(X) VAR(FUNC_##X)

#define SIMD_LEVEL_CLASS FFastNoiseInternal::FASTNOISE_SIMD_CLASS(SIMD_LEVEL)

#if defined(_WIN32) && SIMD_LEVEL > FN_NO_SIMD_FALLBACK
#define VECTORCALL
#else
#define VECTORCALL
#endif

#if SIMD_LEVEL == ESIMDLevel::SL_ARM
const int32 VectorSize = 4;
const int32 MemoryAlignment = 16;
typedef float32x4_t FSIMDFloat;
typedef int32x4_t FSIMDInt;
FORCEINLINE FSIMDFloat SetFloat(FSIMDFloat InValue) { return vdupq_n_f32(InValue); }
FORCEINLINE FSIMDFloat ZeroFloat() { return vdupq_n_f32(0); }
FORCEINLINE FSIMDInt SetInt(FSIMDInt InValue) { return vdupq_n_s32(InValue); }
FORCEINLINE FSIMDInt ZeroInt() { return vdupq_n_s32(0); }
#elif SIMD_LEVEL == ESIMDLevel::SL_AVX512
//const int32 VectorSize = 16;
//const int32 MemoryAlignment = 32;
//typedef _m256 FSIMDFloat;
//typedef _m256i FSIMDInt;
//FORCEINLINE FSIMDFloat SetFloat(FSIMDFloat InValue) { return _mm256_set1_ps(InValue); }
//FORCEINLINE FSIMDFloat ZeroFloat() { return _mm256_setzero_ps; }
//FORCEINLINE FSIMDInt SetInt(FSIMDInt InValue) { return _mm256_set1_epi32(InValue); }
//FORCEINLINE FSIMDInt ZeroInt() { return _mm256_setzero_si256(); }
#elseif SIMD_LEVEL >= ESIMDLevel::SL_SSE2
const int32 VectorSize = 4;
const int32 MemoryAlignment = 16;
typedef _m128 FSIMDFloat;
typedef _m128i FSIMDInt;
FORCEINLINE FSIMDFloat SetFloat(FSIMDFloat InValue) { return _mm_set1_ps(InValue); }
FORCEINLINE FSIMDFloat ZeroFloat() { return _mm_setzero_ps; }
FORCEINLINE FSIMDInt SetInt(FSIMDInt InValue) { return _mm_set1_epi32(A); }
FORCEINLINE FSIMDInt ZeroInt() { return _mm_setzero_si128(); }
#else
const int32 VectorSize = 1;
const int32 MemoryAlignment = 4;
typedef float FSIMDFloat;
typedef int FSIMDInt;
FORCEINLINE FSIMDFloat SetFloat(FSIMDFloat InValue) { return InValue; }
FORCEINLINE FSIMDFloat ZeroFloat() { return 0.0f; }
FORCEINLINE FSIMDInt SetInt(FSIMDInt InValue) { return InValue; }
FORCEINLINE FSIMDInt ZeroInt() { return 0; }

typedef TArray<FSIMDFloat, TAlignedHeapAllocator<MemoryAlignment>> FNoiseArray;

#ifdef SIMD_LEVEL == ESIMDLevel::SL_AVX512
typedef __mask16 FMask;
#else
typedef FSIMDInt FMask;
#endif

static FSIMDFloat SIMDf_NUM(1);
static FSIMDInt SIMDi_NUM(0xffffffff);

#if SIMD_LEVEL == ESIMDLevel::SL_ARM
FORCEINLINE void StoreFloat(FSIMDFloat InDestination, FSIMDFloat InSource) { vst1q_f32(InDestination, InSource); }
FORCEINLINE FSIMDFloat LoadFloat(FSIMDFloat InValue) { return vld1q_f32(InValue); }

FORCEINLINE FSIMDFloat ConvertToFloat(FSIMDInt InValue) { return vcvtq_f32_s32(InValue); }
FORCEINLINE FSIMDFloat CastToFloat(FSIMDInt InValue) { return vreinterpret_f32_s32(InValue); }
FORCEINLINE FSIMDInt ConvertToInt(FSIMDFloat InValue) { return vcvtq_s32_f32(InValue); }
FORCEINLINE FSIMDInt CastToInt(FSIMDFloat InValue) { return vreinterpret_s32_f32(InValue); }

FORCEINLINE FSIMDFloat& operator+(FSIMDFloat InLeft, FSIMDFloat InRight) { return vaddq_f32(InLeft, InRight); }
FORCEINLINE FSIMDFloat& operator-(FSIMDFloat InLeft, FSIMDFloat InRight) { return vsub_f32(InLeft, InRight); }
FORCEINLINE FSIMDFloat& operator*(FSIMDFloat InLeft, FSIMDFloat InRight) { return vmulq_f32(InLeft, InRight); }
FORCEINLINE FSIMDFloat& operator/(FSIMDFloat InLeft, FSIMDFloat InRight)
{
	FSIMDFloat Reciprocal = vrecpeq_f32(InRight);
	Reciprocal = vmulq_f32(vrecpsq_f32(InRight, Reciprocal), Reciprocal);
	return vmulq_f32(InLeft, Reciprocal);
}

//FORCEINLINE FSIMDFloat FloatMin(FSIMDFloat InLeft, FSIMDFloat InRight) { return vminq_f32(InLeft, InRight); }
//FORCEINLINE FSIMDFloat FloatMax(FSIMDFloat InLeft, FSIMDFloat InRight) { return vmax_f32(InLeft, InRight); }
//FORCEINLINE FSIMDFloat FloatInverseSqrt(FSIMDFloat InValue) { return vrsqrteq_f32(InValue); }

FORCEINLINE bool operator<(FSIMDFloat InLeft, FSIMDFloat InRight) { return vreinterpret_f32_u32(vcltq_f32(InLeft, InRight)); }
FORCEINLINE bool operator>(FSIMDFloat InLeft, FSIMDFloat InRight) { return vreinterpret_f32_u32(vcgtq_f32(InLeft, InRight)); }
FORCEINLINE bool operator<=(FSIMDFloat InLeft, FSIMDFloat InRight) { return vreinterpret_f32_u32(vcleq_f32(InLeft, InRight)); }
FORCEINLINE bool operator>=(FSIMDFloat InLeft, FSIMDFloat InRight) { return vreinterpret_f32_u32(vcgeq_f32(InLeft, InRight)); }

FORCEINLINE bool operator&&(FSIMDFloat InLeft, FSIMDFloat InRight) { CastToFloat(vandq_s32(vreinterpret_s32_f32(InLeft), vreinterpret_s32_f32(InRight))); }
FORCEINLINE bool operator&!(FSIMDFloat InLeft, FSIMDFloat InRight) { CastToFloat(vandq_s32(vmvnq_s32(vreinterpret_s32_f32(InLeft), vreinterpret_s32_f32(InRight)))); }
FORCEINLINE bool operator^(FSIMDFloat InLeft, FSIMDFloat InRight) { CastToFloat(veorq_s32(vreinterpret_s32_f32(InLeft), vreinterpret_s32_f32(InRight))); }

#ifndef __aarch64__
static FSIMDFloat FUNC(FLOOR)(const FSIMDFloat& InValue)
{
	FSIMDFloat Result = ConvertToFloat(ConvertToInt(InValue));
	return vsubq_f32(Result, FloatAnd(FloatLessThan(InValue, Result), SIMDf_NUM(1)));
}
FORCEINLINE FSIMDFloat FloatFloor(FSIMDFloat InValue) { return FUNC(FLOOR); }
#else
FORCEINLINE FSIMDFloat FloatFloor(FSIMDFloat InValue) { return vrndmq_f32(InValue); }

FORCEINLINE FSIMDFloat FloatBlend(FSIMDFloat InLeft, FSIMDFloat InRight, FSIMDFloat InMask) { return vbslq_f32(InMask, InRight, InLeft); }

FORCEINLINE FSIMDInt operator+(FSIMDInt InLeft, FSIMDInt InRight) { return vaddq_s32(InLeft, InRight); }
FORCEINLINE FSIMDInt operator-(FSIMDInt InLeft, FSIMDInt InRight) { return vsubq_s32(InLeft, InRight); }
FORCEINLINE FSIMDInt operator*(FSIMDInt InLeft, FSIMDInt InRight) { return vmulq_s32(InLeft, InRight); }

FORCEINLINE bool operator&&(FSIMDInt InLeft, FSIMDInt InRight) { return vandq_s32(InLeft, InRight); }
FORCEINLINE bool operator&!(FSIMDInt InLeft, FSIMDInt InRight) { return vandq_s32(vmvnq_s32(InLeft, InRight)); }
FORCEINLINE bool operator||(FSIMDInt InLeft, FSIMDInt InRight) { return vorrq_s32(InLeft, InRight); }
FORCEINLINE bool operator^(FSIMDInt InLeft, FSIMDInt InRight) { return veorq_s32(InLeft, InRight); }
FORCEINLINE bool operator!(FSIMDInt InValue) { return vmvnq_s32(InValue); }

FORCEINLINE FSIMDInt operator>>(FSIMDInt InLeft, FSIMDInt InRight) { return vshrq_n_s32(InLeft, InRight); }
FORCEINLINE FSIMDInt operator<<(FSIMDInt InLeft, FSIMDInt InRight) { return vshlq_n_s32(InLeft, InRight); }
FORCEINLINE FSIMDInt IntVShiftLeft(FSIMDInt InLeft, FSIMDInt InRight) { return vshlq_s32(InLeft, InRight); }

FORCEINLINE bool operator==(FSIMDInt InLeft, FSIMDInt InRight) { return vreinterpret_s32_u32(InLeft, InRight); }
FORCEINLINE bool operator<(FSIMDInt InLeft, FSIMDInt InRight) { return vreinterpret_s32_u32(vcgtq_s32(InLeft, InRight)); }
FORCEINLINE bool operator>(FSIMDInt InLeft, FSIMDInt InRight) { return vreinterpret_s32_u32(vcltq_s32(InLeft, InRight)); }

#elif SIMD_LEVEL == ESIMDLevel::SL_AVX512

#ifdef FN_ALIGNED_SETS
FORCEINLINE void FloatStore(FSIMDFloat InDestination, FSIMDFloat InSource) { _mm512_store_ps(InDestination, InSource); }
FORCEINLINE FSIMDFloat LoadFloat(FSIMDFloat InValue) { return _mm512_load_ps(InValue); }
#else
FORCEINLINE void FloatStore(FSIMDFloat InDestination, FSIMDFloat InSource) { _mm512_storeu_ps(InDestination, InSource); }
FORCEINLINE FSIMDFloat LoadFloat(FSIMDFloat InValue) { return _mm512_loadu_ps(InValue); }
#endif

//FORCEINLINE FSIMDFloat operator+(FSIMDFloat InLeft, FSIMDFloat InRight) { return _mm512_add_ps(InLeft, InRight); }
//FORCEINLINE FSIMDFloat operator-(FSIMDFloat InLeft, FSIMDFloat InRight) { return _mm512_sub_ps(InLeft, InRight); }
//FORCEINLINE FSIMDFloat operator*(FSIMDFloat InLeft, FSIMDFloat InRight) { return _mm512_mul_ps(InLeft, InRight); }
//FORCEINLINE FSIMDFloat operator/(FSIMDFloat InLeft, FSIMDFloat InRight) { return _mm512_div_ps(InLeft, InRight); }

//FORCEINLINE FSIMDFloat FloatMin(FSIMDFloat InLeft, FSIMDFloat InRight) { return _mm512_min_ps(InLeft, InRight); }
//FORCEINLINE FSIMDFloat FloatMax(FSIMDFloat InLeft, FSIMDFloat InRight) { return _mm512_max_ps(InLeft, InRight); }
//FORCEINLINE FSIMDFloat FloatInverseSqrt(FSIMDFloat InValue) { return _mm512_rsqrt14_ps(InValue); }

//FORCEINLINE bool operator<(FSIMDFloat InLeft, FSIMDFloat InRight) { return _mm512_cmp_ps_mask(InLeft, InRight, _CMP_LT_OQ); }
//FORCEINLINE bool operator>(FSIMDFloat InLeft, FSIMDFloat InRight) { return _mm512_cmp_ps_mask(InLeft, InRight, _CMP_GT_OQ); }
//FORCEINLINE bool operator<=(FSIMDFloat InLeft, FSIMDFloat InRight) { return _mm512_cmp_ps_mask(InLeft, InRight, _CMP_LE_OQ); }
//FORCEINLINE bool operator>=(FSIMDFloat InLeft, FSIMDFloat InRight) { return _mm512_cmp_ps_mask(InLeft, InRight, _CMP_GE_OQ); }

FORCEINLINE bool operator&&(FSIMDFloat InLeft, FSIMDFloat InRight) { return _mm512_and_ps(InLeft, InRight); }
FORCEINLINE bool operator&!(FSIMDFloat InLeft, FSIMDFloat InRight) { return _mm512_andnot_ps(InLeft, InRight); }
FORCEINLINE bool operator^(FSIMDFloat InLeft, FSIMDFloat InRight) { return _mm512_xor_ps(InLeft, InRight); }

//FORCEINLINE FSIMDFloat FloatFloor(FSIMDFloat InValue) { return _mm512_floor_ps(InValue); }
//FORCEINLINE FSIMDFloat FloatBlend(FSIMDFloat InLeft, FSIMDFloat InRight, FSIMDFloat InMask) { return _mm512_mask_blend_ps(InMask, InLeft, InRight); }
//FORCEINLINE FSIMDFloat FloatGather(FSIMDFloat InLeft, FSIMDFloat InRight) { return _mm512_i32gather_ps(InRight, InLeft, 4); }
//FORCEINLINE FSIMDFloat FloatPermute(FSIMDFloat InLeft, FSIMDFloat InRight) { return _mm512_permutexvar_ps(InLeft, InRight); }

//FORCEINLINE FSIMDInt operator+(FSIMDInt InLeft, FSIMDInt InRight) { return _mm512_add_epi32(InLeft, InRight); }
//FORCEINLINE FSIMDInt operator-(FSIMDInt InLeft, FSIMDInt InRight) { return _mm512_sub_epi32(InLeft, InRight); }
//FORCEINLINE FSIMDInt operator*(FSIMDInt InLeft, FSIMDInt InRight) { return _mm512_mullo_epi32(InLeft, InRight); }

FORCEINLINE bool operator&&(FSIMDInt InLeft, FSIMDInt InRight) { return _mm512_and_si512(InLeft, InRight); }
FORCEINLINE bool operator&!(FSIMDInt InLeft, FSIMDInt InRight) { return _mm512_andnot_si512(InLeft, InRight); }
FORCEINLINE bool operator||(FSIMDInt InLeft, FSIMDInt InRight) { return _mm512_or_si512(InLeft, InRight); }
FORCEINLINE bool operator^(FSIMDInt InLeft, FSIMDInt InRight) { return _mm512_xor_si512(InLeft, InRight); }
FORCEINLINE bool operator!(FSIMDInt InValue) { return SIMDi_XOR(InValue, SIMDi_NUM(0xffffffff); }

//FORCEINLINE FSIMDInt operator>>(FSIMDInt InLeft, FSIMDInt InRight) { return _mm512_srai_epi32(InLeft, InRight); }
//FORCEINLINE FSIMDInt operator<<(FSIMDInt InLeft, FSIMDInt InRight) { return _mm512_slli_epi32(InLeft, InRight); }

FORCEINLINE FSIMDInt IntVShiftRight(FSIMDInt InLeft, FSIMDInt InRight) { return _mm512_srl_epi32(InLeft, InRight); }
FORCEINLINE FSIMDInt IntVShiftLeft(FSIMDInt InLeft, FSIMDInt InRight) { return _mm512_sll_epi32(InLeft, InRight); }

//FORCEINLINE bool operator==(FSIMDInt InLeft, FSIMDInt InRight) { return _mm512_cmpeq_epi32_mask(InLeft, InRight); }
//FORCEINLINE bool operator>(FSIMDInt InLeft, FSIMDInt InRight) { return _mm512_cmpgt_epi32_mask(InLeft, InRight); }
//FORCEINLINE bool operator<(FSIMDInt InLeft, FSIMDInt InRight) { return _mm512_cmpgt_epi32_mask(InRight, InLeft); }

//FORCEINLINE FSIMDFloat ConvertToFloat(FSIMDInt InValue) { return _mm512_cvtepi32_ps(InValue); }
//FORCEINLINE FSIMDFloat CastToFloat(FSIMDInt InValue) { return _mm512_castsi512_ps(InValue); }
//FORCEINLINE FSIMDInt ConvertToInt(FSIMDFloat InValue) { return _mm512_cvtps_epi32(InValue); }
//FORCEINLINE FSIMDInt CastToInt(FSIMDFloat InValue) { return _mm512_castps_si512(InValue); }

#elif SIMD_LEVEL == ESIMDLevel::SL_AVX2

#ifdef FN_ALIGNED_SETS
FORCEINLINE void FloatStore(FSIMDFloat InDestination, FSIMDFloat InSource) { _mm256_store_ps(InDestination, InSource); }
FORCEINLINE FSIMDFloat LoadFloat(FSIMDFloat InValue) { return _mm256_load_ps(InValue); }
#else
FORCEINLINE void FloatStore(FSIMDFloat InDestination, FSIMDFloat InSource) { _mm256_storeu_ps(InDestination, InSource); }
FORCEINLINE FSIMDFloat LoadFloat(FSIMDFloat InValue) { return _mm256_loadu_ps(InValue); }
#endif

FORCEINLINE FSIMDFloat ConvertToFloat(FSIMDInt InValue) { return _mm256_cvtepi32_ps(InValue); }
FORCEINLINE FSIMDFloat CastToFloat(FSIMDInt InValue) { return _mm256_castsi256_ps(InValue); }
FORCEINLINE FSIMDInt ConvertToInt(FSIMDFloat InValue) { return _mm256_cvtps_epi32(InValue); }
FORCEINLINE FSIMDInt CastToInt(FSIMDFloat InValue) { return _mm256_castps_si256(InValue); }

FORCEINLINE FSIMDFloat operator+(FSIMDFloat InLeft, FSIMDFloat InRight) { return _mm256_add_ps(InLeft, InRight); }
FORCEINLINE FSIMDFloat operator-(FSIMDFloat InLeft, FSIMDFloat InRight) { return _mm256_sub_ps(InLeft, InRight); }
FORCEINLINE FSIMDFloat operator*(FSIMDFloat InLeft, FSIMDFloat InRight) { return _mm256_mul_ps(InLeft, InRight); }
FORCEINLINE FSIMDFloat operator/(FSIMDFloat InLeft, FSIMDFloat InRight) { return _mm256_div_ps(InLeft, InRight); }

FORCEINLINE FSIMDFloat FloatMin(FSIMDFloat InLeft, FSIMDFloat InRight) { return _mm256_min_ps(InLeft, InRight); }
FORCEINLINE FSIMDFloat FloatMax(FSIMDFloat InLeft, FSIMDFloat InRight) { return _mm256_max_ps(InLeft, InRight); }
FORCEINLINE FSIMDFloat FloatInverseSqrt(FSIMDFloat InValue) { return _mm256_rsqrt_ps(InValue); }

FORCEINLINE bool operator<(FSIMDFloat InLeft, FSIMDFloat InRight) { return CastToInt(_mm256_cmp_ps(InLeft, InRight, _CMP_LT_OQ)); }
FORCEINLINE bool operator>(FSIMDFloat InLeft, FSIMDFloat InRight) { return CastToInt(_mm256_cmp_ps(InLeft, InRight, _CMP_GT_OQ)); }
FORCEINLINE bool operator<=(FSIMDFloat InLeft, FSIMDFloat InRight) { return CastToInt(_mm256_cmp_ps(InLeft, InRight, _CMP_LE_OQ)); }
FORCEINLINE bool operator>=(FSIMDFloat InLeft, FSIMDFloat InRight) { return CastToInt(_mm256_cmp_ps(InLeft, InRight, _CMP_GE_OQ)); }

FORCEINLINE bool operator&&(FSIMDFloat InLeft, FSIMDFloat InRight) { return _mm256_and_ps(InLeft, InRight); }
FORCEINLINE bool operator&!(FSIMDFloat InLeft, FSIMDFloat InRight) { return _mm256_andnot_ps(InLeft, InRight); }
FORCEINLINE bool operator^(FSIMDFloat InLeft, FSIMDFloat InRight) { return _mm256_xor_ps(InLeft, InRight); }

FORCEINLINE FSIMDFloat FloatFloor(FSIMDFloat InValue) { return _mm256_floor_ps(InValue); }
FORCEINLINE FSIMDFloat FloatBlend(FSIMDFloat InLeft, FSIMDFloat InRight, FSIMDFloat InMask) { return _mm256_blendv_ps(InLeft, InRight, CastToFloat(InMask)); }
FORCEINLINE FSIMDFloat FloatGather(FSIMDFloat InLeft, FSIMDFloat InRight) { return _mm256_i32gather_ps(InLeft, InRight, 4); }
FORCEINLINE FSIMDFloat FloatPermute(FSIMDFloat InLeft, FSIMDFloat InRight) { return _mm256_permutevar8x32_ps(InLeft, InRight); }

FORCEINLINE FSIMDInt operator+(FSIMDInt InLeft, FSIMDInt InRight) { return _mm256_add_epi32(InLeft, InRight); }
FORCEINLINE FSIMDInt operator-(FSIMDInt InLeft, FSIMDInt InRight) { return _mm256_sub_epi32(InLeft, InRight); }
FORCEINLINE FSIMDInt operator*(FSIMDInt InLeft, FSIMDInt InRight) { return _mm256_mullo_epi16(InLeft, InRight); }

FORCEINLINE bool operator&&(FSIMDInt InLeft, FSIMDInt InRight) { return _mm256_and_si256(InLeft, InRight); }
FORCEINLINE bool operator&!(FSIMDInt InLeft, FSIMDInt InRight) { return _mm256_andnot_si256(InLeft, InRight); }
FORCEINLINE bool operator||(FSIMDInt InLeft, FSIMDInt InRight) { return _mm256_or_si256(InLeft, InRight); }
FORCEINLINE bool operator^(FSIMDInt InLeft, FSIMDInt InRight) { return _mm256_xor_si256(InLeft, InRight); }
FORCEINLINE bool operator!(FSIMDInt InValue) { return InValue ^ SIMDi_NUM(0xffffffff); }

FORCEINLINE FSIMDInt operator>>(FSIMDInt InLeft, FSIMDInt InRight) { return _mm256_srai_epi32(InLeft, InRight); }
FORCEINLINE FSIMDInt operator<<(FSIMDInt InLeft, FSIMDInt InRight) { return _mm256_slli_epi32(InLeft, InRight); }

FORCEINLINE bool operator==(FSIMDInt InLeft, FSIMDInt InRight) { return _mm256_cmpeq_epi32(InLeft, InRight); }
FORCEINLINE bool operator>(FSIMDInt InLeft, FSIMDInt InRight) { return _mm256_cmpgt_epi32(InLeft, InRight); }
FORCEINLINE bool operator<(FSIMDInt InLeft, FSIMDInt InRight) { return _mm256_cmpgt_epi32(InLeft, InRight); }

#elif SIMD_LEVEL == ESIMDLevel::SL_SSE2

#ifdef FN_ALIGNED_SETS
FORCEINLINE void FloatStore(FSIMDFloat InDestination, FSIMDFloat InSource) { _mm_store_ps(InDestination, InSource); }
FORCEINLINE FSIMDFloat LoadFloat(FSIMDFloat InValue) { return _mm_load_ps(InValue); }
#else
FORCEINLINE void FloatStore(FSIMDFloat InDestination, FSIMDFloat InSource) { _mm_storeu_ps(InDestination, InSource); }
FORCEINLINE FSIMDFloat LoadFloat(FSIMDFloat InValue) { return _mm_loadu_ps(InValue); }
#endif

FORCEINLINE FSIMDFloat ConvertToFloat(FSIMDInt InValue) { return _mm_cvtepi32_ps(InValue); }
FORCEINLINE FSIMDFloat CastToFloat(FSIMDInt InValue) { return _mm_castsi128_ps(InValue); }
FORCEINLINE FSIMDInt ConvertToInt(FSIMDFloat InValue) { return _mm_cvtps_epi32(InValue); }
FORCEINLINE FSIMDInt CastToInt(FSIMDFloat InValue) { return _mm_castps_si128(InValue); }

FORCEINLINE FSIMDFloat operator+(FSIMDFloat InLeft, FSIMDFloat InRight) { return _mm_add_ps(InLeft, InRight); }
FORCEINLINE FSIMDFloat operator-(FSIMDFloat InLeft, FSIMDFloat InRight) { return _mm_sub_ps(InLeft, InRight); }
FORCEINLINE FSIMDFloat operator*(FSIMDFloat InLeft, FSIMDFloat InRight) { return _mm_mul_ps(InLeft, InRight); }
FORCEINLINE FSIMDFloat operator/(FSIMDFloat InLeft, FSIMDFloat InRight) { return _mm_div_ps(InLeft, InRight); }

FORCEINLINE FSIMDFloat FloatMin(FSIMDFloat InLeft, FSIMDFloat InRight) { return _mm_min_ps(InLeft, InRight); }
FORCEINLINE FSIMDFloat FloatMax(FSIMDFloat InLeft, FSIMDFloat InRight) { return _mm_max_ps(InLeft, InRight); }
FORCEINLINE FSIMDFloat FloatInverseSqrt(FSIMDFloat InValue) { return _mm_rsqrt_ps(InValue); }

FORCEINLINE bool operator<(FSIMDFloat InLeft, FSIMDFloat InRight) { return CastToInt(_mm_cmplt_ps(InLeft, InRight)); }
FORCEINLINE bool operator>(FSIMDFloat InLeft, FSIMDFloat InRight) { return CastToInt(_mm_cmpgt_ps(InLeft, InRight)); }
FORCEINLINE bool operator<=(FSIMDFloat InLeft, FSIMDFloat InRight) { return CastToInt(_mm_cmple_ps(InLeft, InRight)); }
FORCEINLINE bool operator>=(FSIMDFloat InLeft, FSIMDFloat InRight) { return CastToInt(_mm_cmpge_ps(InLeft, InRight)); }

FORCEINLINE bool operator&&(FSIMDFloat InLeft, FSIMDFloat InRight) { return _mm_and_ps(InLeft, InRight); }
FORCEINLINE bool operator&!(FSIMDFloat InLeft, FSIMDFloat InRight) { return _mm_andnot_ps(InLeft, InRight); }
FORCEINLINE bool operator^(FSIMDFloat InLeft, FSIMDFloat InRight) { return _mm_xor_ps(InLeft, InRight); }

#if SIMDLEVEL == ESIMDLevel::SL_SSE41
FORCEINLINE FSIMDInt operator*(FSIMDInt InLeft, FSIMDInt InRight) { return _mm_mullo_epi32(InLeft, InRight); }
FORCEINLINE FSIMDFloat FloatFloor(FSIMDFloat InValue) { return _mm_floor_ps(InValue); }
FORCEINLINE FSIMDFloat FloatBlend(FSIMDFloat InLeft, FSIMDFloat InRight, FSIMDFloat InMask) { return _mm_blendv_ps(InLeft, InRight, CastToFloat(InMask)); }
#else
static FSIMDInt VECTORCALL FUNC(MUL)(const FSIMDFloat& InLeft, const FSIMDFloat& InRight)
{
	__m128 Temp1 = _mm_castsi128_ps(_mm_mul_epu32(InLeft, InRight));
	__m128 Temp2 = _mm_castsi128_ps(_mm_mul_epu32(_mm_srli_si128(InLeft, 4), _mm_srli_si128(InRight, 4)));
	return _mm_shuffle_epi32(_mm_castps_si128(_mm_shuffle_ps(Temp1, Temp2, _MM_SHUFFLE(2, 0, 2, 0))), _MM_SHUFFLE(3, 1, 2, 0));
}
FORCEINLINE FSIMDFloat operator*(FSIMDFloat InLeft, FSIMDFloat InRight) { return FUNC(MUL)(InLeft, InRight); }

static FSIMDFloat VECTORCALL FUNC(GATHER)(const FSIMDFloat* InDestination, const FSIMDFloat& InSource)
{
	const uSIMDi* M = reinterpret_cast<const uSIMDi*>(&InSource);
	uSIMDf R;

	for (auto i = 0; i < VectorSize; i++)
		R.a[i] = *(InDestination + M->a[i]);

	return R.m;
}

FORCEINLINE FSIMDFloat FloatGather(FSIMDFloat InLeft, FSIMDFloat InRight) { return FUNC(GATHER)(InLeft, InRight); }

FORCEINLINE FSIMDInt operator+(FSIMDInt InLeft, FSIMDInt InRight) { return _mm_add_epi32(InLeft, InRight); }
FORCEINLINE FSIMDInt operator-(FSIMDInt InLeft, FSIMDInt InRight) { return _mm_sub_epi32(InLeft, InRight); }

FORCEINLINE bool operator&&(FSIMDInt InLeft, FSIMDInt InRight) { return _mm_and_si128(InLeft, InRight); }
FORCEINLINE bool operator&!(FSIMDInt InLeft, FSIMDInt InRight) { return _mm_andnot_si128(InLeft, InRight); }
FORCEINLINE bool operator||(FSIMDInt InLeft, FSIMDInt InRight) { return _mm_or_si128(InLeft, InRight); }
FORCEINLINE bool operator^(FSIMDInt InLeft, FSIMDInt InRight) { return _mm_xor_si128(InLeft, InRight); }
FORCEINLINE bool operator!(FSIMDInt InValue) { return InValue ^ SIMDi_NUM(0xffffffff); }

FORCEINLINE FSIMDInt operator>>(FSIMDInt InLeft, FSIMDInt InRight) { return _mm_srai_epi32(InLeft, InRight); }
FORCEINLINE FSIMDInt operator<<(FSIMDInt InLeft, FSIMDInt InRight) { return _mm_slli_epi32(InLeft, InRight); }

FORCEINLINE bool operator==(FSIMDInt InLeft, FSIMDInt InRight) { return _mm_cmpeq_epi32(InLeft, InRight); }
FORCEINLINE bool operator>(FSIMDFloat InLeft, FSIMDFloat InRight) { return _mm_cmpgt_epi32(InLeft, InRight); }
FORCEINLINE bool operator<(FSIMDFloat InLeft, FSIMDFloat InRight) { return _mm_cmpgt_epi32(InRight, InLeft); }

#else

static int32 FUNC(CAST_TO_INT)(float InValue) { return *reinterpret_cast<int32*>(&InValue); }
static float FUNC(CAST_TO_FLOAT)(int32 InValue) { return *reinterpret_cast<float*>(&InValue); }
FORCEINLINE FSIMDInt CastToInt(FSIMDFloat InValue) { return FUNC(CAST_TO_INT)(InValue); }
FORCEINLINE FSIMDFloat CastToFloat(FSIMDInt InValue) { return FUNC(CAST_TO_FLOAT)(InValue); }

FORCEINLINE void FloatStore(FSIMDFloat InDestination, FSIMDFloat InSource) { (*(InDestination) = InSource); }
FORCEINLINE void FloatLoad(FSIMDFloat InValue) { return (*InValue); }

#endif

#if SIMD_LEVEL == ESIMDLevel::SL_AVX512

//FORCEINLINE bool operator|(FMask InLeft, FMask InRight) { return ((InLeft) | (InRight)); }
//FORCEINLINE bool operator&(FMask InLeft, FMask InRight) { return ((InLeft) & (InRight)); }
//FORCEINLINE bool operator~(FMask InLeft, FMask InRight) { return (~(InLeft) & (InRight)); }
//FORCEINLINE bool operator~(FMask InValue) { return (~(InValue)); }

//FORCEINLINE FSIMDFloat FloatMask(FMask InMask, FSIMDFloat InValue) { return _mm512_maskz_mov_ps(InMask, InValue); }
//FORCEINLINE FSIMDFloat FloatMaskAdd(FMask InMask, FSIMDFloat InLeft, FSIMDFloat InRight) { return _mm512_mask_add_ps(InLeft, InMask, InLeft, InRight); }
//FORCEINLINE FSIMDFloat FloatMaskSubtract(FMask InMask, FSIMDFloat InLeft, FSIMDFloat InRight) { return _mm512_mask_sub_ps(InLeft, InMask, InLeft, InRight); }
//
//FORCEINLINE FSIMDInt IntMaskAdd(FMask InMask, FSIMDInt InLeft, FSIMDInt InRight) { return _mm512_mask_add_epi32(InLeft, InMask, InLeft, InRight); }
//FORCEINLINE FSIMDInt IntMaskSub(FMask InMask, FSIMDInt InLeft, FSIMDInt InRight) { return _mm512_mask_sub_epi32(InLeft, InMask, InLeft, InRight); }

#else

FORCEINLINE bool operator||(FMask InLeft, FMask InRight) { return InLeft || InRight; }
FORCEINLINE bool operator&&(FMask InLeft, FMask InRight) { return InLeft && InRight; }
FORCEINLINE bool operator&&!(FMask InLeft, FMask InRight) { return InLeft && !InRight; }
FORCEINLINE bool operator!(FMask InValue) { return !InValue; }

FORCEINLINE FSIMDFloat FloatMask(FMask InMask, FSIMDFloat InValue) { return CastToFloat(InMask) && InValue; }
FORCEINLINE FSIMDFloat FloatMaskAdd(FMask InMask, FSIMDFloat InLeft, FSIMDFloat InRight) { return InLeft + (CastToFloat(InMask) && InRight); }
FORCEINLINE FSIMDFloat FloatMaskSubtract(FMask InMask, FSIMDFloat InLeft, FSIMDFloat InRight) { return InLeft - (CastToFloat(InMask) && InRight); }

FORCEINLINE FSIMDInt IntMaskAdd(FMask InMask, FSIMDInt InLeft, FSIMDInt InRight) { InLeft + (InMask && InRight); }
FORCEINLINE FSIMDInt IntMaskSub(FMask InMask, FSIMDInt InLeft, FSIMDInt InRight) { InLeft - (InMask && InRight); }

#endif

#if SIMD_LEVEL == FN_NO_SIMD_FALLBACK
FORCEINLINE FSIMDFloat FloatAbs(FSIMDFloat InValue) { return FMath::Abs(InValue); }
#else
FORCEINLINE FSIMDFloat FloatAbs(FSIMDFloat InValue) { InValue && (CastToFloat(SIMDi_NUM(0x7fffffff)); }
#endif

#if SIMD_LEVEL == ESIMDLevel::SL_AVX2
FORCEINLINE void ZeroAll() { }
#else
FORCEINLINE void ZeroAll() { }
#endif

#ifdef FN_USE_FMA
#if SIMD_LEVEL == ESIMDLevel::SL_ARM
FORCEINLINE FSIMDFloat FloatMultiplyAdd(FSIMDFloat InLeft, FSIMDFloat InRight, FSIMDFloat InExtra) { return vmlaq_f32(InRight, InExtra, InLeft); }
FORCEINLINE FSIMDFloat FloatMultiplySubtract(FSIMDFloat InLeft, FSIMDFloat InRight, FSIMDFloat InExtra) { return ((InLeft * InRight) - InExtra); }
#else SIMD_LEVEL == ESIMDLevel::SL_AVX512
//FORCEINLINE FSIMDFloat FloatMultiplyAdd(FSIMDFloat InLeft, FSIMDFloat InRight, FSIMDFloat InExtra) { return _mm512_fmadd_ps(InLeft, InRight, InExtra); }
//FORCEINLINE FSIMDFloat FloatMultiplySubtract(FSIMDFloat InLeft, FSIMDFloat InRight, FSIMDFloat InExtra) { return _mm512_fmsub_ps(InLeft, InRight, InExtra); }
#else SIMD_LEVEL == ESIMDLevel::SL_AVX2
FORCEINLINE FSIMDFloat FloatMultiplyAdd(FSIMDFloat InLeft, FSIMDFloat InRight, FSIMDFloat InExtra) { return _mm256_fmadd_ps(InLeft, InRight, InExtra); }
FORCEINLINE FSIMDFloat FloatMultiplySubtract(FSIMDFloat InLeft, FSIMDFloat InRight, FSIMDFloat InExtra) { return _mm256_fmsub_ps(InLeft, InRight, InExtra); }
#else
FORCEINLINE FSIMDFloat FloatMultiplyAdd(FSIMDFloat InLeft, FSIMDFloat InRight, FSIMDFloat InExtra) { return (InLeft * InRight) + InExtra; }
FORCEINLINE FSIMDFloat FloatMultiplySubtract(FSIMDFloat InLeft, FSIMDFloat InRight, FSIMDFloat InExtra) { return (InLeft, InRight) - InExtra; }
#endif
#endif

static bool VAR(SIMD_Values_Set) = false;

static FSIMDFloat VECTORCALL FUNC(Lerp)(const FSIMDFloat& InLeft, const FSIMDFloat& InRight, const FSIMDFloat& InAlpha)
{
	SIMDr Result;
	Result = InRight - InLeft;
	Result = FloatMultiplyAdd(Result, InAlpha, InLeft);
	return Result;
}

static FSIMDFloat VECTORCALL FUNC(InterpQuintic)(const FSIMDFloat& InValue)
{
	SIMDr Result;
	Result = FloatMultiplySubtract(InValue, SIMDf_NUM(6), SIMDf_NUM(15));
	Result = FloatMultiplyAdd(Result, InValue, SIMDf_NUM(10));
	Result = Result * InValue;
	Result = Result * InValue;
	Result = Result * InValue;
	return Result;
}

static FSIMDFloat FUNC(CubicLerp)(const FSIMDFloat& InA, const FSIMDFloat& InB, const FSIMDFloat& InC, const FSIMDFloat& InD, const FSIMDFloat& InAlpha)
{
	FSIMDFloat Result = (InD - InC) - (InA - InB);
	return FloatMultiplyAdd(InAlpha, (InAlpha * (InAlpha * Result)), FloatMultiplyAdd(InAlpha, (InAlpha * ((InA - InB) - Result)), FloatMultiplyAdd(InAlpha, (InC - InA), InB)));
}

static FSIMDInt VECTORCALL FUNC(Hash)(const FSIMDInt& InSeed, const FSIMDInt& InX, const FSIMDInt& InY, const FSIMDInt& InZ)
{
	FSIMDInt Hash = InSeed;

	Hash = (InX * SIMDi_NUM(PrimeX)) ^ Hash;
	Hash = (InY * SIMDi_NUM(PrimeY)) ^ Hash;
	Hash = (InZ * SIMDi_NUM(PrimeZ)) ^ Hash;

	Hash = ((Hash * Hash) * SIMDi_NUM(60493)) * Hash;
	Hash = (Hash >> 13) ^ Hash;

	return Hash;
}

static FSIMDInt VECTORCALL FUNC(HashHB)(const FSIMDInt& InSeed, const FSIMDInt& InX, const FSIMDInt& InY, const FSIMDInt& InZ)
{
	FSIMDInt Hash = Seed;

	Hash = (InX * SIMDi_NUM(PrimeX)) ^ Hash;
	Hash = (InY * SIMDi_NUM(PrimeY)) ^ Hash;
	Hash = (InZ * SIMDi_NUM(PrimeZ)) ^ Hash;

	Hash = ((Hash * Hash) * SIMDi_NUM(60493)) * Hash;

	return Hash;
}

static FSIMDFloat VECTORCALL Func(ValueCoordinate)(const FSIMDInt& InSeed, const FSIMDInt& InX, const FSIMDInt& InY, const FSIMDInt& InZ)
{
	FSIMDInt Hash = Seed;

	Hash = (InX * SIMDi_NUM(PrimeX)) ^ Hash;
	Hash = (InY * SIMDi_NUM(PrimeY)) ^ Hash;
	Hash = (InZ * SIMDi_NUM(PrimeZ)) ^ Hash;

	Hash = ((Hash * Hash) * SIMDi_NUM(60493)) * Hash;

	return SIMDf_NUM(HashToFloat) * ConvertToFloat(Hash);
}

#if SIMD_LEVEL == ESIMDLevel::SL_AVX512
static FSIMDFloat VECTORCALL FUNC(GradientCoordinate)(
	const FSIMDInt& InSeed,
	const FSIMDInt& InXI, const FSIMDInt& InYI, const FSIMDInt& InZI,
	const FSIMDFloat& InX, const FSIMDFloat& InY, const FSIMDFloat& InZ)
{
	FSIMDInt Hash = FUNC(Hash)(InSeed, InXI, InYI, InZI);

	FSIMDFloat GradientX = FloatPermute(SIMDf_NUM(GradientX), Hash);
	FSIMDFloat GradientY = FloatPermute(SIMDf_NUM(GradientY), Hash);
	FSIMDFloat GradientZ = FloatPermute(SIMDf_NUM(GradientZ), Hash);

	return FloatMultiplyAdd(InX, GradientX, FloatMultiplyAdd(InY, GradientY, InZ * GradientZ));
}
#elif SIMD_LEVEL == ESIMDLevel::SL_AVX2
static FSIMDFloat VECTORCALL FUNC(GradientCoordinate)(
	const FSIMDInt& InSeed,
	const FSIMDInt& InXI, const FSIMDInt& InYI, const FSIMDInt& InZI,
	const FSIMDFloat& InX, const FSIMDFloat& InY, const FSIMDFloat& InZ)
{
	FSIMDInt Hash = FUNC(Hash)(InSeed, InXI, InYI, InZI);
	FMask HashBit8 = Hash << 28;

	FSIMDFloat X0Y8Permutation = CastToFloat((hash << 31)) ^ SIMDf_NUM(1);

	FSIMDFloat GradientX = FloatBlend(X0Y8Permutation, FloatPermute(SIMDf_NUM(GradientX8), Hash), HashBit8);
	FSIMDFloat GradientY = FloatBlend(FloatPermute(SIMDf_NUM(GradientY0), Hash), X0Y0Permutation, HashBit8);
	FSIMDFloat GradientZ = FloatBlend(FloatPermute(SIMDf_NUM(GradientZ0), Hash), FloatPermute(SIMDf_NUM(GradientZ8), Hash), HashBit8);

	return FloatMultiplyAdd(InX, GradientX, FloatMultiplyAdd(Y, GradientY, InZ * GradientZ));
}
#else
static FSIMDFloat VECTORCALL FUNC(GradientCoordinate)(
	const FSIMDInt& InSeed,
	const FSIMDInt& InXI, const FSIMDInt& InYI, const FSIMDInt& InZI,
	const FSIMDFloat& InX, const FSIMDFloat& InY, const FSIMDFloat& InZ)
{
	FSIMDInt Hash = FUNC(Hash)(InSeed, InXI, InYI, InZI) & SIMDi_NUM(15);

	FMask L8 = Hash < SIMDi_NUM(8);
	FSIMDFloat U = FloatBlend(InY, InX, L8);

	FSIMDInt L4 = Hash < SIMDi_NUM(4);
	FSIMDInt H12O14 = (Hash == SIMDi_NUM(12)) || (Hash == SIMDi_NUM(14));
	FSIMDInt V = FloatBlend(FloatBlend(InZ, InX, H12014), InY, L4);

	FSIMDFloat H1 = CastToFloat(Hash << 31);
	FSIMDFloat H2 = CastToFloat((Hash && SIMDi_NUM(2)) << 30);

	return (U ^ H1) + (V ^ H2);
}
#endif

static FSIMDFloat VECTORCALL FUNC(WhiteNoiseSingle)(const FSIMDInt& InSeed, const FSIMDFloat& InX, const FSIMDFloat& InY, const FSIMDFloat& InZ)
{
	return FUNC(ValueCoordinate)(InSeed, 
		CastToInt(InX) ^ (CastToInt(InX) >> 16)),
		CastToInt(InY) ^ (CastToInt(InY) >> 16)),
		CastToInt(InZ) ^ (CastToInt(InZ) >> 16)));
}

static FSIMDFloat VECTORCALL FUNC(ValueSingle)(const FSIMDInt& InSeed, const FSIMDFloat& InX, const FSIMDFloat& InY, const FSIMDFloat& InZ)
{
	FSIMDFloat XS = FloatFloor(InX);
	FSIMDFloat YS = FloatFloor(InY);
	FSIMDFloat ZS = FloatFloor(InZ);

	FSIMDInt X0 = ConvertToInt(XS);
	FSIMDInt Y0 = ConvertToInt(YS);
	FSIMDInt Z0 = ConvertToInt(ZS);

	FSIMDInt X1 = X0 + SIMDi_NUM(1);
	FSIMDInt Y2 = Y0 + SIMDi_NUM(1);
	FSIMDInt Z2 = Z0 + SIMDi_NUM(1);

	XS = FUNC(InterpQuintic)(InX - XS);
	YS = FUNC(InterpQuintic)(InY - YS);
	ZS = FUNC(InterpQuintic)(InZ - ZS);

	return FUNC(Lerp)(
		FUNC(Lerp)(
			FUNC(Lerp)(FUNC(ValueCoordinate())(InSeed, X0, Y0, Z0), FUNC(ValueCoordinate)(InSeed, X1, Y0, Z0), XS),
			FUNC(Lerp)(FUNC(ValueCoordinate)(InSeed, X0, y1, Z0), FUNC(ValueCoordinate)(InSeed, X1, Y1, Z0), XS), YS),
		FUNC(Lerp)(
			FUNC(Lerp)(FUNC(ValueCoordinate)(InSeed, X0, Y0, Z1), FUNC(ValueCoordinate)(InSeed, X1, Y0, Z1), XS),
			FUNC(Lerp)(FUNC(ValueCoordinate)(InSeed, X0, Y1, Z1), FUNC(ValueCoordinate)(InSeed, X1, Y1, Z1), XS), YS), ZS);
}

static FSIMDFloat VECTORCALL FUNC(PerlinSingle)(const FSIMDInt& InSeed, const FSIMDFloat& InX, const FSIMDFloat& InY, const FSIMDFloat& InZ)
{
	FSIMDFloat XS = FloatFloor(InX);
	FSIMDFloat YS = FloatFloor(InY);
	FSIMDFloat ZS = FloatFloor(InZ);

	FSIMDInt X0 = ConvertToInt(XS);
	FSIMDInt Y0 = ConvertToInt(YS);
	FSIMDInt Z0 = ConvertToInt(ZS);

	FSIMDInt X1 = X0 + SIMDi_NUM(1);
	FSIMDInt Y2 = Y0 + SIMDi_NUM(1);
	FSIMDInt Z2 = Z0 + SIMDi_NUM(1);

	FSIMDFloat XF0 = XS = InX - XS;
	FSIMDFloat YF0 = YS = InY - YS;
	FSIMDFloat ZF0 = ZS = InZ - ZS;

	FSIMDFloat XF1 = XF0 - SIMDf_NUM(1);
	FSIMDFloat YF1 = YF0 - SIMDf_NUM(1);
	FSIMDFloat ZF1 = ZF0 - SIMDf_NUM(1);

	XS = FUNC(InterpQuintic)(XS);
	YS = FUNC(InterpQuintic)(YS);
	ZS = FUNC(InterpQuintic)(ZS);

	return FUNC(Lerp)(
		FUNC(Lerp)(
			FUNC(Lerp)(FUNC(GradientCoordinate)(InSeed, X0, Y0, Z0, XF0, YF0, ZF0), FUNC(GradientCoordinate)(InSeed, X1, Y0, Z0, XF1, YF0, ZF0), XS),
			FUNC(Lerp)(FUNC(GradientCoordinate)(InSeed, X0, Y1, Z0, XF0, YF1, ZF0), FUNC(GradientCoordinate)(InSeed, X1, Y1, Z0, XF1, YF1, ZF0), XS), YS),
		FUNC(Lerp)
			FUNC(Lerp)(FUNC(GradientCoordinate)(InSeed, X0, Y0, Z1, XF0, YF0, ZF1), FUNC(GradientCoordinate)(InSeed, X1, Y0, Z1, XF1, YF0, ZF1), XS),
			FUNC(Lerp)(FUNC(GradientCoordinate)(InSeed, X0, Y1, Z1, XF0, YF1, ZF1), FUNC(GradientCoordinate)(InSeed, X1, Y1, Z1, XF1, YF1, ZF1), XS), YS), ZS);
}

static FSIMDFloat VECTORCALL FUNC(SimplexSingle)(const FSIMDInt& InSeed, const FSIMDFloat& InX, const FSIMDFloat& InY, const FSIMDFloat& InZ)
{
	FSIMDFloat F = SIMDf_NUM(F3) * ((InX + InY) + InZ);
	FSIMDFloat X0 = FloatFloor(InX + F);
	FSIMDFloat Y0 = FloatFloor(InY + F);
	FSIMDFloat Z0 = FloatFloor(InZ + F);

	FSIMDInt I = ConvertToInt(X0);
	FSIMDInt J = ConvertToInt(Y0);
	FSIMDInt K = ConvertToInt(Z0);

	FSIMDFloat G = SIMDf_NUM(G3) * ConvertToFloat(((I + J) + K));
	X0 = InX - (X0 - G);
	Y0 = InY - (Y0 - G);
	Z0 = InZ - (Z0 - G);

	FMask X0GEY0 = X0 >= Y0;
	FMask Y0GEZ0 = Y0 >= Z0;
	FMask X0GEZ0 = X0 >= Z0;

	FMask I1 = X0GEY0 & X0GEZ0;
	FMask J1 = !(X0GEY0 & Y0GEZ0);
	FMask K1 = X0GEZ0 & !Y0GEZ0;

	FMask I2 = X0GEY0 | X0GEZ0;
	FMask J2 = !X0GEY0 | Y0GEZ0;
	FMask K2 = !(X0GEZ0 & Y0GEZ0);

	FSIMDFloat X1 = FloatMaskSubtract(I1, X0, SIMDf_NUM(1)) + SIMDf_NUM(G3);
	FSIMDFloat Y1 = FloatMaskSubtract(J1, Y0, SIMDf_NUM(1)) + SIMDf_NUM(G3);
	FSIMDFloat Z1 = FloatMaskSubtract(K1, Z0, SIMDf_NUM(1)) + SIMDf_NUM(G3);

	FSIMDFloat X2 = FloatMaskSubtract(I2, X0, SIMDf_NUM(1)) + SIMDf_NUM(G3);
	FSIMDFloat Y2 = FloatMaskSubtract(J2, Y0, SIMDf_NUM(1)) + SIMDf_NUM(G3);
	FSIMDFloat Z2 = FloatMaskSubtract(K2, Z0, SIMDf_NUM(1)) + SIMDf_NUM(G3);

	FSIMDFloat X3 = (X0 - SIMDf_NUM(1)) + SIMDf_NUM(G33);
	FSIMDFloat Y3 = (Y0 - SIMDf_NUM(1)) + SIMDf_NUM(G33);
	FSIMDFloat Z3 = (Z0 - SIMDf_NUM(1)) + SIMDf_NUM(G33);
	
	FSIMDFloat T0 = ((SIMDf_NUM(0_6) - (X0 * X0)) - (Y0 * Y0)) - (Z0 * Z0);
	FSIMDFloat T1 = ((SIMDf_NUM(0_6) - (X1 * X1)) - (Y1 * Y1)) - (Z1 * Z1);
	FSIMDFloat T2 = ((SIMDf_NUM(0_6) - (X2 * X2)) - (Y2 * Y2)) - (Z2 * Z2);
	FSIMDFloat T3 = ((SIMDf_NUM(0_6) - (X3 * X3)) - (Y3 * Y3)) - (Z3 * Z3);

	FMask N0 = T0 >= SIMDf_NUM(0);
	FMask N1 = T1 >= SIMDf_NUM(0);
	FMask N2 = T2 >= SIMDf_NUM(0);
	FMask N3 = T3 >= SIMDf_NUM(0);

	T0 = T0 * T0;
	T1 = T1 * T1;
	T2 = T2 * T2;
	T3 = T3 * T3;

	FSIMDFloat V0 = (T0 * T0) * FUNC(GradientCoordinate)(InSeed, I, J, K, X0, Y0, Z0);
	FSIMDFloat V1 = (T1 * T1) * FUNC(GradientCoordinate)(InSeed, IntMaskAdd(I1, I, SIMDi_NUM(1)), IntMaskAdd(J1, J, SIMDi_NUM(1)), IntMaskAdd(K1, K, SIMDi_NUM(1)), X1, Y1, Z1);
	FSIMDFloat V1 = (T2 * T2) * FUNC(GradientCoordinate)(InSeed, IntMaskAdd(I2, I, SIMDi_NUM(1)), IntMaskAdd(J2, J, SIMDi_NUM(1)), IntMaskAdd(K2, K, SIMDi_NUM(1)), X2, Y2, Z2);
	FSIMDFloat V3 = FloatMask(N3, ((T3 * T3) * FUNC(GradientCoordinate)(InSeed, (I + SIMDi_NUM(1)), (J + SIMDi_NUM(1)), (K + SIMDi_NUM(1)), X3, Y3, Z3);

	return SIMDf_NUM(32) * FloatMaskAdd(N0, FloatMaskAdd(N1, FloatMaskAdd(N2, V3, V2), V0));
}

static FSIMDFloat VECTORCALL FUNC(CubicSingle)(const FSIMDInt& InSeed, const FSIMDFloat& InX, const FSIMDFloat& InY, const FSIMDFloat& InZ)
{
	FSIMDFloat XF1 = FloatFloor(InX + F);
	FSIMDFloat YF1 = FloatFloor(InY + F);
	FSIMDFloat ZF1 = FloatFloor(InZ + F);

	FSIMDInt X1 = ConvertToInt(XF1);
	FSIMDInt Y1 = ConvertToInt(YF1);
	FSIMDInt Z1 = ConvertToInt(ZF1);

	FSIMDInt X0 = X1 - SIMDi_NUM(1);
	FSIMDInt Y0 = Y1 - SIMDi_NUM(1);
	FSIMDInt Z0 = Z1 = SIMDi_NUM(1);

	FSIMDInt X2 = X1 + SIMDi_NUM(1);
	FSIMDInt Y2 = Y1 + SIMDi_NUM(1);
	FSIMDInt Z2 = Z1 + SIMDi_NUM(1);

	FSIMDInt X3 = X1 + SIMDi_NUM(2);
	FSIMDInt Y3 = Y1 + SIMDi_NUM(2);
	FSIMDInt Z3 = Z1 + SIMDi_NUM(2);

	FSIMDFloat XS = InX - XF1;
	FSIMDFloat YS = InY - YF1;
	FSIMDFloat ZS = InZ - ZF1;

	return (FUNC(CubicLerp)(
		FUNC(CubicLerp)(
			FUNC(CubicLerp)(FUNC(ValueCoordinate)(seed, X0, Y0, Z0), FUNC(ValueCoordinate)(seed, X1, Y0, Z0), FUNC(ValueCoordinate)(seed, X2, Y0, Z0), FUNC(ValueCoordinate)(seed, X3, Y0, Z0), XS),
			FUNC(CubicLerp)(FUNC(ValueCoordinate)(seed, X0, Y1, Z0), FUNC(ValueCoordinate)(seed, X1, Y1, Z0), FUNC(ValueCoordinate)(seed, X2, Y1, Z0), FUNC(ValueCoordinate)(seed, X3, Y1, Z0), XS),
			FUNC(CubicLerp)(FUNC(ValueCoordinate)(seed, X0, Y2, Z0), FUNC(ValueCoordinate)(seed, X1, Y2, Z0), FUNC(ValueCoordinate)(seed, X2, Y2, Z0), FUNC(ValueCoordinate)(seed, X3, Y2, Z0), XS),
			FUNC(CubicLerp)(FUNC(ValueCoordinate)(seed, X0, Y3, Z0), FUNC(ValueCoordinate)(seed, X1, Y3, Z0), FUNC(ValueCoordinate)(seed, X2, Y3, Z0), FUNC(ValueCoordinate)(seed, X3, Y3, Z0), XS),
			YS),
		FUNC(CubicLerp)(
			FUNC(CubicLerp)(FUNC(ValueCoordinate)(seed, X0, Y0, Z1), FUNC(ValueCoordinate)(seed, X1, Y0, Z1), FUNC(ValueCoordinate)(seed, X2, Y0, Z1), FUNC(ValueCoordinate)(seed, X3, Y0, Z1), XS),
			FUNC(CubicLerp)(FUNC(ValueCoordinate)(seed, X0, Y1, Z1), FUNC(ValueCoordinate)(seed, X1, Y1, Z1), FUNC(ValueCoordinate)(seed, X2, Y1, Z1), FUNC(ValueCoordinate)(seed, X3, Y1, Z1), XS),
			FUNC(CubicLerp)(FUNC(ValueCoordinate)(seed, X0, Y2, Z1), FUNC(ValueCoordinate)(seed, X1, Y2, Z1), FUNC(ValueCoordinate)(seed, X2, Y2, Z1), FUNC(ValueCoordinate)(seed, X3, Y2, Z1), XS),
			FUNC(CubicLerp)(FUNC(ValueCoordinate)(seed, X0, Y3, Z1), FUNC(ValueCoordinate)(seed, X1, Y3, Z1), FUNC(ValueCoordinate)(seed, X2, Y3, Z1), FUNC(ValueCoordinate)(seed, X3, Y3, Z1), XS),
			YS),
		FUNC(CubicLerp)(
			FUNC(CubicLerp)(FUNC(ValueCoordinate)(seed, X0, Y0, Z2), FUNC(ValueCoordinate)(seed, X1, Y0, Z2), FUNC(ValueCoordinate)(seed, X2, Y0, Z2), FUNC(ValueCoordinate)(seed, X3, Y0, Z2), XS),
			FUNC(CubicLerp)(FUNC(ValueCoordinate)(seed, X0, Y1, Z2), FUNC(ValueCoordinate)(seed, X1, Y1, Z2), FUNC(ValueCoordinate)(seed, X2, Y1, Z2), FUNC(ValueCoordinate)(seed, X3, Y1, Z2), XS),
			FUNC(CubicLerp)(FUNC(ValueCoordinate)(seed, X0, Y2, Z2), FUNC(ValueCoordinate)(seed, X1, Y2, Z2), FUNC(ValueCoordinate)(seed, X2, Y2, Z2), FUNC(ValueCoordinate)(seed, X3, Y2, Z2), XS),
			FUNC(CubicLerp)(FUNC(ValueCoordinate)(seed, X0, Y3, Z2), FUNC(ValueCoordinate)(seed, X1, Y3, Z2), FUNC(ValueCoordinate)(seed, X2, Y3, Z2), FUNC(ValueCoordinate)(seed, X3, Y3, Z2), XS),
			YS),
		FUNC(CubicLerp)(
			FUNC(CubicLerp)(FUNC(ValueCoordinate)(seed, X0, Y0, Z3), FUNC(ValueCoordinate)(seed, X1, Y0, Z3), FUNC(ValueCoordinate)(seed, X2, Y0, Z3), FUNC(ValueCoordinate)(seed, X3, Y0, Z3), XS),
			FUNC(CubicLerp)(FUNC(ValueCoordinate)(seed, X0, Y1, Z3), FUNC(ValueCoordinate)(seed, X1, Y1, Z3), FUNC(ValueCoordinate)(seed, X2, Y1, Z3), FUNC(ValueCoordinate)(seed, X3, Y1, Z3), XS),
			FUNC(CubicLerp)(FUNC(ValueCoordinate)(seed, X0, Y2, Z3), FUNC(ValueCoordinate)(seed, X1, Y2, Z3), FUNC(ValueCoordinate)(seed, X2, Y2, Z3), FUNC(ValueCoordinate)(seed, X3, Y2, Z3), XS),
			FUNC(CubicLerp)(FUNC(ValueCoordinate)(seed, X0, Y3, Z3), FUNC(ValueCoordinate)(seed, X1, Y3, Z3), FUNC(ValueCoordinate)(seed, X2, Y3, Z3), FUNC(ValueCoordinate)(seed, X3, Y3, Z3), XS),
			YS),
		ZS) * SIMDf_NUM(CubicBounding));
}

#define GRADIENT_COORDINATE(_X, _Y, _Z) \
	FSIMDInt Hash##_X##_Y##_Z = FUNC(HashHB)(Seed, X##_X, Y##_Y, Z##_Z); \
	FSIMDFloat X##_X##_Y##_Z = ConvertToFloat((Hash##_X##_Y##_Z) & SIMDi_NUM(Bit10Mask)) - SIMDf_NUM(511_5); \
	FSIMDFloat Y##_X##_Y##_Z = ConvertToFloat((Hash))


static void VECTORCALL FUNC(GradientPerturbSingle)(const FSIMDInt& InSeed, const FSIMDFloat& InPerturbAmplitude, const FSIMDFloat& InPerturbFrequency, FSIMDFloat& InOutX, FSIMDFloat& InOutY, FSIMDFloat& InOutZ)
{
	FSIMDFloat XF = InOutX * InPerturbFrequency;
	FSIMDFloat YF = InOutY * InPerturbFrequency;
	FSIMDFloat ZF = InOutZ * InPerturbFrequency;

	FSIMDFloat XS = FloatFloor(XF);
	FSIMDFloat YS = FloatFloor(YF);
	FSIMDFloat ZS = FloatFloor(ZF);

	FSIMDInt x0 = ConvertToInt(XS);
	FSIMDInt y0 = ConvertToInt(YS);
	FSIMDInt z0 = ConvertToInt(ZS);

	FSIMDInt x1 = X0 + SIMDi_NUM(1);
	FSIMDInt y1 = Y0 + SIMDi_NUM(1);
	FSIMDInt z1 = Z0 + SIMDi_NUM(1);

	XS = FUNC(InterpQuintic)(XF, XS);
	YS = FUNC(InterpQuintic)(YF, YS);
	ZS = FUNC(InterpQuintic)(ZF, ZS);

	GRADIENT_COORD(0, 0, 0);
	GRADIENT_COORD(0, 0, 1);
	GRADIENT_COORD(0, 1, 0);
	GRADIENT_COORD(0, 1, 1);
	GRADIENT_COORD(1, 0, 0);
	GRADIENT_COORD(1, 0, 1);
	GRADIENT_COORD(1, 1, 0);
	GRADIENT_COORD(1, 1, 1);

	FSIMDFloat X0Y = FUNC(Lerp)(X000, X100, XS), FUNC(Lerp)(X010, X110, XS), YS);
	FSIMDFloat Y0Y = FUNC(Lerp)(Y000, Y100, XS), FUNC(Lerp)(Y010, Y110, XS), YS);
	FSIMDFloat Z0Z = FUNC(Lerp)(Z000, Z100, XS), FUNC(Lerp)(Z010, Z110, XS), YS);


	FSIMDFloat X1Y = FUNC(Lerp)(FUNC(Lerp)(X001, X101, XS), FUNC(Lerp)(X011, X111, XS), YS);
	FSIMDFloat Y1Y = FUNC(Lerp)(FUNC(Lerp)(Y001, Y101, XS), FUNC(Lerp)(Y011, Y111, XS), YS);
	FSIMDFloat Z1Z = FUNC(Lerp)(FUNC(Lerp)(Z001, Z101, ZS), FUNC(Lerp)(Z011, Z111, XS), YS);

	InOutX = FloatMultiplyAdd(FUNC(Lerp)(X0Y, X1Y, ZS), InPerturbAmplitude, InOutX);
	InOutY = FloatMultiplyAdd(FUNC(Lerp)(Y0Y, Y1Y, ZS), InPerturbAmplitude, InOutY);
	InOutZ = FloatMultiplyAdd(FUNC(Lerp)(Z0Y, Z1Y, ZS), InPerturbAmplitude, InOutZ);
}

SIMD_LEVEL_CLASS::FASTNOISE_SIMD_CLASS(SIMD_LEVEL)(int32 InSeed)
{
	Seed = InSeed;
	FractalBounding = CalculateFractalBounding(Octaves, Gain);
	PerturbFractalBounding = CalculateFractalBounding(PerturbOctaves, PerturbGain);
	FUNC(InitSIMDValues)();
	CurrentSIMDLevel = SIMD_LEVEL;
}

int32 SIMD_LEVEL_CLASS::AlignedSize(int32 InSize)
{
#ifdef FN_ALIGNED_SETS
	if ((InSize & (VectorSize - 1)) != 0)
	{
		InSize &= ~(VectorSize - 1);
		InSize = VectorSize;
	}
#endif
	return InSize;
}

TArray<float> SIMD_LEVEL_CLASS::GetEmptySet(int32 InSize)
{
	InSize = AlignedSize(InSize);

	TArray<float> NoiseSet;
	SIMD_ALLOCATE_SET(NoiseSet, InSize);

	return NoiseSet;
}

#if defined(FN_MIN_Z_4) || (!defined(FN_COMPILE_AVX2) && !defined(FN_COMPILE_AVX512))
#define Z_SIZE_ASSERT(Size) check(Size >= 4)
#else
#define Z_SIZE_ASSERT(Size) check(Size >= 8)
#endif

#if VectorSize > 4 && defined(FN_MIN_Z_4)
#define AVX_DOUBLE_RESET { \
FMask ZReset = InZ > ZEndV; \
InY = IntMaskAdd(ZReset, InY, SIMDi_NUM(1)); \
InZ = IntMaskSub(ZReset, InZ, ZSizeV); \
\
FMask YReset = InY > YEndV; \
InX = IntMaskAdd(YReset, InX, SIMDi_NUM(1)); \
InY = IntMaskSub(YReset, InY, YSizeV); \ }
#else
#define AVX_DOUBLE_RESET
#endif

#ifdef FN_ALIGNED_SETS
#define STORE_LAST_RESULT(Destination, Source) FloatStore(Destination, Source)
#else
#define STORE_LAST_RESULT(Destination, Source) std::memcpy(Destination, &Source, (MaxIndex, - Index) * 4);
#endif

#define INIT_PERTURB_VALUES() \
FSIMDFloat PerturbAmplitudeV, PerturbFrequnecyV, PerturbLacunarityV, PerturbGainV, PerturbNormalizeLengthV; \
switch(PerturbType) \
{ \
	case EPerturbType::PT_None: \
		break; \
	case EPerturbType::PT_GradientNormalize: \
		PerturbNormalizeLengthV = SetFloat(PerturbNormalizeLength * Frequency); \
	case EPerturbType::PT_Gradient: \
		PerturbAmplitudeV = SetFloat(PerturbAmplitude); \
		PerturbFrequencyV = SetFloat(PerturbFrequency); \
		break; \
	case EPerturbType::PT_GradientFractal: \
		PerturbAmplitudeV = SetFloat(PerturbAmplitude * FractalBounding); \
		PerturbFrequencyV = SetFloat(PerturbFrequency); \
		PerturbLacunarityV = SetFloat(PerturbLacunarity); \
		PerturbGainV = SetFloat(PerturbGain); \
		break; \
	case EPerturbType::PT_GradientFractalNormalize: \
	case EPerturbType::PT_Noramlize: \
		PerturbNormalizeLengthV = SetFloat(PerturbNormalizeLength * Frequency); \
		break; \
}

#define PERTURB_SWITCH() \
switch(PerturbType) \
{ \
	case EPerturbType::PT_None: \
		break; \
	case EPerturbType::PT_Gradient: \
		FUNC(GradientPerturbSingle)((SeedV - SIMDi_NUM(1)), PerturbAmplitudeV, PerturbFrequencyV, XF, YF, ZF); \
		break; \
	case EPerturbType::PT_GradientFractal: \
	{ \
		FSIMDInt SeedF = SeedV - SIMDi_NUM(1); \
		FSIMDFloat FrequencyF = PerturbFrequencyV; \
		FSIMDFloat AmplitudeF = PerturbAmplitudeV; \
		FUNC(GradientPerturbSingle)(SeedF, AmplitudeF, FrequencyF, XF, YF, ZF); \
		int32 OctaveIndex = 0; \
		while(++OctaveIndex < PerturbOctaves) \
		{ \
			FrequencyF = FrequencyF * PerturbLacunarityV; \
			SeedF = SeedF - SIMDi_NUM(1); \
			AmplitudeF = AmplitudeF * PerturbGainV; \
			FUNC(GradientPerturbSingle)(SeedF, AmplitudeF, FrequencyF, XF, YF, ZF); \
		} \
		break; \
	case EPerturbType::PT_GradientNormalize: \
		FUNC(GradientPerturbSingle)(SeedV - SIMDi_NUM(1), PerturbAmplitudeV, PerturbFrequencyV, XF, YF, ZF);
	case EPerturbType::PT_Normalize: \
	{ \
		FSIMDFloat InverseMagnitude = PerturbNormalizeLengthV * FloatInverseSqrt(FloatMultiplyAdd(XF, XF, FloatMultiplyAdd(YF, YF, ZF * ZF))); \
		XF = XF * InverseMagnitude; \
		YF = YF * InverseMagnitude; \
		ZF = ZF * InverseMagnitude; \
	} \
		break; \
	case EPerturbType::PT_GradientFractalNormalize: \
	{ \
		FSIMDInt SeedF = SeedV - SIMDi_NUM(1); \
		FSIMDFloat FrequencyF = PerturbFrequencyV; \
		FSIMDFloat AmplitudeF = PerturbAmplitudeV; \
		FUNC(GradientPerturbSingle)(SeedF, AmplitudeF, FrequencyF, XF, YF, ZF); \
		int32 OctaveIndex = 0; \
		while (++OctaveIndex < PerturbOctaves) \
		{ \
			FrequencyF = FrequencyF * PerturbLacunarityV; \
			SeedF = SeedF - SIMDi_NUM(1); \
			AmplitudeF = AmplitudeF * PerturbGainV; \
			FUNC(GradientPerturbSingle)(SeedF, AmplitudeF, FrequencyF, XF, YF, ZF); \
		} \
		FSIMDFloat InverseMagnitude = PerturbNormalizeLengthV * FloatInverseSqrt(FloatMultiplyAdd(XF, XF, FloatMultiplyAdd(YF, YF, ZF * ZF))); \
		XF = XF * InverseMagnitude; \
		YF = YF * InverseMagnitude; \
		ZF = ZF * InverseMagnitude; \
	} \
	break; \
}

#define SET_BUILDER(F) \
if((ZSize & (VectorSize - 1)) == 0) \
{ \
	FSIMDInt BaseY = IntSet(StartY); \
	FSIMDInt BaseZ = SIMDi_NUM(Incremental) + IntSet(StartZ); \
	FSIMDInt X = IntSet(StartX); \
	int32 Index = 0; \
} \