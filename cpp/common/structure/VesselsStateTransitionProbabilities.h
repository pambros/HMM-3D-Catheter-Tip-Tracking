#ifndef __Q_VESSELS_STATE_TRANSITION_
#define __Q_VESSELS_STATE_TRANSITION_
#include "common/util/Util.h"
#include "common/structure/Vessels.h"

#define USE_LOG_PROBA
#ifdef USE_LOG_PROBA
	#define MIN_PROBA (MIN_LOG_NUMBER)
	#define MAX_PROBA (0.) // log(1) = 0
#else
	#define MIN_PROBA (0.)
	#define MAX_PROBA (1.)
#endif

BEGIN_Q_NAMESPACE
	class VesselsStateTransitionProbabilities{
	public:
		enum TRANSITION_METRIC_ENUM{
			TRANSITION_METRIC_GAUSSIAN_A_PRIME = 0,
			TRANSITION_METRIC_BINARY_A_DOUBLE_PRIME = 1,
			TRANSITION_METRIC_COUNT
		};

		struct Transition{
			qu32 m_Id;
			qu32 m_TreeId;
			qf64 m_GaussianProba;
#ifdef USE_LOG_PROBA
			qf64 m_LogGaussianProba;
#endif

			Transition(qu32 _id, qu32 _treeId, qf64 _gaussianProba) : m_Id(_id)
				, m_TreeId(_treeId)
				, m_GaussianProba(_gaussianProba){
#ifdef USE_LOG_PROBA
				m_LogGaussianProba = specialLog(m_GaussianProba);
#endif
			}
		};

		typedef std::vector<Transition> TransitionList;

		struct VesselDistribution{
			TransitionList m_TransitionList;
		};

		typedef std::vector<VesselDistribution*> VesselDistributionList;

		VesselsStateTransitionProbabilities(const qString &_fileName);
		VesselsStateTransitionProbabilities(Vessels *_vessels, qu32 _transitionMetric, qf64 _sigma, qf64 _probaMin);
		~VesselsStateTransitionProbabilities(void);

		Q_DLL void Save(const qString &_fileName);

	public:
		std::vector<qu32> m_IdToTreeId;
		// same order as vessel list
		VesselDistributionList m_CompleteVesselList;
		// same order as id in the tree
		//VesselDistributionList m_CompleteVesselIdSortedList;
		// the transition here is from a state to the one in VesselDistribution
		VesselDistributionList m_CompleteVesselListFromState;
	};
END_Q_NAMESPACE

#endif
