#ifndef __SHAPE_SIMILARITY_2D_3D_HEADER_
#define __SHAPE_SIMILARITY_2D_3D_HEADER_
#include "common/registration/ShapeSimilarity.h"
#include "common/medical/Object2D3D.h"

BEGIN_Q_NAMESPACE

class ShapeSimilarity2D3D : public Object2D3D{
public:
	void Init(const q::qString &_paramFileName);
	ShapeSimilarity2D3D(OPTIMIZER_ENUM _optimizer, const q::qString &_paramFileName, q::Vessels *_vessels, const q::qString &_info3DRAFileName);
	ShapeSimilarity2D3D(OPTIMIZER_ENUM _optimizer, const q::qString &_paramFileName, const q::qString &_vesselsFileName, const q::qString &_info3DRAFileName);
	~ShapeSimilarity2D3D(void);
	// be careful _catheter is modified
	int Do2D3DRegistration(q::PtList &_catheter, const q::InfoFluoro &_infoFluoro);

public:
	q::ShapeSimilarity *m_ShapeSimilarity;
};

END_Q_NAMESPACE

#endif
