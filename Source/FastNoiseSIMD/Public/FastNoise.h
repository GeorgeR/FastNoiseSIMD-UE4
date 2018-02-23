#pragma once

#include "FastNoiseTypes.h"
#include "Math/RandomStream.h"


class FFastNoiseBase
{
public:
	FFastNoiseBase(int32 InSeed = 128);

	ESIMDLevel GetSIMDLevel() const;
	void SetSIMDLevel(ESIMDLevel InLevel) { Level = InLevel; }

	int32 GetSeed() const { return Random.GetInitialSeed(); }
	void SetSeed(int32 InSeed) { Random = FRandomStream(InSeed); }

	void SetFrequency(float InFrequency) { Frequency = InFrequency; }
	void SetNoiseType(ENoiseType InNoiseType) { NoiseType = InNoiseType; }
	void SetScale(FVector InScale) { Scale = InScale; }
	void SetFractalOctaves(int32 InOctaves);
	void SetFractalLacunarity(float InLacunarity) { Lacunarity = InLacunarity; }
	void SetFractalGain(float InGain);
	void SetFractalType(EFractalType InFractalType) { FractalType = InFractalType; }
	void SetCellularReturnType(ECellularReturnType InCellularReturnType) { CellularReturnType = InCellularReturnType; }
	void SetCellularDistanceFunction(ECellularDistanceFunction InCellularDistanceFunction) { CellularDistanceFunction = InCellularDistanceFunction; }

	virtual int32 Generate(FIntPoint InOffset, FIntPoint InSize, float InScale, TArray<float>& OutResults);

private:
	FRandomStream Random;
	int32 Seed;
	ESIMDLevel Level;
};

class FValueNoise
	: public FFastNoiseBase
{
public:
	virtual int32 Generate(FIntPoint InOffset, FIntPoint InSize, float InScale, TArray<float>& OutResults) override;
};