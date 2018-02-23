#include "FastNoise_avx512.h"

#if defined(__AVX512F__)

namespace FastNoise
{
	void FSingle::Zero() { Value = _mm512_setzero_ps; }

	FSingle& FSingle::operator=(const _m512& InValue) { Value = _mm512_set1_ps(InValue); }

	FSingle& FSingle::operator+(const FSingle& InValue)
	{
		Value = _mm512_add_ps(Value, InValue);
		return Value;
	}

	FSingle& FSingle::operator-(const FSingle& InValue)
	{
		Value = _mm512_sub_ps(Value, InValue);
		return Value;
	}

	FSingle& FSingle::operator*(const FSingle& InValue)
	{
		Value = _mm512_mul_ps(Value, InValue);
		return Value;
	}

	FSingle& FSingle::operator/(const FSingle& InValue)
	{
		Value = _mm512_div_ps(Value, InValue);
		return Value;
	}

	bool FSingle::operator==(const FSingle& InValue) { return _mm512_cmp_ps_mask(Value, InValue, _CMP_EQ_OQ); }
	bool FSingle::operator!=(const FSingle& InValue) { return _mm512_cmp_ps_mask(Value, InValue, _CMP_NEQ_OQ); }
	bool FSingle::operator<(const FSingle& InValue) { return _mm512_cmp_ps_mask(Value, InValue, _CMP_LT_OQ); }
	bool FSingle::operator>(const FSingle& InValue) { return _mm512_cmp_ps_mask(Value, InValue, _CMP_GT_OQ); }

	FORCEINLINE FSingle FSingle::MultiplyAdd(const FSingle& InMultiply, const FSingle& InAdd) { return _mm512_fmadd_ps(Value, InMultiply, InAdd); }
	FORCEINLINE FSingle FSingle::MultiplySubtract(const FSingle& InMultiply, const FSingle& InSubtract) { return _mm512_fmsub_ps(Value, InMultiply, InSubtract); }

	bool FSingle::operator<=(const FSingle& InValue) { return _mm512_cmp_ps_mask(Value, InValue, _CMP_LE_OQ); }
	bool FSingle::operator>=(const FSingle& InValue) { return _mm512_cmp_ps_mask(Value, InValue, _CMP_GE_OQ); }

	FSingle FastNoise::FSingle::Convert(const FInteger& InValue) { return _mm512_cvtepi32_ps(InValue); }
	FSingle FastNoise::FSingle::Cast(const FInteger& InValue) { return _mm512_castsi512_ps(InValue); }

	void FInteger::Zero() { Value = _mm512_setzero_si256(); }

	FInteger& FInteger::operator=(const _m512i& InValue) { Value = _mm512_set1_epi32(InValue); }

	FInteger& FInteger::operator+(const FInteger& InValue)
	{
		Value = _mm512_add_epi32(Value, InValue);
		return Value;
	}

	FInteger& FInteger::operator-(const FInteger& InValue)
	{
		Value = _mm512_sub_epi32(Value, InValue);
		return Value;
	}

	FInteger& FInteger::operator*(const FInteger& InValue)
	{
		Value = _mm512_mullo_epi32(Value, InValue);
		return Value;
	}

	FInteger& FInteger::operator>>(const FInteger& InValue)	{ return _mm512_srai_epi32(Value, InValue); }
	FInteger& FInteger::operator<<(const FInteger& InValue)	{ return _mm512_slli_epi32(Value, InValue); }

	bool FInteger::operator==(const FInteger& InValue) { return _mm512_cmpeq_epi32_mask(Value, InValue); }
	bool FInteger::operator!=(const FInteger& InValue) { return _mm512_cmpneq_epi32_mask(Value, InValue); }
	bool FInteger::operator<(const FInteger& InValue) { return _mm512_cmplt_epi32_mask(Value, InValue); }
	bool FInteger::operator>(const FInteger& InValue) { return _mm512_cmpgt_epi32_mask(Value, InValue); }
	bool FInteger::operator<=(const FInteger& InValue) { return _mm512_cmple_epi32_mask(Value, InValue); }
	bool FInteger::operator>=(const FInteger& InValue) { return _mm512_cmpge_epi32_mask(Value, InValue); }

	FInteger FInteger::Convert(const FSingle& InValue)	{ return _mm512_cvtps_epi32(InValue); }
	FInteger FInteger::Cast(const FSingle& InValue) { return _mm512_castps_si512(InValue); }
}

#endif