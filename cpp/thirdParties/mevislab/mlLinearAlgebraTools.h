#ifndef __ML_LINEAR_ALGEBRA_HEADER_
#define __ML_LINEAR_ALGEBRA_HEADER_
// REFS
/*
// -----------------------------------------------------------------------
// 
// Copyright (c) 2001-2011, MeVis Medical Solutions AG, Bremen, Germany
// All rights reserved.
// 
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
//     * Redistributions of source code must retain the above copyright
//       notice, this list of conditions and the following disclaimer.
//     * Redistributions in binary form must reproduce the above copyright
//       notice, this list of conditions and the following disclaimer in the
//       documentation and/or other materials provided with the distribution.
//     * Neither the name of MeVis Medical Solutions AG nor the
//       names of its contributors may be used to endorse or promote products
//       derived from this software without specific prior written permission.
// 
// THIS SOFTWARE IS PROVIDED BY MEVIS MEDICAL SOLUTIONS AG ''AS IS'' AND ANY
// EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
// WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
// DISCLAIMED. IN NO EVENT SHALL MEVIS MEDICAL SOLUTIONS AG BE LIABLE FOR ANY
// DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
// (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
// LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
// ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
// (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
// SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/
//=====================================================================================
//! Template class for a 3x3 matrix of 3 rows of Tvec3 vectors.
/*!
// \file    mlLinearAlgebraTools.h
// \author  Wolf Spindler
// \date    12/2006
*/
//=====================================================================================

inline TMatrix44<_TYPE> inverse(qbool *_isInvertible = NULL) const{
	const qu32 Dim = 4;
	TMatrix44<_TYPE> a(*this); // As a evolves from original mat into identity
	TMatrix44<_TYPE> b = TMatrix44<_TYPE>::getIdentity(); // b evolves from identity into inverse(a)
	/*size_t*/qu8 i=0, j=0, i1=0; // Index counters.

	// Loop over columns of a from left to right, eliminating above and below diagonal
	for (j=0; j<Dim; j++){
		// Find largest pivot in column j among rows j..DIM-1
		i1 = j;       // Row with largest pivot candidate
		for (i=j+1; i<Dim; i++){
			if (ABS(a[i][j]) > ABS(a[i1][j])){
				i1 = i;
			}
		}
		// Swap rows i1 and j in a and b to put pivot on diagonal
		Swap(a[i1], a[j]);
		Swap(b[i1], b[j]);      

		// Scale row j to have a identity diagonal
		const _TYPE avjj = a[j][j];
		if (avjj == 0){
			if (_isInvertible == NULL){
				qPrint("ERROR the matrix is not invertible\n");
			}
			else{
				*_isInvertible = false;
			}
			return TMatrix44<_TYPE>::getIdentity();
		}
		b[j] /= avjj;
		a[j] /= avjj;

		// Eliminate off-diagonal elements in col j of a, 
		// applying identical operations to b
		for (i=0; i<Dim; i++){
			if (i!=j){
				b[i] -= a[i][j]*b[j];
				a[i] -= a[i][j]*a[j];
			}
		}
	}

	// Set state and return inverse.
	if (_isInvertible != NULL){ *_isInvertible = true; }
	return b;
}

#endif
