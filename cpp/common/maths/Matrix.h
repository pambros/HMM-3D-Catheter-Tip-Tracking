#ifndef __UTIL_MATRIX_HEADER_
#define __UTIL_MATRIX_HEADER_
#include "common/util/Util.h"
#include "common/maths/Maths.h"
#include "common/maths/Vector.h"

#include "math.h"
 
#ifdef Q_SSE_AVX
	#if defined(__GNUC__)
		// #include <mmintrin.h> // MMX
		// #include <xmmintrin.h> // SSE
		// #include <emmintrin.h> // SSE2
		// #include <pmmintrin.h> // SSE3
		// #include <tmmintrin.h> // SSSE3
		// #include <smmintrin.h> // SSE4.1
		// #include <nmmintrin.h> // SSE4.2
		// #include <ammintrin.h> // SSE4A
		// #include <wmmintrin.h> // AES
		#include <immintrin.h> // AVX
		// #include <zmmintrin.h> // AVX512
	#endif
	
	#define Q_SSE_AVX_MATRIX
#endif
 
BEGIN_Q_NAMESPACE
	enum{	
		POSITION_MATRIX_DIM = 4
	};

	//! Matrix44.
	template<typename _TYPE>
	class TMatrix44
	{
	public:
		inline TMatrix44<_TYPE>(void)
		{
		}

		inline TMatrix44<_TYPE>(const TMatrix44<_TYPE> &_mat)
		{
			memcpy(this, &_mat, sizeof(TMatrix44<_TYPE>));
		}

		inline const TVector4<_TYPE>& operator[](const q::qu8 _i) const
		{
			qAssert(_i >= 0 && _i < 4);
			return m_Vec[_i];
		}

		inline TVector4<_TYPE>& operator[](const q::qu8 _i)
		{
			qAssert(_i >= 0 && _i < 4);
			return m_Vec[_i];
		}

		inline TMatrix44<_TYPE> transpose() const
		{
			TMatrix44<_TYPE> mat;

			for(q::qu8 i = 0; i < q::POSITION_MATRIX_DIM; ++i)
			{
				for(q::qu8 j = 0; j < q::POSITION_MATRIX_DIM; ++j)
				{
					mat[j][i] = m_Vec[i][j];
				}
			}
			return mat;
		}

		// matrix inverse function, inline TMatrix44<_TYPE> inverse(qbool *_isInvertible = NULL) const
#include "thirdParties/mevislab/mlLinearAlgebraTools.h"

		static inline TMatrix44<_TYPE> getIdentity()
		{
			TMatrix44<_TYPE> mat;
			for(q::qu8 i = 0; i < q::POSITION_MATRIX_DIM; ++i)
			{
				for(q::qu8 j = 0; j < q::POSITION_MATRIX_DIM; ++j)
				{
					if(i == j)
					{
						mat[i][j] = 1.;
					}
					else
					{
						mat[i][j] = 0.;
					}
				}
			}
			return mat;
		}

	public:
		TVector4<_TYPE> m_Vec[4];
	};

	template <class _TYPE>
	inline TMatrix44<_TYPE> operator*(const TMatrix44<_TYPE> &_a, const TMatrix44<_TYPE> &_b)
	{
		TMatrix44<_TYPE> res;
		for(q::qu8 i = 0; i < q::POSITION_MATRIX_DIM; ++i)
		{
			for(q::qu8 j = 0; j < q::POSITION_MATRIX_DIM; ++j)
			{
				res[i][j] = _a[i][0]*_b[0][j]
						  + _a[i][1]*_b[1][j]
						  + _a[i][2]*_b[2][j]
						  + _a[i][3]*_b[3][j];
			}
		}
		return res;
	}

#ifdef Q_SSE_AVX_MATRIX
	template <>
	inline TMatrix44 <q::qf64> operator*(const TMatrix44<q::qf64> &_a, const TMatrix44<q::qf64> &_b)
	{
		TMatrix44<q::qf64> res;
		{
			__m256d c11_c14 = _mm256_mul_pd(_mm256_set1_pd(_a.m_Vec[0][0]), _b.m_Vec[0].m_Value); // a11_b11_a11_b14
			__m256d a12_b21_a12_b24 = _mm256_mul_pd(_mm256_set1_pd(_a.m_Vec[0][1]), _b.m_Vec[1].m_Value);
			c11_c14 = _mm256_add_pd(c11_c14, a12_b21_a12_b24);
			__m256d a13_b31_a13_b34 = _mm256_mul_pd(_mm256_set1_pd(_a.m_Vec[0][2]), _b.m_Vec[2].m_Value);
			c11_c14 = _mm256_add_pd(c11_c14, a13_b31_a13_b34);
			__m256d a14_b41_a14_b44 = _mm256_mul_pd(_mm256_set1_pd(_a.m_Vec[0][3]), _b.m_Vec[3].m_Value);
			c11_c14 = _mm256_add_pd(c11_c14, a14_b41_a14_b44);
			res.m_Vec[0].m_Value = c11_c14;
		}
		{
			__m256d c21_c24 = _mm256_mul_pd(_mm256_set1_pd(_a.m_Vec[1][0]), _b.m_Vec[0].m_Value); // a21_b11_a21_b14
			__m256d a22_b21_a22_b24 = _mm256_mul_pd(_mm256_set1_pd(_a.m_Vec[1][1]), _b.m_Vec[1].m_Value);
			c21_c24 = _mm256_add_pd(c21_c24, a22_b21_a22_b24);
			__m256d a23_b31_a23_b34 = _mm256_mul_pd(_mm256_set1_pd(_a.m_Vec[1][2]), _b.m_Vec[2].m_Value);
			c21_c24 = _mm256_add_pd(c21_c24, a23_b31_a23_b34);
			__m256d a24_b41_a24_b44 = _mm256_mul_pd(_mm256_set1_pd(_a.m_Vec[1][3]), _b.m_Vec[3].m_Value);
			c21_c24 = _mm256_add_pd(c21_c24, a24_b41_a24_b44);
			res.m_Vec[1].m_Value = c21_c24;
		}
		{
			__m256d c31_c34 = _mm256_mul_pd(_mm256_set1_pd(_a.m_Vec[2][0]), _b.m_Vec[0].m_Value); // a31_b11_a31_b14
			__m256d a32_b21_a32_b24 = _mm256_mul_pd(_mm256_set1_pd(_a.m_Vec[2][1]), _b.m_Vec[1].m_Value);
			c31_c34 = _mm256_add_pd(c31_c34, a32_b21_a32_b24);
			__m256d a33_b31_a33_b34 = _mm256_mul_pd(_mm256_set1_pd(_a.m_Vec[2][2]), _b.m_Vec[2].m_Value);
			c31_c34 = _mm256_add_pd(c31_c34, a33_b31_a33_b34);
			__m256d a34_b41_a34_b44 = _mm256_mul_pd(_mm256_set1_pd(_a.m_Vec[2][3]), _b.m_Vec[3].m_Value);
			c31_c34 = _mm256_add_pd(c31_c34, a34_b41_a34_b44);
			res.m_Vec[2].m_Value = c31_c34;
		}
		{
			__m256d c41_c44 = _mm256_mul_pd(_mm256_set1_pd(_a.m_Vec[3][0]), _b.m_Vec[0].m_Value); // a41_b11_a41_b14
			__m256d a42_b21_a42_b24 = _mm256_mul_pd(_mm256_set1_pd(_a.m_Vec[3][1]), _b.m_Vec[1].m_Value);
			c41_c44 = _mm256_add_pd(c41_c44, a42_b21_a42_b24);
			__m256d a43_b31_a43_b34 = _mm256_mul_pd(_mm256_set1_pd(_a.m_Vec[3][2]), _b.m_Vec[2].m_Value);
			c41_c44 = _mm256_add_pd(c41_c44, a43_b31_a43_b34);
			__m256d a44_b41_a44_b44 = _mm256_mul_pd(_mm256_set1_pd(_a.m_Vec[3][3]), _b.m_Vec[3].m_Value);
			c41_c44 = _mm256_add_pd(c41_c44, a44_b41_a44_b44);
			res.m_Vec[3].m_Value = c41_c44;
		}
		return res;
	}
#endif

	template <class _TYPE>
	inline TVector4<_TYPE> operator*(const TMatrix44<_TYPE> &_mat, const TVector4<_TYPE> &_v)
	{
		TVector4<_TYPE> vec;
		for(q::qu8 i = 0; i < q::POSITION_MATRIX_DIM; ++i)
		{
			vec[i] = _mat[i][0]*_v[0]
				  + _mat[i][1]*_v[1]
				  + _mat[i][2]*_v[2]
				  + _mat[i][3]*_v[3];
		}
		return vec;
	}

#ifdef Q_SSE_AVX_MATRIX
	template <>
	inline TVector4<q::qf64> operator*(const TMatrix44<q::qf64> &_mat, const TVector4<q::qf64> &_v)
	{
		TVector4<q::qf64> vec;
		{
			__m256d line = _mm256_mul_pd(_mat.m_Vec[0].m_Value, _v.m_Value);
			vec.m_Values[0] = ((q::qf64*)&line)[0] + ((q::qf64*)&line)[1] + ((q::qf64*)&line)[2] + ((q::qf64*)&line)[3];
		}
		{
			__m256d line = _mm256_mul_pd(_mat.m_Vec[1].m_Value, _v.m_Value);
			vec.m_Values[1] = ((q::qf64*)&line)[0] + ((q::qf64*)&line)[1] + ((q::qf64*)&line)[2] + ((q::qf64*)&line)[3];
		}
		{
			__m256d line = _mm256_mul_pd(_mat.m_Vec[2].m_Value, _v.m_Value);
			vec.m_Values[2] = ((q::qf64*)&line)[0] + ((q::qf64*)&line)[1] + ((q::qf64*)&line)[2] + ((q::qf64*)&line)[3];
		}
		{
			__m256d line = _mm256_mul_pd(_mat.m_Vec[3].m_Value, _v.m_Value);
			vec.m_Values[3] = ((q::qf64*)&line)[0] + ((q::qf64*)&line)[1] + ((q::qf64*)&line)[2] + ((q::qf64*)&line)[3];
		}
		return vec;
	}
#endif

	typedef TMatrix44<q::qf64> Matrix44;

	inline void PrintMatrix(const Matrix44 &_mat)
	{
		//qPrint("%f %f %f %f\n", _mat[0][0], _mat[1][0], _mat[2][0], _mat[3][0]);
		//qPrint("%f %f %f %f\n", _mat[0][1], _mat[1][1], _mat[2][1], _mat[3][1]);
		//qPrint("%f %f %f %f\n", _mat[0][2], _mat[1][2], _mat[2][2], _mat[3][2]);
		//qPrint("%f %f %f %f\n", _mat[0][3], _mat[1][3], _mat[2][3], _mat[3][3]);
		qPrint("%f %f %f %f\n", _mat[0][0], _mat[0][1], _mat[0][2], _mat[0][3]);
		qPrint("%f %f %f %f\n", _mat[1][0], _mat[1][1], _mat[1][2], _mat[1][3]);
		qPrint("%f %f %f %f\n", _mat[2][0], _mat[2][1], _mat[2][2], _mat[2][3]);
		qPrint("%f %f %f %f\n", _mat[3][0], _mat[3][1], _mat[3][2], _mat[3][3]);
	}

	Matrix44 GetRotationAlphaBetaGamma(q::qf64 _alpha, q::qf64 _beta, q::qf64 _gamma);

	inline Matrix44 NormalizeMatrix(const Matrix44 &_mat){
		Matrix44 mat;
		for(qu32 i = 0; i < 3; ++i){
			Vector3 vec = Vector3(_mat[0][i], _mat[1][i], _mat[2][i]);
			vec.normalize();
			mat[0][i] = vec[0];
			mat[1][i] = vec[1];
			mat[2][i] = vec[2];
		}
		return mat;
	}

	inline qbool IsZeroMatrix(const Matrix44 &_mat){
		return _mat[0][0] == 0.0 && _mat[0][1] == 0.0 && _mat[0][2] == 0.0 && _mat[0][3] == 0.0
			&& _mat[1][0] == 0.0 && _mat[1][1] == 0.0 && _mat[1][2] == 0.0 && _mat[1][3] == 0.0
			&& _mat[2][0] == 0.0 && _mat[2][1] == 0.0 && _mat[2][2] == 0.0 && _mat[2][3] == 0.0
			&& _mat[3][0] == 0.0 && _mat[3][1] == 0.0 && _mat[3][2] == 0.0 && _mat[3][3] == 0.0;
	}

	inline Matrix44 Orthogonalize(const Matrix44 &_mat){
		Matrix44 mat = NormalizeMatrix(_mat);
		Vector3 vecX = Vector3(mat[0][0], mat[1][0], mat[2][0]);
		Vector3 vecZ = Vector3(mat[0][2], mat[1][2], mat[2][2]);
		Vector3 vecY = - vecX.cross(vecZ);
		vecZ = vecX.cross(vecY);
		mat[0][0] = vecX[0];mat[0][1] = vecY[0];mat[0][2] = vecZ[0];
		mat[1][0] = vecX[1];mat[1][1] = vecY[1];mat[1][2] = vecZ[1];
		mat[2][0] = vecX[2];mat[2][1] = vecY[2];mat[2][2] = vecZ[2];
		mat = NormalizeMatrix(mat);
		return mat;
	}

	inline bool OrthogonalityTest(Matrix44 &_mat)
	{
		Matrix44 matTmp = _mat.transpose();
		matTmp = _mat*matTmp;
		return APPROX_EQUAL(matTmp[0][0], 1., UTIL_LARGE_EPSILON)
			&& APPROX_EQUAL(matTmp[0][1], 0., UTIL_LARGE_EPSILON)
			&& APPROX_EQUAL(matTmp[0][2], 0., UTIL_LARGE_EPSILON)
			&& APPROX_EQUAL(matTmp[1][0], 0., UTIL_LARGE_EPSILON)
			&& APPROX_EQUAL(matTmp[1][1], 1., UTIL_LARGE_EPSILON)
			&& APPROX_EQUAL(matTmp[1][2], 0., UTIL_LARGE_EPSILON)
			&& APPROX_EQUAL(matTmp[2][0], 0., UTIL_LARGE_EPSILON)
			&& APPROX_EQUAL(matTmp[2][1], 0., UTIL_LARGE_EPSILON)
			&& APPROX_EQUAL(matTmp[2][2], 1., UTIL_LARGE_EPSILON);
	}

	//! solve _mat._c = _b
	void SolveLinearSystem(const Matrix44 &_mat, const Vector4 &_b, Vector4 &_c);

	Q_DLL Matrix44 GetRigidTransformMat(q::qf64 _x, q::qf64 _y, q::qf64 _z
							, q::qf64 _alpha, q::qf64 _beta, q::qf64 _gamma);
	Matrix44 GetRigidTransformMatW(q::qf64 _x, q::qf64 _y, q::qf64 _z
							, q::qf64 _alpha, q::qf64 _beta, q::qf64 _gamma, const Matrix44 &_worldToRotationSpace);

END_Q_NAMESPACE

#endif
