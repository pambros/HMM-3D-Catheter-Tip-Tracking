#if !defined(DDM_PARAM_MULTITHREAD)
	#define DDM_PARAM_MULTITHREAD MultiThread
		#define DDM_MULTITHREAD
			#include "DefineDistanceMetric.h"
		#undef DDM_MULTITHREAD
	#undef DDM_PARAM_MULTITHREAD
	#define DDM_PARAM_MULTITHREAD
		#include "DefineDistanceMetric.h"
	#undef DDM_PARAM_MULTITHREAD
#elif !defined(DDM_PARAM_METRIC)
	#define DDM_PARAM_METRIC
		#include "DefineDistanceMetric.h"
	#undef DDM_PARAM_METRIC
	#define DDM_PARAM_METRIC ImprovedMetric 
		#define DDM_IMPROVED_METRIC
			#include "DefineDistanceMetric.h"
		#undef DDM_IMPROVED_METRIC
	#undef DDM_PARAM_METRIC
#elif !defined(DDM_PARAM_WEIGHT)
	#define DDM_PARAM_WEIGHT
		#include "DefineDistanceMetric.h"
	#undef DDM_PARAM_WEIGHT
	#define DDM_PARAM_WEIGHT Weight
		#define DDM_WEIGHT
			#include "DefineDistanceMetric.h"
		#undef DDM_WEIGHT
	#undef DDM_PARAM_WEIGHT
#elif !defined(DDM_PARAM_NORM)
	#define DDM_PARAM_NORM Euclidean
		#include "DefineDistanceMetric.h"
	#undef DDM_PARAM_NORM
	#define DDM_PARAM_NORM Absolute
		#define DDM_NORM_ABSOLUTE
			#include "DefineDistanceMetric.h"
		#undef DDM_NORM_ABSOLUTE
	#undef DDM_PARAM_NORM
	#define DDM_PARAM_NORM Sqr
		#define DDM_NORM_SQR
			#include "DefineDistanceMetric.h"
		#undef DDM_NORM_SQR
	#undef DDM_PARAM_NORM
#elif !defined(DDM_PARAM_RADIUS)
	#define DDM_PARAM_RADIUS
		#include "DefineDistanceMetric.h"
	#undef DDM_PARAM_RADIUS
	#define DDM_PARAM_RADIUS Sigmoid
		#define DDM_RADIUS_SIGMOID
			#include "DefineDistanceMetric.h"
		#undef DDM_RADIUS_SIGMOID
	#undef DDM_PARAM_RADIUS
	#define DDM_PARAM_RADIUS Sigmoid2
		#define DDM_RADIUS_SIGMOID2
			#include "DefineDistanceMetric.h"
		#undef DDM_RADIUS_SIGMOID2
	#undef DDM_PARAM_RADIUS
#elif !defined(DDM_PARAM_LIMIT)
	#if defined(DDM_MULTITHREAD) // limit is used with powell optimizer and multithread is not possible with powell, so we just don't define function with limit
		#define DDM_PARAM_LIMIT
			#include "DefineDistanceMetric.h"
		#undef DDM_PARAM_LIMIT
	#else
		#define DDM_PARAM_LIMIT Limit
			#define DDM_LIMIT
				#include "DefineDistanceMetric.h"
			#undef DDM_LIMIT
		#undef DDM_PARAM_LIMIT
		#define DDM_PARAM_LIMIT
			#include "DefineDistanceMetric.h"
		#undef DDM_PARAM_LIMIT
	#endif
#else
#define DDM_METRIC_NAME_FCT(_a, _b, _c, _d, _e, _f) MetricCallback ## _a ## _b ## _c ## _d ## _e ## _f
	#define DDM_METRIC_NAME(_a, _b, _c, _d, _e, _f) DDM_METRIC_NAME_FCT(_a, _b, _c, _d, _e, _f)
	#if defined(DDM_HEADER)
		#ifdef DDM_MULTITHREAD
	qf64 DDM_METRIC_NAME(DDM_PARAM_MULTITHREAD, DDM_PARAM_METRIC, DDM_PARAM_WEIGHT, DDM_PARAM_NORM, DDM_PARAM_RADIUS, DDM_PARAM_LIMIT)(const qf64 *_currentPos, qu32 _currentThreadId);
		#else
	qf64 DDM_METRIC_NAME(DDM_PARAM_MULTITHREAD, DDM_PARAM_METRIC, DDM_PARAM_WEIGHT, DDM_PARAM_NORM, DDM_PARAM_RADIUS, DDM_PARAM_LIMIT)(const qf64 *_currentPos);
		#endif
	#else
		#ifdef DDM_MULTITHREAD
	qf64 Base2D3DRegistration::DDM_METRIC_NAME(DDM_PARAM_MULTITHREAD, DDM_PARAM_METRIC, DDM_PARAM_WEIGHT, DDM_PARAM_NORM, DDM_PARAM_RADIUS, DDM_PARAM_LIMIT)(const qf64 *_currentPos, qu32 _currentThreadId){
		#else
	qf64 Base2D3DRegistration::DDM_METRIC_NAME(DDM_PARAM_MULTITHREAD, DDM_PARAM_METRIC, DDM_PARAM_WEIGHT, DDM_PARAM_NORM, DDM_PARAM_RADIUS, DDM_PARAM_LIMIT)(const qf64 *_currentPos){
		#endif

		// compute the fitting metric
		qf64 currentFittingMetric = 0.;

		Matrix44 mat = GetRigidTransform(_currentPos[0], _currentPos[1], _currentPos[2]
			, _currentPos[3], _currentPos[4], _currentPos[5]);
		#ifdef COMPUTE_RIGID_TRANSFORM_IN_CARM_SPACE
		// do the rigid transform in the c-arm CS
		//mat = mat*GetBaseParameters()->m_WorldToCArm;
		mat = mat*m_TransformToCArm;
		#else
		// do the rigid transform in the world CS
		#endif

		// compute all pt3dto2d
		#ifdef DDM_MULTITHREAD
		Vector2 *pt3dTo2d = m_Pt3dTo2dThread[_currentThreadId];
		#else
		Vector2 *pt3dTo2d = m_Pt3dTo2d;
		#endif
		mat = GetBaseParameters()->m_CArmProjection*mat;
		for (qu32 k = 0; k < m_Nb3dPts; ++k){
			Vector4 pt3d = mat*m_3dVesselsTmp[k];
			pt3dTo2d[k][0] = pt3d[0] / pt3d[3];
			pt3dTo2d[k][1] = pt3d[1] / pt3d[3];
		}

		#ifdef DDM_IMPROVED_METRIC
		qf64 minDist = UTIL_BIG_POSITIVE_NUMBER;
		qu32 bestId = 0;
		for (qu32 k = 0; k < m_Nb3dPts; ++k){
			#if defined(DDM_NORM_ABSOLUTE)
			qf64 dist = GetAbsoluteDistance(pt3dTo2d[k], m_2dCatheterTmp[0]);
			#elif defined(DDM_NORM_SQR)
			qf64 dist = GetSquareDistance(pt3dTo2d[k], m_2dCatheterTmp[0]);
			#else
			qf64 dist = GetDistance(pt3dTo2d[k], m_2dCatheterTmp[0]);
			#endif
			if (dist < minDist){
				minDist = dist;
				bestId = k;
			}
		}
			#if defined(DDM_RADIUS_SIGMOID)
		const qf64 STIFFNESS = GetBaseParameters()->m_RadiusAlpha1;
		const qf64 RADIUS_BOUND = GetBaseParameters()->m_RadiusAlpha2;
		minDist = minDist*(1/(1 + exp(STIFFNESS*(RADIUS_BOUND*m_3dVesselsRadius[bestId] - minDist))));
			#elif defined(DDM_RADIUS_SIGMOID2)
		// if minDist is superior to radius, we put x - radius, if not just 0
		if(minDist <= m_3dVesselsRadius[bestId]){
			minDist = 0;
		}
		else{
			minDist = (minDist - m_3dVesselsRadius[bestId]);
		}
			#endif

			#ifdef DDM_WEIGHT
		currentFittingMetric = currentFittingMetric - minDist*m_PrecomputedWeightCurve[0];
			#else
		currentFittingMetric = currentFittingMetric - minDist;
			#endif
		
#define CUMUL_DISTANCE
#ifdef CUMUL_DISTANCE
		//const qf64 SQR_NEIGHBORHOOD_DISTANCE = GetBaseParameters()->m_Dmax*GetBaseParameters()->m_Dmax;
		const qf64 NEIGHBORHOOD_DISTANCE = GetBaseParameters()->m_Dmax;
#else
		const qf64 SQR_NEIGHBORHOOD_DISTANCE = GetBaseParameters()->m_Dmax*GetBaseParameters()->m_Dmax;
#endif
		for(qu32 j = 1; j < m_Nb2dPts; ++j){
			minDist = UTIL_BIG_POSITIVE_NUMBER;
#ifdef CUMUL_DISTANCE
			qf64 cumulDist = 0.0;
#endif
			qu32 bestIdTmp = bestId;
			for(qu32 k = bestId; k < m_Nb3dPts; ++k){
#ifdef CUMUL_DISTANCE
				if(k != bestIdTmp){
					//cumulDist = cumulDist + GetSquareDistance(pt3dTo2d[k-1], pt3dTo2d[k]);
					cumulDist = cumulDist + GetDistance(pt3dTo2d[k-1], pt3dTo2d[k]);
				}
				//if(cumulDist > SQR_NEIGHBORHOOD_DISTANCE){
				if(cumulDist > NEIGHBORHOOD_DISTANCE){
#else
				if(GetSquareDistance(pt3dTo2d[bestIdTmp], pt3dTo2d[k]) > SQR_NEIGHBORHOOD_DISTANCE){
#endif
					break;
				}

				#if defined(DDM_NORM_ABSOLUTE)
				qf64 dist = GetAbsoluteDistance(pt3dTo2d[k], m_2dCatheterTmp[j]);
				#elif defined(DDM_NORM_SQR)
				qf64 dist = GetSquareDistance(pt3dTo2d[k], m_2dCatheterTmp[j]);
				#else
				qf64 dist = GetDistance(pt3dTo2d[k], m_2dCatheterTmp[j]);
				#endif
				if(dist < minDist){
					minDist = dist;
					bestId = k;
				}
			}
			#if defined(DDM_RADIUS_SIGMOID)
			minDist = minDist*(1/(1 + exp(STIFFNESS*(RADIUS_BOUND*m_3dVesselsRadius[bestId] - minDist))));
			#elif defined(DDM_RADIUS_SIGMOID2)
			// if minDist is superior to radius, we put x - radius, if not just 0
			if(minDist <= m_3dVesselsRadius[bestId]){
				minDist = 0;
			}
			else{
				minDist = (minDist - m_3dVesselsRadius[bestId]);
			}
			#endif

			#ifdef DDM_WEIGHT
			currentFittingMetric = currentFittingMetric - minDist*m_PrecomputedWeightCurve[j];
			#else
			currentFittingMetric = currentFittingMetric - minDist;
			#endif
		}
		#else
		for (qu32 j = 0; j < m_Nb2dPts; ++j){
		#if defined(DDM_RADIUS_SIGMOID) || defined(DDM_RADIUS_SIGMOID2)
			qu32 bestId = 0;
		#endif
			qf64 minDist = UTIL_BIG_POSITIVE_NUMBER;
			for (qu32 k = 0; k < m_Nb3dPts; ++k){
				#if defined(DDM_NORM_ABSOLUTE)
					qf64 dist = GetAbsoluteDistance(pt3dTo2d[k], m_2dCatheterTmp[j]);
				#elif defined(DDM_NORM_SQR)
					qf64 dist = GetSquareDistance(pt3dTo2d[k], m_2dCatheterTmp[j]);
				#else
					qf64 dist = GetDistance(pt3dTo2d[k], m_2dCatheterTmp[j]);
				#endif
				if (dist < minDist){
					minDist = dist;
				#if defined(DDM_RADIUS_SIGMOID) || defined(DDM_RADIUS_SIGMOID2)
					bestId = k;
				#endif
					//Q_PRINT_DEBUG("minDist %f\n", minDist);
				}
			}

			#if defined(DDM_RADIUS_SIGMOID)
			const qf64 STIFFNESS = GetBaseParameters()->m_RadiusAlpha1;
			const qf64 RADIUS_BOUND = GetBaseParameters()->m_RadiusAlpha2;
			minDist = minDist*(1/(1 + exp(STIFFNESS*(RADIUS_BOUND*m_3dVesselsRadius[bestId] - minDist))));
			#elif defined(DDM_RADIUS_SIGMOID2)
			// if minDist is superior to radius, we put x - radius, if not just 0
			if(minDist <= m_3dVesselsRadius[bestId]){
				minDist = 0;
			}
			else{
				minDist = (minDist - m_3dVesselsRadius[bestId]);
			}
			#endif
			
			#ifdef DDM_WEIGHT
			currentFittingMetric = currentFittingMetric - minDist*m_PrecomputedWeightCurve[j];
			#else
			currentFittingMetric = currentFittingMetric - minDist;
			#endif
		}
		#endif
		
		#ifdef DDM_LIMIT
		if(_currentPos[0] > GetBaseParameters()->m_LimitX || _currentPos[0] < -GetBaseParameters()->m_LimitX
		|| _currentPos[1] > GetBaseParameters()->m_LimitY || _currentPos[1] < -GetBaseParameters()->m_LimitY
		|| _currentPos[3] > GetBaseParameters()->m_LimitAlpha || _currentPos[3] < -GetBaseParameters()->m_LimitAlpha
		|| _currentPos[4] > GetBaseParameters()->m_LimitBeta || _currentPos[4] < -GetBaseParameters()->m_LimitBeta
		|| _currentPos[5] > GetBaseParameters()->m_LimitGamma || _currentPos[5] < -GetBaseParameters()->m_LimitGamma){
			qf64 max = MAX(_currentPos[0] - GetBaseParameters()->m_LimitX,
							MAX(-_currentPos[0] - GetBaseParameters()->m_LimitX, 
								MAX(_currentPos[1] - GetBaseParameters()->m_LimitY,
									MAX(-_currentPos[1] - GetBaseParameters()->m_LimitY, 
										MAX(_currentPos[3] - GetBaseParameters()->m_LimitAlpha,
											MAX(-_currentPos[3] - GetBaseParameters()->m_LimitAlpha, 
												MAX(_currentPos[4] - GetBaseParameters()->m_LimitBeta,
													MAX(-_currentPos[4] - GetBaseParameters()->m_LimitBeta, 
														MAX(_currentPos[5] - GetBaseParameters()->m_LimitGamma, -_currentPos[5] - GetBaseParameters()->m_LimitGamma)
													)
												)
											)
										)
									)
								)
							)
						);
#ifdef CUMUL_DISTANCE
			currentFittingMetric = currentFittingMetric*(max*max/(4.*4.) + 1);
#else
			currentFittingMetric = currentFittingMetric*(max*max/(10.*10.) + 1);
#endif
		}
		#endif
		//qPrint("currentFittingMetric %f _currentPos %f %f %f // %f %f %f\n", currentFittingMetric, _currentPos[0], _currentPos[1], _currentPos[2], _currentPos[3], _currentPos[4], _currentPos[5]);
		return currentFittingMetric;
	}
	#endif

	#undef DDM_METRIC_NAME
	#undef DDM_METRIC_NAME_FCT
#endif
