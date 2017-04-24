#include "common/maths/Matrix.h"
#ifdef NEED_QUATERNION
	#include "common/maths/Quaternion.h"
#endif

BEGIN_Q_NAMESPACE

Matrix44 GetRotationAlphaBetaGamma(q::qf64 _alpha, q::qf64 _beta, q::qf64 _gamma){
//#define USE_QUATERNION_ROT
#if defined(USE_QUATERNION_ROT) && defined(NEED_QUATERNION)
	Quaternion qx = Quaternion(Vector3(1., 0., 0.), DEGREE_TO_RADIAN(_alpha));
	Matrix44 rotX = qx.GetMatrix44();
	Quaternion qy = Quaternion(Vector3(0., 1., 0.), DEGREE_TO_RADIAN(_beta));
	Matrix44 rotY = qy.GetMatrix44();
	Quaternion qz = Quaternion(Vector3(0., 0., 1.), DEGREE_TO_RADIAN(_gamma));
	Matrix44 rotZ = qz.GetMatrix44();
#else
	qf64 radAlpha = DEGREE_TO_RADIAN(_alpha);
	qf64 cosAlpha = cos(radAlpha);
	qf64 sinAlpha = sin(radAlpha);
	Matrix44 rotX = Matrix44::getIdentity();
	rotX[0][0] = cosAlpha;
	rotX[0][2] = sinAlpha;
	rotX[2][0] = -sinAlpha;
	rotX[2][2] = cosAlpha;

	qf64 radBeta = DEGREE_TO_RADIAN(_beta);
	qf64 cosBeta = cos(radBeta);
	qf64 sinBeta = sin(radBeta);
	Matrix44 rotY = Matrix44::getIdentity();
	rotY[1][1] = cosBeta;
	rotY[1][2] = -sinBeta;
	rotY[2][1] = sinBeta;
	rotY[2][2] = cosBeta;

	qf64 radGamma = DEGREE_TO_RADIAN(_gamma);
	qf64 cosGamma = cos(radGamma);
	qf64 sinGamma = sin(radGamma);
	Matrix44 rotZ = Matrix44::getIdentity();
	rotZ[0][0] = cosGamma;
	rotZ[0][1] = -sinGamma;
	rotZ[1][0] = sinGamma;
	rotZ[1][1] = cosGamma;
#endif
	//PrintMatrix(rotX);
	//PrintMatrix(rotY);
	//PrintMatrix(rotZ);
	return rotZ*rotY*rotX;
}

	// Finds solution to set of linear equations A x = b by LU decomposition.
	// REFS Chapter 2, Programs 3-5, Fig. 2.8-2.10
	// Gerald/Wheatley, APPLIED NUMERICAL ANALYSIS (fourth edition)
	// Addison-Wesley, 1989

	//! Find pivot element
	// The function pivot finds the largest element for a pivot in _jcol
	// of Matrix _mat, performs interchanges of the appropriate
	// rows in _mat, and also interchanges the corresponding elements in
	// the order vector.
	// \param _mat POSITION_MATRIX_DIM by POSITION_MATRIX_DIM Matrix of coefficients
	// \param _order integer vector to hold row ordering
	// \param _jcol column of "_mat" being searched for pivot element
	static qs32 Pivot(Matrix44 &_mat, Vector4 &_order, qs32 _jcol)
	{
		qs32 i, ipvt;
		qf64 big, anext;

		// Find biggest element on or below diagonal. This will be the pivot row.
		ipvt = _jcol;
		big = ABS(_mat[ipvt][ipvt]);
		for(i = ipvt + 1; i < POSITION_MATRIX_DIM; ++i)
		{
			anext = ABS(_mat[i][_jcol]);
			if(anext > big)
			{
				big = anext;
				ipvt = i;
			}
		}
		// otherwise Matrix is singular
		qAssert(ABS(big) > UTIL_EPSILON);
		
		// Interchange pivot row (ipvt) with current row (jcol).
		if(ipvt == _jcol)
		{
			return 0;
		}

		// swap rows _jcol and ipvt
		for(qs32 j = 0; j < POSITION_MATRIX_DIM; ++j)
		{
			qf64 tmp = _mat[_jcol][j];
			_mat[_jcol][j] = _mat[ipvt][j];
			_mat[ipvt][j] = tmp;
		}

		i = static_cast<qs32>(_order[_jcol]);
		_order[_jcol] = _order[ipvt];
		_order[ipvt] = i;
		return 1;
	}

	void SolveLinearSystem(const Matrix44 &_mat, const Vector4 &_b, Vector4 &_c)
	{
		Matrix44 mat = _mat;
		Vector4 order;

		// do the LU decomposition
		// changes sign with each row interchange
		qs32 flag = 1;
		qf64 sum, diag;

		// establish initial ordering in order vector
		for(qs32 i = 0; i < POSITION_MATRIX_DIM; ++i)
		{
			order[i] = i;
		}

		// do pivoting for first column and check for singularity
		if(Pivot(mat, order, 0) != 0)
		{
			flag = -flag;
		}
		diag = 1.0/mat[0][0];
		for(qs32 i = 1; i < POSITION_MATRIX_DIM; i++)
		{
			mat[0][i] *= diag;
		}
	
		// Now complete the computing of L and U elements.
		// The general plan is to compute a column of L's, then
		// call pivot to interchange rows, and then compute
		// a row of U's.
	
		qs32 nm1 = POSITION_MATRIX_DIM - 1;
		for(qs32 j = 1; j < nm1; ++j)
		{
			// column of L's
			for(qs32 i = j; i < POSITION_MATRIX_DIM; ++i)
			{
				sum = 0.0;
				for(qs32 k = 0; k < j; ++k)
				{
					sum += mat[i][k]*mat[k][j];
				}
				mat[i][j] -= sum;
			}

			// pivot, and check for singularity
			if(Pivot(mat, order, j) != 0)
			{
				flag = -flag;
			}

			// row of U's
			diag = 1.0/mat[j][j];
			for(qs32 k = j + 1; k < POSITION_MATRIX_DIM; ++k)
			{
				sum = 0.0;
				for(qs32 i = 0; i < j; ++i)
				{
					sum += mat[j][i]*mat[i][k];
				}
				mat[j][k] = (mat[j][k] - sum)*diag;
			}
		}

		// still need to get last element in L Matrix
		sum = 0.0;
		for(qs32 k = 0; k < nm1; ++k)
		{
			sum += mat[nm1][k]*mat[k][nm1];
		}
		mat[nm1][nm1] -= sum;

#ifdef DEBUG
		// verify determinant, if near zero, coefficient matrix is singular
		qf64 det = flag;
		for(qs32 i = 0; i < POSITION_MATRIX_DIM; ++i)
		{
			det *= mat[i][i];
		}
		qAssert(ABS(det) > UTIL_EPSILON);
#endif


		// solve the linear system
		// rearrange the elements of the _b vector. _c is used to hold them.
		for(qs32 i = 0; i < POSITION_MATRIX_DIM; ++i)
		{
			qs32 j = static_cast<qs32>(order[i]);
			_c[i] = _b[j];
		}

		// do forward substitution, replacing _c vector.
		_c[0] /= mat[0][0];
		for(qs32 i = 1; i < POSITION_MATRIX_DIM; ++i)
		{
			sum = 0.0;
			for(qs32 j = 0; j < i; ++j)
			{
				sum += mat[i][j]*_c[j];
			}
			_c[i] = (_c[i] - sum)/mat[i][i];
		}

		// now get the solution vector, _c[POSITION_MATRIX_DIM - 1] is already done
		for(qs32 i = POSITION_MATRIX_DIM - 2; i >= 0; --i)
		{
			sum = 0.0;
			for(qs32 j = i + 1; j < POSITION_MATRIX_DIM; ++j)
			{
				sum += mat[i][j]*_c[j];
			}
			_c[i] -= sum;
		}
	}

	Matrix44 GetRigidTransformMat(q::qf64 _x, q::qf64 _y, q::qf64 _z
							, q::qf64 _alpha, q::qf64 _beta, q::qf64 _gamma){
		Matrix44 translation = translation.getIdentity();
		translation[0][3] = _x;
		translation[1][3] = _y;
		translation[2][3] = _z;
		return translation*GetRotationAlphaBetaGamma(_alpha, _beta, _gamma);
	}

	Matrix44 GetRigidTransformMatW(q::qf64 _x, q::qf64 _y, q::qf64 _z
							, q::qf64 _alpha, q::qf64 _beta, q::qf64 _gamma, const Matrix44 &_worldToRotationSpace){
		Matrix44 translation = translation.getIdentity();
		translation[0][3] = _x;
		translation[1][3] = _y;
		translation[2][3] = _z;
		return translation*_worldToRotationSpace.inverse()*GetRotationAlphaBetaGamma(_alpha, _beta, _gamma)*_worldToRotationSpace;
	}
END_Q_NAMESPACE
