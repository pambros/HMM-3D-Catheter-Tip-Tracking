#include "Object2D3D.h"
#include "common/registration/MultiParameter2D3DRegistration.h"
#include "common/registration/Powell2D3DRegistration.h"

using namespace std;

BEGIN_Q_NAMESPACE

void Object2D3D::Init(OPTIMIZER_ENUM _optimizer, const q::qString &_paramFileName, q::Vessels *_vessels, const q::qString &_info3DRAFileName){
	m_PreviousTransformInWorldCS = Matrix44::getIdentity();
	m_Info3DRA = Info3DRA(_info3DRAFileName.c_str());
	m_Vessels = _vessels;
	m_Vessels->Transform(m_Info3DRA.m_3DRAPixelToMM);

	if(_optimizer == OPTIMIZER_BRUTE_FORCE){
		m_Base2D3DRegistration = static_cast<Base2D3DRegistration*>(Q_NEW MultiParameter2D3DRegistration(_paramFileName));
	}
	else if(_optimizer == OPTIMIZER_POWELL){
#if defined(USE_VXL) || defined(USE_ITK)
		m_Base2D3DRegistration = static_cast<Base2D3DRegistration*>(Q_NEW Powell2D3DRegistration(_paramFileName));
#else
		q::qPrintStdErr("OPTIMIZER_POWELL cannot be executed because you did not compile with USE_ITK=ON or USE_VXL=ON\n");
		exit(1);
#endif
	}
	else{
		q::qPrintStdErr("Unknown optimizer %d\n", _optimizer);
		exit(1);
	}
}

Object2D3D::Object2D3D(OPTIMIZER_ENUM _optimizer, const q::qString &_paramFileName, q::Vessels *_vessels, const q::qString &_info3DRAFileName){
	Object2D3D::Init(_optimizer, _paramFileName, _vessels, _info3DRAFileName);
}

Object2D3D::Object2D3D(OPTIMIZER_ENUM _optimizer, const qString &_paramFileName, const qString &_vesselsFileName, const qString &_info3DRAFileName){
	m_Vessels = Q_NEW Vessels(_vesselsFileName);
	Object2D3D::Init(_optimizer, _paramFileName, m_Vessels, _info3DRAFileName);
}

Object2D3D::~Object2D3D(void){
	SAFE_DELETE_UNIQUE(m_Base2D3DRegistration);
	SAFE_DELETE_UNIQUE(m_Vessels);
}

END_Q_NAMESPACE