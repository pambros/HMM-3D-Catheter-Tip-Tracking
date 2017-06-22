#ifndef __MULTI_PARAMETER_SEARCH_HEADER_
#define __MULTI_PARAMETER_SEARCH_HEADER_
#include "common/util/Util.h"
#include "thirdParties/fastDelegate/FastDelegate.h"
#include "common/util/Thread.h"

#include <vector>

#ifdef Q_USE_THREAD
	#define MULTI_PARAMETER_SEARCH_USE_THREAD 1
	#define NB_USED_PROCS NB_PROCS
#endif

BEGIN_Q_NAMESPACE

class MultiParameterSearch{
public:
#ifdef MULTI_PARAMETER_SEARCH_USE_THREAD
	friend class MultiParameterSearchThread;
	typedef fastdelegate::FastDelegate2<const qf64*, qu32, qf64> MetricCallback;
#else
	typedef fastdelegate::FastDelegate1<const qf64*, qf64> MetricCallback;
#endif

	struct Parameters{
		std::vector<qf64> m_OffsetList;
		std::vector<qf64> m_IntervalList;
		std::vector<qu32> m_NbSamplingList;
		qf64 m_ReductionCoeff;
		qu32 m_NbIterations;
		std::vector<qf64> m_EpsilonLimitList;
		MetricCallback m_MetricCallback;
	};

	// _parameters will be destroyed with the instance MultiParameter2D3DRegistration
	MultiParameterSearch(Parameters *_parameters) : m_Parameters(_parameters){												
		m_BestPost = Q_NEW qf64[_parameters->m_IntervalList.size()];
	}

	~MultiParameterSearch(void){
		SAFE_DELETE_UNIQUE(m_Parameters);
		SAFE_DELETE(m_BestPost);
	}

	void Apply(void);
private:
	// input
	Parameters *m_Parameters;

public:
	// output
	qf64 *m_BestPost;
	qf64 m_BestFittingMetric;
};

END_Q_NAMESPACE

#endif
