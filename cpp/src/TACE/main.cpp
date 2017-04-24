#include "common/util/Util.h"
#include "Registration.h"
#include "ResampleAndSmoothPtList.h"
#include "ResampleAndSmoothVessels.h"
#include "TransformPtList.h"
#include "TransformVessels.h"

USING_Q_NAMESPACE
using namespace std;

int main(int _argc, char **_argv){
	//q::qPrint("# main\n");
	//q::qPrint("_argv[1] %s\n", _argv[1]);

	if(_argc > 1){
		qString typeStr = _argv[1];
		if(typeStr == "Registration"){
			Registration(_argc, _argv);
		}
		else if(typeStr == "ResampleAndSmoothPtLlist"){
			ResampleAndSmoothPtList(_argc, _argv);
		}
		else if(typeStr == "ResampleAndSmoothVessels"){
			ResampleAndSmoothVessels(_argc, _argv);
		}
		else if(typeStr == "TransformPtList"){
			TransformPtList(_argc, _argv);
		}
		else if(typeStr == "TransformVessels"){
			TransformVessels(_argc, _argv);
		}
	}
	else{
		qPrint("# not enough arguments\n");
	}

	//system("Pause");

	return EXIT_SUCCESS;
}
