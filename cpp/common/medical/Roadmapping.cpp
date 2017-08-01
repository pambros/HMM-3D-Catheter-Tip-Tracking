#include "Roadmapping.h"
#include "common/image/ImageItk.h"
#include "common/structure/Vessels.h"

#define DRAW_PT_RADIUS (2)
#define OFFSET_TIP_ID (13)

BEGIN_Q_NAMESPACE

void Draw2DCatheterAnd3DVessels(ImageType::Pointer _img, const PtList &_catheter, Vessels &_vessels){
	PtList vesselPtList;
	_vessels.GetPtList(vesselPtList);
	RGBPixelType::ComponentType colorGreen[3] = { 0, 255, 0 };
	DrawPtList(_img, vesselPtList, DRAW_PT_RADIUS, RGBPixelType(colorGreen));

	RGBPixelType::ComponentType colorRed[3] = { 255, 0, 0 };
	DrawPtList(_img, _catheter, DRAW_PT_RADIUS, RGBPixelType(colorRed));
}

void Draw2DCatheterAnd3DVesselsAfterTheTipPosition(ImageType::Pointer _img, const PtList &_catheter, Vessels &_vessels, const FusionResult &_fusionResult) {
	// get point id where the tip is
	qu32 tipId = _fusionResult.m_FusionTipVesselId[0];
	// we have an offset to give a bit of delay before to remove vessel branches close-by behind the tip position
	tipId = _vessels.GetPreviousVesselId(tipId, OFFSET_TIP_ID);

	Vessels *subVessels = NULL;
	_vessels.Get3dVesselsSubTree(tipId, Q_TRUE, subVessels);

	Draw2DCatheterAnd3DVessels(_img, _catheter, *subVessels);

	SAFE_DELETE_UNIQUE(subVessels);
}

END_Q_NAMESPACE
