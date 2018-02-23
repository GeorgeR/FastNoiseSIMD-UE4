#pragma once

#include "CoreMinimal.h"
#include "Math/RandomStream.h"

UENUM(BlueprintType)
enum class ENoiseType : uint8
{
	NT_Value				UMETA(DisplayName = "Value"),
	NT_ValueFractal			UMETA(DisplayName = "Value Fractal"),
	NT_Perlin				UMETA(DisplayName = "Perlin"),
	NT_PerlinFractal		UMETA(DisplayName = "Perlin Fractal"),
	NF_Simplex				UMETA(DisplayName = "Simplex"),
	NF_SimplexFractal		UMETA(DisplayName = "Simplex Fractal"),
	NT_WhiteNoise			UMETA(DisplayName = "White Noise"),
	NT_Cellular				UMETA(DisplayName = "Cellular"),
	NT_Cubic				UMETA(DisplayName = "Cubic"),
	NT_CubicFractal			UMETA(DisplayName = "Cubic Fractal")
};

UENUM(BlueprintType)
enum class EFractalType : uint8
{
	FT_FBM					UMETA(DisplayName = "FBM/Fractal Brownian Motion"),
	FT_Billow				UMETA(DisplayName = "Billow"),
	FT_RigidMulti			UMETA(DisplayName = "Rigid Multi Fractal")
};

UENUM(BlueprintType)
enum class EPerturbType : uint8
{
	PT_None							UMETA(DisplayName = "None"),
	PT_Gradient						UMETA(DisplayName = "Gradient"),
	PT_GradientFractal				UMETA(DisplayName = "Gradient Fractal"),
	PT_Normalize					UMETA(DisplayName = "Normalize"),
	PT_GradientNormalize			UMETA(DisplayName = "Gradient Normalize"),
	PT_GradientFractalNormalize		UMETA(DisplayName = "Gradient Fractal Normalize")
};

UENUM(BlueprintType)
enum class ECellularDistanceFunction : uint8
{
	CDF_Euclidean			UMETA(DisplayName = "Euclidean"),
	CDF_Manhattan			UMETA(DisplayName = "Manhattan"),
	CDF_Natural				UMETA(DisplayName = "Natural")
};

UENUM(BlueprintType)
enum class ECellularReturnType : uint8
{
	CRT_CellValue			UMETA(DisplayName = "Cell Value"),
	CRT_Distance			UMETA(DisplayName = "Distance"),
	CRT_Distance2			UMETA(DisplayName = "Distance2"),
	CRT_Distance2Add		UMETA(DisplayName = "Distance2 Add"),
	CRT_Distance2Sub		UMETA(DisplayName = "Distance2 Subtract"),
	CRT_Distance2Mul		UMETA(DisplayName = "Distance2 Multiply"),
	CRT_Distance2Div		UMETA(DisplayName = "Distance2 Divide"),
	CRT_NoiseLookup			UMETA(DisplayName = "Noise Lookup"),
	CRT_Distance2Cave		UMETA(DisplayName = "Distance2 Cave")
};

UENUM(BlueprintType)
enum class ESIMDLevel : uint8
{
	SL_None			UMETA(DisplayName = "None"),
	SL_SSE2			UMETA(DisplayName = "SSE2"),
	SL_SSE41		UMETA(DisplayName = "SSE4.1"),
	SL_AVX2_FMA3	UMETA(DisplayName = "AVX2 & FMA3"),
	SL_AVX512		UMETA(DisplayName = "AVX-512F"),
	SL_ARM			UMETA(DisplayName = "ARM NEON")
};