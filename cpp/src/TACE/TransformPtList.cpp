#include "TransformPtList.h"
#include "common/structure/PtList.h"

USING_Q_NAMESPACE
using namespace std;

int TransformPtList(int _argc, char **_argv){
	qPrint("TransformPtList\n");

	qString matrixFileName = qString(_argv[2]);
	qString ptListInFileName = qString(_argv[3]);
	qString ptListOutFileName = qString(_argv[4]);
	qs32 ptListDimension = atoi(_argv[5]);
	qString inverse;
	if(_argc > 6){
		inverse = qString(_argv[6]);
	}

	Matrix44 mat = LoadMatrix44(matrixFileName);
	if(inverse == "Inverse"){
		mat = mat.inverse();
	}

	qu32 flag = PtList::FLAG_POS_X | PtList::FLAG_POS_Y;
	if(ptListDimension == 3){
		flag |= PtList::FLAG_POS_Z;
	}

	PtList ptl(ptListInFileName);
	ptl.Transform(mat);
	ptl.SaveFile(ptListOutFileName, flag);

	return EXIT_SUCCESS;
}
