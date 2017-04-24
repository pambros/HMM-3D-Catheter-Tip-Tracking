#ifndef __SHAPE_SIMILARITY_HEADER_
#define __SHAPE_SIMILARITY_HEADER_
#include "common/util/Util.h"
#include "common/maths/Matrix.h"
#include "common/structure/Vessels.h"
#include "common/structure/PtList.h"

#include <map>

BEGIN_Q_NAMESPACE
	class ShapeSimilarity{
	public:
		struct Parameters{
			Matrix44 m_CArmProjection;
			Matrix44 m_WorldToCArm;
			// in [0, 1]
			qf64 m_RatioNbPtCatheter;
			qu32 m_NbRankPathToRegister_k;
			std::vector<qf64> m_ShapeSimilarityTree;
		};

		static const q::qf64 DEFAULT_RATIO_NB_PT_CATHETER;
		static const q::qu32 DEFAULT_NB_RANK_PATH_TO_REGISTER;

		// _parameters will be destroyed with the instance ShapeSimilarity
		ShapeSimilarity(Parameters *_parameters);
		ShapeSimilarity(const qString &_fileName);
		inline virtual ~ShapeSimilarity(void){
			SAFE_DELETE_UNIQUE(m_Parameters);
		}

		void Apply(const PtList &_2dCatheter, Vessels &_3dVessels);

		union VesselId {
			struct {
				qu16 tipVesselId;
				qu16 tipPartId;
				qu16 leafVesselId;
				qu16 leafPartId;
			};
			qu64 asU64;
		};
		//typedef std::map<qf64, VesselId> ScoresList;
		typedef std::multimap<qf64, VesselId> ScoresList;
	private:
		void Init(void);

		qf64 GetBestPathRecursively(Vessel *_v, qu32 &_inc, ScoresList &_scores, std::map<qu32, qf64> &_partScores, qu32 &_part);
		void GetBestPath(Vessels &_3dVessels, ScoresList &_scores);
		void ComputeVesselDirectionRecursively(Vessel *_vessel, Vessel *_previousVessel);
		void ComputeVesselsDirection(Vessels &_3dVessels);

		// temporary
		qsize_t m_Nb2dPts;
		Vector2 *m_Direction2dCatheter;
		qf64 *m_CumulDistance2dCatheter;

	public:
		// input
		Parameters *m_Parameters;

		// output
		ScoresList m_ScoresList;
	};
END_Q_NAMESPACE

#endif
