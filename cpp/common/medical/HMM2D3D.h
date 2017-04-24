#ifndef __HMM_2D_3D_HEADER_
#define __HMM_2D_3D_HEADER_
#include "common/medical/Object2D3D.h"

BEGIN_Q_NAMESPACE

class HMM2D3D : public Object2D3D{
public:
	enum PLANE_ENUM{
		PLANE_TABLE = 0,
		PLANE_PROJECTION = 1,
		PLANE_COUNT
	};

	struct Parameters{
		qu32 m_Plane; // PLANE_ENUM
		qu32 m_NumberStateEvaluatedNo;
		qf64 m_SigmaS;
		qu32 m_TransitionMetric; // TRANSITION_METRIC_ENUM
		qf64 m_SigmaA;
		qf64 m_Theta;
		qf64 m_ProbaMinTransition;

		Parameters(void){
			m_Plane = PLANE_TABLE;
			m_NumberStateEvaluatedNo = 49;
			m_SigmaS = 1.5;
			m_TransitionMetric = VesselsStateTransitionProbabilities::TRANSITION_METRIC_GAUSSIAN_A_PRIME;
			m_SigmaA = 12.;
			m_Theta = 12.;
			m_ProbaMinTransition = 0.0001;
		}

		Parameters(const q::qString &_paramFileName);
	};

	HMM2D3D(OPTIMIZER_ENUM _optimizer, const q::qString &_paramFileName, q::Vessels *_vessels
		, q::VesselsStateTransitionProbabilities *_vstp, const q::qString &_info3DRAFileName);
	HMM2D3D(OPTIMIZER_ENUM _optimizer, const q::qString &_paramFileName, const q::qString &_vesselsFileName
		, q::VesselsStateTransitionProbabilities *_vstp, const q::qString &_info3DRAFileName);
	HMM2D3D(OPTIMIZER_ENUM _optimizer, const q::qString &_paramFileName, const q::qString &_vesselsFileName
		, const q::qString &_info3DRAFileName);
	HMM2D3D(OPTIMIZER_ENUM _optimizer, const q::qString &_paramFileName, const q::qString &_vesselsFileName
			, const q::qString &_inVesselsStateTransitionFileName, const q::qString &_info3DRAFileName);
	~HMM2D3D(void);
	int Do2D3DRegistration(q::PtList &_catheter, q::InfoFluoro &_infoFluoro);

public:
	Parameters m_Parameters;

	q::VesselsStateTransitionProbabilities *m_Vstp;
	std::vector<q::FusionResult> m_PreviousFusionResultList;
};

END_Q_NAMESPACE

#endif
