#include "Powell2D3DRegistration.h"
//#include "common/util/UtilTime.h"
#include "common/util/File.h"

#if defined(USE_VXL) || defined(USE_ITK)

#include <vnl/vnl_vector.h>
#include <vnl/algo/vnl_powell.h>

//#define __DEBUG_PRINT 1
#ifdef __DEBUG_PRINT
	#define Q_PRINT_DEBUG(...) q::qPrint(__VA_ARGS__)
#else
	#define Q_PRINT_DEBUG(...)
#endif

using namespace std;

BEGIN_Q_NAMESPACE

class MetricCallbackVNLCostFunctionTranslation : public vnl_cost_function{
public:
	MetricCallbackVNLCostFunctionTranslation(Base2D3DRegistration::MetricCallback _metricCallback) : vnl_cost_function(2)
																	, m_MetricCallback(_metricCallback){
	}
private:
	qf64 f(vnl_vector<qf64> const& params){
		qf64 currentPos[6];
		currentPos[0] = params[0];
		currentPos[1] = params[1];
		currentPos[2] = 0.0;
		currentPos[3] = 0.0;
		currentPos[4] = 0.0;
		currentPos[5] = 0.0;
		return -m_MetricCallback(currentPos);
	}
	Base2D3DRegistration::MetricCallback m_MetricCallback;
};

class MetricCallbackVNLCostFunctionSwitchTranslation : public vnl_cost_function{
public:
	MetricCallbackVNLCostFunctionSwitchTranslation(Base2D3DRegistration::MetricCallback _metricCallback) : vnl_cost_function(2)
																	, m_MetricCallback(_metricCallback){
	}
private:
	qf64 f(vnl_vector<qf64> const& params){
		qf64 currentPos[6];
		currentPos[0] = params[1];
		currentPos[1] = params[0];
		currentPos[2] = 0.0;
		currentPos[3] = 0.0;
		currentPos[4] = 0.0;
		currentPos[5] = 0.0;
		return -m_MetricCallback(currentPos);
	}
	Base2D3DRegistration::MetricCallback m_MetricCallback;
};

class MetricCallbackVNLCostFunctionRotation : public vnl_cost_function{
public:
	MetricCallbackVNLCostFunctionRotation(Base2D3DRegistration::MetricCallback _metricCallback) : vnl_cost_function(5)
																	, m_MetricCallback(_metricCallback){
	}
private:
	qf64 f(vnl_vector<qf64> const& params){
		qf64 currentPos[6];
	#define SWITCH_ROTATION
	#ifdef SWITCH_ROTATION
		currentPos[0] = params[3];
		currentPos[1] = params[4];
		currentPos[2] = 0.0;
		currentPos[3] = params[0];
		currentPos[4] = params[1];
		currentPos[5] = params[2];
	#else
		currentPos[0] = params[0];
		currentPos[1] = params[1];
		currentPos[2] = 0.0;
		currentPos[3] = params[2];
		currentPos[4] = params[3];
		currentPos[5] = params[4];
	#endif
		return -m_MetricCallback(currentPos);
	}
	Base2D3DRegistration::MetricCallback m_MetricCallback;
};

class MetricCallbackVNLCostFunctionContinuous : public vnl_cost_function{
public:
//#define _DEBUG_METRIC
#ifdef _DEBUG_METRIC
	MetricCallbackVNLCostFunctionContinuous(Base2D3DRegistration::MetricCallback _metricCallback, Powell2D3DRegistration *_powell2D3DRegistration) : vnl_cost_function(4)
																	, m_MetricCallback(_metricCallback)
																	, m_Powell2D3DRegistration(_powell2D3DRegistration){
	}
#else
	MetricCallbackVNLCostFunctionContinuous(Base2D3DRegistration::MetricCallback _metricCallback) : vnl_cost_function(4)
																	, m_MetricCallback(_metricCallback){
	}
#endif
private:
	qf64 f(vnl_vector<qf64> const& params){
		qf64 currentPos[6];
		currentPos[0] = params[3];
		currentPos[1] = 0.0;
		currentPos[2] = 0.0;
		currentPos[3] = params[0];
		currentPos[4] = params[1];
		currentPos[5] = params[2];
		// m_MetricCallback give a number in [0, -inf], the closer to 0 the better the result (the maximum number -> maximization)
		// we return a number in [0, +inf], the closer to 0 the better (the minimum number -> minimization)
		qf64 metric = -m_MetricCallback(currentPos);
		//qPrint("metric %f x %f y %f z %f alpha %f beta %f gamma %f\n", metric, currentPos[0], currentPos[1], currentPos[2], currentPos[3], currentPos[4], currentPos[5]);
		
#ifdef _DEBUG_METRIC
		Matrix44 mat = m_Powell2D3DRegistration->GetRigidTransform(params[3], 0.0, 0.0, params[0], params[1], params[2]);
		Matrix44 matCArmToWorld = m_Powell2D3DRegistration->GetBaseParameters()->m_WorldToCArm.inverse();
		Matrix44 rigidTransform3DInWorldCS = matCArmToWorld*mat*m_Powell2D3DRegistration->m_TransformToCArm;
		PrintMatrix(rigidTransform3DInWorldCS);
#endif
		return metric;
	}
	
	Base2D3DRegistration::MetricCallback m_MetricCallback;
#ifdef _DEBUG_METRIC
	Powell2D3DRegistration *m_Powell2D3DRegistration;
#endif	
};

class MetricCallbackVNLCostFunctionContinuous3dof : public vnl_cost_function{
public:
	MetricCallbackVNLCostFunctionContinuous3dof(Base2D3DRegistration::MetricCallback _metricCallback) : vnl_cost_function(3)
																	, m_MetricCallback(_metricCallback){
	}
private:
	qf64 f(vnl_vector<qf64> const& params){
		qf64 currentPos[6];
		currentPos[0] = 0.0;
		currentPos[1] = 0.0;
		currentPos[2] = 0.0;
		currentPos[3] = params[0];
		currentPos[4] = params[1];
		currentPos[5] = params[2];
		// m_MetricCallback give a number in [0, -inf], the closer to 0 the better the result (the maximum number -> maximization)
		// we return a number in [0, +inf], the closer to 0 the better (the minimum number -> minimization)
		return -m_MetricCallback(currentPos);
	}
	
	Base2D3DRegistration::MetricCallback m_MetricCallback;
};

Powell2D3DRegistration::Powell2D3DRegistration(Parameters *_parameters) : m_Parameters(_parameters){
	Init();
}

Powell2D3DRegistration::Powell2D3DRegistration(const qString &_fileName){
	Init();

	m_Parameters = Q_NEW Parameters();

	qFile *file = NULL;
	try{
		qu32 err = qFOpen(file, _fileName.c_str(), "rb");
		if(err == 0){
			throw gDefaultException;
		}

		/*qu32 iErr =*/ Base2D3DRegistration::LoadParameters(file);
		/*iErr =*/ FileReachLine(file, qString("#\tPowell2D3DParameters"));
		/*iErr =*/ FileReadU32(file, m_Parameters->m_Dof);
		/*iErr =*/ FileReadF64(file, m_Parameters->m_InitialStep);
		/*iErr =*/ FileReadBool(file, m_Parameters->m_OnlyTranslation);
		/*iErr =*/ FileReadBool(file, m_Parameters->m_SwitchTranslation);
		/*iErr =*/ FileReadBool(file, m_Parameters->m_TranslationThenRotation);
	}
	catch(q::qDefaultException){
		q::qPrint("Powell2D3DRegistration::Powell2D3DRegistration Error during loading\n");
		qFClose(file);
		throw gDefaultException;
	}

	qFClose(file);
}

void Powell2D3DRegistration::Apply(const q::PtList &_2dCatheter, const q::PtList &_3dVessels){
	//START_CHRONO(powellChrono);
	PreApply(_2dCatheter, _3dVessels);

	MetricCallback metricCallback;
	{
		qbool ddmParamMetric = m_Parameters->m_MetricUsed == METRIC_IMPROVED_FROM_CATHETER ? true : false;
		qbool ddmParamWeight = m_Parameters->m_UseWeight;
		qu32 ddmParamNorm = m_Parameters->m_DistanceUsed;
		qu32 ddmParamRadius = m_Parameters->m_RadiusMetric;
		qbool ddmLimit = m_Parameters->m_UseLimits;
	#define DDM_FUNCTION_DELEGATE metricCallback
	#define DDM_CLASS_NAME Powell2D3DRegistration
	#define DDM_PARAM_MULTITHREAD // no multithread possible with powell optimizer
		#include "CallDistanceMetric.h"
	#undef DDM_PARAM_MULTITHREAD
	#undef DDM_CLASS_NAME
	#undef DDM_FUNCTION_DELEGATE
	}
	
	Matrix44 mat;
	vnl_vector<qf64> parametersTranslation = vnl_vector<qf64>(2);
	parametersTranslation[0] = 0.0;
	parametersTranslation[1] = 0.0;
	if(m_useContinuousMethod == Q_FALSE){
		if(m_Parameters->m_OnlyTranslation == true || m_Parameters->m_TranslationThenRotation == true){
			MetricCallbackVNLCostFunctionTranslation costFunctionTranslation(metricCallback);
			vnl_powell powellOptimizerTranslation(&costFunctionTranslation);
			//powellOptimizerTranslation.set_verbose(Q_TRUE);
			//powellOptimizerTranslation.set_trace(Q_TRUE);
			powellOptimizerTranslation.set_initial_step(m_Parameters->m_InitialStep);
			powellOptimizerTranslation.minimize(parametersTranslation);
			m_FittingScore = -powellOptimizerTranslation.get_end_error();
			
			if(m_Parameters->m_SwitchTranslation == true){
				vnl_vector<qf64> parametersTranslation2 = vnl_vector<qf64>(2);
				parametersTranslation2[0] = 0.0;
				parametersTranslation2[1] = 0.0;
				MetricCallbackVNLCostFunctionSwitchTranslation costFunctionTranslation2(metricCallback);
				vnl_powell powellOptimizerTranslation2(&costFunctionTranslation2);
				//powellOptimizerTranslation2.set_verbose(Q_TRUE);
				//powellOptimizerTranslation2.set_trace(Q_TRUE);
				powellOptimizerTranslation2.set_initial_step(m_Parameters->m_InitialStep);
				powellOptimizerTranslation2.minimize(parametersTranslation2);
				if(-powellOptimizerTranslation2.get_end_error() > m_FittingScore){
					m_FittingScore = -powellOptimizerTranslation2.get_end_error();
					parametersTranslation[0] = parametersTranslation2[1];
					parametersTranslation[1] = parametersTranslation2[0];
				}
			}
			mat = GetRigidTransform(parametersTranslation[0], parametersTranslation[1], 0.0, 0.0, 0.0, 0.0);
		}

		if(m_Parameters->m_OnlyTranslation == false){
			vnl_vector<qf64> parameters = vnl_vector<qf64>(5);
		#ifdef SWITCH_ROTATION
			parameters[0] = 0.0;
			parameters[1] = 0.0;
			parameters[2] = 0.0;
			parameters[3] = parametersTranslation[0];
			parameters[4] = parametersTranslation[1];
		#else
			parameters[0] = parametersTranslation[0];
			parameters[1] = parametersTranslation[1];
			parameters[2] = 0.0;
			parameters[3] = 0.0;
			parameters[4] = 0.0;
		#endif
			MetricCallbackVNLCostFunctionRotation costFunction(metricCallback);
			vnl_powell powellOptimizer(&costFunction);
			//powellOptimizer.set_verbose(Q_TRUE);
			//powellOptimizer.set_trace(Q_TRUE);
			powellOptimizer.set_initial_step(m_Parameters->m_InitialStep);
			powellOptimizer.minimize(parameters);

			if(-powellOptimizer.get_end_error() > m_FittingScore){
			#ifdef SWITCH_ROTATION
				mat = GetRigidTransform(parameters[3], parameters[4], 0.0, parameters[0], parameters[1], parameters[2]);
			#else
				mat = GetRigidTransform(parameters[0], parameters[1], 0.0, parameters[2], parameters[3], parameters[4]);
			#endif
				m_FittingScore = -powellOptimizer.get_end_error();
			}
		}
	}
	else{
		if(m_Parameters->m_Dof == 4){
			vnl_vector<qf64> parameters = vnl_vector<qf64>(4);
			parameters[0] = 0.0;
			parameters[1] = 0.0;
			parameters[2] = 0.0;
			parameters[3] = 0.0;
			
#ifdef _DEBUG_METRIC
			MetricCallbackVNLCostFunctionContinuous costFunction(metricCallback, this);
#else
			MetricCallbackVNLCostFunctionContinuous costFunction(metricCallback);
#endif
			vnl_powell powellOptimizer(&costFunction);
			//powellOptimizer.set_verbose(Q_TRUE);
			//powellOptimizer.set_trace(Q_TRUE);
			powellOptimizer.set_initial_step(m_Parameters->m_InitialStep);
			// default is 1e-9; see vnl_nonlinear_minimizer.h
			powellOptimizer.set_f_tolerance(0.1);
			//powellOptimizer.set_f_tolerance(0.001);
			//powellOptimizer.set_max_function_evals(5);
			powellOptimizer.minimize(parameters);

			if(-powellOptimizer.get_end_error() > m_FittingScore){
				mat = GetRigidTransform(parameters[3], 0.0, 0.0, parameters[0], parameters[1], parameters[2]);
				// m_FittingScore give a number in [0, -inf], the closer to 0 the better the result (the maximum number -> maximization)
				m_FittingScore = -powellOptimizer.get_end_error();

#ifdef _DEBUG_METRIC
				Matrix44 matCArmToWorld = GetBaseParameters()->m_WorldToCArm.inverse();
				Matrix44 rigidTransform3DInWorldCS = matCArmToWorld*mat*m_TransformToCArm;
				PrintMatrix(rigidTransform3DInWorldCS);
#endif
			}

			//qPrint("powell evaluations %d iterations %d error %f\n", powellOptimizer.get_num_evaluations(), powellOptimizer.get_num_iterations(), powellOptimizer.get_end_error());
		}
		else{
			vnl_vector<qf64> parameters = vnl_vector<qf64>(3);
			parameters[0] = 0.0;
			parameters[1] = 0.0;
			parameters[2] = 0.0;

			MetricCallbackVNLCostFunctionContinuous3dof costFunction(metricCallback);
			vnl_powell powellOptimizer(&costFunction);
			//powellOptimizer.set_verbose(Q_TRUE);
			//powellOptimizer.set_trace(Q_TRUE);
			powellOptimizer.set_initial_step(m_Parameters->m_InitialStep);
			// default is 1e-9; see vnl_nonlinear_minimizer.h
			powellOptimizer.set_f_tolerance(0.1);
			//powellOptimizer.set_f_tolerance(0.001);
			powellOptimizer.minimize(parameters);

			if(-powellOptimizer.get_end_error() > m_FittingScore){
				mat = GetRigidTransform(0.0, 0.0, 0.0, parameters[0], parameters[1], parameters[2]);
				m_FittingScore = -powellOptimizer.get_end_error();
			}
		}
	}


	m_FittingScore = m_FittingScore/static_cast<qf64>(m_Nb2dPts);

	PostApply(mat);
	//END_CHRONO(powellChrono);
	//PRINT_CHRONO(powellChrono);
}

END_Q_NAMESPACE

#endif
