#ifndef __NPY_MATH_HEADER_
#define __NPY_MATH_HEADER_

// perform log(1 + _x)
// REFS code from numpy python library npy_math.c.src
// should be more stable numerically than computing log(1 + _x)
inline q::qf64 log1p(q::qf64 _x){
	if (std::isinf(_x) && _x > 0){
		return _x;
	}
	else{
		const q::qf64 u = 1. + _x;
		const q::qf64 d = u - 1.;

		if (d == 0.){
			return _x;
		}
		else{
			return log(u)*_x/d;
		}
	}
}

// _a = log(a') is the log probability of a'
// _b = log(b') is the log probability of b'
// perform log(a' + b') = log(exp(_a) + exp(_b))
// = a + log(1 + exp(b - a))
// REFS code from numpy python library npy_math.c.src
// should be more stable numerically
inline q::qf64 logaddexp(q::qf64 _a, q::qf64 _b){
	if(_a == _b){
		/* Handles infinities of the same sign without warnings */
		return _a + UTIL_LOG2E;
	}

	const q::qf64 tmp = _a - _b;
	if(tmp > 0.){
		return _a + log1p(exp(-tmp));
	}
	else if(tmp <= 0.){
		return _b + log1p(exp(tmp));
	}
	
	/* NaNs */
	return tmp;
}

#endif
