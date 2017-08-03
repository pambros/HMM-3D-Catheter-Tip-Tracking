#ifndef __TACE_HEADER_
#define __TACE_HEADER_
#include "common/maths/Matrix.h"
#include "common/util/File.h"

BEGIN_Q_NAMESPACE

//#define PRINT_ALL_DEBUG

//#define USE_ALPHA_LOG_PROBA
#ifdef USE_ALPHA_LOG_PROBA
	#define ALPHA_MIN_PROBA (MIN_LOG_NUMBER)
	#define ALPHA_MAX_PROBA (0.) // log(1) = 0
#else
	#define ALPHA_MIN_PROBA (0.)
	#define ALPHA_MAX_PROBA (1.)
#endif

class FusionResult {
public:
	enum {
		NB_SAVED_FUSION_RESULT = 1
	};

	struct Unit {
		Matrix44 m_RigidTransform3DInWorldCS;
		qu32 m_TipVesselId;
		qu32 m_LeafVesselId;
		qu32 m_SelectedVesselRank;
		// log information
		qu32 m_FusionTipVesselId;
		qf64 m_FusionMetric;
		qf64 m_ShapeSimilarityMetric;
	};

	FusionResult(void){
		for(qu32 i = 0; i < NB_SAVED_FUSION_RESULT; ++i){
			m_RigidTransform3DInWorldCS[i] = Matrix44::getIdentity();
			m_TipVesselId[i] = 0;
			m_LeafVesselId[i] = 0;
			m_SelectedVesselRank[i] = 0;

			m_FusionTipVesselId[i] = 0;
			m_FusionMetric[i] = 0.;
			m_ShapeSimilarityMetric[i] = 0.;
		}
		
		//m_RegistrationTime = 0;
		//m_SimulatedTipVesselId = 0;
		//m_UpsampledSimulatedTipVesselId = 0;
	}
	
	FusionResult(const FusionResult &_fusionResult){
		for(qu32 i = 0; i < NB_SAVED_FUSION_RESULT; ++i){
			m_RigidTransform3DInWorldCS[i] = _fusionResult.m_RigidTransform3DInWorldCS[i];
			m_TipVesselId[i] = _fusionResult.m_TipVesselId[i];
			m_LeafVesselId[i] = _fusionResult.m_LeafVesselId[i];
			m_SelectedVesselRank[i] = _fusionResult.m_SelectedVesselRank[i];

			m_FusionTipVesselId[i] = _fusionResult.m_FusionTipVesselId[i];
			m_FusionMetric[i] = _fusionResult.m_FusionMetric[i];
			m_ShapeSimilarityMetric[i] = _fusionResult.m_ShapeSimilarityMetric[i];
		}
		
		//m_RegistrationTime = _fusionResult.m_RegistrationTime;
		//m_SimulatedTipVesselId = _fusionResult.m_SimulatedTipVesselId;
		//m_UpsampledSimulatedTipVesselId = _fusionResult.m_UpsampledSimulatedTipVesselId;
		//m_SimulatedTipPosition = _fusionResult.m_SimulatedTipPosition;
		//m_SimulatedRigidRegistration = _fusionResult.m_SimulatedRigidRegistration;
		m_NormalizedAlphaTree = _fusionResult.m_NormalizedAlphaTree;
		m_DeltaTree = _fusionResult.m_DeltaTree;
#ifdef PRINT_ALL_DEBUG
		m_TreeBeginning = _fusionResult.m_TreeBeginning;
		m_TreeAfterDistribution = _fusionResult.m_TreeAfterDistribution;
		m_TreeAfterRedistribution = _fusionResult.m_TreeAfterRedistribution;
#endif
	}

	FusionResult(const Matrix44 &_rigidTransform3DInWorldCS, qu32 _tipVesselId, qu32 _leafVesselId, qu32 _selectedVesselRank){
		m_RigidTransform3DInWorldCS[0] = _rigidTransform3DInWorldCS;
		m_TipVesselId[0] = _tipVesselId;
		m_LeafVesselId[0] = _leafVesselId;
		m_SelectedVesselRank[0] = _selectedVesselRank;
	}

	FusionResult(const char *_fileName){
		qFile *file = NULL;
		try{
			qu32 err = qFOpen(file, _fileName, "rb");
			if(err == 0){
				throw gDefaultException;
			}
			
			/*qu32 iErr =*/ FileReachLine(file, qString("#\tFusionResult"));
			for(qu32 i = 0; i < NB_SAVED_FUSION_RESULT; ++i){
				/*iErr =*/ FileReadMatrix44(file, m_RigidTransform3DInWorldCS[i]);
				/*iErr =*/ FileReadU32(file, m_TipVesselId[i]);
				/*iErr =*/ FileReadU32(file, m_LeafVesselId[i]);
				/*iErr =*/ FileReadU32(file, m_SelectedVesselRank[i]);

				/*iErr =*/ FileReadU32(file, m_FusionTipVesselId[i]);
				/*iErr =*/ FileReadF64(file, m_FusionMetric[i]);
				/*iErr =*/ FileReadF64(file, m_ShapeSimilarityMetric[i]);
			}

			/*iErr =*/ FileReadF64List(file, m_NormalizedAlphaTree);
			/*iErr =*/ FileReadF64List(file, m_DeltaTree);
#ifdef PRINT_ALL_DEBUG
			/*iErr =*/ FileReadF64List(file, m_TreeBeginning);
			/*iErr =*/ FileReadF64List(file, m_TreeAfterDistribution);
			/*iErr =*/ FileReadF64List(file, m_TreeAfterRedistribution);
#endif
		}
		catch (q::qDefaultException){
			q::qPrintStdErr("FusionResult::FusionResult Error during loading %s\n", _fileName);
			qFClose(file);
			throw gDefaultException;
		}

		qFClose(file);
	}

	void Save(const char *_fileName){
		qFile *file = NULL;
		try{
			qu32 err = qFOpen(file, _fileName, "wb");
			if(err == 0){
				throw gDefaultException;
			}

			/*qu32 iErr =*/ FileSaveString(file, "#\tFusionResult\n");
			for(qu32 i = 0; i < NB_SAVED_FUSION_RESULT; ++i){
				/*iErr =*/ FileSaveString(file, "# m_RigidTransform3DInWorldCS[%d]\n", i);
				/*iErr =*/ FileSaveMatrix44(file, m_RigidTransform3DInWorldCS[i]);
				/*iErr =*/ FileSaveString(file, "# m_TipVesselId[%d] (only from ShapeSimilarity2D3D)\n", i);
				/*iErr =*/ FileSaveU32(file, m_TipVesselId[i]);
				/*iErr =*/ FileSaveString(file, "# m_LeafVesselId[%d] (only from ShapeSimilarity2D3D)\n", i);
				/*iErr =*/ FileSaveU32(file, m_LeafVesselId[i]);
				/*iErr =*/ FileSaveString(file, "# m_SelectedVesselRank[%d] (only from ShapeSimilarity2D3D)\n", i);
				/*iErr =*/ FileSaveU32(file, m_SelectedVesselRank[i]);
				
				/*iErr =*/ FileSaveString(file, "# m_FusionTipVesselId[%d]\n", i);
				/*iErr =*/ FileSaveU32(file, m_FusionTipVesselId[i]);
				/*iErr =*/ FileSaveString(file, "# m_FusionMetric[%d] (only from ShapeSimilarity2D3D)\n", i);
				/*iErr =*/ FileSaveF64(file, m_FusionMetric[i]);
				/*iErr =*/ FileSaveString(file, "# m_ShapeSimilarityMetric[%d] (only from ShapeSimilarity2D3D)\n", i);
				/*iErr =*/ FileSaveF64(file, m_ShapeSimilarityMetric[i]);
			}

			/*iErr =*/ FileSaveString(file, "# m_NormalizedAlphaTree\n");
			/*iErr =*/ FileSaveF64List(file, m_NormalizedAlphaTree);
			/*iErr =*/ FileSaveString(file, "# m_DeltaTree\n");
			/*iErr =*/ FileSaveF64List(file, m_DeltaTree);
#ifdef PRINT_ALL_DEBUG
			/*iErr =*/ FileSaveString(file, "# m_TreeBeginning\n");
			/*iErr =*/ FileSaveF64List(file, m_TreeBeginning);
			/*iErr =*/ FileSaveString(file, "# m_TreeAfterDistribution\n");
			/*iErr =*/ FileSaveF64List(file, m_TreeAfterDistribution);
			/*iErr =*/ FileSaveString(file, "# m_TreeAfterRedistribution\n");
			/*iErr =*/ FileSaveF64List(file, m_TreeAfterRedistribution);
#endif
		}
		catch (q::qDefaultException){
			q::qPrintStdErr("FusionResult::Save Error during saving %s\n", _fileName);
			qFClose(file);
			throw gDefaultException;
		}

		qFClose(file);
	}

	Matrix44 m_RigidTransform3DInWorldCS[NB_SAVED_FUSION_RESULT];
	qu32 m_TipVesselId[NB_SAVED_FUSION_RESULT];
	qu32 m_LeafVesselId[NB_SAVED_FUSION_RESULT];
	qu32 m_SelectedVesselRank[NB_SAVED_FUSION_RESULT];
	// log information
	qu32 m_FusionTipVesselId[NB_SAVED_FUSION_RESULT];
	qf64 m_FusionMetric[NB_SAVED_FUSION_RESULT];
	qf64 m_ShapeSimilarityMetric[NB_SAVED_FUSION_RESULT];

	std::vector<qf64> m_NormalizedAlphaTree;
	std::vector<qf64> m_DeltaTree;
#ifdef PRINT_ALL_DEBUG
	std::vector<qf64> m_TreeBeginning;
	std::vector<qf64> m_TreeAfterDistribution;
	std::vector<qf64> m_TreeAfterRedistribution;
#endif
};

class InfoFluoro {
public:
	void Init(qu16 _rows, qu16 _columns, qf64 _distanceSourceToDetector, qf64 _distanceSourceToPatient
		, qf64 _primaryAngle, qf64 _secondaryAngle, qf64 _pixelSpacingX, qf64 _pixelSpacingY){

		m_FluoroPixelToMM[0][0] = _pixelSpacingX;m_FluoroPixelToMM[0][1] = 0.0;           m_FluoroPixelToMM[0][2] = 0.0;m_FluoroPixelToMM[0][3] = 0.0;
		m_FluoroPixelToMM[1][0] = 0.0;           m_FluoroPixelToMM[1][1] = _pixelSpacingY;m_FluoroPixelToMM[1][2] = 0.0;m_FluoroPixelToMM[1][3] = 0.0;
		m_FluoroPixelToMM[2][0] = 0.0;           m_FluoroPixelToMM[2][1] = 0.0;           m_FluoroPixelToMM[2][2] = 1.0;m_FluoroPixelToMM[2][3] = 0.0;
		m_FluoroPixelToMM[3][0] = 0.0;           m_FluoroPixelToMM[3][1] = 0.0;           m_FluoroPixelToMM[3][2] = 0.0;m_FluoroPixelToMM[3][3] = 1.0;

		qf64 halfHeight = 0.5*_rows*_pixelSpacingY;
		qf64 halfWidth = 0.5*_columns*_pixelSpacingX;
		qf64 patientToDetector = _distanceSourceToDetector - _distanceSourceToPatient;

		m_WorldToCArm = GetRigidTransformMat(halfWidth, halfHeight, patientToDetector, _primaryAngle, _secondaryAngle, 0.0);

		m_CArmProjection[0][0] = _distanceSourceToDetector;m_CArmProjection[0][1] = 0.0;                      m_CArmProjection[0][2] = -halfWidth; m_CArmProjection[0][3] = 0.0;
		m_CArmProjection[1][0] = 0.0;                      m_CArmProjection[1][1] = _distanceSourceToDetector;m_CArmProjection[1][2] = -halfHeight;m_CArmProjection[1][3] = 0.0;
		m_CArmProjection[2][0] = 0.0;                      m_CArmProjection[2][1] = 0.0;                      m_CArmProjection[2][2] = 0.0;        m_CArmProjection[2][3] = 0.0;
		m_CArmProjection[3][0] = 0.0;                      m_CArmProjection[3][1] = 0.0;                      m_CArmProjection[3][2] = -1.0;       m_CArmProjection[3][3] = _distanceSourceToDetector;

		m_IsoCenterToCArm[0][0] = 1.0; m_IsoCenterToCArm[0][1] = 0.0; m_IsoCenterToCArm[0][2] = 0.0; m_IsoCenterToCArm[0][3] = halfWidth;
		m_IsoCenterToCArm[1][0] = 0.0; m_IsoCenterToCArm[1][1] = 1.0; m_IsoCenterToCArm[1][2] = 0.0; m_IsoCenterToCArm[1][3] = halfHeight;
		m_IsoCenterToCArm[2][0] = 0.0; m_IsoCenterToCArm[2][1] = 0.0; m_IsoCenterToCArm[2][2] = 1.0; m_IsoCenterToCArm[2][3] = patientToDetector;
		m_IsoCenterToCArm[3][0] = 0.0; m_IsoCenterToCArm[3][1] = 0.0; m_IsoCenterToCArm[3][2] = 0.0; m_IsoCenterToCArm[3][3] = 1.0;

		//m_CalibratedCArmProjection = Matrix44::getIdentity();
		//m_CalibratedWorldToCArm = Matrix44::getIdentity();
	}

	InfoFluoro(qu16 _rows, qu16 _columns, qf64 _distanceSourceToDetector, qf64 _distanceSourceToPatient
			, qf64 _primaryAngle, qf64 _secondaryAngle, qf64 _pixelSpacingX, qf64 _pixelSpacingY){
		m_UseCarmParameters = Q_TRUE;
		m_Rows = _rows;
		m_Columns = _columns;
		m_DistanceSourceToDetector = _distanceSourceToDetector;
		m_DistanceSourceToPatient = _distanceSourceToPatient;
		m_PrimaryAngle = _primaryAngle;
		m_SecondaryAngle = _secondaryAngle;
		m_PixelSpacingX = _pixelSpacingX;
		m_PixelSpacingY = _pixelSpacingY;
		Init(_rows, _columns, _distanceSourceToDetector, _distanceSourceToPatient, _primaryAngle, _secondaryAngle, _pixelSpacingX, _pixelSpacingY);
	}

	InfoFluoro(void){
		m_UseCarmParameters = Q_FALSE;
		m_FluoroPixelToMM = Matrix44::getIdentity();
		m_WorldToCArm = Matrix44::getIdentity();
		m_CArmProjection = Matrix44::getIdentity();
		m_IsoCenterToCArm = Matrix44::getIdentity();
		//m_CalibratedCArmProjection = Matrix44::getIdentity();
		//m_CalibratedWorldToCArm = Matrix44::getIdentity();
	}

	InfoFluoro(const InfoFluoro &_infoFluoro){
		m_UseCarmParameters = _infoFluoro.m_UseCarmParameters;
		m_FluoroPixelToMM = _infoFluoro.m_FluoroPixelToMM;
		m_WorldToCArm = _infoFluoro.m_WorldToCArm;
		m_CArmProjection = _infoFluoro.m_CArmProjection;
		m_IsoCenterToCArm = _infoFluoro.m_IsoCenterToCArm;
		//m_CalibratedCArmProjection = _infoFluoro.m_CalibratedCArmProjection;
		//m_CalibratedWorldToCArm = _infoFluoro.m_CalibratedWorldToCArm;

		m_Rows = _infoFluoro.m_Rows;
		m_Columns = _infoFluoro.m_Columns;
		m_DistanceSourceToDetector = _infoFluoro.m_DistanceSourceToDetector;
		m_DistanceSourceToPatient = _infoFluoro.m_DistanceSourceToPatient;
		m_PrimaryAngle = _infoFluoro.m_PrimaryAngle;
		m_SecondaryAngle = _infoFluoro.m_SecondaryAngle;
		m_PixelSpacingX = _infoFluoro.m_PixelSpacingX;
		m_PixelSpacingY = _infoFluoro.m_PixelSpacingY;
	}

	InfoFluoro(Matrix44 &_fluoroPixelToMM, Matrix44 &_worldToCArm, Matrix44 &_cArmProjection, Matrix44 &_isoCenterToCArm){
	//InfoFluoro(Matrix44 &_fluoroPixelToMM, Matrix44 &_worldToCArm, Matrix44 &_cArmProjection, Matrix44 &_isoCenterToCArm, Matrix44 &_calibratedCArmProjection, Matrix44 &_calibratedWorldToCArm){
		m_UseCarmParameters = Q_FALSE;
		m_FluoroPixelToMM = _fluoroPixelToMM;
		m_WorldToCArm = _worldToCArm;
		m_CArmProjection = _cArmProjection;
		m_IsoCenterToCArm = _isoCenterToCArm;
		//m_CalibratedCArmProjection = _calibratedCArmProjection;
		//m_CalibratedWorldToCArm = _calibratedWorldToCArm;
	}

	InfoFluoro(const char *_fileName){
		qFile *file = NULL;
		try{
			qu32 err = qFOpen(file, _fileName, "rb");
			if(err == 0){
				throw gDefaultException;
			}
			
			/*qu32 iErr =*/ FileReachLine(file, qString("#\tInfoFluoro"));
			/*iErr =*/ FileReadBool(file, m_UseCarmParameters);
			if(m_UseCarmParameters == Q_FALSE){
				/*iErr =*/ FileReadMatrix44(file, m_FluoroPixelToMM);
				/*iErr =*/ FileReadMatrix44(file, m_WorldToCArm);
				/*iErr =*/ FileReadMatrix44(file, m_CArmProjection);
				m_IsoCenterToCArm = Matrix44::getIdentity();

				m_IsoCenterToCArm[0][3] = m_WorldToCArm[0][3];
				m_IsoCenterToCArm[1][3] = m_WorldToCArm[1][3];
				m_IsoCenterToCArm[2][3] = m_WorldToCArm[2][3];
				/*iErr =*/ //FileReadMatrix44(file, m_IsoCenterToCArm);
				/*iErr =*/ //FileReadMatrix44(file, m_CalibratedCArmProjection);
				/*iErr =*/ //FileReadMatrix44(file, m_CalibratedWorldToCArm);
			}
			else{
				/*iErr =*/ FileReadU16(file, m_Rows);
				/*iErr =*/ FileReadU16(file, m_Columns);
				/*iErr =*/ FileReadF64(file, m_DistanceSourceToDetector);
				/*iErr =*/ FileReadF64(file, m_DistanceSourceToPatient);
				/*iErr =*/ FileReadF64(file, m_PrimaryAngle);
				/*iErr =*/ FileReadF64(file, m_SecondaryAngle);
				/*iErr =*/ FileReadF64(file, m_PixelSpacingX);
				/*iErr =*/ FileReadF64(file, m_PixelSpacingY);

				Init(m_Rows, m_Columns, m_DistanceSourceToDetector, m_DistanceSourceToPatient, m_PrimaryAngle, m_SecondaryAngle, m_PixelSpacingX, m_PixelSpacingY);
			}
		}
		catch (q::qDefaultException){
			q::qPrintStdErr("InfoFluoro::InfoFluoro Error during loading %s\n", _fileName);
			qFClose(file);
			throw gDefaultException;
		}

		qFClose(file);
	}

	void Save(const char *_fileName){
		qFile *file = NULL;
		try{
			qu32 err = qFOpen(file, _fileName, "wb");
			if(err == 0){
				throw gDefaultException;
			}
			
			/*qu32 iErr =*/ FileSaveString(file, "#\tInfoFluoro\n");
			/*iErr =*/ FileSaveString(file, "# m_UseCarmParameters (True, False)\n");
			/*iErr =*/ FileSaveBool(file, m_UseCarmParameters);
			if(m_UseCarmParameters == Q_TRUE){
				/*iErr =*/ FileSaveString(file, "# m_Rows // in pixel\n");
				/*iErr =*/ FileSaveU16(file, m_Rows);
				/*iErr =*/ FileSaveString(file, "# m_Columns // in pixel\n");
				/*iErr =*/ FileSaveU16(file, m_Columns);
				/*iErr =*/ FileSaveString(file, "# m_DistanceSourceToDetector // in mm\n");
				/*iErr =*/ FileSaveF64(file, m_DistanceSourceToDetector);
				/*iErr =*/ FileSaveString(file, "# m_DistanceSourceToPatient // in mm\n");
				/*iErr =*/ FileSaveF64(file, m_DistanceSourceToPatient);
				/*iErr =*/ FileSaveString(file, "# m_PrimaryAngle // in degree\n");
				/*iErr =*/ FileSaveF64(file, m_PrimaryAngle);
				/*iErr =*/ FileSaveString(file, "# m_SecondaryAngle // in degree\n");
				/*iErr =*/ FileSaveF64(file, m_SecondaryAngle);
				/*iErr =*/ FileSaveString(file, "# m_PixelSpacingX // in mm\n");
				/*iErr =*/ FileSaveF64(file, m_PixelSpacingX);
				/*iErr =*/ FileSaveString(file, "# m_PixelSpacingY // in mm\n");
				/*iErr =*/ FileSaveF64(file, m_PixelSpacingY);
			}
			/*iErr =*/ FileSaveString(file, "# m_FluoroPixelToMM\n");
			/*iErr =*/ FileSaveMatrix44(file, m_FluoroPixelToMM);
			/*iErr =*/ FileSaveString(file, "# m_WorldToCArm\n");
			/*iErr =*/ FileSaveMatrix44(file, m_WorldToCArm);
			/*iErr =*/ FileSaveString(file, "# m_CArmProjection\n");
			/*iErr =*/ FileSaveMatrix44(file, m_CArmProjection);
			/*iErr =*/ //FileSaveString(file, "# m_IsoCenterToCArm\n");
			/*iErr =*/ //FileSaveMatrix44(file, m_IsoCenterToCArm);
			/*iErr =*/ //FileSaveString(file, "# m_CalibratedCArmProjection\n");
			/*iErr =*/ //FileSaveMatrix44(file, m_CalibratedCArmProjection);
			/*iErr =*/ //FileSaveString(file, "# m_CalibratedWorldToCArm\n");
			/*iErr =*/ //FileSaveMatrix44(file, m_CalibratedWorldToCArm);
		}
		catch (q::qDefaultException){
			q::qPrintStdErr("InfoFluoro::Save Error during saving %s\n", _fileName);
			qFClose(file);
			throw gDefaultException;
		}

		qFClose(file);
	}

	qbool m_UseCarmParameters;

	Matrix44 m_FluoroPixelToMM;
	Matrix44 m_WorldToCArm;
	Matrix44 m_CArmProjection;
	Matrix44 m_IsoCenterToCArm;
	//Matrix44 m_CalibratedCArmProjection;
	//Matrix44 m_CalibratedWorldToCArm;
	
	// if m_FluoroCoordinate == FLUORO_COORDINATE_CARM_PARAMETERS
	qu16 m_Rows;
	qu16 m_Columns;
	qf64 m_DistanceSourceToDetector;
	qf64 m_DistanceSourceToPatient;
	qf64 m_PrimaryAngle;
	qf64 m_SecondaryAngle;
	qf64 m_PixelSpacingX;
	qf64 m_PixelSpacingY;
};

class Info3DRA {
public:
	Info3DRA(void){
		m_3DRAPixelToMM = Matrix44::getIdentity();
	}

	Info3DRA(Matrix44 &_3DRAPixelToMM){
		m_3DRAPixelToMM = _3DRAPixelToMM;
	}

	Info3DRA(const char *_fileName){
		qFile *file = NULL;
		try{
			qu32 err = qFOpen(file, _fileName, "rb");
			if(err == 0){
				throw gDefaultException;
			}
			
			/*qu32 iErr =*/ FileReachLine(file, qString("#\tInfo3DRA"));
			/*qu32 iErr =*/ FileReadMatrix44(file, m_3DRAPixelToMM);
		}
		catch (q::qDefaultException){
			q::qPrintStdErr("Info3DRA::Info3DRA Error during loading %s\n", _fileName);
			qFClose(file);
			throw gDefaultException;
		}

		qFClose(file);
	}

	void Save(const char *_fileName){
		qFile *file = NULL;
		try{
			qu32 err = qFOpen(file, _fileName, "wb");
			if(err == 0){
				throw gDefaultException;
			}
			
			/*qu32 iErr =*/ FileSaveString(file, "#\tInfo3DRA\n");
			/*iErr =*/ FileSaveString(file, "# m_3DRAPixelToMM\n");
			/*iErr =*/ FileSaveMatrix44(file, m_3DRAPixelToMM);
		}
		catch (q::qDefaultException){
			q::qPrintStdErr("Info3DRA::Save Error during saving %s\n", _fileName);
			qFClose(file);
			throw gDefaultException;
		}

		qFClose(file);
	}

	Matrix44 m_3DRAPixelToMM;
};

END_Q_NAMESPACE

#endif
