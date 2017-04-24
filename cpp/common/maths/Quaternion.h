#ifndef __UTIL_QUATERNION_HEADER_
#define __UTIL_QUATERNION_HEADER_
#include "common/util/Util.h"
#include "common/maths/Maths.h"
#include "common/maths/Matrix.h"

#include "math.h"
 
BEGIN_Q_NAMESPACE

	template <class _TYPE>
	class TQuaternion
	{
	public:
		inline TQuaternion<_TYPE>(_TYPE _q1, _TYPE _q2, _TYPE _q3, _TYPE _q4)
		{
			m_Q[0] = _q1;
			m_Q[1] = _q2;
			m_Q[2] = _q3;
			m_Q[3] = _q4;
		}

		// axis-angle to quaternion
		// _angle in radian
		inline TQuaternion<_TYPE>(TVector3<_TYPE> _vec, _TYPE _angle){
			_TYPE s = sin(_angle/2.);
			m_Q[0] = _vec[0]*s;
			m_Q[1] = _vec[1]*s;
			m_Q[2] = _vec[2]*s;
			m_Q[3] = cos(_angle/2.);
		}

		//! The matrix must be a rotation matrix -> _mat.T(_mat) = I, det(_mat) = 1
		inline TQuaternion<_TYPE>(TMatrix44<_TYPE> &_mat)
		{ 
			if(_mat[0][0] + _mat[1][1] + _mat[2][2] > 0.0 )
			{
				_TYPE t = + _mat[0][0] + _mat[1][1] + _mat[2][2] + 1.0;
				_TYPE s = (1./sqrt(t))*0.5;
				m_Q[3] = s*t;
				m_Q[2] = (_mat[0][1] - _mat[1][0])*s;
				m_Q[1] = (_mat[2][0] - _mat[0][2])*s;
				m_Q[0] = (_mat[1][2] - _mat[2][1])*s;
			}
			else if(_mat[0][0] > _mat[1][1] && _mat[0][0] > _mat[2][2])
			{
				_TYPE t = + _mat[0][0] - _mat[1][1] - _mat[2][2] + 1.0; 
				_TYPE s = (1./sqrt(t))*0.5;
				m_Q[0] = s*t;
				m_Q[1] = (_mat[0][1] + _mat[1][0])*s;
				m_Q[2] = (_mat[2][0] + _mat[0][2])*s;
				m_Q[3] = (_mat[1][2] - _mat[2][1])*s;
			}
			else if(_mat[1][1] > _mat[2][2])
			{
				_TYPE t = - _mat[0][0] + _mat[1][1] - _mat[2][2] + 1.0;
				_TYPE s = (1./sqrt(t))*0.5;
				m_Q[1] = s*t;
				m_Q[0] = (_mat[0][1] + _mat[1][0])*s;
				m_Q[3] = (_mat[2][0] - _mat[0][2])*s;
				m_Q[2] = (_mat[1][2] + _mat[2][1])*s;
			}
			else
			{
				_TYPE t = - _mat[0][0] - _mat[1][1] + _mat[2][2] + 1.0;
				_TYPE s = (1./sqrt(t))*0.5;
				m_Q[2] = s*t;
				m_Q[3] = (_mat[0][1] - _mat[1][0])*s;
				m_Q[0] = (_mat[2][0] + _mat[0][2])*s;
				m_Q[1] = (_mat[1][2] + _mat[2][1])*s;
			}
		}

		inline TMatrix44<_TYPE> GetMatrix44(void) const
		{
			TMatrix44<_TYPE> mat;
			mat = mat.getIdentity();

			_TYPE x2 = m_Q[0] + m_Q[0];
			_TYPE y2 = m_Q[1] + m_Q[1];
			_TYPE z2 = m_Q[2] + m_Q[2];
			{
				_TYPE xx2 = m_Q[0]*x2;
				_TYPE yy2 = m_Q[1]*y2;
				_TYPE zz2 = m_Q[2]*z2;
				mat[0][0] = 1.0 - yy2 - zz2;
				mat[1][1] = 1.0 - xx2 - zz2;
				mat[2][2] = 1.0 - xx2 - yy2;
			}
			{
				_TYPE yz2 = m_Q[1]*z2;
				_TYPE wx2 = m_Q[3]*x2;
				mat[2][1] = yz2 - wx2;
				mat[1][2] = yz2 + wx2;
			}
			{
				_TYPE xy2 = m_Q[0]*y2;
				_TYPE wz2 = m_Q[3]*z2;
				mat[1][0] = xy2 - wz2;
				mat[0][1] = xy2 + wz2;
			}
			{
				_TYPE xz2 = m_Q[0]*z2;
				_TYPE wy2 = m_Q[3]*y2;
				mat[0][2] = xz2 - wy2;
				mat[2][0] = xz2 + wy2;
			}

			return mat;
		}

		inline TQuaternion<_TYPE> operator+(const TQuaternion<_TYPE> &_q) const
		{
			return TQuaternion<_TYPE>(m_Q[0] + _q.m_Q[0], m_Q[1] + _q.m_Q[1], m_Q[2] + _q.m_Q[2], m_Q[3] + _q.m_Q[3]);
		}

		inline TQuaternion<_TYPE> operator-(const TQuaternion<_TYPE> &_q) const
		{
			return TQuaternion<_TYPE>(m_Q[0] - _q.m_Q[0], m_Q[1] - _q.m_Q[1], m_Q[2] - _q.m_Q[2], m_Q[3] - _q.m_Q[3]);
		}

		inline TQuaternion<_TYPE> operator*(const _TYPE _a) const
		{
			return TQuaternion<_TYPE>(m_Q[0]*_a, m_Q[1]*_a, m_Q[2]*_a, m_Q[3]*_a);
		}

		inline TQuaternion<_TYPE> operator/(const _TYPE _a) const
		{
			return TQuaternion<_TYPE>(m_Q[0]/_a, m_Q[1]/_a, m_Q[2]/_a, m_Q[3]/_a);
		}

		inline _TYPE Length(void) const
		{
			return sqrt(m_Q[0]*m_Q[0] + m_Q[1]*m_Q[1] + m_Q[2]*m_Q[2] + m_Q[3]*m_Q[3]);
		}

		inline void Normalize(void)
		{
			_TYPE len = Length();
			m_Q[0] = m_Q[0]/len;
			m_Q[1] = m_Q[1]/len;
			m_Q[2] = m_Q[2]/len;
			m_Q[3] = m_Q[3]/len;
		}

		static inline _TYPE DotProduct(const TQuaternion<_TYPE> &_q1, const TQuaternion<_TYPE> &_q2)
		{
			return _q1.m_Q[0]*_q2.m_Q[0] + _q1.m_Q[1]*_q2.m_Q[1] + _q1.m_Q[2]*_q2.m_Q[2] + _q1.m_Q[3]*_q2.m_Q[3];
		}

		// REFS http://number-none.com/product/Understanding%20Slerp,%20Then%20Not%20Using%20It/index.html
		static inline TQuaternion<_TYPE> Slerp(const TQuaternion<_TYPE> &_q1, const TQuaternion<_TYPE> &_q2, _TYPE _t)
		{
			// compute the cosine of the angle between the two vectors.
			_TYPE dot = DotProduct(_q1, _q2);
			const _TYPE DOT_THRESHOLD = 0.9995;
			if(dot > DOT_THRESHOLD)
			{
				// if the inputs are too close for comfort, linearly interpolate and normalize the result.
				TQuaternion<_TYPE> result = _q1 + _t*(_q2 - _q1);
				result.Normalize();
				return result;
			}
			
			// robustness: Stay within domain of acos()
			CLAMP(dot, -1, 1); 
			// theta_0 = angle between input vectors
			_TYPE theta_0 = acos(dot);
			// theta = angle between _q1 and result
			_TYPE theta = theta_0*_t;

			TQuaternion<_TYPE> q3 = _q2 - (_q1*dot);
			// {_q1, q3} is now an orthonormal basis
			q3.Normalize(); 

			return (_q1*cos(theta)) + (q3*sin(theta));
		}

	private:
		_TYPE m_Q[4];
	};

	template <class _TYPE>
	inline TQuaternion<_TYPE> operator*(const _TYPE _a, const TQuaternion<_TYPE> _q)
	{
		return _q*_a;
	}

	typedef TQuaternion<q::qf64> Quaternion;

END_Q_NAMESPACE

#endif
