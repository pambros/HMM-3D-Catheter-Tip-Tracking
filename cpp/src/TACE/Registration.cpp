#include "Registration.h"
#include "common/util/Util.h"
#include "common/util/UtilTime.h"
#include "common/medical/Registration2D3D.h"

#ifdef USE_ITK
	#include "common/image/ImageItk.h"
	#include "common/medical/Roadmapping.h"
#endif

USING_Q_NAMESPACE
using namespace std;

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
			PtList catheter(registration2D3D->m_DataList->m_CatheterCenterlineList[registration2D3D->m_CurrentFrame]);

			// draw the 3D vessel centerline in the 2D coordinate space
			InfoFluoro infoFluoro(registration2D3D->m_DataList->m_InfoFluoroFileName.c_str());

			Vessels vessels(registration2D3D->m_Object2D3DRegistration->m_Vessels);
			Matrix44 matRegistration = registration2D3D->m_Object2D3DRegistration->m_FusionResult.m_RigidTransform3DInWorldCS[0];
			matRegistration = infoFluoro.m_CArmProjection*infoFluoro.m_WorldToCArm*matRegistration;
			vessels.Transform(matRegistration, Q_TRUE);
			vessels.Transform(infoFluoro.m_FluoroPixelToMM.inverse());

			// draw all the point of the 3D vessel
			ImageType::Pointer img = CreateImage(1024, 1024);
			Draw2DCatheterAnd3DVessels(img, catheter, vessels);
			qSprintf(resultFileName, MAX_STR_BUFFER, "%s/imgRegistrationAllVesselPoints%d.png", outputFolder.c_str(), static_cast<qs32>(registration2D3D->m_CurrentFrame));
			SavePNGImage(img, resultFileName);

			// draw only point of the 3D vessel after the tip position
			img = CreateImage(1024, 1024);
			Draw2DCatheterAnd3DVesselsAfterTheTipPosition(img, catheter, vessels, registration2D3D->m_Object2D3DRegistration->m_FusionResult);
			qSprintf(resultFileName, MAX_STR_BUFFER, "%s/imgRegistrationForwardPoints%d.png", outputFolder.c_str(), static_cast<qs32>(registration2D3D->m_CurrentFrame));
			SavePNGImage(img, resultFileName);
		}
#endif

		ret = registration2D3D->NextFrame();
	} while(ret == 0);

	//END_CHRONO(RegistrationChrono);
	//PRINT_CHRONO(RegistrationChrono);

	SAFE_DELETE_UNIQUE(registration2D3D);

	return EXIT_SUCCESS;
}
