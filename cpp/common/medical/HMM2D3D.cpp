#include "HMM2D3D.h"
#include "common/util/UtilTime.h"
#include "common/util/Thread.h"

using namespace std;

//#define __DEBUG_PRINT 1
#ifdef __DEBUG_PRINT
	#define Q_PRINT_DEBUG(...) q::qPrint(__VA_ARGS__)
#else
	#define Q_PRINT_DEBUG(...)
#endif

#ifdef Q_USE_THREAD
	#define REGISTRATION_USE_THREAD
	#define REGISTRATION_NB_USED_PROCS (7)
#endif

// In the paper alpha function is not mentioned, it is like delta function but with sum instead of max
// alpha_t = sum_j(alpha_{t-1}(j)*aji)*O_t(i)
// alphaVariable = sum_j(alpha_{t-1}(j)*aji). Here alpha_{t} is normalized at the end.
// TODO not sure the normalization implementation in log scale is correct (when USE_ALPHA_LOG_PROBA is defined)

// In the paper we have delta function
// delta_t = max_j(delta_{t-1}(j)*aji)*O_t(i)
// In the paper maxForwardVariable = max_j(delta_{t-1}*aji). It is not normalized.
// In the code we also propose sumForwardVariable = sum_j(delta_{t-1}(j)*aji). It is not normalized.

// If SORT_WITH_ALPHA_VARIABLE is defined, the observation score is only computed with the points with the best sum_j(alpha_{t-1}(j)*aji) i.e. alphaVariable
// If not, the observation score is only computed with the points with the best max_j(delta_{t-1}*aji) i.e. maxForwardVariable
#define SORT_WITH_ALPHA_VARIABLE

// If SORT_WITH_SUM_DELTA is defined, the observation score is only computed with the points with the best sum_j(delta_{t-1}(j)*aji) i.e. sumForwardVariable
// If not, the observation score is only computed with the points with the best max_j(delta_{t-1}*aji) i.e. maxForwardVariable
//#define SORT_WITH_SUM_DELTA

// Normalize also maxForwardVariable at the end
// TODO not sure the normalization implementation in log scale is correct (when USE_LOG_PROBA is defined)
//#define DELTA_NORMALIZED

BEGIN_Q_NAMESPACE

HMM2D3D::Parameters::Parameters(const q::qString &_paramFileName){
	qFile *file = NULL;
	try{
		qu32 err = qFOpen(file, _paramFileName.c_str(), "rb");
		if(err == 0){
			throw gDefaultException;
		}

		/*qu32 iErr =*/ FileReachLine(file, qString("#\tHMM2D3DParameters"));
		/*iErr =*/ FileReadU32(file, m_Plane);
		/*iErr =*/ FileReadU32(file, m_NumberStateEvaluatedNo);
		/*iErr =*/ FileReadF64(file, m_SigmaS);
		/*iErr =*/ FileReadU32(file, m_TransitionMetric);
		/*iErr =*/ FileReadF64(file, m_SigmaA);
		/*iErr =*/ FileReadF64(file, m_Theta);
		/*iErr =*/ FileReadF64(file, m_ProbaMinTransition);
	}
	catch(q::qDefaultException){
		q::qPrint("HMM2D3D::Parameters::Parameters Error during loading\n");
		qFClose(file);
		throw gDefaultException;
	}

	qFClose(file);
}

HMM2D3D::HMM2D3D(OPTIMIZER_ENUM _optimizer, const q::qString &_paramFileName, q::Vessels *_vessels
			, q::VesselsStateTransitionProbabilities *_vstp, const q::qString &_info3DRAFileName)
			: Object2D3D(_optimizer, _paramFileName, _vessels, _info3DRAFileName)
			, m_Parameters(_paramFileName){
	m_Vstp = _vstp;
}

HMM2D3D::HMM2D3D(OPTIMIZER_ENUM _optimizer, const q::qString &_paramFileName, const qString &_vesselsFileName
	, q::VesselsStateTransitionProbabilities *_vstp, const q::qString &_info3DRAFileName)
	: Object2D3D(_optimizer, _paramFileName, _vesselsFileName, _info3DRAFileName)
	, m_Parameters(_paramFileName){
	m_Vstp = _vstp;
}

HMM2D3D::HMM2D3D(OPTIMIZER_ENUM _optimizer, const q::qString &_paramFileName, const qString &_vesselsFileName
	, const q::qString &_info3DRAFileName)
	: Object2D3D(_optimizer, _paramFileName, _vesselsFileName, _info3DRAFileName)
	, m_Parameters(_paramFileName){

	qf64 sigma = m_Parameters.m_SigmaA;
	if(m_Parameters.m_TransitionMetric == VesselsStateTransitionProbabilities::TRANSITION_METRIC_BINARY_A_DOUBLE_PRIME){
		sigma = m_Parameters.m_Theta;
	}
	m_Vstp = Q_NEW VesselsStateTransitionProbabilities(m_Vessels, m_Parameters.m_TransitionMetric, sigma, m_Parameters.m_ProbaMinTransition);
}

HMM2D3D::HMM2D3D(OPTIMIZER_ENUM _optimizer, const qString &_paramFileName, const qString &_vesselsFileName
												   , const qString &_inVesselsStateTransitionFileName, const qString &_info3DRAFileName)
												   : Object2D3D(_optimizer, _paramFileName, _vesselsFileName, _info3DRAFileName)
												   , m_Parameters(_paramFileName){
	m_Vstp = Q_NEW VesselsStateTransitionProbabilities(_inVesselsStateTransitionFileName);
}

HMM2D3D::~HMM2D3D(void){
	SAFE_DELETE_UNIQUE(m_Vstp);
}

int HMM2D3D::Do2D3DRegistration(PtList &_catheter, InfoFluoro &_infoFluoro){
	PtList vesselsPtl;
	PtList &catheter = _catheter;

	// transform catheter and vessels points set in mm
	catheter.Transform(_infoFluoro.m_FluoroPixelToMM);
	
	m_Base2D3DRegistration->GetBaseParameters()->m_WorldToCArm = _infoFluoro.m_WorldToCArm;
	m_Base2D3DRegistration->GetBaseParameters()->m_CArmProjection = _infoFluoro.m_CArmProjection;
	m_Base2D3DRegistration->GetBaseParameters()->m_IsoCenterToCArm = _infoFluoro.m_IsoCenterToCArm;
	m_Base2D3DRegistration->GetBaseParameters()->m_TransformInWorldCS = m_PreviousTransformInWorldCS;

	m_FusionResult = FusionResult();

	//START_CHRONO(doHMM2D3DRegistrationChrono);
		
#ifdef PRINT_ALL_DEBUG
	m_FusionResult.m_TreeBeginning = m_PreviousFusionResultList[0].m_NormalizedAlphaTree;
#endif
	// compute the forward variable delta as well as alpha for each state with the current image
	VesselsStateTransitionProbabilities::VesselDistributionList::iterator itDist = m_Vstp->m_CompleteVesselListFromState.begin();
	multimap<qf64, Vessel*> sortedAlpha;
	for(Vessel::VesselList::iterator it = m_Vessels->m_CompleteVesselList.begin(); it != m_Vessels->m_CompleteVesselList.end(); ++it, ++itDist){
		Vessel *v = (*it);
		VesselsStateTransitionProbabilities::VesselDistribution *vesselDistribution = (*itDist);

		qf64 alphaVariable = ALPHA_MIN_PROBA;
		qf64 maxForwardVariable = MIN_PROBA;
#ifdef SORT_WITH_SUM_DELTA
		qf64 sumForwardVariable = MIN_PROBA;
#endif
		for(VesselsStateTransitionProbabilities::TransitionList::iterator transIt = vesselDistribution->m_TransitionList.begin();
			transIt != vesselDistribution->m_TransitionList.end(); ++transIt){
#ifdef USE_ALPHA_LOG_PROBA
			if(m_PreviousFusionResultList[0].m_NormalizedAlphaTree[(*transIt).m_TreeId] > MIN_LOG_NUMBER){
				alphaVariable = logaddexp(alphaVariable, m_PreviousFusionResultList[0].m_NormalizedAlphaTree[(*transIt).m_TreeId] + (*transIt).m_GaussianProba);
			}
#else
			alphaVariable = alphaVariable + m_PreviousFusionResultList[0].m_NormalizedAlphaTree[(*transIt).m_TreeId]*(*transIt).m_GaussianProba;
#endif
#ifdef USE_LOG_PROBA
			qf64 forwardVariable = MIN_LOG_NUMBER;
			if(m_PreviousFusionResultList[0].m_DeltaTree[(*transIt).m_TreeId] > MIN_LOG_NUMBER){
				forwardVariable = m_PreviousFusionResultList[0].m_DeltaTree[(*transIt).m_TreeId] + (*transIt).m_LogGaussianProba;
			}
#ifdef SORT_WITH_SUM_DELTA
			sumForwardVariable = logaddexp(sumForwardVariable, forwardVariable);
#endif
#else
			qf64 forwardVariable = m_PreviousFusionResultList[0].m_DeltaTree[(*transIt).m_TreeId]*(*transIt).m_GaussianProba;
#ifdef SORT_WITH_SUM_DELTA
			sumForwardVariable = sumForwardVariable + forwardVariable;
#endif
#endif

			//qPrint("(*transIt).m_LogGaussianProba %f (*transIt).m_GaussianProba %f\n", (*transIt).m_LogGaussianProba, (*transIt).m_GaussianProba);
			if(forwardVariable > maxForwardVariable){
				maxForwardVariable = forwardVariable;
			}
		}
#ifdef SORT_WITH_ALPHA_VARIABLE
		if(alphaVariable > ALPHA_MIN_PROBA)
		{
			sortedAlpha.insert(pair<qf64, Vessel*>(alphaVariable, v));
		}
#else
#ifdef SORT_WITH_SUM_DELTA
	#ifdef USE_LOG_PROBA
		if(sumForwardVariable > MIN_LOG_NUMBER)
	#else
		//if(sumForwardVariable > 0.)
	#endif
		{
			sortedAlpha.insert(pair<qf64, Vessel*>(sumForwardVariable, v));
		}
#else
	#ifdef USE_LOG_PROBA
		if(maxForwardVariable > MIN_LOG_NUMBER)
	#else
		//if(maxForwardVariable > 0.)
	#endif
		{
			sortedAlpha.insert(pair<qf64, Vessel*>(maxForwardVariable, v));
		}
#endif
#endif
		v->m_Data[Vessel::INFO_MISC_3] = alphaVariable;
		v->m_Data[Vessel::INFO_MISC_2] = maxForwardVariable;
	}
		
#ifdef PRINT_ALL_DEBUG
	// todo warning: sum is equal to 1 only if the A matrix is symmetric
	qf64 checkSum = 0.;
	for(multimap<qf64, Vessel*>::iterator it = sortedAlpha.begin(); it != sortedAlpha.end(); ++it){
		checkSum = checkSum + it->first;
	}
	qPrint("checkSum %f\n", checkSum);
#endif
		
#ifdef PRINT_ALL_DEBUG
	m_Vessels->GetVesselsValue(Vessel::INFO_MISC_3, m_FusionResult.m_TreeAfterDistribution);
	m_Vessels->GetVesselsValue(Vessel::INFO_MISC_2, m_FusionResult.m_TreeAfterRedistribution);
#endif
		
	// registration
	Matrix44 cArmToWorld = _infoFluoro.m_WorldToCArm.inverse();
	Vector4 realTipPos(catheter[0].pos[PT_X], catheter[0].pos[PT_Y], 0., 1.0);
	realTipPos = cArmToWorld*realTipPos;
	Vector4 focalPos(-_infoFluoro.m_CArmProjection[0][2], -_infoFluoro.m_CArmProjection[1][2], _infoFluoro.m_CArmProjection[0][0], 1.);
	focalPos = cArmToWorld*focalPos;

	Vector3 vecX(realTipPos[PT_X] - focalPos[PT_X], realTipPos[PT_Y] - focalPos[PT_Y], realTipPos[PT_Z] - focalPos[PT_Z]);
	vecX.normalize();
	Vector3 vecY = vecX.cross(Vector3(0., 1., 0.));
	vecY.normalize();
	Vector3 vecZ = vecX.cross(vecY);
	vecZ.normalize();
	Matrix44 rotationToTip;
	//rotationToTip[0][0] = vecX[0];rotationToTip[0][1] = vecY[0];rotationToTip[0][2] = vecZ[0];rotationToTip[0][3] = 0.;
	//rotationToTip[1][0] = vecX[1];rotationToTip[1][1] = vecY[1];rotationToTip[1][2] = vecZ[1];rotationToTip[1][3] = 0.;
	//rotationToTip[2][0] = vecX[2];rotationToTip[2][1] = vecY[2];rotationToTip[2][2] = vecZ[2];rotationToTip[2][3] = 0.;
	//rotationToTip[3][0] = 0.;     rotationToTip[3][1] = 0.;     rotationToTip[3][2] = 0.;     rotationToTip[3][3] = 1.;
	rotationToTip[0][0] = vecX[0];rotationToTip[0][1] = vecX[1];rotationToTip[0][2] = vecX[2];rotationToTip[0][3] = 0.;
	rotationToTip[1][0] = vecY[0];rotationToTip[1][1] = vecY[1];rotationToTip[1][2] = vecY[2];rotationToTip[1][3] = 0.;
	rotationToTip[2][0] = vecZ[0];rotationToTip[2][1] = vecZ[1];rotationToTip[2][2] = vecZ[2];rotationToTip[2][3] = 0.;
	rotationToTip[3][0] = 0.;     rotationToTip[3][1] = 0.;     rotationToTip[3][2] = 0.;     rotationToTip[3][3] = 1.;
	//rotationToTip = rotationToTip.inverse();
		
	Vector3 normalProjectionPlane;
	//if(m_Parameters.m_Plane == HMM2D3D::PLANE_PROJECTION)
	{
		Vector4 centerImagePos(_infoFluoro.m_IsoCenterToCArm[0][3], _infoFluoro.m_IsoCenterToCArm[1][3], 0., 1.0);
		centerImagePos = cArmToWorld*centerImagePos;
		normalProjectionPlane = Vector3(centerImagePos[PT_X] - focalPos[PT_X], centerImagePos[PT_Y] - focalPos[PT_Y], centerImagePos[PT_Z] - focalPos[PT_Z]);
		normalProjectionPlane.normalize();
	}

	m_Vessels->TagIdInData(Vessel::INFO_MISC_6);
	qf64 maxScore = UTIL_BIG_NEGATIVE_NUMBER;

	m_FusionResult.m_DeltaTree.resize(m_Vessels->GetNbVessel(), MIN_PROBA);
	m_FusionResult.m_NormalizedAlphaTree.resize(m_Vessels->GetNbVessel(), ALPHA_MIN_PROBA);
	//m_Vessels->SetAllData(Vessel::INFO_MISC_5, 0.);

#ifdef REGISTRATION_USE_THREAD
	class RegistrationThread : public qThread{
	public:
		inline RegistrationThread(qu32 _id
								, const multimap<qf64, Vessel*> &_sortedAlpha
								, const Matrix44 &_previousTransformInWorldCS
								, const Base2D3DRegistration * const _base2D3DRegistration
								, const HMM2D3D::Parameters &_parameters
								, const q::Vector4 &_focalPos
								, const q::Vector4 &_realTipPos
								, const q::Vector3 &_normalProjectionPlane
								, const Matrix44 &_rotationToTip
								, FusionResult &_fusionResult
								, const Vessels * const _vessels
								, const q::PtList &_catheter) : m_Id(_id)
									, sortedAlpha(_sortedAlpha)
									, m_PreviousTransformInWorldCS(_previousTransformInWorldCS)
									, m_Base2D3DRegistration(_base2D3DRegistration)
									, m_Parameters(_parameters)
									, focalPos(_focalPos)
									, realTipPos(_realTipPos)
									, normalProjectionPlane(_normalProjectionPlane)
									, rotationToTip(_rotationToTip)
									, m_FusionResult(_fusionResult)
									, m_Vessels(_vessels)
									, catheter(_catheter)
									, maxScore(UTIL_BIG_NEGATIVE_NUMBER){
		}
		inline ~RegistrationThread(void){
		}
			
		qf64 maxScore;
		Matrix44 m_RigidTransform3DInWorldCS;
		qu32 m_FusionTipVesselId;

	private:
		const multimap<qf64, Vessel*> &sortedAlpha;
		const Matrix44 &m_PreviousTransformInWorldCS;
		const Base2D3DRegistration * const m_Base2D3DRegistration;
		const HMM2D3D::Parameters &m_Parameters;
		const q::Vector4 &focalPos;
		const q::Vector4 &realTipPos;
		const q::Vector3 &normalProjectionPlane;
		const Matrix44 &rotationToTip;
		FusionResult &m_FusionResult;
		const Vessels * const m_Vessels;
		const q::PtList &catheter;

		qu32 m_Id;

	private:
		inline void run(void){
			Q_PRINT_DEBUG("RegistrationThread::run %d\n", m_Id);

			Base2D3DRegistration *base2D3DRegistrationTmp = m_Base2D3DRegistration->Copy();
#else
	Base2D3DRegistration *base2D3DRegistrationTmp = m_Base2D3DRegistration;
#endif

	qu32 nbRegistration = 0;
	//map<qu32, Matrix44> registrationResultList;
	for(multimap<qf64, Vessel*>::const_reverse_iterator it = sortedAlpha.rbegin(); it != sortedAlpha.rend(); ++it){	
#ifdef REGISTRATION_USE_THREAD
		if(nbRegistration%REGISTRATION_NB_USED_PROCS != m_Id){
			++nbRegistration;
			if(nbRegistration >= m_Parameters.m_NumberStateEvaluatedNo){
				break;
			}
			continue;
		}
#endif
		Vessel *v = it->second;
		
		Vector4 supposedTipPos(v->m_Data[Vessel::INFO_X], v->m_Data[Vessel::INFO_Y], v->m_Data[Vessel::INFO_Z], 1.);
		supposedTipPos = m_PreviousTransformInWorldCS*supposedTipPos;

		Vector4 intersectTipPos;
		Matrix44 translationToTip = Matrix44::getIdentity();
		if(m_Parameters.m_Plane == HMM2D3D::PLANE_TABLE){
			// We assume that the tip stay on the same z-plane. The ray from the focal position to the 2D catheter tip should intersect the plane z = supposedTipPos.z
			// focalPos.z + t*(realTipPos.z - focalPos.z) = supposedTipPos.z
			// -> t = (supposedTipPos.z - focalPos.z)/(realTipPos.z - focalPos.z)
			qf64 t = (supposedTipPos[PT_Z] - focalPos[PT_Z])/(realTipPos[PT_Z] - focalPos[PT_Z]);
			intersectTipPos = Vector4(focalPos[PT_X] + t*(realTipPos[PT_X] - focalPos[PT_X]), focalPos[PT_Y] + t*(realTipPos[PT_Y] - focalPos[PT_Y]), supposedTipPos[PT_Z], 1.0);
			translationToTip[0][3] = (intersectTipPos[PT_X] - supposedTipPos[PT_X]);
			translationToTip[1][3] = (intersectTipPos[PT_Y] - supposedTipPos[PT_Y]);
		}
		else{
			// the parallel plane equation of the projection image has the normal equal to the normal of projection plane normalProjectionPlane = (a, b, c)
			// we find the equation plane ax + by + cz + d = 0
			qf64 a = normalProjectionPlane[PT_X];
			qf64 b = normalProjectionPlane[PT_Y];
			qf64 c = normalProjectionPlane[PT_Z];
			qf64 d = -a*supposedTipPos[PT_X] - b*supposedTipPos[PT_Y] - c*supposedTipPos[PT_Z];
			// the ray from the focal to the realTipPos (focalPos + t*(realTipPos - focalPos)) should intersect the plane ax + by + cz + d = 0
			// we replace x by focalPos.x + t*(realTipPos.x - focalPos.x), y by ... and we obtain t = ...
			qf64 t = (-d - a*focalPos[PT_X] - b*focalPos[PT_Y] - c*focalPos[PT_Z])/(a*(realTipPos[PT_X] - focalPos[PT_X]) + b*(realTipPos[PT_Y] - focalPos[PT_Y]) + c*(realTipPos[PT_Z] - focalPos[PT_Z]));
			intersectTipPos = Vector4(focalPos[PT_X] + t*(realTipPos[PT_X] - focalPos[PT_X])
									, focalPos[PT_Y] + t*(realTipPos[PT_Y] - focalPos[PT_Y])
									, focalPos[PT_Z] + t*(realTipPos[PT_Z] - focalPos[PT_Z])
									, 1.0);
			translationToTip[0][3] = (intersectTipPos[PT_X] - supposedTipPos[PT_X]);
			translationToTip[1][3] = (intersectTipPos[PT_Y] - supposedTipPos[PT_Y]);
			translationToTip[2][3] = (intersectTipPos[PT_Z] - supposedTipPos[PT_Z]);
		}

			
#ifdef __DEBUG_PRINT
		Matrix44 translationToTip2 = translationToTip;
#endif

		// optimization to fit with the catheter
		base2D3DRegistrationTmp->m_useContinuousMethod = Q_TRUE;
//#define BREATHING_ON_THE_TABLE_PLANE
#ifdef BREATHING_ON_THE_TABLE_PLANE
		base2D3DRegistrationTmp->GetBaseParameters()->m_TransformInWorldCS = m_PreviousTransformInWorldCS*translationToTip;
#else
		base2D3DRegistrationTmp->GetBaseParameters()->m_TransformInWorldCS = translationToTip*m_PreviousTransformInWorldCS;
#endif
		Matrix44 matWorldToIntersect = Matrix44::getIdentity();
		matWorldToIntersect[0][3] = -intersectTipPos[PT_X];
		matWorldToIntersect[1][3] = -intersectTipPos[PT_Y];
		matWorldToIntersect[2][3] = -intersectTipPos[PT_Z];
			
		base2D3DRegistrationTmp->m_WorldToSupposedTip = rotationToTip*matWorldToIntersect;

		//vesselsPtl.clear();
		PtList vesselsPtl;
		m_Vessels->Get3dVesselsBranchesConst(static_cast<qu32>(v->m_Data[Vessel::INFO_MISC_6]), Q_FALSE, vesselsPtl);
		base2D3DRegistrationTmp->Apply(catheter, vesselsPtl);

		qf64 score = UTIL_GAUSSIAN_2(base2D3DRegistrationTmp->m_FittingScore, m_Parameters.m_SigmaS);
		score = MAX(score, UTIL_MICRO_EPSILON);
		//Q_PRINT_DEBUG("translationToTip2 [%f,%f] id %f alpha %.*f maxforwarddelta %.*f m_FittingScore %.*f score %.*f "
		Q_PRINT_DEBUG("translationToTip2 [%f,%f] id %f alpha %.12f maxforwarddelta %.12f m_FittingScore %.12f score %.12f "
			, translationToTip2[0][3], translationToTip2[1][3], v->m_Data[Vessel::INFO_MISC_6], v->m_Data[Vessel::INFO_MISC_3], v->m_Data[Vessel::INFO_MISC_2]
			, base2D3DRegistrationTmp->m_FittingScore, score);
#ifdef USE_LOG_PROBA
		m_FusionResult.m_DeltaTree[static_cast<qu32>(v->m_Data[Vessel::INFO_MISC_6])] = v->m_Data[Vessel::INFO_MISC_2] + specialLog(score);
#else
		m_FusionResult.m_DeltaTree[static_cast<qu32>(v->m_Data[Vessel::INFO_MISC_6])] = v->m_Data[Vessel::INFO_MISC_2]*score;
#endif
#ifdef USE_ALPHA_LOG_PROBA
		m_FusionResult.m_NormalizedAlphaTree[static_cast<qu32>(v->m_Data[Vessel::INFO_MISC_6])] = v->m_Data[Vessel::INFO_MISC_3] + specialLog(score);
#else
		m_FusionResult.m_NormalizedAlphaTree[static_cast<qu32>(v->m_Data[Vessel::INFO_MISC_6])] = v->m_Data[Vessel::INFO_MISC_3]*score;
#endif
		Q_PRINT_DEBUG("alphaXscore %.12f deltaXscore %.12f \n", m_FusionResult.m_NormalizedAlphaTree[static_cast<qu32>(v->m_Data[Vessel::INFO_MISC_6])],
			m_FusionResult.m_DeltaTree[static_cast<qu32>(v->m_Data[Vessel::INFO_MISC_6])]);

		//registrationResultList.insert(pair<qu32, Matrix44>(static_cast<qu32>(v->m_Data[Vessel::INFO_MISC_6]), base2D3DRegistrationTmp->m_RigidTransform3DInWorldCS));
		//registrationResultList.insert(pair<qu32, Matrix44>(static_cast<qu32>(v->m_Data[Vessel::INFO_MISC_6]), translationToTip2*m_PreviousTransformInWorldCS));
		if(m_FusionResult.m_DeltaTree[static_cast<qu32>(v->m_Data[Vessel::INFO_MISC_6])] > maxScore){
			maxScore = m_FusionResult.m_DeltaTree[static_cast<qu32>(v->m_Data[Vessel::INFO_MISC_6])];
#ifdef REGISTRATION_USE_THREAD
			m_RigidTransform3DInWorldCS = base2D3DRegistrationTmp->m_RigidTransform3DInWorldCS;
			//m_RigidTransform3DInWorldCS = translationToTip2*m_PreviousTransformInWorldCS;
			m_FusionTipVesselId = static_cast<qu32>(v->m_Data[Vessel::INFO_MISC_6]);
#else
			m_FusionResult.m_RigidTransform3DInWorldCS[0] = base2D3DRegistrationTmp->m_RigidTransform3DInWorldCS;
			//m_FusionResult.m_RigidTransform3DInWorldCS[0] = translationToTip2*m_PreviousTransformInWorldCS;
			m_FusionResult.m_FusionTipVesselId[0] = static_cast<qu32>(v->m_Data[Vessel::INFO_MISC_6]);
#endif
		}

		++nbRegistration;
		if(nbRegistration >= m_Parameters.m_NumberStateEvaluatedNo){
			break;
		}
	}

#ifdef REGISTRATION_USE_THREAD
			SAFE_DELETE_UNIQUE(base2D3DRegistrationTmp);
		}
	};

	RegistrationThread **thread = Q_NEW RegistrationThread*[REGISTRATION_NB_USED_PROCS];
	for(qu32 i = 0; i < REGISTRATION_NB_USED_PROCS; ++i){
		thread[i] = Q_NEW RegistrationThread(i, sortedAlpha, m_PreviousTransformInWorldCS, m_Base2D3DRegistration, m_Parameters, focalPos
											, realTipPos, normalProjectionPlane, rotationToTip, m_FusionResult, m_Vessels, catheter);
		thread[i]->start();
	}
		
	// wait for the work
	for(qu32 i = 0; i < REGISTRATION_NB_USED_PROCS; ++i){
		thread[i]->wait();
		if(thread[i]->maxScore > maxScore){
			maxScore = thread[i]->maxScore;
			m_FusionResult.m_RigidTransform3DInWorldCS[0] = thread[i]->m_RigidTransform3DInWorldCS;
			m_FusionResult.m_FusionTipVesselId[0] = thread[i]->m_FusionTipVesselId;
		}
	}

	for(qu32 i = 0; i < REGISTRATION_NB_USED_PROCS; ++i){
		SAFE_DELETE_UNIQUE(thread[i]);
	}
	SAFE_DELETE(thread);
#endif

	// normalization of the alpha
	qf64 sum = ALPHA_MIN_PROBA;
#ifdef USE_ALPHA_LOG_PROBA
	for(vector<qf64>::iterator it = mFusionResult.m_NormalizedAlphaTree.begin(); it != m_FusionResult.m_NormalizedAlphaTree.end(); ++it){
		if((*it) > MIN_LOG_NUMBER)
		{
			sum = logaddexp(sum, (*it));
		}
	}
	for(vector<qf64>::iterator it = m_FusionResult.m_NormalizedAlphaTree.begin(); it != m_FusionResult.m_NormalizedAlphaTree.end(); ++it){
		if((*it) > MIN_LOG_NUMBER)
		{
			(*it) = (*it) - sum;
		}
	}
#else
	for(vector<qf64>::iterator it = m_FusionResult.m_NormalizedAlphaTree.begin(); it != m_FusionResult.m_NormalizedAlphaTree.end(); ++it){
		sum = sum + (*it);
	}
	for(vector<qf64>::iterator it = m_FusionResult.m_NormalizedAlphaTree.begin(); it != m_FusionResult.m_NormalizedAlphaTree.end(); ++it){
		(*it) = (*it)/sum;
	}
#endif

#ifdef DELTA_NORMALIZED
	// normalization of the delta
	sum = MIN_PROBA;
#ifdef USE_LOG_PROBA
	for(vector<qf64>::iterator it = m_FusionResult.m_DeltaTree.begin(); it != m_FusionResult.m_DeltaTree.end(); ++it){
		if((*it) > MIN_LOG_NUMBER)
		{
			sum = logaddexp(sum, (*it));
		}
	}
	for(vector<qf64>::iterator it = m_FusionResult.m_DeltaTree.begin(); it != m_FusionResult.m_DeltaTree.end(); ++it){
		if((*it) > MIN_LOG_NUMBER)
		{
			(*it) = (*it) - sum;
		}
	}
#else
	for(vector<qf64>::iterator it = m_FusionResult.m_DeltaTree.begin(); it != m_FusionResult.m_DeltaTree.end(); ++it){
		sum = sum + (*it);
	}
	for(vector<qf64>::iterator it = m_FusionResult.m_DeltaTree.begin(); it != m_FusionResult.m_DeltaTree.end(); ++it){
		(*it) = (*it)/sum;
	}
#endif
#endif

	//END_CHRONO(doHMM2D3DRegistrationChrono);
	//PRINT_CHRONO(doHMM2D3DRegistrationChrono);
		
//#ifdef USE_CHRONO
	//m_FusionResult.m_RegistrationTime = doHMM2D3DDRegistrationChrono.GetTime();
//#endif

	// prepare for the next frame
	m_PreviousFusionResultList.clear();
	m_PreviousFusionResultList.push_back(m_FusionResult);
	
	return EXIT_SUCCESS;
}

END_Q_NAMESPACE