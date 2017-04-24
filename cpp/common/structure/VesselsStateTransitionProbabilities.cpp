#include "VesselsStateTransitionProbabilities.h"
#include "common/maths/Maths.h"

BEGIN_Q_NAMESPACE
	VesselsStateTransitionProbabilities::VesselsStateTransitionProbabilities(const qString &_fileName){
		qFile *file = NULL;
		qString buffer;

		try{
			qu32 err = qFOpen(file, _fileName.c_str(), "rb");
			if(err == 0){
				throw gDefaultException;
			}

			// get nb of vessel
			qu32 iErr = FileReadLine(file, buffer);
			if(iErr != 1){
				throw gDefaultException;
			}
			qu32 nbVessel = atoi(buffer.c_str());
			
			iErr = FileReadLine(file, buffer);
			if(iErr != 1){
				throw gDefaultException;
			}
			std::vector<qString> vecList;
			SplitString(buffer, " ", vecList);
			for(qu32 i = 0; i < nbVessel; ++i){
				VesselsStateTransitionProbabilities::VesselDistribution *vesselDistribution = Q_NEW VesselsStateTransitionProbabilities::VesselDistribution();
				m_CompleteVesselListFromState.push_back(vesselDistribution);
				m_IdToTreeId.push_back(atoi(vecList[i].c_str()));
			}

			for(qu32 i = 0; i < nbVessel; ++i){
				VesselDistribution *vesselDistribution = Q_NEW VesselDistribution();
				m_CompleteVesselList.push_back(vesselDistribution);
				
				qu32 iErr = FileReadLine(file, buffer);
				if(iErr != 1){
					throw gDefaultException;
				}
				qu32 nbTransition = atoi(buffer.c_str());
				
				iErr = FileReadLine(file, buffer);
				if(iErr != 1){
					throw gDefaultException;
				}
				std::vector<qString> vecList;
				SplitString(buffer, " ", vecList);

				for(qu32 j = 0; j < nbTransition; ++j){
					qu32 id = atoi(vecList[j*2 + 0].c_str());
					qf64 gaussianProba = atof(vecList[j*2 + 1].c_str());
					vesselDistribution->m_TransitionList.push_back(Transition(id, m_IdToTreeId[id], gaussianProba));
					m_CompleteVesselListFromState[id]->m_TransitionList.push_back(Transition(i, m_IdToTreeId[i], gaussianProba));
				}
			}
			
			/*iErr = FileReadLine(file, buffer);
			if(iErr != 1){
				throw gDefaultException;
			}
			std::vector<qString> vecList;
			SplitString(qString(buffer), " ", vecList);
			for(qu32 i = 0; i < m_CompleteVesselList.size(); ++i){
				m_CompleteVesselIdSortedList.push_back(m_CompleteVesselList[atoi(vecList[i].c_str())]);
			}*/
		}
		catch(q::qDefaultException){
			q::qPrint("VesselsStateTransitionProbabilities::VesselsStateTransitionProbabilities Error during loading\n");
			qFClose(file);
			throw gDefaultException;
		}
		
		qFClose(file);		
	}
	
	void ComputeStateTransitionProbabilitiesRecursively(Vessel *_v, qu32 _transitionMetric, qf64 _sigma, qf64 _probaMin, qf64 _distance, qf64 _previousDistance){
		_v->m_Flag = Q_TRUE;

		if(_transitionMetric == VesselsStateTransitionProbabilities::TRANSITION_METRIC_GAUSSIAN_A_PRIME){
			_v->m_Data[Vessel::INFO_MISC_5] = UTIL_GAUSSIAN_5(_distance, _sigma, 0, 1, 1);
		}
		else{
			if(_distance <= _sigma || (_previousDistance < _sigma && (_sigma - _previousDistance)/(_distance - _previousDistance) > 0.5)){
			//if(_distance <= _sigma){
				_v->m_Data[Vessel::INFO_MISC_5] = 1.0;
			}
			else{
				_v->m_Data[Vessel::INFO_MISC_5] = 0.0;
			}
		}
		if(_v->m_Data[Vessel::INFO_MISC_5] > _probaMin){
			for(Vessel::VesselList::iterator it = _v->m_VesselList.begin(); it != _v->m_VesselList.end(); ++it){
				Vessel *tmp = (*it);
				if(tmp->m_Flag == Q_FALSE){
					qf64 distance = _distance + GetDistance(Vector3(_v->m_Data[Vessel::INFO_X], _v->m_Data[Vessel::INFO_Y], _v->m_Data[Vessel::INFO_Z])
															, Vector3(tmp->m_Data[Vessel::INFO_X], tmp->m_Data[Vessel::INFO_Y], tmp->m_Data[Vessel::INFO_Z]));
					ComputeStateTransitionProbabilitiesRecursively(tmp, _transitionMetric, _sigma, _probaMin, distance, _distance);
				}
			}
		}
	}

	VesselsStateTransitionProbabilities::VesselsStateTransitionProbabilities(Vessels *_vessels, qu32 _transitionMetric, qf64 _sigma, qf64 _probaMin){
		//qPrint("VesselsStateTransitionProbabilities::VesselsStateTransitionProbabilities\n");
		_vessels->TagIdInData(Vessel::INFO_MISC_1);

		for(Vessel::VesselList::iterator it = _vessels->m_CompleteVesselList.begin(); it != _vessels->m_CompleteVesselList.end(); ++it){
			VesselsStateTransitionProbabilities::VesselDistribution *vesselDistribution = Q_NEW VesselsStateTransitionProbabilities::VesselDistribution();
			m_CompleteVesselListFromState.push_back(vesselDistribution);
			m_IdToTreeId.push_back(static_cast<qu32>((*it)->m_Data[Vessel::INFO_MISC_1]));
		}

		//m_CompleteVesselIdSortedList.resize(_vessels->m_CompleteVesselList.size(), NULL);
		for(Vessel::VesselList::iterator it = _vessels->m_CompleteVesselList.begin(); it != _vessels->m_CompleteVesselList.end(); ++it){
			VesselsStateTransitionProbabilities::VesselDistribution *vesselDistribution = Q_NEW VesselsStateTransitionProbabilities::VesselDistribution();
			m_CompleteVesselList.push_back(vesselDistribution);
			//m_CompleteVesselIdSortedList[static_cast<qu32>((*it)->m_Data[Vessel::INFO_MISC_1])] = vesselDistribution;
			
			_vessels->ResetAllFlags();
			_vessels->SetAllData(Vessel::INFO_MISC_5, 0.);
			ComputeStateTransitionProbabilitiesRecursively((*it), _transitionMetric, _sigma, _probaMin, 0, 0);

			qf64 sumProba = 0.;
			for(Vessel::VesselList::iterator it2 = _vessels->m_CompleteVesselList.begin(); it2 != _vessels->m_CompleteVesselList.end(); ++it2){
				// TODO implement the USE_LOG_PROBA part also, could perhaps improve results?
				sumProba = sumProba + (*it2)->m_Data[Vessel::INFO_MISC_5];
			}

			for(qu32 i = 0; i < _vessels->m_CompleteVesselList.size(); ++i){
				Vessel *v = _vessels->m_CompleteVesselList[i];
				if(v->m_Data[Vessel::INFO_MISC_5] != 0.){
					vesselDistribution->m_TransitionList.push_back(Transition(i, static_cast<qu32>(v->m_Data[Vessel::INFO_MISC_1]), v->m_Data[Vessel::INFO_MISC_5]/sumProba));
					m_CompleteVesselListFromState[i]->m_TransitionList.push_back(
							Transition(static_cast<qu32>(m_CompleteVesselList.size() - 1), static_cast<qu32>((*it)->m_Data[Vessel::INFO_MISC_1]), v->m_Data[Vessel::INFO_MISC_5]/sumProba));
				}
			}
		}

		// compute m_CompleteVesselIdSortedList
		/*std::map<qu32, VesselDistribution*> sortedList;

		VesselDistributionList::iterator itDist = m_CompleteVesselList.begin();
		for(Vessel::VesselList::iterator it = _vessels->m_CompleteVesselList.begin(); it != _vessels->m_CompleteVesselList.end(); ++it, ++itDist){
			Vessel *v = (*it);
			sortedList.insert(std::pair<qu32, VesselDistribution*>(static_cast<qu32>(v->m_Data[Vessel::INFO_MISC_1]), (*itDist)));
		}

		for(std::map<qu32, VesselDistribution*>::iterator it = sortedList.begin(); it != sortedList.end(); ++it){
			m_CompleteVesselIdSortedList.push_back(it->second);
		}*/
	}

	VesselsStateTransitionProbabilities::~VesselsStateTransitionProbabilities(void){
		for(VesselDistributionList::iterator it = m_CompleteVesselList.begin(); it != m_CompleteVesselList.end(); ++it){
			SAFE_DELETE_UNIQUE((*it));
		}
		m_CompleteVesselList.clear();
		//m_CompleteVesselIdSortedList.clear();
		
		for(VesselDistributionList::iterator it = m_CompleteVesselListFromState.begin(); it != m_CompleteVesselListFromState.end(); ++it){
			SAFE_DELETE_UNIQUE((*it));
		}
		m_CompleteVesselListFromState.clear();
	}

	void VesselsStateTransitionProbabilities::Save(const qString &_fileName){
		qFile *file = NULL;
		char buffer[MAX_STR_BUFFER];
		
		try{
			qu32 err = qFOpen(file, _fileName.c_str(), "wb");
			if(err == 0){
				throw gDefaultException;
			}
			
			int iErr = file->Fprintf("%d\n", m_CompleteVesselList.size());
			if(iErr < 0){
				throw gDefaultException;
			}
			
			qString str;
			for(std::vector<qu32>::iterator it = m_IdToTreeId.begin(); it != m_IdToTreeId.end(); ++it){
				qSprintf(buffer, MAX_STR_BUFFER, "%d ", (*it));
				str += qString(buffer);
			}
			str += "\n";
			iErr = file->Fprintf("%s", str.c_str());
			if(iErr < 0){
				throw gDefaultException;
			}

			for(VesselDistributionList::iterator it = m_CompleteVesselList.begin(); it != m_CompleteVesselList.end(); ++it){
				str.clear();
				qSprintf(buffer, MAX_STR_BUFFER, "%lu\n", (*it)->m_TransitionList.size());
				str += qString(buffer);

				for(TransitionList::iterator it2 = (*it)->m_TransitionList.begin(); it2 != (*it)->m_TransitionList.end(); ++it2){
					//qSprintf(buffer, MAX_STR_BUFFER, "%d %f ", (*it2).m_Id, (*it2).m_GaussianProba);
					//qSprintf(buffer, MAX_STR_BUFFER, "%d %e ", (*it2).m_Id, (*it2).m_GaussianProba);
					qSprintf(buffer, MAX_STR_BUFFER, "%d %.12e ", (*it2).m_Id, (*it2).m_GaussianProba);
					str += qString(buffer);
				}
				str += "\n";

				iErr = file->Fprintf("%s", str.c_str());
				if(iErr < 0){
					throw gDefaultException;
				}
			}

			/*qString str;
			for(VesselDistributionList::iterator it = m_CompleteVesselIdSortedList.begin(); it != m_CompleteVesselIdSortedList.end(); ++it){
				for(qu32 i = 0; i < m_CompleteVesselList.size(); ++i){
					if((*it) == m_CompleteVesselList[i]){
						qSprintf(buffer, MAX_STR_BUFFER, "%d ", i);
						str += qString(buffer);
						break;
					}
				}
			}
			str += "\n";

			iErr = fprintf(file, "%s", str.c_str());
			if(iErr < 0){
				throw gDefaultException;
			}*/
		}
		catch(q::qDefaultException){
			q::qPrint("VesselsStateTransitionProbabilities::Save Error during saving\n");
		}

		qFClose(file);
	}
END_Q_NAMESPACE
