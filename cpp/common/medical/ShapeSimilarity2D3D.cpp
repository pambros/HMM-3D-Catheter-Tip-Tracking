#include "ShapeSimilarity2D3D.h"
#include "common/util/UtilTime.h"

//#define INITIAL_STATE_PROBABILITIES_WITH_SHAPE_SIMILARITIES // CODE NOT MAINTAINED ANYMORE
//#define INITIAL_STATE_PROBABILITIES_WITH_REGISTRATION // CODE NOT MAINTAINED ANYMORE

using namespace std;

BEGIN_Q_NAMESPACE

void ShapeSimilarity2D3D::Init(const q::qString &_paramFileName){
	m_ShapeSimilarity = Q_NEW ShapeSimilarity(_paramFileName);
}

ShapeSimilarity2D3D::ShapeSimilarity2D3D(OPTIMIZER_ENUM _optimizer, const q::qString &_paramFileName, q::Vessels *_vessels, const q::qString &_info3DRAFileName)
	: Object2D3D(_optimizer, _paramFileName, _vessels, _info3DRAFileName){
	ShapeSimilarity2D3D::Init(_paramFileName);
}

ShapeSimilarity2D3D::ShapeSimilarity2D3D(OPTIMIZER_ENUM _optimizer, const qString &_paramFileName, const qString &_vesselsFileName, const qString &_info3DRAFileName)
	: Object2D3D(_optimizer, _paramFileName, _vesselsFileName, _info3DRAFileName){
	ShapeSimilarity2D3D::Init(_paramFileName);
}

ShapeSimilarity2D3D::~ShapeSimilarity2D3D(void){
	SAFE_DELETE_UNIQUE(m_ShapeSimilarity);
}

int ShapeSimilarity2D3D::Do2D3DRegistration(PtList &_catheter, InfoFluoro &_infoFluoro){
	PtList vesselsPtl;

	m_ShapeSimilarity->m_ScoresList.clear();

	// transform catheter and vessels points set in mm
	_catheter.Transform(_infoFluoro.m_FluoroPixelToMM);
	
	m_Base2D3DRegistration->GetBaseParameters()->m_WorldToCArm = _infoFluoro.m_WorldToCArm;
	m_Base2D3DRegistration->GetBaseParameters()->m_CArmProjection = _infoFluoro.m_CArmProjection;
	m_Base2D3DRegistration->GetBaseParameters()->m_IsoCenterToCArm = _infoFluoro.m_IsoCenterToCArm;
	m_Base2D3DRegistration->GetBaseParameters()->m_TransformInWorldCS = m_PreviousTransformInWorldCS;

	//START_CHRONO(do2D3DRegistrationChrono);

	m_ShapeSimilarity->m_Parameters->m_WorldToCArm = _infoFluoro.m_WorldToCArm;
	//_shapeSimilarity->m_Parameters->m_WorldToCArm = _infoFluoro.m_WorldToCArm*previousTransformInWorldCS;
	m_ShapeSimilarity->m_Parameters->m_CArmProjection = _infoFluoro.m_CArmProjection;
	/*if(inPreviousFusionResultList.empty() == Q_FALSE){
		_shapeSimilarity->m_Parameters->m_ShapeSimilarityTree = inPreviousFusionResultList[0].m_ShapeSimilarityTree;
	}
	else {*/
		for(qu32 i = 0; i < m_Vessels->m_CompleteVesselList.size(); ++i){
			m_ShapeSimilarity->m_Parameters->m_ShapeSimilarityTree.push_back(0.0);
		}
	//}
	m_ShapeSimilarity->Apply(_catheter, *m_Vessels);
	//qPrint("_shapeSimilarity->m_ScoresList.size() %d\n", _shapeSimilarity->m_ScoresList.size());
		
#ifdef INITIAL_STATE_PROBABILITIES_WITH_REGISTRATION
	for(std::vector<qf64>::iterator it = m_ShapeSimilarity->m_Parameters->m_ShapeSimilarityTree.begin(); it != m_ShapeSimilarity->m_Parameters->m_ShapeSimilarityTree.end(); ++it){
		(*it) = UTIL_BIG_POSITIVE_NUMBER;
	}
#endif


	/*std::map<qu32, qf64> shapeSimilarityVesselIdList;
	for(q::ShapeSimilarity::ScoresList::iterator it = m_ShapeSimilarity->m_ScoresList.begin(); it != m_ShapeSimilarity->m_ScoresList.end(); ++it){
		shapeSimilarityVesselIdList.insert(pair<qu32, qf64>(it->second.tipVesselId, it->first));
	}*/

	//ShapeSimilarity::VesselId bestVesselId;
	//Matrix44 bestMatRigidInWorldCS;
	//qu32 bestId = 0;
	//qf64 bestScore = UTIL_BIG_NEGATIVE_NUMBER;
	qu32 currentId = 0;
	multimap<qf64, FusionResult::Unit> results;
	for(q::ShapeSimilarity::ScoresList::reverse_iterator it = m_ShapeSimilarity->m_ScoresList.rbegin(); it != m_ShapeSimilarity->m_ScoresList.rend(); ++it){
		vesselsPtl.clear();
		m_Vessels->Get3dVesselsBranches(it->second.leafVesselId, Q_TRUE, vesselsPtl);
		m_Base2D3DRegistration->Apply(_catheter, vesselsPtl);
		//qPrint("it->second.leafVesselId %d _base2D3DRegistration->m_FittingScore %f\n", it->second.leafVesselId, _base2D3DRegistration->m_FittingScore);

		FusionResult::Unit unit;
		unit.m_RigidTransform3DInWorldCS = m_Base2D3DRegistration->m_RigidTransform3DInWorldCS;
		unit.m_TipVesselId = it->second.tipVesselId;
		unit.m_LeafVesselId = it->second.leafVesselId;
		unit.m_SelectedVesselRank = currentId;

		PtList vessels2DPtl = PtList(vesselsPtl);
		vessels2DPtl.Transform(m_Base2D3DRegistration->m_RigidTransform3DInWorldCS);
		vessels2DPtl.Transform(_infoFluoro.m_WorldToCArm);
		vessels2DPtl.Transform(_infoFluoro.m_CArmProjection, Q_TRUE);
		qsize_t id = vessels2DPtl.GetClosestPtId(_catheter[0]);
		Vessel *closestVessel = m_Vessels->GetClosestVesselFrom(Vector3(vesselsPtl[id].pos[0], vesselsPtl[id].pos[1], vesselsPtl[id].pos[2]));
		//unit.m_FusionTipVesselId = vessels->GetIdFromVesselPointer(closestVessel);
		unit.m_FusionTipVesselId = m_Vessels->GetVesselId(closestVessel);

		unit.m_FusionMetric = m_Base2D3DRegistration->m_FittingScore;
		unit.m_ShapeSimilarityMetric = it->first;
		results.insert(std::pair<qf64,FusionResult::Unit>(m_Base2D3DRegistration->m_FittingScore, unit));

		//qPrint("m_SelectedVesselRank %d, m_FittingScore %f m_ShapeSimilarityMetric %f\n", currentId, m_Base2D3DRegistration->m_FittingScore, it->first);
		//qf64 score = m_Base2D3DRegistration->m_FittingScore;
		//if(score > bestScore){
			//bestVesselId = it->second;
			//bestMatRigidInWorldCS = m_Base2D3DRegistration->m_RigidTransform3DInWorldCS;
			//bestId = currentId;
			//bestScore = score;
		//}

#ifdef INITIAL_STATE_PROBABILITIES_WITH_REGISTRATION
		m_ShapeSimilarity->m_Parameters->m_ShapeSimilarityTree[unit.m_FusionTipVesselId] = -m_Base2D3DRegistration->m_FittingScore;
#endif

		currentId++;
		if(currentId >= m_ShapeSimilarity->m_Parameters->m_NbRankPathToRegister_k){
			break;
		}
	}

	//END_CHRONO(do2D3DRegistrationChrono);
	//PRINT_CHRONO(do2D3DRegistrationChrono);

	FusionResult::Unit &unit = (*results.rbegin()).second;

	// check consistency of the best matrix
	//if(IsZeroMatrix(bestMatRigidInWorldCS) == Q_TRUE){
	if(IsZeroMatrix(unit.m_RigidTransform3DInWorldCS) == Q_TRUE){
		qPrintStdErr("Error the registration method didn't work properly. The transformation matrix is set to identity.\n");
		unit.m_RigidTransform3DInWorldCS = Matrix44::getIdentity();
		unit.m_TipVesselId = 0;
		unit.m_LeafVesselId = 0;
		//bestVesselId.asU64 = 0;
	}

	// save the result of the registration
	m_FusionResult = FusionResult();
//#ifdef USE_CHRONO
	//m_FusionResult.m_RegistrationTime = do2D3DRegistrationChrono.GetTime();
//#endif

	qu32 resultId = 0;
	for(multimap<qf64, FusionResult::Unit>::reverse_iterator it = results.rbegin(); it != results.rend(); ++it){
		m_FusionResult.m_RigidTransform3DInWorldCS[resultId] = it->second.m_RigidTransform3DInWorldCS;
		m_FusionResult.m_TipVesselId[resultId] = it->second.m_TipVesselId;
		m_FusionResult.m_LeafVesselId[resultId] = it->second.m_LeafVesselId;
		m_FusionResult.m_SelectedVesselRank[resultId] = it->second.m_SelectedVesselRank;
		m_FusionResult.m_FusionTipVesselId[resultId] = it->second.m_FusionTipVesselId;
		m_FusionResult.m_FusionMetric[resultId] = it->second.m_FusionMetric;
		//qPrint("it->second.m_FusionMetric %f\n", it->second.m_FusionMetric);
		m_FusionResult.m_ShapeSimilarityMetric[resultId] = it->second.m_ShapeSimilarityMetric;
		//qPrint("it->second.m_ShapeSimilarityMetric %f\n", it->second.m_ShapeSimilarityMetric);

#ifdef INITIAL_STATE_PROBABILITIES_WITH_SHAPE_SIMILARITIES
		//qf64 beta = 0.3333;
		//shapeSimilarity->m_Parameters->m_ShapeSimilarityTree[it->second.m_FusionTipVesselId] = (1.0 - beta)*shapeSimilarity->m_Parameters->m_ShapeSimilarityTree[it->second.m_FusionTipVesselId] + beta;
#endif

		resultId++;
		if(resultId >= FusionResult::NB_SAVED_FUSION_RESULT){
			break;
		}
	}

	for(qu32 i = 0; i < m_Vessels->GetNbVessel(); ++i){
		m_FusionResult.m_NormalizedAlphaTree.push_back(ALPHA_MIN_PROBA);
		m_FusionResult.m_DeltaTree.push_back(MIN_PROBA);
#ifdef PRINT_ALL_DEBUG
		m_FusionResult.m_TreeBeginning.push_back(0.);
		m_FusionResult.m_TreeAfterDistribution.push_back(0.);
		m_FusionResult.m_TreeAfterRedistribution.push_back(0.);
#endif
	}
	m_FusionResult.m_NormalizedAlphaTree[m_FusionResult.m_FusionTipVesselId[0]] = ALPHA_MAX_PROBA;
	m_FusionResult.m_DeltaTree[m_FusionResult.m_FusionTipVesselId[0]] = MAX_PROBA;

#ifdef INITIAL_STATE_PROBABILITIES_WITH_REGISTRATION
	// CODE NOT MAINTAINED ANYMORE
	m_FusionResult.m_NormalizedAlphaTree = m_ShapeSimilarity->m_Parameters->m_ShapeSimilarityTree;

	// normalize
	qf64 sumDotProduct = ALPHA_MIN_PROBA;
	for(std::vector<qf64>::iterator it = m_FusionResult.m_NormalizedAlphaTree.begin(); it != m_FusionResult.m_NormalizedAlphaTree.end(); ++it){
#ifdef USE_ALPHA_LOG_PROBA
		if((*it) < UTIL_BIG_POSITIVE_NUMBER){
			(*it) = UTIL_GAUSSIAN_2(*it, _base2D3DRegistration->GetBaseParameters()->m_SigmaS);
			(*it) = specialLog((*it));
			sumDotProduct = logaddexp(sumDotProduct, (*it));
		}
		else{
			(*it) = MIN_LOG_NUMBER;
		}
#else
		(*it) = UTIL_GAUSSIAN_2(*it, m_Base2D3DRegistration->GetBaseParameters()->m_SigmaS);
		sumDotProduct = sumDotProduct + (*it);
#endif
	}
	for(std::vector<qf64>::iterator it = m_FusionResult.m_NormalizedAlphaTree.begin(); it != m_FusionResult.m_NormalizedAlphaTree.end(); ++it){
#ifdef USE_ALPHA_LOG_PROBA
		if((*it) > MIN_LOG_NUMBER)
		{
			(*it) = (*it) - sumDotProduct;
		}
#else
		(*it) = (*it)/sumDotProduct;
#endif
#ifdef USE_LOG_PROBA
		m_FusionResult.m_DeltaTree.push_back(specialLog((*it)));
#else
		m_FusionResult.m_DeltaTree.push_back((*it));
#endif
	}
#endif
#ifdef INITIAL_STATE_PROBABILITIES_WITH_SHAPE_SIMILARITIES
	// CODE NOT MAINTAINED ANYMORE
	_fusionResult.m_NormalizedAlphaTree = shapeSimilarity->m_Parameters->m_ShapeSimilarityTree;
	/*for(std::map<qu32, qf64>::iterator it = shapeSimilarityVesselIdList.begin(); it != shapeSimilarityVesselIdList.end(); ++it){
		fusionResult.m_NormalizedAlphaTree.push_back(it->second);
	}*/

	// normalize
	qf64 sumDotProduct = 0.;
	const qf64 DOT_PRODUCT_MINIMUM = 0.5;
	for(std::vector<qf64>::iterator it = _fusionResult.m_NormalizedAlphaTree.begin(); it != _fusionResult.m_NormalizedAlphaTree.end(); ++it){
		if((*it) < DOT_PRODUCT_MINIMUM){
			(*it) = 0.0;
		}
		//(*it) = (*it) + 1.0;
		sumDotProduct = sumDotProduct + (*it);
	}
	for(std::vector<qf64>::iterator it = _fusionResult.m_NormalizedAlphaTree.begin(); it != _fusionResult.m_NormalizedAlphaTree.end(); ++it){
		(*it) = *(it)/sumDotProduct;
	}
#endif
		
	// debug check
	/*qf64 sumCheck = 0.;
	for(std::vector<qf64>::iterator it = _fusionResult.m_NormalizedAlphaTree.begin(); it != _fusionResult.m_NormalizedAlphaTree.end(); ++it){
		sumCheck = sumCheck + (*it);
	}
	qPrint("sumCheck %f\n", sumCheck);*/

	return EXIT_SUCCESS;
}

END_Q_NAMESPACE