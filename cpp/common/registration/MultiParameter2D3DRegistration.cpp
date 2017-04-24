#include "MultiParameter2D3DRegistration.h"
#include "common/util/UtilTime.h"
#include "common/util/File.h"
#include "common/optimization/MultiParameterSearch.h"

//#define __DEBUG_PRINT 1
#ifdef __DEBUG_PRINT
	#define Q_PRINT_DEBUG(...) q::qPrint(__VA_ARGS__)
#else
	#define Q_PRINT_DEBUG(...)
#endif

using namespace std;

BEGIN_Q_NAMESPACE

MultiParameter2D3DRegistration::MultiParameter2D3DRegistration(Parameters *_parameters) : m_Parameters(_parameters){
	Init();
}

MultiParameter2D3DRegistration::MultiParameter2D3DRegistration(const qString &_fileName){
	Init();

	m_Parameters = Q_NEW Parameters();

	qFile *file = NULL;
	try{
		qu32 err = qFOpen(file, _fileName.c_str(), "rb");
		if(err == 0){
			throw gDefaultException;
		}
		/*qu32 iErr =*/ Base2D3DRegistration::LoadParameters(file);
		/*iErr =*/ FileReachLine(file, qString("#\tMultiParameter2D3DParameters"));
		/*iErr =*/ FileReadF64(file, m_Parameters->m_OffsetX);
		/*iErr =*/ FileReadF64(file, m_Parameters->m_OffsetY);
		/*iErr =*/ FileReadF64(file, m_Parameters->m_OffsetZ);
		/*iErr =*/ FileReadF64(file, m_Parameters->m_OffsetAlpha);
		/*iErr =*/ FileReadF64(file, m_Parameters->m_OffsetBeta);
		/*iErr =*/ FileReadF64(file, m_Parameters->m_OffsetGamma);
		/*iErr =*/ FileReadF64(file, m_Parameters->m_IntervalX);
		/*iErr =*/ FileReadF64(file, m_Parameters->m_IntervalY);
		/*iErr =*/ FileReadF64(file, m_Parameters->m_IntervalZ);
		/*iErr =*/ FileReadF64(file, m_Parameters->m_IntervalAlpha);
		/*iErr =*/ FileReadF64(file, m_Parameters->m_IntervalBeta);
		/*iErr =*/ FileReadF64(file, m_Parameters->m_IntervalGamma);
		/*iErr =*/ FileReadU32(file, m_Parameters->m_NbSamplingX);
		/*iErr =*/ FileReadU32(file, m_Parameters->m_NbSamplingY);
		/*iErr =*/ FileReadU32(file, m_Parameters->m_NbSamplingZ);
		/*iErr =*/ FileReadU32(file, m_Parameters->m_NbSamplingAlpha);
		/*iErr =*/ FileReadU32(file, m_Parameters->m_NbSamplingBeta);
		/*iErr =*/ FileReadU32(file, m_Parameters->m_NbSamplingGamma);
		/*iErr =*/ FileReadF64(file, m_Parameters->m_ReductionCoeff);
		/*iErr =*/ FileReadU32(file, m_Parameters->m_NbIterations);
	}
	catch(q::qDefaultException){
		q::qPrint("MultiParameter2D3DRegistration::MultiParameter2D3DRegistration Error during loading\n");
		qFClose(file);
		throw gDefaultException;
	}

	qFClose(file);
}

void MultiParameter2D3DRegistration::Apply(const q::PtList &_2dCatheter, const q::PtList &_3dVessels){
	//START_CHRONO(multiresolutionChrono);
#ifdef MULTI_PARAMETER_SEARCH_USE_THREAD
	qu32 nbProcUsed = NB_USED_PROCS;
#else
	qu32 nbProcUsed = 1;
#endif
	PreApply(_2dCatheter, _3dVessels, nbProcUsed);

	MultiParameterSearch::Parameters *parameters = Q_NEW MultiParameterSearch::Parameters;
	parameters->m_OffsetList.push_back(m_Parameters->m_OffsetX);
	parameters->m_OffsetList.push_back(m_Parameters->m_OffsetY);
	parameters->m_OffsetList.push_back(m_Parameters->m_OffsetZ);
	parameters->m_OffsetList.push_back(m_Parameters->m_OffsetAlpha);
	parameters->m_OffsetList.push_back(m_Parameters->m_OffsetBeta);
	parameters->m_OffsetList.push_back(m_Parameters->m_OffsetGamma);

	parameters->m_IntervalList.push_back(m_Parameters->m_IntervalX);
	parameters->m_IntervalList.push_back(m_Parameters->m_IntervalY);
	parameters->m_IntervalList.push_back(m_Parameters->m_IntervalZ);
	parameters->m_IntervalList.push_back(m_Parameters->m_IntervalAlpha);
	parameters->m_IntervalList.push_back(m_Parameters->m_IntervalBeta);
	parameters->m_IntervalList.push_back(m_Parameters->m_IntervalGamma);

	parameters->m_NbSamplingList.push_back(m_Parameters->m_NbSamplingX);
	parameters->m_NbSamplingList.push_back(m_Parameters->m_NbSamplingY);
	parameters->m_NbSamplingList.push_back(m_Parameters->m_NbSamplingZ);
	parameters->m_NbSamplingList.push_back(m_Parameters->m_NbSamplingAlpha);
	parameters->m_NbSamplingList.push_back(m_Parameters->m_NbSamplingBeta);
	parameters->m_NbSamplingList.push_back(m_Parameters->m_NbSamplingGamma);

	parameters->m_ReductionCoeff = m_Parameters->m_ReductionCoeff;
	parameters->m_NbIterations = m_Parameters->m_NbIterations;

	parameters->m_EpsilonLimitList.push_back(0.1);
	parameters->m_EpsilonLimitList.push_back(0.1);
	parameters->m_EpsilonLimitList.push_back(0.1);
	parameters->m_EpsilonLimitList.push_back(0.1);
	parameters->m_EpsilonLimitList.push_back(0.1);
	parameters->m_EpsilonLimitList.push_back(0.1);

	{
		qbool ddmParamMetric = m_Parameters->m_MetricUsed == METRIC_IMPROVED_FROM_CATHETER ? Q_TRUE : Q_FALSE;
		qbool ddmParamWeight = m_Parameters->m_UseWeight;
		qu32 ddmParamNorm = m_Parameters->m_DistanceUsed;
		qu32 ddmParamRadius = m_Parameters->m_RadiusMetric;
	#define DDM_FUNCTION_DELEGATE parameters->m_MetricCallback
	#define DDM_CLASS_NAME MultiParameter2D3DRegistration
	#ifdef MULTI_PARAMETER_SEARCH_USE_THREAD
		#define DDM_PARAM_MULTITHREAD MultiThread
		#define DDM_MULTITHREAD
	#else
		#define DDM_PARAM_MULTITHREAD
	#endif
	#define DDM_PARAM_LIMIT // no need to limit with brute force as we test all the possibilities in a specific range
		#include "CallDistanceMetric.h"
	#undef DDM_PARAM_LIMIT
	#undef DDM_MULTITHREAD
	#undef DDM_PARAM_MULTITHREAD
	#undef DDM_CLASS_NAME
	#undef DDM_FUNCTION_DELEGATE
	}
	
	MultiParameterSearch multiParameterSearch = MultiParameterSearch(parameters);

	multiParameterSearch.Apply();

	// set the rigid transform
	//Q_PRINT_DEBUG("MultiParameter2D3DRegistration:: GetRigidTransform\n");
	//qPrint("multiParameterSearch %f %f %f %f %f\n", multiParameterSearch.m_BestPost[0], multiParameterSearch.m_BestPost[1], multiParameterSearch.m_BestPost[2], multiParameterSearch.m_BestPost[3], multiParameterSearch.m_BestPost[4], multiParameterSearch.m_BestPost[5]);
	Matrix44 mat = GetRigidTransform(multiParameterSearch.m_BestPost[0], multiParameterSearch.m_BestPost[1], multiParameterSearch.m_BestPost[2]
									,multiParameterSearch.m_BestPost[3], multiParameterSearch.m_BestPost[4], multiParameterSearch.m_BestPost[5]);
	//Matrix44 mat = GetRigidTransform(0., 0., 0.
	//								,0., 0., 0.);

	//Q_PRINT_DEBUG("MultiParameter2D3DRegistration:: finalBestFittingMetric\n");
	// fittingScore has to be the highest
	m_FittingScore = multiParameterSearch.m_BestFittingMetric/static_cast<qf64>(m_Nb2dPts);

	PostApply(mat, nbProcUsed);
	//END_CHRONO(multiresolutionChrono);
	//PRINT_CHRONO(multiresolutionChrono);
}

END_Q_NAMESPACE
