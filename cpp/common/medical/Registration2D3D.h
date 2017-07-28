#ifndef __REGISTRATION_2D_3D_HEADER_
#define __REGISTRATION_2D_3D_HEADER_
#include "common/util/Util.h"
#include "common/util/File.h"
#include "common/medical/Object2D3D.h"

BEGIN_Q_NAMESPACE

class DataList{
public:
	DataList(const qString &_dataFolder){
		m_OnlineRegistration = Q_FALSE;
		m_DataFolder = _dataFolder;
		qString dataFolder = _dataFolder + qString("/");

		qString fileName = dataFolder + qString("dataList.txt");
		qFile *file = NULL;
		try{
			qu32 err = qFOpen(file, fileName.c_str(), "rb");
			if(err == 0){
				throw gDefaultException;
			}

			FileReachLine(file, qString("#\tDataList"));
			FileReadString(file, m_Info3draFileName);
			m_Info3draFileName = dataFolder + m_Info3draFileName;
			FileReadString(file, m_Vessel3draFileName);
			m_Vessel3draFileName = dataFolder + m_Vessel3draFileName;
			FileReadBool(file, m_InitializeFromTipPosition);
			if(m_InitializeFromTipPosition == Q_TRUE){
				FileReadU32(file, m_TipPositionId);
			}
			else{
				FileReadString(file, m_FirstFusion);
				m_FirstFusion = dataFolder + m_FirstFusion;
			}
			FileReadString(file, m_InfoFluoroFileName);
			m_InfoFluoroFileName = dataFolder + m_InfoFluoroFileName;
			FileReadU32(file, m_NbFrame);
			FileReadString(file, m_CatheterCenterlineFileName);

			qsize_t posPct = m_CatheterCenterlineFileName.find("%");
			qs32 digits = atoi(m_CatheterCenterlineFileName.substr(posPct + 1, 2).c_str());
			qchar8 buffer[MAX_STR_BUFFER];
			for(qu32 i = 0; i < m_NbFrame; ++i){
				qString str = dataFolder;
				str = str + m_CatheterCenterlineFileName.substr(0, posPct);
				qSprintf(buffer, MAX_STR_BUFFER, "%04d", i);
				str = str + qString(buffer);
				str = str + m_CatheterCenterlineFileName.substr(posPct + 4);
				m_CatheterCenterlineList.push_back(str);
			}
		}
		catch(q::qDefaultException){
			q::qPrint("DataList::DataList Error during loading %s\n", fileName.c_str());
			qFClose(file);
			throw gDefaultException;
		}

		qFClose(file);
	}

	DataList(const qString &_vessel3DFilename, const qString &_3DRAinfoFilename, qu32 _tipPosition){
		m_OnlineRegistration = Q_TRUE;
		m_Info3draFileName = _3DRAinfoFilename;
		m_Vessel3draFileName = _vessel3DFilename;
		m_InitializeFromTipPosition = Q_TRUE;
		m_TipPositionId = _tipPosition;
	}

	/*void PrintDebug(void){
		qPrint("m_DataFolder %s\n", m_DataFolder.c_str());
		qPrint("m_Info3draFileName %s\n", m_Info3draFileName.c_str());
		qPrint("m_Vessel3draFileName %s\n", m_Vessel3draFileName.c_str());
		qPrint("m_InfoFluoroFileName %s\n", m_InfoFluoroFileName.c_str());
		qPrint("m_NbFrame %d\n", m_NbFrame);
		qPrint("m_CatheterCenterlineFileName %s\n", m_CatheterCenterlineFileName.c_str());
		for(std::vector<qString>::iterator it = m_CatheterCenterlineList.begin(); it != m_CatheterCenterlineList.end(); ++it){
			qPrint("m_CatheterCenterlineList it %s\n", (*it).c_str());
		}
	}*/

	qString m_DataFolder;
	qString m_Info3draFileName;
	qString m_Vessel3draFileName;
	qbool m_InitializeFromTipPosition;
	qu32 m_TipPositionId;
	qString m_FirstFusion;
	qString m_InfoFluoroFileName;
	qbool m_OnlineRegistration;
	qu32 m_NbFrame;
	qString m_CatheterCenterlineFileName;
	std::vector<qString> m_CatheterCenterlineList;
};

class Registration2D3D{
public:
	enum METHOD_ENUM{
		METHOD_SHAPE_SIMILARITY = 0,
		METHOD_HMM = 1,
		METHOD_COUNT
	};

	struct Parameters{
		Parameters(void){
			m_Method = METHOD_HMM;
			m_Optimizer = OPTIMIZER_POWELL;
			//m_UsePreviousRegistration = Q_TRUE;
		}

		Parameters(const qString &_filename){
			qFile *file = NULL;
			try{
				qu32 err = qFOpen(file, _filename.c_str(), "rb");
				if(err == 0){
					throw gDefaultException;
				}

				FileReachLine(file, qString("#\tConfig"));

				FileReadU32(file, m_Method);
				FileReadU32(file, m_Optimizer);
				//FileReadBool(file, m_UsePreviousRegistration);
			}
			catch(q::qDefaultException){
				q::qPrint("Config::Config Error during loading %s\n", _filename.c_str());
				qFClose(file);
				throw gDefaultException;
			}

			qFClose(file);
		}

		qu32 m_Method; // METHOD_ENUM
		qu32 m_Optimizer; // OPTIMIZER_ENUM
		//qbool m_UsePreviousRegistration;
	};

private:
	void InitRegistration2D3D(const qString &_confileFile);

public:
	Q_DLL Registration2D3D(const qString &_confileFile, const qString &_vessel3DFilename, const qString &_3DRAinfoFilename, qu32 _tipPosition);
	Q_DLL Registration2D3D(const qString &_confileFile, const qString &_dataFolder);
	Q_DLL ~Registration2D3D();

	// Compute a 2D/3D registration
	// be careful _catheter is modified
	Q_DLL void Register(PtList &_catheter, const InfoFluoro &_infoFluoro);
	Q_DLL void Register(void);

	// If there is still frame to register, return 0.
	// If there is no frame anymore to register, return -1.
	qs32 NextFrame(void){
		qAssert(m_DataList->m_OnlineRegistration == Q_FALSE);
		if(m_CurrentFrame < m_DataList->m_NbFrame - 1){
			m_CurrentFrame = m_CurrentFrame + 1;
			return 0;
		}
		return -1;
	}

	Parameters *m_Parameters;
	DataList *m_DataList;
	Object2D3D *m_Object2D3DRegistration;

	qsize_t m_CurrentFrame;
};

END_Q_NAMESPACE

#endif
