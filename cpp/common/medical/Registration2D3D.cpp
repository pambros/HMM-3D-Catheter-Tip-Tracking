#include "Registration2D3D.h"
#include "common/medical/HMM2D3D.h"
#include "common/medical/ShapeSimilarity2D3D.h"

using namespace std;

BEGIN_Q_NAMESPACE

void Registration2D3D::InitRegistration2D3D(const qString &_confileFile){
	m_Parameters = Q_NEW Parameters(_confileFile);

	if(m_Parameters->m_Method == METHOD_SHAPE_SIMILARITY){
		m_Object2D3DRegistration = static_cast<Object2D3D*>(Q_NEW ShapeSimilarity2D3D(static_cast<OPTIMIZER_ENUM>(m_Parameters->m_Optimizer),
			_confileFile, m_DataList->m_Vessel3draFileName, m_DataList->m_Info3draFileName));
	}
	// initialize 2D/3D registration using the HMM
	else{
		HMM2D3D *hmm2D3DRegistration = Q_NEW HMM2D3D(static_cast<OPTIMIZER_ENUM>(m_Parameters->m_Optimizer), _confileFile, m_DataList->m_Vessel3draFileName
			, m_DataList->m_Info3draFileName);
		m_Object2D3DRegistration = static_cast<Object2D3D*>(hmm2D3DRegistration);

		// initialize the HMM
		FusionResult fusionResult;
		if(m_DataList->m_InitializeFromTipPosition == Q_TRUE){
			fusionResult.m_RigidTransform3DInWorldCS[0] = Matrix44::getIdentity();
			fusionResult.m_FusionTipVesselId[0] = m_DataList->m_TipPositionId;
			fusionResult.m_TipVesselId[0] = m_DataList->m_TipPositionId;
			fusionResult.m_LeafVesselId[0] = m_DataList->m_TipPositionId; // hmm2D3DRegistration->m_Vessels->GetTheFirstLeafIdCloseTo(fusionResult.m_TipVesselId[0]); // just take random leaf after the tip
			for(qu32 i = 0; i < hmm2D3DRegistration->m_Vessels->GetNbVessel(); ++i){
				fusionResult.m_NormalizedAlphaTree.push_back(ALPHA_MIN_PROBA);
				fusionResult.m_DeltaTree.push_back(MIN_PROBA);
#ifdef PRINT_ALL_DEBUG
				fusionResult.m_TreeBeginning.push_back(0.);
				fusionResult.m_TreeAfterDistribution.push_back(0.);
				fusionResult.m_TreeAfterRedistribution.push_back(0.);
#endif
			}
			fusionResult.m_NormalizedAlphaTree[fusionResult.m_FusionTipVesselId[0]] = ALPHA_MAX_PROBA;
			fusionResult.m_DeltaTree[fusionResult.m_FusionTipVesselId[0]] = MAX_PROBA;
		}
		else{
			fusionResult = FusionResult(m_DataList->m_FirstFusion.c_str());
		}

		hmm2D3DRegistration->m_PreviousFusionResultList.push_back(fusionResult);

//#define USE_PREVIOUS_TRANSFORMATION
#ifdef USE_PREVIOUS_TRANSFORMATION
		hmm2D3DRegistration->m_PreviousTransformInWorldCS = hmm2D3DRegistration->m_PreviousFusionResultList[0].m_RigidTransform3DInWorldCS[0];
#endif

#ifdef PRINT_ALL_DEBUG
		q::PrintMatrix(hmm2D3DRegistration->m_PreviousTransformInWorldCS);
#endif
	}
}

Registration2D3D::Registration2D3D(const qString &_confileFile, const qString &_vessel3DFilename, const qString &_3DRAinfoFilename, qu32 _tipPosition) : m_Parameters(NULL)
			, m_Object2D3DRegistration(NULL){
	m_DataList = Q_NEW DataList(_vessel3DFilename, _3DRAinfoFilename, _tipPosition);
	InitRegistration2D3D(_confileFile);
}

Registration2D3D::Registration2D3D(const qString &_confileFile, const qString &_dataFolder) : m_Parameters(NULL)
			, m_Object2D3DRegistration(NULL)
			, m_CurrentFrame(0){
	m_DataList = Q_NEW DataList(_dataFolder);
	InitRegistration2D3D(_confileFile);
}

Registration2D3D::~Registration2D3D(){
	//qPrint("Registration2D3D::~Registration2D3D\n");
	SAFE_DELETE_UNIQUE(m_Object2D3DRegistration);
	SAFE_DELETE_UNIQUE(m_Parameters);
	SAFE_DELETE_UNIQUE(m_DataList);
}

// be careful _catheter is modified
void Registration2D3D::Register(PtList &_catheter, const InfoFluoro &_infoFluoro){
	qs32 ret = m_Object2D3DRegistration->Do2D3DRegistration(_catheter, _infoFluoro);
}

void Registration2D3D::Register(void){
	qAssert(m_DataList->m_OnlineRegistration == Q_FALSE);
	InfoFluoro infoFluoro(m_DataList->m_InfoFluoroFileName.c_str());
	PtList catheter(m_DataList->m_CatheterCenterlineList[m_CurrentFrame]);
	if(catheter.size() == 0){
		qPrintStdErr("ERROR catheter centerline file %s is empty\n", m_DataList->m_CatheterCenterlineList[m_CurrentFrame].c_str());
		exit(EXIT_FAILURE);
	}

	Register(catheter, infoFluoro);
}

END_Q_NAMESPACE
