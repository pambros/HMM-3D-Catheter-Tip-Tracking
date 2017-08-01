#ifndef __OBJECT_2D_3D_REGISTRATION_HEADER_
#define __OBJECT_2D_3D_REGISTRATION_HEADER_
#include "common/registration/Base2D3DRegistration.h"
#include "common/medical/Fluoro3draParams.h"

BEGIN_Q_NAMESPACE

enum OPTIMIZER_ENUM{
	OPTIMIZER_BRUTE_FORCE = 0,
	OPTIMIZER_POWELL = 1,
	OPTIMIZER_COUNT
};

class Object2D3D{
private:
	// be careful m_Vessels is transformed in mm coordinate space (m_Vessels->Transform(m_Info3DRA.m_3DRAPixelToMM))
	void Init(OPTIMIZER_ENUM _optimizer, const q::qString &_paramFileName, q::Vessels *_vessels, const q::qString &_info3DRAFileName);

public:
	Object2D3D(OPTIMIZER_ENUM _optimizer, const q::qString &_paramFileName, q::Vessels *_vessels, const q::qString &_info3DRAFileName);
	Object2D3D(OPTIMIZER_ENUM _optimizer, const q::qString &_paramFileName, const q::qString &_vesselsFileName, const q::qString &_info3DRAFileName);
	virtual ~Object2D3D(void);
	// be careful _catheter is modified
	virtual int Do2D3DRegistration(q::PtList &_catheter, const q::InfoFluoro &_infoFluoro) = 0;

public:
	q::Vessels *m_Vessels;
	q::Base2D3DRegistration *m_Base2D3DRegistration;
	q::Info3DRA m_Info3DRA;
	q::FusionResult m_FusionResult;
	q::Matrix44 m_PreviousTransformInWorldCS;
};

END_Q_NAMESPACE

#endif
