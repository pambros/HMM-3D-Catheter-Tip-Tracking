#if !defined(DDM_PARAM_MULTITHREAD)
	if(ddmParamMultiThread == true){
	#define DDM_PARAM_MULTITHREAD MultiThread
		#define DDM_MULTITHREAD
			#include "CallDistanceMetric.h"
		#undef DDM_MULTITHREAD
	#undef DDM_PARAM_MULTITHREAD
	}
	else{
	#define DDM_PARAM_MULTITHREAD
		#include "CallDistanceMetric.h"
	#undef DDM_PARAM_MULTITHREAD
	}
#elif !defined(DDM_PARAM_METRIC)
	if(ddmParamMetric == true){
	#define DDM_PARAM_METRIC ImprovedMetric
		#include "CallDistanceMetric.h"
	#undef DDM_PARAM_METRIC
	}
	else{
	#define DDM_PARAM_METRIC
		#include "CallDistanceMetric.h"
	#undef DDM_PARAM_METRIC
	}
#elif !defined(DDM_PARAM_WEIGHT)
	if(ddmParamWeight == true){
	#define DDM_PARAM_WEIGHT Weight
		#include "CallDistanceMetric.h"
	#undef DDM_PARAM_WEIGHT
	}
	else{
	#define DDM_PARAM_WEIGHT
		#include "CallDistanceMetric.h"
	#undef DDM_PARAM_WEIGHT
	}
#elif !defined(DDM_PARAM_NORM)
	if(ddmParamNorm == DISTANCE_EUCLIDEAN){
	#define DDM_PARAM_NORM Euclidean
		#include "CallDistanceMetric.h"
	#undef DDM_PARAM_NORM
	}
	else if(ddmParamNorm == DISTANCE_ABSOLUTE){
	#define DDM_PARAM_NORM Absolute
		#include "CallDistanceMetric.h"
	#undef DDM_PARAM_NORM
	}
	else{
	#define DDM_PARAM_NORM Sqr
		#include "CallDistanceMetric.h"
	#undef DDM_PARAM_NORM
	}
#elif !defined(DDM_PARAM_RADIUS)
	if(ddmParamRadius == RADIUS_METRIC_FCOST1){
	#define DDM_PARAM_RADIUS
		#include "CallDistanceMetric.h"
	#undef DDM_PARAM_RADIUS
	}
	else if(ddmParamRadius == RADIUS_METRIC_FCOST2){
	#define DDM_PARAM_RADIUS Sigmoid
		#include "CallDistanceMetric.h"
	#undef DDM_PARAM_RADIUS
	}
	else{
	#define DDM_PARAM_RADIUS Sigmoid2
		#include "CallDistanceMetric.h"
	#undef DDM_PARAM_RADIUS
	}
#elif !defined(DDM_PARAM_LIMIT)
	#if defined(DDM_MULTITHREAD)
		#define DDM_PARAM_LIMIT
			#include "CallDistanceMetric.h"
		#undef DDM_PARAM_LIMIT
	#else
	if(ddmLimit == true){
		#define DDM_PARAM_LIMIT Limit
			#include "CallDistanceMetric.h"
		#undef DDM_PARAM_LIMIT
	}
	else{
		#define DDM_PARAM_LIMIT
			#include "CallDistanceMetric.h"
		#undef DDM_PARAM_LIMIT
	}
	#endif
#else
	#define DDM_METRIC_NAME_FCT(_a, _b, _c, _d, _e, _f) MetricCallback ## _a ## _b ## _c ## _d ## _e ## _f
	#define DDM_METRIC_NAME(_a, _b, _c, _d, _e, _f) DDM_METRIC_NAME_FCT(_a, _b, _c, _d, _e, _f)
	DDM_FUNCTION_DELEGATE = fastdelegate::MakeDelegate(this, &DDM_CLASS_NAME::DDM_METRIC_NAME(DDM_PARAM_MULTITHREAD, DDM_PARAM_METRIC, DDM_PARAM_WEIGHT, DDM_PARAM_NORM, DDM_PARAM_RADIUS, DDM_PARAM_LIMIT));
	#undef DDM_METRIC_NAME
	#undef DDM_METRIC_NAME_FCT
#endif