#include "Vessels.h"
#include "common/maths/Matrix.h"
#include "common/util/File.h"
#ifdef NEED_RESAMPLE
	#include "thirdParties/bigr/ResampleAndSmoothXMarkerlist.h"
#endif
#include "common/maths/Maths.h"

BEGIN_Q_NAMESPACE
	void Vessel::UnlinkAll(void){
		for(Vessel::VesselList::iterator it = m_VesselList.begin(); it != m_VesselList.end(); ++it){
			(*it)->Unlink(this);
		}
		m_VesselList.clear();
	}

	void Vessel::Unlink(Vessel *_v){
		Vessel::VesselList::iterator it = std::find(m_VesselList.begin(), m_VesselList.end(), _v);
		if(it != m_VesselList.end()){
			m_VesselList.erase(it);
		}
	}

	Vessels::Vessels(Vessel *_vessel) : m_Root(_vessel){
		//q::qPrint("Vessels\n");
		qAssert(_vessel != NULL);
		m_CompleteVesselList.push_back(_vessel);
	}

	Vessels::Vessels(Vessels *_vessels) : m_Root(NULL){
		qAssert(_vessels != NULL);
		DestroyAndCopyFrom(_vessels);
	}

	Vessels::Vessels(const qString &_fileName) : m_Root(NULL){
		qFile *file = NULL;
		qString buffer;

		try{
			qu32 err = qFOpen(file, _fileName.c_str(), "rb");
			if(err == 0){
				throw gDefaultException;
			}

			// get nb of vessel
			//int iErr = fscanf_s(file, "%s\n", buffer);
			qu32 iErr = FileReadLine(file, buffer);
			if(iErr != 1){
				throw gDefaultException;
			}
			qu32 nbVessel = atoi(buffer.c_str());

			// get the root id
			//iErr = fscanf_s(file, "%s\n", buffer);
			iErr = FileReadLine(file, buffer);
			if(iErr != 1){
				throw gDefaultException;
			}
			qu32 rootId = atoi(buffer.c_str());

			for(qu32 i = 0; i < nbVessel; ++i){
				Vessel *vessel = Q_NEW Vessel(0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0);
				m_CompleteVesselList.push_back(vessel);
			}
			m_Root = m_CompleteVesselList[rootId];

			for(Vessel::VesselList::iterator it = m_CompleteVesselList.begin(); it != m_CompleteVesselList.end(); ++it){
				Vessel *vessel = (*it);

				//iErr = fscanf_s(file, "%s\n", buffer);
				iErr = FileReadLine(file, buffer);
				if(iErr != 1){
					throw gDefaultException;
				}

				std::vector<qString> vecList;
				SplitString(buffer, ";", vecList);
				for(qu32 i = 0; i < vecList.size(); ++i){
					vessel->m_Data[i] = atof(vecList[i].c_str());
				}
				
				//iErr = fscanf_s(file, "%s\n", buffer);
				iErr = FileReadLine(file, buffer);
				if(iErr != 1){
					throw gDefaultException;
				}
				
				std::vector<qString> vecList2;
				SplitString(buffer, ";", vecList2);
				for(std::vector<qString>::iterator it2 = vecList2.begin(); it2 != vecList2.end(); ++it2){
					qString idStr = *it2;
					vessel->m_VesselList.push_back(m_CompleteVesselList[atoi(idStr.c_str())]);
				}
			}
		}
		catch(q::qDefaultException){
			q::qPrint("Vessels::Vessels Error during loading\n");
			DestroyAll();
			qFClose(file);
			throw gDefaultException;
		}
		
		qFClose(file);

		// we don't compute segment type if at least one point doesn't have '0' value in the data Vessel::INFO_MISC_6
		qbool computePartType = Q_TRUE;
		for(Vessel::VesselList::iterator it = m_CompleteVesselList.begin(); it != m_CompleteVesselList.end(); ++it){
			Vessel *vessel = (*it);
			if(vessel->m_Data[Vessel::INFO_MISC_6] != 0.){
				computePartType = Q_FALSE;
				break;
			}
		}
		if(computePartType == Q_TRUE){
			ComputePartType();
		}
	}

	Vessels::Vessels(const PtList &_ptList){
		AddPart(VESSEL_NO_ID, _ptList);
	}

	Vessels::~Vessels(void){
		//q::qPrint("~Vessels\n");
		DestroyAll();
	}
	
	void Vessels::Save(const qString &_fileName){
		qFile *file = NULL;
		char buffer[MAX_STR_BUFFER];

		try{
			qu32 err = qFOpen(file, _fileName.c_str(), "wb");
			if(err == 0){
				throw gDefaultException;
			}

			// nb of vessel and root
			Vessel *root = GetRoot();
			Vessel::VesselList::iterator itRoot = std::find(m_CompleteVesselList.begin(), m_CompleteVesselList.end(), root);
			qu32 id = static_cast<qu32>(itRoot - m_CompleteVesselList.begin());

			int iErr = file->Fprintf("%d\n%d\n", m_CompleteVesselList.size(), id);
			if(iErr < 0){
				throw gDefaultException;
			}

			for(Vessel::VesselList::iterator it = m_CompleteVesselList.begin(); it != m_CompleteVesselList.end(); ++it){
				qString str;
				Vessel *vessel = (*it);

				for(qu32 i = 0; i < Vessel::INFO_COUNT; ++i){
					qSprintf(buffer, MAX_STR_BUFFER, "%f;", vessel->m_Data[i]);
					str += qString(buffer);
				}
				str += "\n";

				for(Vessel::VesselList::iterator it2 = vessel->m_VesselList.begin(); it2 != vessel->m_VesselList.end(); ++it2){
					Vessel *vessel2 = (*it2);
					Vessel::VesselList::iterator it3 = std::find(m_CompleteVesselList.begin(), m_CompleteVesselList.end(), vessel2);
					id = static_cast<qu32>(it3 - m_CompleteVesselList.begin());
					
					qSprintf(buffer, MAX_STR_BUFFER, "%d;", id);
					str += qString(buffer);
				}
				str += "\n";
				
				iErr = file->Fprintf("%s", str.c_str());
				if(iErr < 0){
					throw gDefaultException;
				}
			}
		}
		catch(q::qDefaultException){
			q::qPrint("Vessels::Save Error during saving\n");
		}

		qFClose(file);
	}

	void Vessels::DestroyAndCopyFrom(Vessels *_vessels){
		DestroyAll();

		for(Vessel::VesselList::iterator it = _vessels->m_CompleteVesselList.begin(); it != _vessels->m_CompleteVesselList.end(); ++it){
			Vessel *vessel = (*it);
			Vessel *newVessel = Q_NEW Vessel(vessel);
			qAssert(newVessel != 0);
			m_CompleteVesselList.push_back(newVessel);
		}
		
		qu32 i = 0;
		for(Vessel::VesselList::iterator it = _vessels->m_CompleteVesselList.begin(); it != _vessels->m_CompleteVesselList.end(); ++it){
			Vessel *vessel = (*it);
			for(Vessel::VesselList::iterator it2 = vessel->m_VesselList.begin(); it2 != vessel->m_VesselList.end(); ++it2){
				Vessel *vessel2 = (*it2);
				Vessel::VesselList::iterator it3 = std::find(_vessels->m_CompleteVesselList.begin(), _vessels->m_CompleteVesselList.end(), vessel2);
				qu32 j = static_cast<qu32>(it3 - _vessels->m_CompleteVesselList.begin());
				m_CompleteVesselList[i]->m_VesselList.push_back(m_CompleteVesselList[j]);
			}
			++i;
		}

		// set root
		Vessel *root = _vessels->GetRoot();
		Vessel::VesselList::iterator it = std::find(_vessels->m_CompleteVesselList.begin(), _vessels->m_CompleteVesselList.end(), root);
		i = static_cast<qu32>(it - _vessels->m_CompleteVesselList.begin());
		m_Root = m_CompleteVesselList[i];
	}
	
	void Vessels::DestroyAll(void){
		for(Vessel::VesselList::iterator it = m_CompleteVesselList.begin(); it != m_CompleteVesselList.end(); ++it){
			SAFE_DELETE_UNIQUE((*it));
		}
		m_CompleteVesselList.clear();
	}

	void Vessels::ComputeVesselDirectionRecursively(Vessel *_vessel, Vessel *_previousVessel){
		if(_vessel->m_Flag == Q_FALSE){
			_vessel->m_Flag = Q_TRUE;

			q::Vector3 dir = q::Vector3(0.0, 0.0, 0.0);
			q::Vector3 vec = q::Vector3(_vessel->m_Data[Vessel::INFO_X], _vessel->m_Data[Vessel::INFO_Y], _vessel->m_Data[Vessel::INFO_Z]);

			if(_previousVessel != NULL){
				dir = dir + vec - q::Vector3(_previousVessel->m_Data[Vessel::INFO_X]
											, _previousVessel->m_Data[Vessel::INFO_Y]
											, _previousVessel->m_Data[Vessel::INFO_Z]);
			}
			
			for(Vessel::VesselList::iterator it = _vessel->m_VesselList.begin(); it != _vessel->m_VesselList.end(); ++it){
				if((*it) != _previousVessel){
					dir = dir + q::Vector3((*it)->m_Data[Vessel::INFO_X]
										, (*it)->m_Data[Vessel::INFO_Y]
										, (*it)->m_Data[Vessel::INFO_Z])
								- vec;
				}
			}

			dir.normalize();
			_vessel->m_Data[Vessel::INFO_U] = dir.x;
			_vessel->m_Data[Vessel::INFO_V] = dir.y;
			_vessel->m_Data[Vessel::INFO_W] = dir.z;

			// find a perpendicular vector
			// find the most perpendicular vector among (1,0,0) (0,1,0) and (0,0,1)
			const qu32 NB_UNIT_VEC = 3;
			q::Vector3 unitVec[NB_UNIT_VEC] = {q::Vector3(1.0, 0.0, 0.0)
											,q::Vector3(0.0, 1.0, 0.0)
											,q::Vector3(0.0, 0.0, 1.0)};
			qf64 minDotProduct = UTIL_BIG_POSITIVE_NUMBER;
			qu32 minIndex = 0;
			for(qu32 i = 0; i < NB_UNIT_VEC; ++i){
				qf64 dotProduct = ABS(dir.dot(unitVec[i]));
				if(dotProduct < minDotProduct){
					minDotProduct = dotProduct;
					minIndex = i;
				}
			}
			q::Vector3 crossVec = dir.cross(unitVec[minIndex]);
			//crossVec *= _vessel->m_Data[Vessel::INFO_RADIUS];
			_vessel->m_Data[Vessel::INFO_RADIUS_ARROW_X] = crossVec.x;
			_vessel->m_Data[Vessel::INFO_RADIUS_ARROW_Y] = crossVec.y;
			_vessel->m_Data[Vessel::INFO_RADIUS_ARROW_Z] = crossVec.z;
			
			for(Vessel::VesselList::iterator it = _vessel->m_VesselList.begin(); it != _vessel->m_VesselList.end(); ++it){
				ComputeVesselDirectionRecursively(*it, _vessel);
			}
		}
	}

	Vessel* Vessels::GetClosestVesselFrom(const Vector2 &_v){
		Vessel *closestVessel = NULL;
		qf64 distMin = UTIL_BIG_POSITIVE_NUMBER;
		for(Vessel::VesselList::iterator it = m_CompleteVesselList.begin(); it != m_CompleteVesselList.end(); ++it){
			Vessel *v = (*it);
			qf64 dist = GetSquareDistance(_v, Vector2(v->m_Data[Vessel::INFO_X], v->m_Data[Vessel::INFO_Y]));
			if(dist < distMin){
				closestVessel = v;
				distMin = dist;
			}
		}
		return closestVessel;
	}
	
	Vessel* Vessels::GetClosestVesselFrom(const Vector3 &_v){
		Vessel *closestVessel = NULL;
		qf64 distMin = UTIL_BIG_POSITIVE_NUMBER;
		for(Vessel::VesselList::iterator it = m_CompleteVesselList.begin(); it != m_CompleteVesselList.end(); ++it){
			Vessel *v = (*it);
			qf64 dist = GetSquareDistance(_v, Vector3(v->m_Data[Vessel::INFO_X], v->m_Data[Vessel::INFO_Y], v->m_Data[Vessel::INFO_Z]));
			if(dist < distMin){
				closestVessel = v;
				distMin = dist;
			}
		}
		return closestVessel;
	}

	q::qu32 Vessels::GetNbOfParts(void){
		ResetAllFlags();
		return GetNbOfPartsRecursively(GetRoot()) + 1;
	}

	q::qu32 Vessels::GetNbOfPartsRecursively(Vessel *_vessel){
		_vessel->m_Flag = Q_TRUE;

		q::qu32 inc = 0;
		if(_vessel->m_VesselList.size() > 2 || (_vessel == m_Root && _vessel->m_VesselList.size() > 1)){
			inc = 1;
		}

		q::qu32 nbParts = 0;
		for(Vessel::VesselList::iterator it = _vessel->m_VesselList.begin(); it != _vessel->m_VesselList.end(); ++it){
			if((*it)->m_Flag == Q_FALSE){
				nbParts = nbParts + inc + GetNbOfPartsRecursively((*it));
			}
		}

		return nbParts;
	}

	void Vessels::GetVesselPart(q::qu32 _selectedPartId, q::qs32 &_previousPartId, PtList &_ptList){
		Vessels *vessels = NULL;
		GetVesselPart(_selectedPartId, _previousPartId, vessels);
		vessels->GetPtList(_ptList);
		Q_DELETE_UNIQUE vessels;
	}

	void Vessels::GetVesselPart(q::qu32 _selectedPartId, q::qs32 &_previousPartId, Vessels* &_vessels){
		Vessel *previousVessel = NULL;
		Vessel *vessel = GetVesselPart(_selectedPartId, _previousPartId, previousVessel);
		Vessel *previousNewVessel = NULL;
		while(vessel != NULL){
			Vessel *newVessel = Q_NEW Vessel(vessel);
			if(_vessels == NULL){
				_vessels = Q_NEW Vessels(newVessel);
			}
			else{
				_vessels->AddVessel(newVessel, previousNewVessel);
			}
			previousNewVessel = newVessel;

			// if it's the root
			if(vessel == GetRoot()){
				if(vessel->m_VesselList.size() == 1){
					previousVessel = vessel;
					vessel = vessel->m_VesselList[0];
				}
				else if(vessel->m_VesselList.size() == 0 || vessel->m_VesselList.size() > 1){
					vessel = NULL;
				}
			}
			else{
				if(vessel->m_VesselList.size() == 2){
					q::qu32 nextId = 0;
					if(vessel->m_VesselList[0] == previousVessel){
						nextId = 1;
					}
					previousVessel = vessel;
					vessel = vessel->m_VesselList[nextId];
				}
				else if(vessel->m_VesselList.size() == 1 || vessel->m_VesselList.size() > 2){
					vessel = NULL;
				}
			}
		}
	}

	Vessel* Vessels::GetVesselPart(q::qu32 _selectedPartId, q::qs32 &_previousPartId, Vessel* &_previousVessel){
		ResetAllFlags();
		_previousVessel = NULL;
		_previousPartId = VESSEL_NO_ID;
		qu32 currentPartId = 0;
		return GetVesselPartRecursively(GetRoot(), _selectedPartId, currentPartId, _previousPartId, _previousVessel);
	}

	Vessel* Vessels::GetVesselPartRecursively(Vessel *_vessel, q::qu32 _selectedPartId, q::qu32 &_currentPartId, q::qs32 &_previousPartId, Vessel* &_previousVessel){
		if(_selectedPartId == _currentPartId){
			return _vessel;
		}
		_vessel->m_Flag = Q_TRUE;

		q::qs32 inc = 0;
		if(_vessel->m_VesselList.size() > 2 || (_vessel == m_Root && _vessel->m_VesselList.size() > 1)){
			inc = 1;
		}

		qs32 previousPartId = _currentPartId;
		Vessel *previousVessel = _vessel;
		for(Vessel::VesselList::iterator it = _vessel->m_VesselList.begin(); it != _vessel->m_VesselList.end(); ++it){
			if((*it)->m_Flag == Q_FALSE){
				_currentPartId = _currentPartId + inc;
				_previousPartId = previousPartId;
				_previousVessel = previousVessel;
				Vessel *tmp = GetVesselPartRecursively((*it), _selectedPartId, _currentPartId, _previousPartId, _previousVessel);
				if(tmp != NULL){
					return tmp;
				}
			}
		}
		return NULL;
	}

	q::Vessel* Vessels::GetEndingVesselPart(q::qu32 _selectedPartId){
		ResetAllFlags();
		Vessel *previousVessel = NULL;
		qs32 previousPartId = VESSEL_NO_ID;
		qu32 currentPartId = 0;
		Vessel *vessel = GetEndingVesselPartRecursively(GetRoot(), _selectedPartId, currentPartId, previousPartId, previousVessel);
		//Vessel *vessel = GetVesselPart(_selectedPartId, previousPartId, previousVessel);

		while(vessel != NULL){
			qu32 nextId = 0;
			if(previousVessel == NULL){
				if(vessel->m_Flag2 == Q_TRUE){
				//if(vessel->m_VesselList.size() != 1){
					return vessel;
				}
			}
			else{
				if(vessel->m_Flag2 == Q_TRUE){
				//if(vessel->m_VesselList.size() != 2){
					return vessel;
				}
				if(vessel->m_VesselList[0] == previousVessel){
					nextId = 1;
				}
			}

			previousVessel = vessel;
			vessel = vessel->m_VesselList[nextId];
		}
		return NULL;
	}

	q::Vessel* Vessels::GetEndingVesselPartRecursively(Vessel *_vessel, q::qu32 _selectedPartId, q::qu32 &_currentPartId, q::qs32 &_previousPartId, Vessel* &_previousVessel){
		if(_selectedPartId == _currentPartId){
			return _vessel;
		}
		_vessel->m_Flag = Q_TRUE;

		q::qs32 inc = 0;
		if(_vessel->m_Flag2 == Q_TRUE){
		//if(_vessel->m_VesselList.size() > 2){
			inc = 1;
		}

		qs32 previousPartId = _currentPartId;
		Vessel *previousVessel = _vessel;
		for(Vessel::VesselList::iterator it = _vessel->m_VesselList.begin(); it != _vessel->m_VesselList.end(); ++it){
			if((*it)->m_Flag == Q_FALSE){
				_currentPartId = _currentPartId + inc;
				_previousPartId = previousPartId;
				_previousVessel = previousVessel;
				Vessel *tmp = GetEndingVesselPartRecursively((*it), _selectedPartId, _currentPartId, _previousPartId, _previousVessel);
				if(tmp != NULL){
					return tmp;
				}
			}
		}
		return NULL;
	}

	void Vessels::AddPart(q::qs32 _nextPartLinkedFrom, const PtList &_ptList){
		Vessel *vessel = Q_NEW Vessel(_ptList[0]);
		Vessel *previousVessel = NULL;
		if(_nextPartLinkedFrom != VESSEL_NO_ID){
			previousVessel = GetEndingVesselPart(_nextPartLinkedFrom);
			AddVessel(vessel, previousVessel);
		}
		else{
			m_Root = vessel;
			m_CompleteVesselList.push_back(vessel);
		}
		previousVessel = vessel;

		// add the complete vessel
		qsize_t sizePtl = _ptList.getSize();
		for(size_t i = 1; i < sizePtl; ++i){
			vessel = Q_NEW Vessel(_ptList[i]);
			AddVessel(vessel, previousVessel);
			previousVessel = vessel;
		}

		// set the flag2 on the last vessel to indicate bifurcation when we're gonna add other vessel part
		previousVessel->m_Flag2 = Q_TRUE;
	}

	void Vessels::AddPartAuto(const PtList &_ptList){
		Vector3 pos = Vector3(_ptList[0].x(), _ptList[0].y(), _ptList[0].z());
		Vessel *previousVessel = GetClosestVesselFrom(pos);
		qsize_t sizePtl = _ptList.getSize();
		for(size_t i = 0; i < sizePtl; ++i){
			Vessel *vessel = Q_NEW Vessel(_ptList[i]);
			AddVessel(vessel, previousVessel);
			previousVessel = vessel;
		}
	}

	Vessel* Vessels::GetVesselId(qu32 _id){
		ResetAllFlags();
		qu32 inc = 0;
		return GetVesselIdRecursively(GetRoot(), _id, inc);
	}

	Vessel* Vessels::GetVesselIdRecursively(Vessel *_v, qu32 _id, qu32 &_inc){
		_v->m_Flag = Q_TRUE;		
		Vessel *ret = _v;
		if(_inc < _id){
			for(Vessel::VesselList::iterator it = _v->m_VesselList.begin(); it != _v->m_VesselList.end(); ++it){
				Vessel *tmp = (*it);
				if(tmp->m_Flag == Q_FALSE){
					_inc++;
					ret = GetVesselIdRecursively(tmp, _id, _inc);
					if(_inc >= _id){
						break;
					}
				}
			}
		}
		return ret;
	}

	qu32 Vessels::GetVesselId(Vessel *_v){
		ResetAllFlags();
		qu32 inc = 0;
		GetVesselIdRecursively(GetRoot(), _v, inc);
		return inc;
	}

	qbool Vessels::GetVesselIdRecursively(Vessel *_v, Vessel *_lookForV, qu32 &_inc){
		_v->m_Flag = Q_TRUE;
		if(_lookForV != _v){
			for(Vessel::VesselList::iterator it = _v->m_VesselList.begin(); it != _v->m_VesselList.end(); ++it){
				Vessel *tmp = (*it);
				if(tmp->m_Flag == Q_FALSE){
					_inc++;
					qbool qret = GetVesselIdRecursively(tmp, _lookForV, _inc);
					if(qret == Q_TRUE){
						return Q_TRUE;
					}
				}
			}
			return Q_FALSE;
		}
		return Q_TRUE;
	}
	
	qu32 Vessels::GetPreviousVesselId(qu32 _id, qu32 _offset){
		ResetAllFlags();
		qu32 inc = 0;
		qu32 offset = _offset;
		return GetPreviousVesselIdRecursively(GetRoot(), _id, offset, inc);
	}

	qu32 Vessels::GetPreviousVesselIdRecursively(Vessel *_v, qu32 _id, qu32 &_offset, qu32 &_inc){
		_v->m_Flag = Q_TRUE;
		qu32 tmpId = _inc;
		if(_inc < _id){
			for(Vessel::VesselList::iterator it = _v->m_VesselList.begin(); it != _v->m_VesselList.end(); ++it){
				Vessel *tmp = (*it);
				if(tmp->m_Flag == Q_FALSE){
					_inc++;
					qu32 ret = GetPreviousVesselIdRecursively(tmp, _id, _offset, _inc);
					if(_inc >= _id){
						if(_offset == 0){
							tmpId = ret;
						}
						else{
							_offset--;
						}
						break;
					}
				}
			}
		}
		return tmpId;
	}
	
	// warning: it works only if we put the id of the vessels in m_Data[Vessel::INFO_MISC_6]
	void Vessels::Get3dVesselsBranchesConst(qu32 _id, qbool _getBranchUntilOneLeaf, PtList &_ptList) const {
		Get3dVesselsBranchesConstRecursively(GetRoot(), _id, Q_FALSE, _getBranchUntilOneLeaf, _ptList);
	}

	qbool Vessels::Get3dVesselsBranchesConstRecursively(Vessel *_v, qu32 _id, qbool _found
												, qbool _getBranchUntilOneLeaf, PtList &_ptList) const{
		if(_v->m_Data[Vessel::INFO_MISC_6] == _id){
			_found = Q_TRUE;
		}
		
		qbool ret = Q_FALSE;
		if(_getBranchUntilOneLeaf == Q_TRUE || _found == Q_FALSE){
			for(Vessel::VesselList::iterator it = _v->m_VesselList.begin(); it != _v->m_VesselList.end(); ++it){
				Vessel *tmp = (*it);
				if(tmp->m_Data[Vessel::INFO_MISC_6] > _v->m_Data[Vessel::INFO_MISC_6]){
					ret = Get3dVesselsBranchesConstRecursively(tmp, _id, _found, _getBranchUntilOneLeaf, _ptList);
					if(_found == Q_TRUE || ret == Q_TRUE){
						break;
					}
				}
			}
		}

		if(_found == Q_TRUE || ret == Q_TRUE){
			Pt pt;
			pt.pos[AXE_X] = _v->m_Data[Vessel::INFO_X];
			pt.pos[AXE_Y] = _v->m_Data[Vessel::INFO_Y];
			pt.pos[AXE_Z] = _v->m_Data[Vessel::INFO_Z];
			pt.pos[PT_U] = _v->m_Data[Vessel::INFO_RADIUS];
			pt.vec[AXE_X] = _v->m_Data[Vessel::INFO_RADIUS_ARROW_X];
			pt.vec[AXE_Y] = _v->m_Data[Vessel::INFO_RADIUS_ARROW_Y];
			pt.vec[AXE_Z] = _v->m_Data[Vessel::INFO_RADIUS_ARROW_Z];
			pt.type = static_cast<qs32>(_v->m_Data[Vessel::INFO_MISC_6]);
			_ptList.appendItem(pt);
			return Q_TRUE;
		}
		return Q_FALSE;
	}

	void Vessels::Get3dVesselsBranches(qu32 _id, qbool _getBranchUntilOneLeaf, PtList &_ptList){
		Vessels *vessels = NULL;
		Get3dVesselsBranches(_id, _getBranchUntilOneLeaf, vessels);
		if(vessels != NULL){
			vessels->GetPtList(_ptList);
			SAFE_DELETE_UNIQUE(vessels);
		}
	}

	void Vessels::Get3dVesselsBranches(qu32 _id, qbool _getBranchUntilOneLeaf, Vessels* &_vessels){
		ResetAllFlags();
		qu32 inc = 0;
		Vessel *previousVessel = NULL;
		Get3dVesselsBranchesRecursively(GetRoot(), _id, inc, Q_FALSE, _getBranchUntilOneLeaf, _vessels, previousVessel);
	}

	qbool Vessels::Get3dVesselsBranchesRecursively(Vessel *_v, qu32 _id, qu32 &_inc, qbool _found
												, qbool _getBranchUntilOneLeaf, Vessels* &_vessels, Vessel* &_previousVessel){
		_v->m_Flag = Q_TRUE;

		if(_inc == _id){
			_found = Q_TRUE;
		}
		
		qbool ret = Q_FALSE;
		if(_getBranchUntilOneLeaf == Q_TRUE || _found == Q_FALSE){
			for(Vessel::VesselList::iterator it = _v->m_VesselList.begin(); it != _v->m_VesselList.end(); ++it){
				Vessel *tmp = (*it);
				if(tmp->m_Flag == Q_FALSE){
					_inc++;
					ret = Get3dVesselsBranchesRecursively(tmp, _id, _inc, _found, _getBranchUntilOneLeaf, _vessels, _previousVessel);
					if(_found == Q_TRUE || ret == Q_TRUE){
						break;
					}
				}
			}
		}

		if(_found == Q_TRUE || ret == Q_TRUE){
			Vessel *v = Q_NEW Vessel(_v);
			if(_previousVessel == NULL){
				_vessels = Q_NEW Vessels(v);
			}
			else{
				_vessels->AddVessel(v, _previousVessel);
			}
			_previousVessel = v;
			return Q_TRUE;
		}
		return Q_FALSE;
	}

	void Vessels::Get3dVesselsSubTree(qu32 _id, qbool _keepPathFromTheRoot, Vessels* &_vessels){
		ResetAllFlags();
		qu32 inc = 0;
		Vessel *previousVesselUp = NULL;
		Get3dVesselsSubTreeRecursively(GetRoot(), _id, inc, Q_FALSE, _keepPathFromTheRoot, _vessels, previousVesselUp, NULL);
		if (_vessels != NULL && _id != 0 && _keepPathFromTheRoot == Q_TRUE){
			_vessels->m_Root = _vessels->m_CompleteVesselList.back();
		}
	}

	qbool Vessels::Get3dVesselsSubTreeRecursively(Vessel *_v, qu32 _id, qu32 &_inc, qbool _found
										, qbool _keepPathFromTheRoot, Vessels* &_vessels
										, Vessel* &_previousVesselUp, Vessel *_previousVesselDown){
		_v->m_Flag = Q_TRUE;

		qbool ret = Q_FALSE;
		if(_inc == _id){
			_found = Q_TRUE;
			ret = Q_TRUE;
			Vessel *v = Q_NEW Vessel(_v);
			_vessels = Q_NEW Vessels(v);
			_previousVesselUp = v;
			_previousVesselDown = v;
		}
		else if (_found == Q_TRUE){
			Vessel *v = Q_NEW Vessel(_v);
			_vessels->AddVessel(v, _previousVesselDown);
			_previousVesselDown = v;
		}

		qbool bret = Q_FALSE;
		for(Vessel::VesselList::iterator it = _v->m_VesselList.begin(); it != _v->m_VesselList.end(); ++it){
			Vessel *tmp = (*it);
			if(tmp->m_Flag == Q_FALSE){
				_inc++;
				bret = Get3dVesselsSubTreeRecursively(tmp, _id, _inc, _found, _keepPathFromTheRoot, _vessels, _previousVesselUp, _previousVesselDown);
				if(bret == Q_TRUE){
					break;
				}
			}
		}

		if(bret == Q_TRUE && _keepPathFromTheRoot == Q_TRUE){
			Vessel *v = Q_NEW Vessel(_v);
			_vessels->AddVessel(v, _previousVesselUp);
			_previousVesselUp = v;
		}
		return ret || bret;
	}

	void Vessels::GetNeighborhoodVessels(qu32 _id, qu32 _nbPtsAfter, qu32 _nbPtsBefore, Vessels* &_vessels){
		ResetAllFlags();
		Vessel *v = GetVesselId(_id);
		
		Vessel *vesselFromRoot = NULL;
		for(Vessel::VesselList::iterator it = v->m_VesselList.begin(); it != v->m_VesselList.end(); ++it){
			Vessel *tmp = (*it);
			// going to the root
			if(tmp->m_Flag == Q_TRUE){
				vesselFromRoot = tmp;
				break;
			}
		}

		_vessels = Q_NEW Vessels(Q_NEW Vessel(*v));
		CopyAndLinkNeighborhoodVesselsRecursively(_vessels, _vessels->m_Root, v, _nbPtsAfter);

		ResetAllFlags();
		for(Vessel::VesselList::iterator it = v->m_VesselList.begin(); it != v->m_VesselList.end(); ++it){
			Vessel *tmp = (*it);
			// going to the leaves
			if(tmp != vesselFromRoot){
				tmp->m_Flag = Q_TRUE;
			}
		}
		CopyAndLinkNeighborhoodVesselsRecursively(_vessels, _vessels->m_Root, v, _nbPtsBefore);
	}
	
	void Vessels::CopyAndLinkNeighborhoodVesselsRecursively(Vessels *_vessels, Vessel *_vesselDest,  Vessel *_vesselSource, qu32 _nbPts){
		_vesselSource->m_Flag = Q_TRUE;
		if(_nbPts != 0){
			if(_nbPts != VESSEL_UNLIMITED){
				_nbPts = _nbPts - 1;
			}

			for(Vessel::VesselList::iterator it = _vesselSource->m_VesselList.begin(); it != _vesselSource->m_VesselList.end(); ++it){
				Vessel *tmp = (*it);
				if(tmp->m_Flag == Q_FALSE){
					Vessel *newVessel = Q_NEW Vessel(tmp);
					_vessels->AddVessel(newVessel, _vesselDest);
					CopyAndLinkNeighborhoodVesselsRecursively(_vessels, newVessel, tmp, _nbPts);
				}
			}
		}
	}

	qu32 Vessels::GetTheFirstLeafIdCloseTo(qu32 _id){
		ResetAllFlags();
		qu32 inc = 0;
		qbool found = Q_FALSE;
		return GetTheFirstLeafIdCloseToRecursively(GetRoot(), _id, inc, found);
	}

	qu32 Vessels::GetTheFirstLeafIdCloseToRecursively(Vessel *_v, qu32 _id, qu32 &_inc, qbool &_found){
		_v->m_Flag = Q_TRUE;

		if(_inc == _id){
			_found = Q_TRUE;
		}
		
		qu32 ret = _id;
		for(Vessel::VesselList::iterator it = _v->m_VesselList.begin(); it != _v->m_VesselList.end(); ++it){
			Vessel *tmp = (*it);
			if(tmp->m_Flag == Q_FALSE){
				_inc++;
				ret = GetTheFirstLeafIdCloseToRecursively(tmp, _id, _inc, _found);
				if(_found == Q_TRUE){
					break;
				}
			}
		}
		return ret;	
	}

	qbool Vessels::IsVesselInsideThisPath(qu32 _vesselId, qu32 _pathLeafId){
		ResetAllFlags();
		qu32 inc = 0;
		qbool found = Q_FALSE;
		return IsVesselInsideThisPathRecursively(GetRoot(), _vesselId, _pathLeafId, inc, found);
	}

	qbool Vessels::IsVesselInsideThisPathRecursively(Vessel *_v, qu32 _vesselId, qu32 _pathLeafId, qu32 &_inc, qbool &_found){
		_v->m_Flag = Q_TRUE;

		qu32 currentInc = _inc;
		if(currentInc == _pathLeafId){
			_found = Q_TRUE;
		}
		
		qbool ret = Q_FALSE;
		for(Vessel::VesselList::iterator it = _v->m_VesselList.begin(); it != _v->m_VesselList.end(); ++it){
			Vessel *tmp = (*it);
			if(tmp->m_Flag == Q_FALSE){
				_inc++;
				ret = IsVesselInsideThisPathRecursively(tmp, _vesselId, _pathLeafId, _inc, _found);
				if(_found == Q_TRUE){
					break;
				}
			}
		}

		if(_found == Q_TRUE && currentInc == _vesselId){
			return Q_TRUE;
		}

		return ret;
	}
	
	void Vessels::ComputePartType(void){
		ResetAllFlags();
		qu32 inc = 0;
		ComputePartTypeRecursively(GetRoot(), inc);
	}

	void Vessels::ComputePartTypeRecursively(Vessel *_v, qu32 &_inc){
		_v->m_Flag = Q_TRUE;
		_v->m_Data[Vessel::INFO_MISC_6] = _inc;
		
		qbool isBirfucation = Q_FALSE;
		if(_v->m_VesselList.size() > 2){
			isBirfucation = Q_TRUE;
			//_inc = _inc + 1;
		}

		for(Vessel::VesselList::iterator it = _v->m_VesselList.begin(); it != _v->m_VesselList.end(); ++it){
			Vessel *tmp = (*it);
			if(tmp->m_Flag == Q_FALSE){
				if(isBirfucation == Q_TRUE){
					_inc = (_inc + 1); //%12;
				}
				ComputePartTypeRecursively(tmp, _inc);
			}
		}
	}

	void Vessels::PropagatePartType(Vessel *_v, qf64 _type){
		ResetAllFlags();
		qu32 part = 0;
		qs32 goodPart = VESSEL_NO_ID;
		PropagatePartTypeRecursively(GetRoot(), _v, _type, part, goodPart);
	}
	
	void Vessels::PropagatePartTypeRecursively(Vessel *_v, Vessel *_selectedVessel, qf64 _type, qu32 &_part, qs32 &_goodPart){
		_v->m_Flag = Q_TRUE;

		if(_v == _selectedVessel){
			_goodPart = _part;
		}
		
		qu32 currentPart = _part;
		qu32 inc = 0;
		if(_v->m_VesselList.size() > 2 || (_v == m_Root && _v->m_VesselList.size() > 1)){
			inc = 1;
		}

		if(inc != 1 || _goodPart == VESSEL_NO_ID){
			for(Vessel::VesselList::iterator it = _v->m_VesselList.begin(); it != _v->m_VesselList.end(); ++it){
				Vessel *tmp = (*it);
				if(tmp->m_Flag == Q_FALSE){
					_part = _part + inc;
					PropagatePartTypeRecursively(tmp, _selectedVessel, _type, _part, _goodPart);
					if(_goodPart != VESSEL_NO_ID){
						break;
					}
				}
			}
		}

		if(static_cast<qs32>(currentPart) == _goodPart){
			_v->m_Data[Vessel::INFO_MISC_6] = _type;
		}
	}

	void Vessels::TagIdInData(qu32 _dataId){
		ResetAllFlags();
		qu32 inc = 0;
		TagIdInDataRecursively(GetRoot(), inc, _dataId);
	}
		
	void Vessels::TagIdInDataRecursively(Vessel *_v, qu32 &_inc, qu32 _dataId){
		_v->m_Flag = Q_TRUE;
		_v->m_Data[_dataId] = _inc;

		for(Vessel::VesselList::iterator it = _v->m_VesselList.begin(); it != _v->m_VesselList.end(); ++it){
			Vessel *tmp = (*it);
			if(tmp->m_Flag == Q_FALSE){
				_inc++;
				TagIdInDataRecursively(tmp, _inc, _dataId);
			}
		}
	}
	
	/*q::qu32 Vessels::GetIdFromVesselPointer(q::Vessel *_v){
		ResetAllFlags();
		qu32 inc = 0;
		return GetIdFromVesselPointerRecursively(_v, GetRoot(), inc);
	}

	q::qu32 Vessels::GetIdFromVesselPointerRecursively(q::Vessel *_lookForV, q::Vessel *_v, qu32 &_inc){
		_v->m_Flag = Q_TRUE;
		qu32 ret = _inc;
		if(_v != _lookForV){
			for(Vessel::VesselList::iterator it = _v->m_VesselList.begin(); it != _v->m_VesselList.end(); ++it){
				Vessel *tmp = (*it);
				if(tmp->m_Flag == Q_FALSE){
					_inc++;
					ret = GetIdFromVesselPointerRecursively(_lookForV, tmp, _inc);
					if(_inc == 0){
						break;
					}
				}
			}
		}
		else{
			_inc = 0;
		}
		return ret;
	}*/

	q::qu32 Vessels::GetNumberOfPartBtwTwoVessels(q::Vessel *_v1, q::Vessel *_v2){
		ResetAllFlags();
		qbool iAmTheOne = Q_FALSE;
		qbool found = Q_FALSE;
		return GetNumberOfPartBtwTwoVesselsRecursively(_v1, _v2, found, iAmTheOne);
	}

	q::qu32 Vessels::GetNumberOfPartBtwTwoVesselsRecursively(q::Vessel *_v1, q::Vessel *_v2, q::qbool &_found, q::qbool &_iAmTheOne){
		_v1->m_Flag = Q_TRUE;
		if(_v1 == _v2){
			_found = Q_TRUE;
			_iAmTheOne = Q_TRUE;
			return 0;
		}
		else{
			for(Vessel::VesselList::iterator it = _v1->m_VesselList.begin(); it != _v1->m_VesselList.end(); ++it){
				Vessel *tmp = (*it);
				if(tmp->m_Flag == Q_FALSE){
					qbool iAmTheOne = Q_FALSE;
					qu32 ret = GetNumberOfPartBtwTwoVesselsRecursively(tmp, _v2, _found, iAmTheOne);
					if(_found == Q_TRUE){
						if(tmp->m_VesselList.size() > 2 && iAmTheOne != Q_TRUE){
							ret = ret + 1;
						}
						return ret;
					}
				}
			}
		}
		return 10000;
	}

	void Vessels::GetPtList(PtList &_ptList){
		_ptList.clear();
		ResetAllFlags();
		Vessel *vessel = GetRoot();
		GetPtListRecursively(vessel, _ptList);
	}

	void Vessels::GetPtListRecursively(Vessel *_vessel, PtList &_ptList){
		if(_vessel->m_Flag == Q_FALSE) {
			_vessel->m_Flag = Q_TRUE;
			Pt pt;
			pt.pos[AXE_X] = _vessel->m_Data[Vessel::INFO_X];
			pt.pos[AXE_Y] = _vessel->m_Data[Vessel::INFO_Y];
			pt.pos[AXE_Z] = _vessel->m_Data[Vessel::INFO_Z];
			pt.pos[PT_U] = _vessel->m_Data[Vessel::INFO_RADIUS];
			pt.vec[AXE_X] = _vessel->m_Data[Vessel::INFO_RADIUS_ARROW_X];
			pt.vec[AXE_Y] = _vessel->m_Data[Vessel::INFO_RADIUS_ARROW_Y];
			pt.vec[AXE_Z] = _vessel->m_Data[Vessel::INFO_RADIUS_ARROW_Z];
			pt.type = static_cast<qs32>(_vessel->m_Data[Vessel::INFO_MISC_6]);
			_ptList.appendItem(pt);

			for(Vessel::VesselList::iterator it = _vessel->m_VesselList.begin(); it != _vessel->m_VesselList.end(); ++it){
				GetPtListRecursively((*it), _ptList);
			}
		}
	}

	void Vessels::GetLinkPtList(PtList &_ptList){
		_ptList.clear();
		ResetAllFlags();
		Vessel *vessel = GetRoot();
		GetLinkPtListRecursively(vessel, _ptList);
	}

	void Vessels::GetLinkPtListRecursively(Vessel *_vessel, PtList &_ptList){
		if(_vessel->m_Flag == Q_FALSE){
			_vessel->m_Flag = Q_TRUE;

			for(Vessel::VesselList::iterator it = _vessel->m_VesselList.begin(); it != _vessel->m_VesselList.end(); ++it){
				if((*it)->m_Flag == Q_FALSE){
					Pt pt;
					pt.pos[AXE_X] = _vessel->m_Data[Vessel::INFO_X] + ((*it)->m_Data[Vessel::INFO_X] - _vessel->m_Data[Vessel::INFO_X])/2.;
					pt.pos[AXE_Y] = _vessel->m_Data[Vessel::INFO_Y] + ((*it)->m_Data[Vessel::INFO_Y] - _vessel->m_Data[Vessel::INFO_Y])/2.;
					pt.pos[AXE_Z] = _vessel->m_Data[Vessel::INFO_Z] + ((*it)->m_Data[Vessel::INFO_Z] - _vessel->m_Data[Vessel::INFO_Z]) / 2.;
					//pt.pos[PT_U] = 0;
					pt.vec[AXE_X] = 0;
					pt.vec[AXE_Y] = 0;
					pt.vec[AXE_Z] = 0;
					pt.type = 0;
					_ptList.appendItem(pt);
				}
			}
			
			for(Vessel::VesselList::iterator it = _vessel->m_VesselList.begin(); it != _vessel->m_VesselList.end(); ++it){
				GetLinkPtListRecursively((*it), _ptList);
			}
		}
	}
			
	void Vessels::SetVesselsValueRecursively(Vessel *_v, qsize_t _dataId, std::vector<qf64>::iterator &_itValue){
		_v->m_Flag = Q_TRUE;
		_v->m_Data[_dataId] = *_itValue;
		for(Vessel::VesselList::iterator it = _v->m_VesselList.begin(); it != _v->m_VesselList.end(); ++it){
			Vessel *tmp = (*it);
			if(tmp->m_Flag == Q_FALSE){
				_itValue++;
				SetVesselsValueRecursively(tmp, _dataId, _itValue);
			}
		}
	}

	void Vessels::SetVesselsValue(qsize_t _dataId, std::vector<qf64> &_valueList){
		ResetAllFlags();
		std::vector<qf64>::iterator it = _valueList.begin();
		SetVesselsValueRecursively(GetRoot(), _dataId, it);
	}
			
	void Vessels::GetVesselsValueRecursively(Vessel *_v, qsize_t _dataId, std::vector<qf64> &_valueList){
		_v->m_Flag = Q_TRUE;
		_valueList.push_back(_v->m_Data[_dataId]);
		for(Vessel::VesselList::iterator it = _v->m_VesselList.begin(); it != _v->m_VesselList.end(); ++it){
			Vessel *tmp = (*it);
			if(tmp->m_Flag == Q_FALSE){
				GetVesselsValueRecursively(tmp, _dataId, _valueList);
			}
		}
	}

	void Vessels::GetVesselsValue(qsize_t _dataId, std::vector<qf64> &_valueList){
		_valueList.clear();
		ResetAllFlags();
		GetVesselsValueRecursively(GetRoot(), _dataId, _valueList);
	}

	void Vessels::GetSortedVesselsValue(qsize_t _dataId, std::multimap<qf64, Vessel*> &_sortedList){
		for(Vessel::VesselList::iterator it = m_CompleteVesselList.begin(); it != m_CompleteVesselList.end(); ++it){
			if((*it)->m_Data[_dataId] != 0.){
				_sortedList.insert(std::pair<qf64, Vessel*>((*it)->m_Data[_dataId], (*it)));
			}
		}
	}

#ifdef NEED_RESAMPLE
	void Vessels::Resample(Vessels* &_vessels, qf64 _blurSigma, qf64 _sampleDistance){
		qu32 nbPart = GetNbOfParts();
		PtList *ptListPart = Q_NEW PtList[nbPart];
		qs32 *previousPartList = Q_NEW qs32[nbPart];
		//qPrint("Resample\n");
		for(qu32 i = 0; i < nbPart; ++i){
			PtList ptlist;
			GetVesselPart(i, previousPartList[i], ptlist);
			//Vessels *vesselsPart = NULL;
			//GetVesselPart(i, previousPartList[i], vesselsPart);
			//vesselsPart->GetPtList(ptlist);

			//qPrint("ptlist[%d] %d\n", i, ptlist[i].size());

			if(ptlist.size() > 1){
				ResampleAndSmoothXMarkerlist::Parameters *param = Q_NEW ResampleAndSmoothXMarkerlist::Parameters();
				param->m_FitSpline = Q_TRUE;
				param->m_Sigma = static_cast<qf32>(_blurSigma);
				param->m_SampleDistance = static_cast<qf32>(_sampleDistance);
				ResampleAndSmoothXMarkerlist resampleAndSmoothPtlist = ResampleAndSmoothXMarkerlist(param);
				resampleAndSmoothPtlist.Apply(ptlist);
				//qPrint("ptlist %d %d\n", i, ptlist.size());
				ptListPart[i] = resampleAndSmoothPtlist.m_MarkerListOutput;
				//qPrint("ptlist %d %d\n", i, ptlist.size());

				// add the radius
				for(qu32 resampledPtId = 0; resampledPtId < ptListPart[i].size(); ++resampledPtId){
					Vector3 resampledPos = Vector3(ptListPart[i][resampledPtId].x(), ptListPart[i][resampledPtId].y(), ptListPart[i][resampledPtId].z());
					// map is not enough because several points can have the same distance, and map will erase the previous point with the same distance
					//std::map<qf64, Pt*> closestPoints;
					std::multimap<qf64, Pt*> closestPoints;
					for(qu32 ptId = 0; ptId < ptlist.size(); ++ptId){
						Vector3 pos = Vector3(ptlist[ptId].x(), ptlist[ptId].y(), ptlist[ptId].z());
						qf64 distance = GetDistance(resampledPos, pos);
						closestPoints.insert(std::pair<qf64, Pt*>(distance, &ptlist[ptId]));
					}

					//std::map<qf64, Pt*>::iterator it = closestPoints.begin();
					std::multimap<qf64, Pt*>::iterator it = closestPoints.begin();
					qf64 radiusFirstClosest = it->second->u();
					qf64 distanceFirstClosest = it->first;
					++it;
					qf64 radiusSecondClosest = it->second->u();
					qf64 distanceSecondClosest = it->first;
					qf64 coeff = distanceFirstClosest/distanceSecondClosest;
					ptListPart[i][resampledPtId].u() = radiusFirstClosest*(1. - coeff) + radiusSecondClosest*coeff;
				}
			}
			else{
				ptListPart[i] = ptlist;
			}
		}

		//qPrint("ptListPart[0] %d\n", ptListPart[0].size());
		_vessels = Q_NEW Vessels(ptListPart[0]);
		for(qu32 i = 1; i < nbPart; ++i){
			_vessels->AddPart(previousPartList[i], ptListPart[i]);
		}

		Q_DELETE previousPartList;
		Q_DELETE ptListPart;
	}
#endif
END_Q_NAMESPACE
