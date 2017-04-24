#include "ResampleAndSmoothPtList.h"
#include "thirdParties/bigr/ResampleAndSmoothXMarkerlist.h"

USING_Q_NAMESPACE
using namespace std;

int ResampleAndSmoothPtList(int _argc, char **_argv){
	qPrint("# ResampleAndSmoothPtList\n");

	qString parametersFileName = qString(_argv[2]);
	qString ptListInFileName = qString(_argv[3]);
	qString ptListOutFileName = qString(_argv[4]);
	qs32 ptListDimension = atoi(_argv[5]);
	qf64 sampleDistance = 0.;
	if(parametersFileName == std::string("0")){
		sampleDistance = atof(_argv[6]);
	}

	PtList ptListInput(ptListInFileName);
	
	ResampleAndSmoothXMarkerlist *resampleAndSmoothXMarkerlist = NULL;
	if(parametersFileName == std::string("0")){
		ResampleAndSmoothXMarkerlist::Parameters *param = Q_NEW ResampleAndSmoothXMarkerlist::Parameters();
		param->m_FitSpline = Q_TRUE;
		param->m_Sigma = 0.f;
		param->m_SampleDistance = static_cast<qf32>(sampleDistance);
		resampleAndSmoothXMarkerlist = Q_NEW ResampleAndSmoothXMarkerlist(param);
	}
	else{
		resampleAndSmoothXMarkerlist = Q_NEW ResampleAndSmoothXMarkerlist(parametersFileName);
	}

	resampleAndSmoothXMarkerlist->Apply(ptListInput);

	qu32 flag = PtList::FLAG_POS_X | PtList::FLAG_POS_Y;
	if(ptListDimension == 3){
		flag |= PtList::FLAG_POS_Z;
	}
	
	if(ptListOutFileName != std::string("0")){
		resampleAndSmoothXMarkerlist->m_MarkerListOutput.SaveFile(ptListOutFileName, flag);
	}
	else{
		qPrint("ptList = [");
		for(qsize_t i = 0; i < resampleAndSmoothXMarkerlist->m_MarkerListOutput.size(); ++i){
			if(i != 0){
				qPrint(",");
			}
			
			if(ptListDimension == 3){
				qPrint("[%f, %f, %f]", resampleAndSmoothXMarkerlist->m_MarkerListOutput[i].pos[PT_X]
				, resampleAndSmoothXMarkerlist->m_MarkerListOutput[i].pos[PT_Y]
				, resampleAndSmoothXMarkerlist->m_MarkerListOutput[i].pos[PT_Z]);
			}
			else{
				qPrint("[%f, %f]", resampleAndSmoothXMarkerlist->m_MarkerListOutput[i].pos[PT_X]
				, resampleAndSmoothXMarkerlist->m_MarkerListOutput[i].pos[PT_Y]);
			}
		}
		qPrint("]\n");
	}

	Q_DELETE_UNIQUE(resampleAndSmoothXMarkerlist);

	return EXIT_SUCCESS;
}
