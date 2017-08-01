#ifndef __ROADMAPPING_HEADER_
#define __ROADMAPPING_HEADER_
#include "common/util/Util.h"
#include "common/structure/Vessels.h"
#include "common/medical/Fluoro3draParams.h"
#include "common/image/ImageItk.h"

BEGIN_Q_NAMESPACE

Q_DLL void Draw2DCatheterAnd3DVessels(ImageType::Pointer _img, const PtList &_catheter, Vessels &_vessels);
Q_DLL void Draw2DCatheterAnd3DVesselsAfterTheTipPosition(ImageType::Pointer _img, const PtList &_catheter, Vessels &_vessels, const FusionResult &_fusionResult);

END_Q_NAMESPACE

#endif
