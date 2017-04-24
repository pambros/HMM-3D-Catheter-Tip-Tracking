#include "ResampleAndSmoothVessels.h"
#include "common/structure/Vessels.h"

USING_Q_NAMESPACE
using namespace std;

int ResampleAndSmoothVessels(int _argc, char **_argv){
	qPrint("ResampleAndSmoothVessels\n");

	qf64 sampleDistance = atof(_argv[2]);
	qString vesselsInFileName = qString(_argv[3]);
	qString vesselsOutFileName = qString(_argv[4]);

	Vessels vessels(vesselsInFileName);
	Vessels *resampleVessels = NULL;
	vessels.Resample(resampleVessels, 0., sampleDistance);
	resampleVessels->Save(vesselsOutFileName);
	SAFE_DELETE_UNIQUE(resampleVessels);

	return EXIT_SUCCESS;
}
