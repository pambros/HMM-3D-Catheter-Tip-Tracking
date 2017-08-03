#include "Base2D3DRegistration.h"

#define COMPUTE_RIGID_TRANSFORM_IN_CARM_SPACE 1
//#define COMPUTE_RIGID_TRANSFORM_IN_WORLD_SPACE 1

//#define __DEBUG_PRINT 1
#ifdef __DEBUG_PRINT
	#define Q_PRINT_DEBUG(...) q::qPrint(__VA_ARGS__)
#else
	#define Q_PRINT_DEBUG(...)
#endif

using namespace std;

BEGIN_Q_NAMESPACE

qu32 Base2D3DRegistration::LoadParameters(qFile *_file){
	qu32 iErr = FileReachLine(_file, qString("#\tBase2D3DParameters"));
	iErr = FileReadU32(_file, GetBaseParameters()->m_DistanceUsed);
	iErr = FileReadU32(_file, GetBaseParameters()->m_MetricUsed);
	iErr = FileReadF64(_file, GetBaseParameters()->m_Dmax);
	iErr = FileReadBool(_file, GetBaseParameters()->m_UseWeight);
	iErr = FileReadF64(_file, GetBaseParameters()->m_WeightSigma);
	iErr = FileReadF64(_file, GetBaseParameters()->m_WeightLambda);
	iErr = FileReadU32(_file, GetBaseParameters()->m_RadiusMetric);
	iErr = FileReadF64(_file, GetBaseParameters()->m_RadiusAlpha1);
	iErr = FileReadF64(_file, GetBaseParameters()->m_RadiusAlpha2);
	iErr = FileReadBool(_file, GetBaseParameters()->m_UseLimits);
	iErr = FileReadF64(_file, GetBaseParameters()->m_LimitX);
	iErr = FileReadF64(_file, GetBaseParameters()->m_LimitY);
	iErr = FileReadF64(_file, GetBaseParameters()->m_LimitZ);
	iErr = FileReadF64(_file, GetBaseParameters()->m_LimitAlpha);
	iErr = FileReadF64(_file, GetBaseParameters()->m_LimitBeta);
	iErr = FileReadF64(_file, GetBaseParameters()->m_LimitGamma);
	return iErr;
}

void Base2D3DRegistration::PreApply(const q::PtList &_2dCatheter, const q::PtList &_3dVessels, qu32 _nbProcUsed){
	m_FittingScore = UTIL_BIG_NEGATIVE_NUMBER;

	m_Nb2dPts = static_cast<qu32>(_2dCatheter.size());
	m_Nb3dPts = static_cast<qu32>(_3dVessels.size());
	m_2dCatheterTmp = Q_NEW Vector2[m_Nb2dPts];
	m_PrecomputedWeightCurve = Q_NEW qf64[m_Nb2dPts];
	m_3dVesselsTmp = Q_NEW Vector4[m_Nb3dPts];
	if(GetBaseParameters()->m_RadiusMetric != RADIUS_METRIC_FCOST1){
		m_3dVesselsRadius = Q_NEW qf64[m_Nb3dPts];
	}
	
	if(_nbProcUsed > 1){
		m_Pt3dTo2dThread = Q_NEW Vector2*[_nbProcUsed];
		for(qu32 i = 0; i < _nbProcUsed; ++i){
			m_Pt3dTo2dThread[i] = Q_NEW Vector2[m_Nb3dPts];
		}
	}
	else{
		m_Pt3dTo2d = Q_NEW Vector2[m_Nb3dPts];
	}

	for(qu32 i = 0; i < m_Nb2dPts; ++i){
		m_2dCatheterTmp[i] = Vector2(_2dCatheter[i].x(), _2dCatheter[i].y());
	}
	
	// compute a gaussian curve to get a weight decreasing curve
	qf64 cumulateCatheterDistance = 0.0;
	m_PrecomputedWeightCurve[0] = 1.0;
	qf64 sigma = GetBaseParameters()->m_WeightSigma;
	qf64 lambda = GetBaseParameters()->m_WeightLambda;
	qf64 x = 0.0;
	for(qu32 i = 1; i < m_Nb2dPts; ++i){
		cumulateCatheterDistance += GetDistance(m_2dCatheterTmp[i], m_2dCatheterTmp[i - 1]);
		x = cumulateCatheterDistance;
		m_PrecomputedWeightCurve[i] = UTIL_GAUSSIAN_2(x, sigma)*(1.0 - lambda) + lambda;
	}

	for(qu32 i = 0; i < m_Nb3dPts; ++i){
		m_3dVesselsTmp[i] = Vector4(_3dVessels[i].x(), _3dVessels[i].y(), _3dVessels[i].z(), 1.0);

		if(GetBaseParameters()->m_RadiusMetric != RADIUS_METRIC_FCOST1){
			m_3dVesselsRadius[i] = _3dVessels[i].u();
			//q::qPrint("radius %f\n", m_3dVesselsRadius[i]);
		}
	}

#ifdef __DEBUG_PRINT
	Q_PRINT_DEBUG("matCArmProjection\n");
	PrintMatrix(GetBaseParameters()->m_CArmProjection);
	Q_PRINT_DEBUG("matWorldToCArm\n");
	PrintMatrix(GetBaseParameters()->m_WorldToCArm);
#endif

	if(m_ComputeRigidTransformInSupposedTipSpace == Q_FALSE){
		m_CArmToIsoCenter = GetBaseParameters()->m_IsoCenterToCArm.inverse();
	}
	else{
		Matrix44 cArmToWorld = GetBaseParameters()->m_WorldToCArm.inverse();
		m_CArmToSupposedTip = m_WorldToSupposedTip*cArmToWorld;
		m_SupposedTipToCArm = m_CArmToSupposedTip.inverse();
	}
	m_TransformToCArm = GetBaseParameters()->m_WorldToCArm*GetBaseParameters()->m_TransformInWorldCS;
}

void Base2D3DRegistration::PostApply(const q::Matrix44 &_mat, qu32 _nbProcUsed){
#ifdef COMPUTE_RIGID_TRANSFORM_IN_CARM_SPACE
	Matrix44 matCArmToWorld = GetBaseParameters()->m_WorldToCArm.inverse();
	m_RigidTransform3DInWorldCS = matCArmToWorld*_mat*m_TransformToCArm;
#else
	m_RigidTransform3DInWorldCS = _mat*GetBaseParameters()->m_TransformInWorldCS;
#endif

	Q_PRINT_DEBUG("bestFittingMetric final %f\n", m_FittingScore);

	if(_nbProcUsed > 1){
		for(qu32 i = 0; i < _nbProcUsed; ++i){
			SAFE_DELETE(m_Pt3dTo2dThread[i]);
		}
		SAFE_DELETE(m_Pt3dTo2dThread);
	}
	else{
		SAFE_DELETE(m_Pt3dTo2d);
	}
	
	if(GetBaseParameters()->m_RadiusMetric != RADIUS_METRIC_FCOST1){
		SAFE_DELETE(m_3dVesselsRadius);
	}
	SAFE_DELETE(m_3dVesselsTmp);
	SAFE_DELETE(m_PrecomputedWeightCurve);
	SAFE_DELETE(m_2dCatheterTmp);
}

void Base2D3DRegistration::Init(void){
	m_Nb2dPts = 0;
	m_2dCatheterTmp = NULL;
	m_Nb3dPts = 0;
	m_3dVesselsTmp = NULL;
	m_3dVesselsRadius = NULL;
	m_Pt3dTo2dThread = NULL;
	m_Pt3dTo2d = NULL;
	m_FittingScore = UTIL_BIG_NEGATIVE_NUMBER;
	
	m_ComputeRigidTransformInSupposedTipSpace = Q_FALSE;
}

#include "DefineDistanceMetric.h"

q::Matrix44 Base2D3DRegistration::GetRigidTransform(q::qf64 _x, q::qf64 _y, q::qf64 _z
							, q::qf64 _alpha, q::qf64 _beta, q::qf64 _gamma){
	Matrix44 translation = translation.getIdentity();
	translation[0][3] = _x;
	translation[1][3] = _y;
	translation[2][3] = _z;
	if(m_ComputeRigidTransformInSupposedTipSpace == Q_FALSE){
#ifdef COMPUTE_RIGID_TRANSFORM_IN_CARM_SPACE
		return translation*GetBaseParameters()->m_IsoCenterToCArm*GetRotationAlphaBetaGamma(_alpha, _beta, _gamma)*m_CArmToIsoCenter;
#else
		return translation*GetRotationAlphaBetaGamma(_alpha, _beta, _gamma);
#endif
	}
#ifdef COMPUTE_RIGID_TRANSFORM_IN_CARM_SPACE
	return m_SupposedTipToCArm*translation*GetRotationAlphaBetaGamma(_alpha, _beta, _gamma)*m_CArmToSupposedTip;
#else
	return m_SupposedTipToWorld*translation*GetRotationAlphaBetaGamma(_alpha, _beta, _gamma)*m_WorldToSupposedTip;
#endif
}
END_Q_NAMESPACE
