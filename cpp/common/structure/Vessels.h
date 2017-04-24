#ifndef __Q_VESSELS_
#define __Q_VESSELS_
#include "common/util/Util.h"
#include "common/maths/Maths.h"
#include "common/maths/Matrix.h"
#include "common/structure/PtList.h"

#include "math.h"
#include <algorithm>
#include "limits.h" // UINT_MAX

#define NEED_RESAMPLE

#define VESSEL_NO_ID (-1)
//#define VESSEL_UNLIMITED (-1)
//#define VESSEL_UNLIMITED ((2<<32) - 1)
#define VESSEL_UNLIMITED (UINT_MAX)

BEGIN_Q_NAMESPACE
	class Vessel {
		friend class Vessels;
	public:
		enum {
			INFO_X = 0,
			INFO_Y = 1,
			INFO_Z,
			INFO_U,
			INFO_V,
			INFO_W,
			INFO_RADIUS,
			INFO_RADIUS_ARROW_X,
			INFO_RADIUS_ARROW_Y,
			INFO_RADIUS_ARROW_Z,
			INFO_MISC_1,
			INFO_MISC_2,
			INFO_MISC_3,
			INFO_MISC_4,
			INFO_MISC_5,
			INFO_MISC_6,
			INFO_COUNT
		};
		
		typedef std::vector<Vessel*> VesselList;

		inline Vessel(q::qf64 _x = 0., q::qf64 _y = 0., q::qf64 _z = 0.
					,q::qf64 _u = 1., q::qf64 _v = 0., q::qf64 _w = 0.
					,q::qf64 _radius = 1.) {
			Init();
			m_Data[INFO_X] = _x;
			m_Data[INFO_Y] = _y;
			m_Data[INFO_Z] = _z;
			m_Data[INFO_U] = _u;
			m_Data[INFO_V] = _v;
			m_Data[INFO_W] = _w;
			m_Data[INFO_RADIUS] = _radius;
		}

		inline Vessel(const Pt &_pt){
			Init();
			m_Data[Vessel::INFO_X] = _pt.x();
			m_Data[Vessel::INFO_Y] = _pt.y();
			m_Data[Vessel::INFO_Z] = _pt.z();
			m_Data[Vessel::INFO_RADIUS] = _pt.u();
			m_Data[Vessel::INFO_MISC_6] = static_cast<qf64>(_pt.type);
		}

		inline Vessel(q::Vessel *_vessel){
			CopyInfo(_vessel);
		}

		inline ~Vessel(void) {
		}

		inline static q::qbool AreVesselsLinked(Vessel *_v1, Vessel *_v2){
			return std::find(_v1->m_VesselList.begin(), _v1->m_VesselList.end(), _v2) != _v1->m_VesselList.end()
				&& std::find(_v2->m_VesselList.begin(), _v2->m_VesselList.end(), _v1) != _v2->m_VesselList.end();
		}

		inline static q::qf64 SquareDistance(Vessel *_v1, Vessel *_v2){
			return (_v2->m_Data[INFO_X] - _v1->m_Data[INFO_X])*(_v2->m_Data[INFO_X] - _v1->m_Data[INFO_X])
				+ (_v2->m_Data[INFO_Y] - _v1->m_Data[INFO_Y])*(_v2->m_Data[INFO_Y] - _v1->m_Data[INFO_Y])
				+ (_v2->m_Data[INFO_Z] - _v1->m_Data[INFO_Z])*(_v2->m_Data[INFO_Z] - _v1->m_Data[INFO_Z]);
		}

		inline static q::qf64 Distance(Vessel *_v1, Vessel *_v2){
			return sqrt(SquareDistance(_v1, _v2));
		}

		inline Vessel* GetClosestVessel(void){
			Vessel *vesselClosest = NULL;
			qf64 distMin = UTIL_BIG_POSITIVE_NUMBER;
			for(Vessel::VesselList::iterator it = m_VesselList.begin(); it != m_VesselList.end(); ++it){
				qf64 dist = Vessel::SquareDistance(this, *it);
				if(dist < distMin){
					vesselClosest = *it;
					distMin = dist;
				}
			}
			return vesselClosest;
		}

	public:
		q::qf64 m_Data[INFO_COUNT];
		q::qbool m_Flag;
		q::qbool m_Flag2;
		VesselList m_VesselList;

	private:
		inline void Init(void){
			memset(m_Data, 0, INFO_COUNT*sizeof(m_Data[0]));
			m_Flag = Q_FALSE;
			m_Flag2 = Q_FALSE;
		}

		void UnlinkAll(void);
		void Unlink(Vessel *_v);

		inline void CopyInfo(q::Vessel *_vessel){
			memcpy(m_Data, _vessel->m_Data, INFO_COUNT*sizeof(m_Data[0]));
			m_Flag = _vessel->m_Flag;
			m_Flag2 = _vessel->m_Flag2;
		}
	};

	class Vessels {
		Vessels(const Vessels&);
	public:
		// _vessel will be part of the instance Vessels. It will be freed automatically.
		Vessels(Vessel *_vessel);
		Q_DLL Vessels(Vessels *_vessels);
		Q_DLL Vessels(const q::qString &_fileName);
		Vessels(const PtList &_ptList);
		Q_DLL ~Vessels(void);
		
		Q_DLL void Save(const q::qString &_fileName);

		void DestroyAndCopyFrom(Vessels *_vessels);

		inline Vessel* GetRoot(void) const {
			return m_Root;
		}

		inline void AddVessel(Vessel *_newVessel, Vessel *_vessel){
			qAssert(_newVessel != 0);
			m_CompleteVesselList.push_back(_newVessel);
			AddLink(_newVessel, _vessel);
		}

		// be careful, when you remove a vessel, some vessel branch linked to this vessel can be unlinked from the root also. There is no check.
		inline void RemoveVessel(Vessel *_vessel){
			Vessel::VesselList::iterator it = std::find(m_CompleteVesselList.begin(), m_CompleteVesselList.end(), _vessel);
			qAssert(it != m_CompleteVesselList.end());
			if(it != m_CompleteVesselList.end()){
				Vessel *tmp = *it;
				tmp->UnlinkAll();
				m_CompleteVesselList.erase(it);
				SAFE_DELETE_UNIQUE(tmp);
			}
		}

		inline void AddLink(Vessel *_v1, Vessel *_v2){
			qAssert(std::find(m_CompleteVesselList.begin(), m_CompleteVesselList.end(), _v1) != m_CompleteVesselList.end());
			qAssert(std::find(m_CompleteVesselList.begin(), m_CompleteVesselList.end(), _v2) != m_CompleteVesselList.end());
			qAssert(std::find(_v1->m_VesselList.begin(), _v1->m_VesselList.end(), _v2) == _v1->m_VesselList.end());
			qAssert(std::find(_v2->m_VesselList.begin(), _v2->m_VesselList.end(), _v1) == _v2->m_VesselList.end());
			_v1->m_VesselList.push_back(_v2);
			_v2->m_VesselList.push_back(_v1);
		}

		inline void RemoveLink(Vessel *_v1, Vessel *_v2){
			qAssert(std::find(m_CompleteVesselList.begin(), m_CompleteVesselList.end(), _v1) != m_CompleteVesselList.end());
			qAssert(std::find(m_CompleteVesselList.begin(), m_CompleteVesselList.end(), _v2) != m_CompleteVesselList.end());
			qAssert(std::find(_v1->m_VesselList.begin(), _v1->m_VesselList.end(), _v2) != _v1->m_VesselList.end());
			qAssert(std::find(_v2->m_VesselList.begin(), _v2->m_VesselList.end(), _v1) != _v2->m_VesselList.end());
			_v1->Unlink(_v2);
			_v2->Unlink(_v1);
		}

		inline void RemoveAllLink(Vessel *_v){
			_v->UnlinkAll();
		}

		inline void ResetAllFlags(void){
			for(Vessel::VesselList::iterator it = m_CompleteVesselList.begin(); it != m_CompleteVesselList.end(); ++it){
				(*it)->m_Flag = Q_FALSE;
			}
		}

		inline void ResetAllFlags2(void){
			for(Vessel::VesselList::iterator it = m_CompleteVesselList.begin(); it != m_CompleteVesselList.end(); ++it){
				(*it)->m_Flag2 = Q_FALSE;
			}
		}

		inline void SetAllData(qsize_t _dataId, qf64 _value){
			for(Vessel::VesselList::iterator it = m_CompleteVesselList.begin(); it != m_CompleteVesselList.end(); ++it){
				(*it)->m_Data[_dataId] = _value;
			}
		}

		inline void CopyAllData(qsize_t _dataIdDest, qsize_t _dataIdSrc){
			for(Vessel::VesselList::iterator it = m_CompleteVesselList.begin(); it != m_CompleteVesselList.end(); ++it){
				(*it)->m_Data[_dataIdDest] = (*it)->m_Data[_dataIdSrc];
			}
		}

		inline qf64 SumAllData(qsize_t _dataId){
			qf64 sum = 0.;
			for(Vessel::VesselList::iterator it = m_CompleteVesselList.begin(); it != m_CompleteVesselList.end(); ++it){
				sum = sum + (*it)->m_Data[_dataId];
			}
			return sum;
		}

		inline void Transform(const q::Matrix44 &_mat, qbool _divideByHomogeneousCoordinate = Q_FALSE){
			for(Vessel::VesselList::iterator it = m_CompleteVesselList.begin(); it != m_CompleteVesselList.end(); ++it){
				Vessel *v = (*it);
				Vector4 position = Vector4(v->m_Data[Vessel::INFO_X], v->m_Data[Vessel::INFO_Y], v->m_Data[Vessel::INFO_Z], 1.0);
				position = _mat*position;
				if(_divideByHomogeneousCoordinate == Q_TRUE){
					position[AXE_X] = position[AXE_X]/position[3];
					position[AXE_Y] = position[AXE_Y]/position[3];
					position[AXE_Z] = position[AXE_Z]/position[3];
				}
				v->m_Data[Vessel::INFO_X] = position[AXE_X];
				v->m_Data[Vessel::INFO_Y] = position[AXE_Y];
				v->m_Data[Vessel::INFO_Z] = position[AXE_Z];
			}
		}

		inline void ComputeVesselsDirection(void){
			ResetAllFlags();
			ComputeVesselDirectionRecursively(GetRoot(), NULL);
		}

		Vessel* GetClosestVesselFrom(const Vector2 &_v);
		Vessel* GetClosestVesselFrom(const Vector3 &_v);

		inline q::qsize_t GetNbVessel(void){
			return m_CompleteVesselList.size();
		}

		// the following functions work only with tree (connected graph without cycles)
		// warning no check is done
		q::qu32 GetNbOfParts(void);
		void GetVesselPart(q::qu32 _selectedPartId, q::qs32 &_previousPartId, PtList &_ptList);
		void GetVesselPart(q::qu32 _selectedPartId, q::qs32 &_previousPartId, Vessels* &_vessels);
		// these functions are used when we're reconstructing the vessels tree. Not all vessel part are there.
		// So we use the m_Flag2 to know when we have bifurcation and thus get specific vessel part
		q::Vessel* GetEndingVesselPart(q::qu32 _selectedPartId);
		void AddPart(q::qs32 nextPartLinkedFrom, const PtList &_ptList);
		void AddPartAuto(const PtList &_ptList);

		Vessel* GetVesselId(qu32 _id);
		qu32 GetVesselId(Vessel *_v);
		Q_DLL qu32 GetPreviousVesselId(qu32 _id, qu32 _offset);
		// warning: it works only if we put the id of the vessels in m_Data[Vessel::INFO_MISC_6]
		void Get3dVesselsBranchesConst(qu32 _id, qbool _getBranchUntilOneLeaf, PtList &_ptList) const;
		void Get3dVesselsBranches(qu32 _id, qbool _getBranchUntilOneLeaf, PtList &_ptList);
		void Get3dVesselsBranches(qu32 _id, qbool _getBranchUntilOneLeaf, Vessels* &_vessels);
		Q_DLL void Get3dVesselsSubTree(qu32 _id, qbool _keepPathToTheRoot, Vessels* &_vessels);
		void GetNeighborhoodVessels(qu32 _id, qu32 _nbPtsAfter, qu32 _nbPtsBefore, Vessels* &_vessels);
		qu32 GetTheFirstLeafIdCloseTo(qu32 _id);
		qbool IsVesselInsideThisPath(qu32 _vesselId, qu32 _pathLeafId);

		void ComputePartType(void);
		void PropagatePartType(Vessel *_v, qf64 _type);

		//q::qu32 GetIdFromVesselPointer(q::Vessel *_v);

		q::qu32 GetNumberOfPartBtwTwoVessels(q::Vessel *_v1, q::Vessel *_v2);

		void TagIdInData(qu32 _dataId);

		Q_DLL void GetPtList(PtList &_ptList);
		void GetLinkPtList(PtList &_ptList);
				
		void SetVesselsValue(qsize_t _dataId, std::vector<qf64> &_valueList);
		void GetVesselsValue(qsize_t _dataId, std::vector<qf64> &_valueList);
		void GetSortedVesselsValue(qsize_t _dataId, std::multimap<qf64, Vessel*> &_sortedList);

#ifdef NEED_RESAMPLE
		Q_DLL void Resample(Vessels* &_vessels, qf64 _blurSigma = 1.0, qf64 _sampleDistance = 1.0);
#endif
	private:
		Vessels(void) : m_Root(NULL) {
		}

		void DestroyAll(void);

		void ComputeVesselDirectionRecursively(q::Vessel *_vessel, q::Vessel *_previousVessel);
		// work only with tree (connected graph without cycles)
		// warning no check is done
		q::qu32 GetNbOfPartsRecursively(q::Vessel *_vessel);
		q::Vessel* GetVesselPart(q::qu32 _selectedPartId, q::qs32 &_previousPartId, Vessel* &_previousVessel);
		q::Vessel* GetVesselPartRecursively(q::Vessel *_vessel, q::qu32 _selectedPartId, q::qu32 &_currentPartId, q::qs32 &_previousPartId, Vessel* &_previousVessel);
		// this function is used when we're reconstructing the vessels tree. Not all vessel part are there.
		// So we use the m_Flag2 to know when we have bifurcation and thus get specific vessel part
		q::Vessel* GetEndingVesselPartRecursively(Vessel *_vessel, q::qu32 _selectedPartId, q::qu32 &_currentPartId, q::qs32 &_previousPartId, Vessel* &_previousVessel);

		Vessel* GetVesselIdRecursively(Vessel *_v, qu32 _id, qu32 &_inc);
		qbool GetVesselIdRecursively(Vessel *_v, Vessel *_lookForV, qu32 &_inc);
		qu32 GetPreviousVesselIdRecursively(Vessel *_v, qu32 _id, qu32 &_offset, qu32 &_inc);
		qbool Get3dVesselsBranchesConstRecursively(Vessel *_v, qu32 _id, qbool _found
											, qbool _getBranchUntilOneLeaf, PtList &_ptList) const;
		qbool Get3dVesselsBranchesRecursively(Vessel *_v, qu32 _id, qu32 &_inc, qbool _found
											, qbool _getBranchUntilOneLeaf, Vessels* &_vessels
											, Vessel* &_previousVessel);
		qbool Get3dVesselsSubTreeRecursively(Vessel *_v, qu32 _id, qu32 &_inc, qbool _found
											, qbool _keepPathFromTheRoot, Vessels* &_vessels
											, Vessel* &_previousVesselUp, Vessel *_previousVesselDown);
		
		void CopyAndLinkNeighborhoodVesselsRecursively(Vessels *_vessels, Vessel *_vesselDest,  Vessel *_vesselSource, qu32 _nbPts);
		qu32 GetTheFirstLeafIdCloseToRecursively(Vessel *_v, qu32 _id, qu32 &_inc, qbool &_found);
		qbool IsVesselInsideThisPathRecursively(Vessel *_v, qu32 _vesselId, qu32 _pathLeafId, qu32 &_inc, qbool &_found);
		
		void ComputePartTypeRecursively(Vessel *_v, qu32 &_inc);
		void PropagatePartTypeRecursively(Vessel *_v, Vessel *_selectedVessel, qf64 _type, qu32 &_part, qs32 &_goodPart);

		//q::qu32 GetIdFromVesselPointerRecursively(q::Vessel *_lookForV, q::Vessel *_v, qu32 &_inc);
		
		q::qu32 GetNumberOfPartBtwTwoVesselsRecursively(q::Vessel *_v1, q::Vessel *_v2, q::qbool &_found, q::qbool &_iAmTheOne);

		void TagIdInDataRecursively(Vessel *_v, qu32 &_inc, qu32 _dataId);
		
		void GetPtListRecursively(Vessel *_vessel, PtList &_ptList);
		void GetLinkPtListRecursively(Vessel *_vessel, PtList &_ptList);
		
		void SetVesselsValueRecursively(Vessel *_v, qsize_t _dataId, std::vector<qf64>::iterator &_itValue);
		void GetVesselsValueRecursively(Vessel *_v, qsize_t _dataId, std::vector<qf64> &_valueList);
	public:
		Vessel::VesselList m_CompleteVesselList;

	private:
		Vessel *m_Root;
	};
END_Q_NAMESPACE

#endif
