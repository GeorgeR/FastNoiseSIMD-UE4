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

#include "CoreMinimal.h"
#include "ThreadSafeBool.h"

// TODO: includes depending on defines
#include "FastNoise_Fallback.h"

namespace FastNoise
{
	static FSingle FSingle_1 = 1.0f;
	static FSingle FSingle_Incremental = 0;
	static FSingle FSingle_0 = 0.0f;
	static FSingle FSingle_2 = 2.0f;
	static FSingle FSingle_6 = 6.0f;
	static FSingle FSingle_10 = 10.0f;
	static FSingle FSingle_15 = 15.0f;
	static FSingle FSingle_32 = 32.0f;
	static FSingle FSingle_999999 = 999999.0f;
	static FSingle FSingle_0_45 = 0.45f;
	static FSingle FSingle_0_5 = 0.5;
	static FSingle FSingle_0_6 = 0.6f;
	static FSingle FSingle_511_5 = 511.5f;
	static FSingle FSingle_F3 = 1.0f / 3.0f;
	static FSingle FSingle_G3 = 1.0f / 6.0f;
	static FSingle FSingle_G32 = 2.0f / 6.0f;
	static FSingle FSingle_G33 = 3.0f / 6.0f;
	static FSingle FSingle_HashToFloat = 1.0f / 2147483648.0f;
	static FSingle FSingle_VectorSize = 0.0f;
	static FSingle FSingle_CubicBounding = 1.0f / (1.5f * 1.5f * 1.5f);
	static FSingle FSingle_GradientX = 0.0f;
	static FSingle FSingle_GradientY = 0.0f;
	static FSingle FSingle_GradientZ = 0.0f;
	static FSingle FSingle_GradientX8 = 0.0f;
	static FSingle FSingle_GradientY0 = 0.0f;
	static FSingle FSingle_GradientZ0 = 0.0f;
	static FSingle FSingle_GradientZ8 = 0.0f;

	static FInteger FInteger_0 = -1;
	static FInteger FInteger_Incremental = 0;
	static FInteger FInteger_1 = 1;
	static FInteger FInteger_2 = 2;
	static FInteger FInteger_3 = 3;
	static FInteger FInteger_4 = 4;
	static FInteger FInteger_8 = 8;
	static FInteger FInteger_12 = 12;
	static FInteger FInteger_14 = 14;
	static FInteger FInteger_15 = 15;
	static FInteger FInteger_255 = 255;
	static FInteger FInteger_60493 = 60493;
	static FInteger FInteger_0x7fffffff = 0x7fffffff;
	static FInteger FInteger_PrimeX = 1619;
	static FInteger FInteger_PrimeY = 31337;
	static FInteger FInteger_PrimeZ = 6971;
	static FIntegerVector FIntegerVector_Prime = FIntegerVector(1619, 31337, 6971);
	static FInteger FInteger_Bitmap10Mask = 1023;
	static FInteger FInteger_VectorSize;

	static FThreadSafeBool bValuesSet;

	static void InitValues()
	{
		if (bValuesSet)
			return;

		FSingle IncrementFloat;
		FInteger IncrementInteger;
		for (auto i = 0; i < VectorSize; i++)
		{
			IncrementFloat.a[i] = static_cast<float>(i);
			IncrementInteger.a[i] = i;
		}

		FSingle_Incremental = IncrementFloat.m;
		FInteger_Incremental = IncrementInteger.m;

		FSingle_VectorSize = static_cast<float>(VectorSize);

#if defined(__AVX512F__)
		GradientX = _mm512_set_ps(0, -1, 0, 1, 0, 0, 0, 0, -1, 1, -1, 1, -1, 1, -1, 1);
		GradientY = _mm512_set_ps(-1, 1, -1, 1, -1, 1, -1, 1, 0, 0, 0, 0, -1, -1, 1, 1);
		GradientZ = _mm512_set_ps(-1, 0, 1, 0, -1, -1, 1, 1, -1, -1, 1, 1, 0, 0, 0, 0);
#elif defined(__AVX__)
		GradientX8 = _mm256_set_ps(0, -1, 0, 1, 0, 0, 0, 0);
		GradientY0 = _mm256_set_ps(0, 0, 0, 0, -1, -1, 1, 1);
		GradientZ0 = _mm256_set_ps(-1, -1, 1, 1, 0, 0, 0, 0);
		GradientZ8 = _mm256_set_ps(-1, 0, 1, 0, -1, -1, 1, 1);
#endif

		FInteger_VectorSize = VectorSize;

		bValuesSet = true;
}

#if defined(_WIN32) && SIMD_LEVEL > 0
#define VECTORCALL __vectorcall
#else
#define VECTORCALL
#endif

	struct FFastNoiseVectorSet
	{
	public:
		int32 Size = -1;
		TArray<FVector> Set;

		int32 SampleScale = 0;
		FIntVector SampleSize;

		FFastNoiseVectorSet() { }
		FFastNoiseVectorSet(int32 InSize) { SetSize(InSize); }

		void SetSize(int32 InSize);
	};

	class FFastNoiseSIMD
	{
	public:
		FFastNoiseSIMD(ESIMDLevel InSIMDLevel, int32 InSeed);

	private:
		ESIMDLevel Level;
		int32 Seed;

		FSingle VECTORCALL Lerp(FSingle InLeft, FSingle InRight, FSingle InAlpha);
		FSingle VECTORCALL InterpolateQuintic(FSingle InValue);
		FSingle VECTORCALL CubicLerp(FSingle InA, FSingle InB, FSingle InC, FSingle InD, FSingle InAlpha);
		FInteger VECTORCALL Hash(FInteger InSeed, FIntegerVector InPoint);
		FInteger VECTORCALL HashHB(FInteger InSeed, FIntegerVector InPoint);
		FSingle VECTORCALL ValueCoordinate(FInteger InSeed, FIntegerVector InPoint);
		FSingle VECTORCALL GradientCoordinate(FInteger InSeed, FIntegerVector InPointI, FSingleVector InPoint);
		FSingle VECTORCALL ValueSingle(FInteger InSeed, FSingleVector InPoint);
		FSingle VECTORCALL CubicSingle(FInteger InSeed, FSingleVector InPoint);
		void VECTORCALL GradientPerturbSingle(FInteger InSeed, FSingle InPerturbAmplitude, FSingle InPerturbFrequency, FSingleVector& InOutPoint);
	};
}