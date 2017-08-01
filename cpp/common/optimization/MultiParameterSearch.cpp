#include "MultiParameterSearch.h"
#include "common/util/Util.h"
#include "common/maths/Maths.h"

#ifdef MULTI_PARAMETER_SEARCH_USE_THREAD
	#include "common/util/Thread.h"

	#define SMALL_SLEEP_TIME (10) // in ms
#endif

//#define __DEBUG_PRINT 1
#ifdef __DEBUG_PRINT
	#define Q_PRINT_DEBUG(...) q::qPrint(__VA_ARGS__)
#else
	#define Q_PRINT_DEBUG(...)
#endif

using namespace std;

BEGIN_Q_NAMESPACE

#ifdef MULTI_PARAMETER_SEARCH_USE_THREAD

enum {
	STEP_START = 0,
	STEP_FINISHED = 1,
	STEP_END
};

class MultiParameterSearchThread : public qThread{
public:
	inline MultiParameterSearchThread() : m_Step(STEP_FINISHED)
										, m_BestFittingMetric(UTIL_BIG_NEGATIVE_NUMBER)
										, m_BestPos(NULL)
										, m_MutexIsSetting(NULL)
										, m_MultiParameterSearch(NULL)
										, m_Id(0)
										, m_CenterPos(NULL)
										, m_Interval(NULL){
	}
	inline ~MultiParameterSearchThread(void){
	}

	inline void SetParameters(qu32 _id, MultiParameterSearch* _multiParameterSearch, qMutex *_mutexIsSetting
			, qf64 *_centerPos, qf64 *_interval){
		m_Id = _id;
		m_MultiParameterSearch = _multiParameterSearch;
		m_MutexIsSetting = _mutexIsSetting;
		m_CenterPos = _centerPos;
		m_Interval = _interval;
	}

private:
	void run(void);
	void WaitForParent(void);

public:
	qu32 m_Step;
	qf64 m_BestFittingMetric;
	qf64 *m_BestPos;

private:
	qMutex *m_MutexIsSetting;
	MultiParameterSearch *m_MultiParameterSearch;
	qu32 m_Id;
	qf64 *m_CenterPos;
	qf64 *m_Interval;
};

void MultiParameterSearchThread::WaitForParent(void){
	qbool bTrue = Q_TRUE;
	while(bTrue){
		{
			//qMutexLocker mutexLocker(m_MutexIsSetting);
			if(m_Step != STEP_FINISHED){
				// for debug only
				/*if(m_Id == 4){
					break;
				}*/
				break;
			}
		}
		qSleep(SMALL_SLEEP_TIME);
	}
}

void MultiParameterSearchThread::run(void){
	Q_PRINT_DEBUG("MultiParameterSearchThread::run %d\n", m_Id);

	qu32 nbParameters = static_cast<qu32>(m_MultiParameterSearch->m_Parameters->m_IntervalList.size());
	m_BestPos = Q_NEW qf64[nbParameters];
	qf64 *currentPos = Q_NEW qf64[nbParameters];
	qu32 *id = Q_NEW qu32[nbParameters];
	std::vector<qu32> &nbSamplingList = m_MultiParameterSearch->m_Parameters->m_NbSamplingList;

	qu32 limitSamplingFirstParameter = nbSamplingList[0]/NB_USED_PROCS;
	qu32 id0 = nbSamplingList[0]%NB_USED_PROCS;
	if(m_Id < id0){
		limitSamplingFirstParameter++;
		id0 = m_Id*limitSamplingFirstParameter;
	}
	else{
		// original to understand
		// id0 = id0*(limitSamplingFirstParameter + 1) + (m_Id - id0)*limitSamplingFirstParameter;
		// after simplification
		id0 = id0 + m_Id*limitSamplingFirstParameter;
	}
	limitSamplingFirstParameter = id0 + limitSamplingFirstParameter;
	Q_PRINT_DEBUG("id[0] %d, limit %d, threadId %d\n", id0, limitSamplingFirstParameter, m_Id);

	WaitForParent();

	memcpy(currentPos, m_CenterPos, nbParameters*sizeof(*currentPos));

	while(m_Step != STEP_END){
		qMemset(id, 0, nbParameters*sizeof(*id));
		id[0] = id0;
		
		qu32 currentParam = nbParameters;
		while(currentParam != 0){
			for(qu32 j = 0; j < nbParameters; ++j){
				if(nbSamplingList[j] > 1){
					currentPos[j] = (m_CenterPos[j] - m_Interval[j]/2.) + id[j]*m_Interval[j]/(nbSamplingList[j] - 1);
				}
			}

			qf64 currentFittingMetric = m_MultiParameterSearch->m_Parameters->m_MetricCallback(currentPos, m_Id);

			if(currentFittingMetric > m_BestFittingMetric){
				m_BestFittingMetric = currentFittingMetric;
				memcpy(m_BestPos, currentPos, nbParameters*sizeof(*m_BestPos));
				/*for(qu32 i = 0; i < nbParameters; ++i){
					m_BestPos[i] = currentPos[i];
				}*/
			}

			// iterations (same as nbParameters "for loop")
			currentParam = nbParameters;
			while(currentParam != 0){
				id[currentParam - 1] = id[currentParam - 1] + 1;
				if((currentParam - 1 != 0 && id[currentParam - 1] < nbSamplingList[currentParam - 1])
					|| (currentParam - 1 == 0 && id[0] < limitSamplingFirstParameter)){
					break;
				}

				id[currentParam - 1] = 0;
				--currentParam;
			}
		}

		{
			qMutexLocker mutexLocker(m_MutexIsSetting);
			m_Step = STEP_FINISHED;
		}

		WaitForParent();
	}

	Q_DELETE id;
	Q_DELETE currentPos;
	SAFE_DELETE(m_BestPos);
}

void MultiParameterSearch::Apply(void){
	qMutex m_MutexIsSetting[NB_USED_PROCS];
	for(qu32 i = 0; i < NB_USED_PROCS; ++i){
		m_MutexIsSetting[i].lock();
	}

	qu32 nbIterations = m_Parameters->m_NbIterations;
	qu32 nbParameters = static_cast<qu32>(m_Parameters->m_IntervalList.size());
	qf64 *centerPos = Q_NEW qf64[nbParameters];
	qf64 *interval = Q_NEW qf64[nbParameters];
	qf64 bestFittingMetric = UTIL_BIG_NEGATIVE_NUMBER;
	qu32 bestThreadId = 0;

	MultiParameterSearchThread *thread = Q_NEW MultiParameterSearchThread[NB_USED_PROCS];
	for(qu32 i = 0; i < NB_USED_PROCS; ++i){
		thread[i].SetParameters(i, this, &m_MutexIsSetting[i], centerPos, interval);
		thread[i].start();
	}

	for(qu32 i = 0; i < nbParameters; ++i){
		centerPos[i] = m_Parameters->m_OffsetList[i];
		interval[i] = m_Parameters->m_IntervalList[i];
	}

	while(nbIterations != 0){
		for(qu32 i = 0; i < NB_USED_PROCS; ++i){
			thread[i].m_Step = STEP_START;
		}
		
		for(qu32 i = 0; i < NB_USED_PROCS; ++i){
			m_MutexIsSetting[i].unlock();
		}

		// wait for the work
		qu32 nbThreadFinished = 0;
		do{
			for(nbThreadFinished = 0; nbThreadFinished < NB_USED_PROCS; ++nbThreadFinished){
				{
					//qMutexLocker mutexLocker(&m_MutexIsSetting[nbThreadFinished]);
					if(thread[nbThreadFinished].m_Step != STEP_FINISHED){
						break;
					}
				}
			}
			qMainThreadSleep(SMALL_SLEEP_TIME);
		}while(nbThreadFinished != NB_USED_PROCS);

		for(qu32 i = 0; i < NB_USED_PROCS; ++i){
			m_MutexIsSetting[i].lock();
		}

		// gather data from all threads
		for(qu32 i = 0; i < NB_USED_PROCS; ++i){
			if(thread[i].m_BestFittingMetric > bestFittingMetric){
				bestFittingMetric = thread[i].m_BestFittingMetric;
				bestThreadId = i;
				// new center and interval
				memcpy(centerPos, thread[bestThreadId].m_BestPos, nbParameters*sizeof(*centerPos));
#ifdef __DEBUG_PRINT
				Q_PRINT_DEBUG("bestFittingMetric %f iter %d id", bestFittingMetric, nbIterations);
				Q_PRINT_DEBUG(" pos");
				for(qu32 i = 0; i < nbParameters; ++i){
					Q_PRINT_DEBUG(" %f", centerPos[i]);
				}
				Q_PRINT_DEBUG("\n");
#endif
			}
		}

		// new center and interval
		/*for(qu32 i = 0; i < nbParameters; ++i){
			centerPos[i] = thread[bestThreadId].m_BestPos[i];
		}*/
		for(qu32 i = 0; i < nbParameters; ++i){
			interval[i] = interval[i]*m_Parameters->m_ReductionCoeff;
		}
		--nbIterations;
	}

	// output
	memcpy(m_BestPost, centerPos, nbParameters*sizeof(*m_BestPost));
	/*for(qu32 i = 0; i < nbParameters; ++i){
		m_BestPost[i] = centerPos[i];
	}*/
	m_BestFittingMetric = bestFittingMetric;

	SAFE_DELETE(interval);
	SAFE_DELETE(centerPos);

	// wait for all threads and delete them
	for(qu32 i = 0; i < NB_USED_PROCS; ++i){
		thread[i].m_Step = STEP_END;
	}

	for(qu32 i = 0; i < NB_USED_PROCS; ++i){
		m_MutexIsSetting[i].unlock();
	}

	for(qu32 i = 0; i < NB_USED_PROCS; ++i){
		thread[i].wait();
	}
	SAFE_DELETE(thread);
}

#else

void MultiParameterSearch::Apply(void){
	qu32 nbIterations = m_Parameters->m_NbIterations;
	qu32 nbParameters = static_cast<qu32>(m_Parameters->m_IntervalList.size());
	qf64 *currentPos = Q_NEW qf64[nbParameters];
	qf64 *bestPos = Q_NEW qf64[nbParameters];
	qf64 *centerPos = Q_NEW qf64[nbParameters];
	qf64 *interval = Q_NEW qf64[nbParameters];
	qf64 bestFittingMetric = UTIL_BIG_NEGATIVE_NUMBER;
	qu32 *id = Q_NEW qu32[nbParameters];

	for(qu32 i = 0; i < nbParameters; ++i){
		centerPos[i] = m_Parameters->m_OffsetList[i];
		//currentPos[i] = centerPos[i];
		interval[i] = m_Parameters->m_IntervalList[i];
	}
	memcpy(currentPos, centerPos, nbParameters*sizeof(*currentPos));

	while(nbIterations != 0){
		qMemset(id, 0, nbParameters*sizeof(*id));
		
		qu32 currentParam = nbParameters;
#ifdef __DEBUG_PRINT
		qu32 currentI1 = 0;
#endif
		while(currentParam != 0){
			for(qu32 j = 0; j < nbParameters; ++j){
				if(m_Parameters->m_NbSamplingList[j] > 1){
					currentPos[j] = (centerPos[j] - interval[j]/2.) + id[j]*interval[j]/(m_Parameters->m_NbSamplingList[j] - 1);
				}
			}
#ifdef __DEBUG_PRINT
			if(currentI1 != id[1]){
				Q_PRINT_DEBUG("iter %d id", nbIterations);
				for(qu32 i = 0; i < nbParameters; ++i){
					Q_PRINT_DEBUG(" %d", id[i]);
				}
				Q_PRINT_DEBUG(" pos");
				for(qu32 i = 0; i < nbParameters; ++i){
					Q_PRINT_DEBUG(" %f", currentPos[i]);
				}
				Q_PRINT_DEBUG("\n");
				currentI1 = id[0];
			}
#endif

			qf64 currentFittingMetric = m_Parameters->m_MetricCallback(currentPos);

			if(currentFittingMetric > bestFittingMetric){
				bestFittingMetric = currentFittingMetric;
				memcpy(bestPos, currentPos, nbParameters*sizeof(*bestPos));
				/*for(qu32 i = 0; i < nbParameters; ++i){
					bestPos[i] = currentPos[i];
				}*/
#ifdef __DEBUG_PRINT
				Q_PRINT_DEBUG("bestFittingMetric %f iter %d id", bestFittingMetric, nbIterations);
				for(qu32 i = 0; i < nbParameters; ++i){
					Q_PRINT_DEBUG(" %d", id[i]);
				}
				Q_PRINT_DEBUG(" pos");
				for(qu32 i = 0; i < nbParameters; ++i){
					Q_PRINT_DEBUG(" %f", currentPos[i]);
				}
				Q_PRINT_DEBUG("\n");
#endif
			}

			// iterations (same as nbParameters "for loop")
			currentParam = nbParameters;
			while(currentParam != 0){
				id[currentParam - 1] = id[currentParam - 1] + 1;
				if(id[currentParam - 1] < m_Parameters->m_NbSamplingList[currentParam - 1]){
					break;
				}

				id[currentParam - 1] = 0;
				--currentParam;
			}
		}

		// new center and interval
		memcpy(centerPos, bestPos, nbParameters*sizeof(*centerPos));
		/*for(qu32 i = 0; i < nbParameters; ++i){
			centerPos[i] = bestPos[i];
		}*/
		for(qu32 i = 0; i < nbParameters; ++i){
			interval[i] = interval[i]*m_Parameters->m_ReductionCoeff;
		}
		--nbIterations;
	}

	//Q_PRINT_DEBUG("MultiParameterSearch:: delete\n");
	Q_DELETE id;
	Q_DELETE interval;
	Q_DELETE centerPos;
	Q_DELETE currentPos;

	// output
	//Q_PRINT_DEBUG("MultiParameterSearch:: output\n");
	memcpy(m_BestPost, bestPos, nbParameters*sizeof(*m_BestPost));
	/*for(qu32 i = 0; i < nbParameters; ++i){
		m_BestPost[i] = bestPos[i];
	}*/
	m_BestFittingMetric = bestFittingMetric;
}
#endif

END_Q_NAMESPACE
