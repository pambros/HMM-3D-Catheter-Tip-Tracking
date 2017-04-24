#include "TransformVessels.h"
#include "common/structure/Vessels.h"

USING_Q_NAMESPACE
using namespace std;

int TransformVessels(int _argc, char **_argv){
	qPrint("TransformVessels\n");

	qString matrixFileName = qString(_argv[2]);
	qString vesselsInFileName = qString(_argv[3]);
	qString vesselsOutFileName = qString(_argv[4]);
	qString inverse;
	if(_argc > 5){
		inverse = qString(_argv[5]);
	}

	Matrix44 mat = LoadMatrix44(matrixFileName);
	if(inverse == "Inverse"){
		mat = mat.inverse();
	}

	Vessels vessels(vesselsInFileName);
	vessels.Transform(mat);
	vessels.Save(vesselsOutFileName);

	return EXIT_SUCCESS;
}
