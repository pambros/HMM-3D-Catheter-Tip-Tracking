#include "ShapeSimilarity.h"
#include "common/util/UtilTime.h"
#include "common/util/File.h"

//#define __DEBUG_PRINT 1
#ifdef __DEBUG_PRINT
	#define Q_PRINT_DEBUG(...) q::qPrint(__VA_ARGS__)
#else
	#define Q_PRINT_DEBUG(...)
#endif

using namespace std;

BEGIN_Q_NAMESPACE

// in [0, 1] if less than 10% of the catheter has been used in the score, we discard this score
const q::qf64 ShapeSimilarity::DEFAULT_RATIO_NB_PT_CATHETER = 10./100.;
const q::qu32 ShapeSimilarity::DEFAULT_NB_RANK_PATH_TO_REGISTER = 6;

ShapeSimilarity::ShapeSimilarity(Parameters *_parameters) : m_Parameters(_parameters){
	Init();
}

ShapeSimilarity::ShapeSimilarity(const qString &_fileName){
	Init();

	m_Parameters = Q_NEW Parameters();

	qFile *file = NULL;
	try{
		qu32 err = qFOpen(file, _fileName.c_str(), "rb");
		if(err == 0){
			throw gDefaultException;
		}

		/*u32 iErr =*/ FileReachLine(file, qString("#\tShapeSimilarityParameterFile"));
		/*iErr =*/ //FileReadMatrix44(file, m_Parameters->m_CArmProjection);
		/*iErr =*/ //FileReadMatrix44(file, m_Parameters->m_WorldToCArm);
		/*iErr =*/ FileReadF64(file, m_Parameters->m_RatioNbPtCatheter);
		/*iErr =*/ FileReadU32(file, m_Parameters->m_NbRankPathToRegister_k);
	}
	catch(q::qDefaultException){
		q::qPrint("ShapeSimilarity::ShapeSimilarity Error during loading\n");
		qFClose(file);
		throw gDefaultException;
	}

	qFClose(file);
}

qf64 ShapeSimilarity::GetBestPathRecursively(Vessel *_v, qu32 &_inc, ScoresList &_scores, map<qu32, qf64> &_partScores, qu32 &_part){
	_v->m_Flag = Q_TRUE;
	_v->m_Data[Vessel::INFO_MISC_1] = _inc;
//#define USE_DEBUG_BREAKPOINT 1
#ifdef USE_DEBUG_BREAKPOINT
	if(_inc == 182){
		qbool debug = Q_TRUE;
	}
#endif

	qu32 currentPart = _part;
	qu32 partInc = 0;
	if(_v->m_VesselList.size() > 2){
		partInc = 1;
	}
	
	// only for breakpoint in the leaves
	//if(_v->m_VesselList.size() == 1){
	//	currentPart = _part;
	//}

	// compute the score of the current 3d vessel path shape compared to the 2d catheter shape
	qf64 currentDist = 0;
	qu32 currentPt2d = 0;
	Vessel *vessel = _v;
	qf64 score = 0;
	qu32 nbDotProduct = 0;
	do{
		// get the interpolated position of the _2dCatheter following the distance
		Vector2 dir2d = m_Direction2dCatheter[currentPt2d];
		if(m_CumulDistance2dCatheter[currentPt2d] > currentDist){
			qf64 delta = (currentDist - m_CumulDistance2dCatheter[currentPt2d - 1])/(m_CumulDistance2dCatheter[currentPt2d] - m_CumulDistance2dCatheter[currentPt2d - 1]);
			dir2d = (1.0 - delta)*m_Direction2dCatheter[currentPt2d - 1] + delta*m_Direction2dCatheter[currentPt2d];
			dir2d.normalize();
		}

		// compute the dot product between the direction of the current position in the 3d projection and the 2d catheter
		Vector2 dirProj3d = Vector2(vessel->m_Data[Vessel::INFO_MISC_4], vessel->m_Data[Vessel::INFO_MISC_5]);
		score += dir2d.dot(dirProj3d);
		nbDotProduct++;
	
#ifdef USE_DEBUG_BREAKPOINT
	if(_inc == 182){
		Q_PRINT_DEBUG("m_Direction2dCatheter[%u]\n", currentPt2d);
		PrintVector2(m_Direction2dCatheter[currentPt2d]);
		Q_PRINT_DEBUG("dir2d\n");
		PrintVector2(dir2d);
		Q_PRINT_DEBUG("dirProj3d %f\n", vessel->m_Data[Vessel::INFO_MISC_1]);
		PrintVector2(dirProj3d);
		Q_PRINT_DEBUG("score %f\n", score/static_cast<qf64>(nbDotProduct));
	}
#endif
		
		// continue the path with the previous vessel
		qu32 min = 0xFFFFFFFF;
		Vessel *vesselMin = NULL;
		for(Vessel::VesselList::iterator it = vessel->m_VesselList.begin(); it != vessel->m_VesselList.end(); ++it){
			Vessel *tmp = (*it);
			if(tmp->m_Data[Vessel::INFO_MISC_1] < min){
				min = static_cast<qu32>(tmp->m_Data[Vessel::INFO_MISC_1]);
				vesselMin = tmp;
			}
		}

		if(vesselMin != NULL){
			Vector2 posProj3d = Vector2(vessel->m_Data[Vessel::INFO_MISC_2], vessel->m_Data[Vessel::INFO_MISC_3]);
			Vector2 newPosProj3d = Vector2(vesselMin->m_Data[Vessel::INFO_MISC_2], vesselMin->m_Data[Vessel::INFO_MISC_3]);
			currentDist += GetDistance(posProj3d, newPosProj3d);

			while(currentPt2d < m_Nb2dPts && m_CumulDistance2dCatheter[currentPt2d] < currentDist){
				currentPt2d++;
			}
		}
		vessel = vesselMin;
	}while(vessel != NULL && currentPt2d < m_Nb2dPts);

	// if less than m_RatioNbPtCatheter*100 % of the catheter has been used in the score, we discard this score
	if(currentPt2d < m_Nb2dPts*m_Parameters->m_RatioNbPtCatheter){
		score = 0;
	}
	else{
		score = score/static_cast<qf64>(nbDotProduct);
	}

	// add in the tree for temporal knowledge
	/*qf64 coefCatheterSize = MIN(currentPt2d, 100);
	qf64 alpha = 0.333*(coefCatheterSize/100.0);
	score = (1.0 - alpha)*m_Parameters->m_ShapeSimilarityTree[_inc] + alpha*score;*/
	m_Parameters->m_ShapeSimilarityTree[_inc] = score;

	// compute the other path recursively and keep every score in a sorted list
	qf64 maxBranchesScore = UTIL_BIG_NEGATIVE_NUMBER; //0;
	for(Vessel::VesselList::iterator it = _v->m_VesselList.begin(); it != _v->m_VesselList.end(); ++it){
		Vessel *tmp = (*it);
		if(tmp->m_Flag == Q_FALSE){
			_inc++;
			_part = _part + partInc;
			qf64 tmpScore = GetBestPathRecursively(tmp, _inc, _scores, _partScores, _part);
			if(tmpScore > maxBranchesScore){
				maxBranchesScore = tmpScore;
			}
		}
	}

	if(score > maxBranchesScore){
		//qPrint("\t\t_partScores.size() %d\n", _partScores.size());

		VesselId vesselId = {static_cast<qu16>(_v->m_Data[Vessel::INFO_MISC_1])
							, static_cast<qu16>(currentPart)
							, static_cast<qu16>(_v->m_Data[Vessel::INFO_MISC_1])
							, static_cast<qu16>(currentPart)};

		map<qu32, qf64>::iterator it = _partScores.find(currentPart);
		// if it's the first score in the part, look at the leaves
		if(it == _partScores.end()){
			// take the first part child of the currentPart found in the scores (thanks to map::lower_bound)
			it = _partScores.lower_bound(currentPart + 1);
		}

		if(it != _partScores.end()){
			// get the score and find it in the multimap _scores
			std::pair<ScoresList::iterator, ScoresList::iterator> itPair = _scores.equal_range(it->second);
			ScoresList::iterator it2 = itPair.first;
			qAssert(it2 != _scores.end());
			if(it2 != _scores.end()){ // TODO this line is useless?
				for(; it2 != itPair.second; ++it2){
					if(it2->second.tipPartId == it->first){
						vesselId.leafVesselId = it2->second.leafVesselId;
						vesselId.leafPartId = it2->second.leafPartId;
						_scores.erase(it2);
						//qPrint("_scores.erase it2->second.tipPartId %d\n", it2->second.tipPartId);
						break;
					}
				}
			}

			_partScores.erase(it);
		}
		//else {
		//	qPrint("_scores.insert vesselId.tipPartId %d\n", vesselId.tipPartId);
		//}

		//qPrint("_scores.insert vesselId.tipPartId %d\n", vesselId.tipPartId);
		_partScores[currentPart] = score;
		_scores.insert(std::pair<qf64, VesselId>(score, vesselId));
		maxBranchesScore = score;
	}

	return maxBranchesScore;
}

void ShapeSimilarity::GetBestPath(Vessels &_3dVessels, ScoresList &_scores){
	_3dVessels.ResetAllFlags();
	_3dVessels.SetAllData(Vessel::INFO_MISC_1, UTIL_BIG_POSITIVE_NUMBER);
	qu32 inc = 0;
	map<qu32, qf64> partScores;
	qu32 part = 0;
	GetBestPathRecursively(_3dVessels.GetRoot(), inc, _scores, partScores, part);
}

void ShapeSimilarity::ComputeVesselDirectionRecursively(Vessel *_vessel, Vessel *_previousVessel){
	if(_vessel->m_Flag == Q_FALSE){
		_vessel->m_Flag = Q_TRUE;

		q::Vector2 dir = q::Vector2(0.0, 0.0);
		q::Vector2 vec = q::Vector2(_vessel->m_Data[Vessel::INFO_MISC_2], _vessel->m_Data[Vessel::INFO_MISC_3]);

		if(_previousVessel != NULL){
			Vector2 tmp = vec - q::Vector2(_previousVessel->m_Data[Vessel::INFO_MISC_2]
										, _previousVessel->m_Data[Vessel::INFO_MISC_3]);
			tmp.normalize();
			dir = dir + tmp;
		}
		
		for(Vessel::VesselList::iterator it = _vessel->m_VesselList.begin(); it != _vessel->m_VesselList.end(); ++it){
			if((*it) != _previousVessel){
				Vector2 tmp = q::Vector2((*it)->m_Data[Vessel::INFO_MISC_2]
									, (*it)->m_Data[Vessel::INFO_MISC_3])
							- vec;
				tmp.normalize();
				dir = dir + tmp;
			}
		}

		dir.normalize();
		_vessel->m_Data[Vessel::INFO_MISC_4] = dir.x;
		_vessel->m_Data[Vessel::INFO_MISC_5] = dir.y;
		
		for(Vessel::VesselList::iterator it = _vessel->m_VesselList.begin(); it != _vessel->m_VesselList.end(); ++it){
			ComputeVesselDirectionRecursively(*it, _vessel);
		}
	}
}

void ShapeSimilarity::ComputeVesselsDirection(Vessels &_3dVessels){
	_3dVessels.ResetAllFlags();
	ComputeVesselDirectionRecursively(_3dVessels.GetRoot(), NULL);
}

void ShapeSimilarity::Apply(const PtList &_2dCatheter, Vessels &_3dVessels){
	//START_CHRONO(shapeSimilarityChrono);

	Q_PRINT_DEBUG("ShapeSimilarity::Apply\n");
	m_Nb2dPts = _2dCatheter.size();

	// sort each path of the vessels following his curve

	// compute the direction for each point of the 2d curve
	m_Direction2dCatheter = Q_NEW Vector2[m_Nb2dPts];
	for(qsize_t i = 0; i < m_Nb2dPts; ++i){
		Vector2 tmp = Vector2(0.0, 0.0);
		if(i != 0){
			tmp = Vector2(_2dCatheter[i - 1].pos[PT_X] - _2dCatheter[i].pos[PT_X], _2dCatheter[i - 1].pos[PT_Y] - _2dCatheter[i].pos[PT_Y]);
			tmp.normalize();
		}
		if(i < m_Nb2dPts - 1){
			Vector2 tmp2 = Vector2(_2dCatheter[i].pos[PT_X] - _2dCatheter[i + 1].pos[PT_X], _2dCatheter[i].pos[PT_Y] - _2dCatheter[i + 1].pos[PT_Y]);
			tmp2.normalize();
			tmp += tmp2;
		}
		tmp.normalize();
		m_Direction2dCatheter[i] = tmp;
		Q_PRINT_DEBUG("m_Direction2dCatheter[%u] = %f %f\n", i, tmp[0], tmp[1]);
	}

	// compute the accumulated distance from the tips for each point of the 2d curve
	m_CumulDistance2dCatheter = Q_NEW qf64[m_Nb2dPts];
	m_CumulDistance2dCatheter[0] = 0.0;
	for(qsize_t i = 1; i < m_Nb2dPts; ++i){
		m_CumulDistance2dCatheter[i] = m_CumulDistance2dCatheter[i - 1] + GetDistance(_2dCatheter[i].pos, _2dCatheter[i - 1].pos);
	}

	// compute the 2d projection of each point of the 3d blood vessel curve
	Matrix44 mat = m_Parameters->m_CArmProjection*m_Parameters->m_WorldToCArm;
	for(Vessel::VesselList::iterator it = _3dVessels.m_CompleteVesselList.begin(); it != _3dVessels.m_CompleteVesselList.end(); ++it){
		Vessel *v = (*it);
		Vector4 pos = Vector4(v->m_Data[Vessel::INFO_X], v->m_Data[Vessel::INFO_Y], v->m_Data[Vessel::INFO_Z], 1.0);
		pos = mat*pos;
		v->m_Data[Vessel::INFO_MISC_2] = pos[0]/pos[3];
		v->m_Data[Vessel::INFO_MISC_3] = pos[1]/pos[3];
	}

	ComputeVesselsDirection(_3dVessels);

	GetBestPath(_3dVessels, m_ScoresList);

#ifdef __DEBUG_PRINT
	Q_PRINT_DEBUG("MultiParameter2D3DRegistration::Apply scores\n");
	for(ScoresList::reverse_iterator it = scores.rbegin(); it != scores.rend(); ++it){
		Q_PRINT_DEBUG("scores %f leafPartId %d leafVesselId %d tipPartId %d tipVesselId %d\n", it->first
			, it->second.leafPartId, it->second.leafVesselId, it->second.tipPartId, it->second.tipVesselId);
	}
#endif

	SAFE_DELETE(m_CumulDistance2dCatheter);
	SAFE_DELETE(m_Direction2dCatheter);

	//END_CHRONO(shapeSimilarityChrono);
	//PRINT_CHRONO(shapeSimilarityChrono);
}

void ShapeSimilarity::Init(void){
	m_Nb2dPts = 0;
	m_Direction2dCatheter = NULL;
	m_CumulDistance2dCatheter = NULL;
}

END_Q_NAMESPACE
