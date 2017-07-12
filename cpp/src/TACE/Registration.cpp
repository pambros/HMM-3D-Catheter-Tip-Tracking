#include "Registration.h"
#include "common/util/Util.h"
#include "common/util/UtilTime.h"
#include "common/medical/Registration2D3D.h"

#include "ImageTlk.h"

USING_Q_NAMESPACE
using namespace std;

#ifdef USE_ITK
	#define DRAW_PT_RADIUS (2)
	#define OFFSET_TIP_ID (13)

void Draw2DCatheterAnd3DVessels(PtList _catheter, Vessels *_vessels, const qString &_fileName){
	ImageType::Pointer img = CreateImage(1024, 1024);
	
	PtList vesselPtList;
	_vessels->GetPtList(vesselPtList);
	RGBPixelType::ComponentType colorGreen[3] = { 0, 255, 0 };
	DrawPtList(img, vesselPtList, DRAW_PT_RADIUS, RGBPixelType(colorGreen));

	RGBPixelType::ComponentType colorRed[3] = { 255, 0, 0 };
	DrawPtList(img, _catheter, DRAW_PT_RADIUS, RGBPixelType(colorRed));

	SavePNGImage(img, _fileName);
}
#endif

int Registration(int _argc, char **_argv){
	qPrint("# Registration\n");
	qString dataFolder = qString(_argv[2]);
	qString configFile = qString(_argv[3]);
	qString outputFolder = qString(_argv[4]);
	qString outputDebugImages = qString(_argv[5]); // -debugImages or -nodebugImages

	Registration2D3D *registration2D3D = Q_NEW Registration2D3D(configFile, dataFolder);

	// we already manually gave the fusion result of the first frame, so we skip it
	//registration2D3D->NextFrame();

	//START_CHRONO(RegistrationChrono);

	qs32 ret = 0;
	do{
		registration2D3D->Register();

		qchar8 resultFileName[MAX_STR_BUFFER];
		qSprintf(resultFileName, MAX_STR_BUFFER, "%s/result%d.txt", outputFolder.c_str(), static_cast<qs32>(registration2D3D->m_CurrentFrame));
		registration2D3D->m_Object2D3DRegistration->m_FusionResult.Save(resultFileName);

#ifdef USE_ITK
		// save the overlay of the 3D registered vessel on the 2D X-ray image
		if(outputDebugImages == "-debugImages"){
			PtList catheter(registration2D3D->m_DataList.m_CatheterCenterlineList[registration2D3D->m_CurrentFrame]);

			// draw the 3D vessel centerline in the 2D coordinate space
			InfoFluoro infoFluoro(registration2D3D->m_DataList.m_InfoFluoroFileName.c_str());

			Vessels vessels(registration2D3D->m_Object2D3DRegistration->m_Vessels);
			Matrix44 &matRegistration = registration2D3D->m_Object2D3DRegistration->m_FusionResult.m_RigidTransform3DInWorldCS[0];
			vessels.Transform(matRegistration);
			vessels.Transform(infoFluoro.m_WorldToCArm);
			vessels.Transform(infoFluoro.m_CArmProjection, Q_TRUE);
			vessels.Transform(infoFluoro.m_FluoroPixelToMM.inverse());

			// draw all the point of the 3D vessel
			qSprintf(resultFileName, MAX_STR_BUFFER, "%s/imgRegistrationAllVesselPoints%d.png", outputFolder.c_str(), static_cast<qs32>(registration2D3D->m_CurrentFrame));
			Draw2DCatheterAnd3DVessels(catheter, &vessels, resultFileName);

			// draw only point of the 3D vessel after the tip position
			{
				// get point id where the tip is
				qu32 tipId = registration2D3D->m_Object2D3DRegistration->m_FusionResult.m_FusionTipVesselId[0];
				// we have an offset to give a bit of delay before to remove vessel branches close-by behind the tip position
				tipId = vessels.GetPreviousVesselId(tipId, OFFSET_TIP_ID);

				Vessels *subVessels = NULL;
				vessels.Get3dVesselsSubTree(tipId, Q_TRUE, subVessels);

				qSprintf(resultFileName, MAX_STR_BUFFER, "%s/imgRegistrationForwardPoints%d.png", outputFolder.c_str(), static_cast<qs32>(registration2D3D->m_CurrentFrame));
				Draw2DCatheterAnd3DVessels(catheter, subVessels, resultFileName);

				SAFE_DELETE_UNIQUE(subVessels);
			}
		}
#endif

		ret = registration2D3D->NextFrame();
	} while(ret == 0);

	//END_CHRONO(RegistrationChrono);
	//PRINT_CHRONO(RegistrationChrono);

	SAFE_DELETE_UNIQUE(registration2D3D);

	return EXIT_SUCCESS;
}
