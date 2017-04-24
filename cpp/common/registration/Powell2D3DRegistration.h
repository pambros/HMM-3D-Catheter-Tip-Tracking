#ifndef __POWELL_2D_3D_REGISTRATION_HEADER_
#define __POWELL_2D_3D_REGISTRATION_HEADER_
#include "Base2D3DRegistration.h"
#include "common/util/Util.h"
#include "common/maths/Matrix.h"
#include "common/structure/PtList.h"

BEGIN_Q_NAMESPACE
class Powell2D3DRegistration : public Base2D3DRegistration{
	public:
		struct Parameters : public Base2D3DRegistration::BaseParameters{
			qu32 m_Dof; // 4 (3 rotations + 1 translation) or 3 (3 rotations)
			qf64 m_InitialStep;
			qbool m_OnlyTranslation;
			qbool m_SwitchTranslation;
			qbool m_TranslationThenRotation;
			
			Parameters(void){
				m_Dof = 4;
				m_InitialStep = 1.0;
				m_OnlyTranslation = false;
				m_SwitchTranslation = true;
				m_TranslationThenRotation = true;
			}
		};

		// _parameters will be destroyed with the instance Powell2D3DRegistration
		Powell2D3DRegistration(Parameters *_parameters);
		Powell2D3DRegistration(const qString &_fileName);
		inline virtual ~Powell2D3DRegistration(void){
			SAFE_DELETE_UNIQUE(m_Parameters);
		}

		void Apply(const q::PtList &_2dCatheter, const q::PtList &_3dVessels);

		inline BaseParameters* GetBaseParameters(void){
			return static_cast<BaseParameters*>(m_Parameters);
		}

		inline Base2D3DRegistration* Copy(void) const{
			Parameters *param = Q_NEW Parameters(*m_Parameters);
			return Q_NEW Powell2D3DRegistration(param);
		}

	public:
		// input
		Parameters *m_Parameters;

		// output
	};
END_Q_NAMESPACE

#endif
