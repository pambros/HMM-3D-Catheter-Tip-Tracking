#ifndef __MULTI_PARAMETER_2D_3D_REGISTRATION_HEADER_
#define __MULTI_PARAMETER_2D_3D_REGISTRATION_HEADER_
#include "Base2D3DRegistration.h"
#include "common/util/Util.h"
#include "common/maths/Matrix.h"
#include "common/structure/PtList.h"

BEGIN_Q_NAMESPACE
class MultiParameter2D3DRegistration : public Base2D3DRegistration{
	public:
		struct Parameters : public Base2D3DRegistration::BaseParameters{
			// in mm
			qf64 m_OffsetX;
			qf64 m_OffsetY;
			qf64 m_OffsetZ;
			// in degree
			qf64 m_OffsetAlpha;
			qf64 m_OffsetBeta;
			qf64 m_OffsetGamma;

			// in mm
			qf64 m_IntervalX;
			qf64 m_IntervalY;
			qf64 m_IntervalZ;
			// in degree
			qf64 m_IntervalAlpha;
			qf64 m_IntervalBeta;
			qf64 m_IntervalGamma;

			qu32 m_NbSamplingX;
			qu32 m_NbSamplingY;
			qu32 m_NbSamplingZ;
			qu32 m_NbSamplingAlpha;
			qu32 m_NbSamplingBeta;
			qu32 m_NbSamplingGamma;

			qf64 m_ReductionCoeff;
			qu32 m_NbIterations;

			Parameters(void){
				m_OffsetX = 0.;
				m_OffsetY = 0.;
				m_OffsetZ = 0.;
				m_OffsetAlpha = 0.;
				m_OffsetBeta = 0.;
				m_OffsetGamma = 0.;
				m_IntervalX = 100.;
				m_IntervalY = 100.;
				m_IntervalZ = 100.;
				m_IntervalAlpha = 15.;
				m_IntervalBeta = 15.;
				m_IntervalGamma = 15.;
				m_NbSamplingX = 7;
				m_NbSamplingY = 7;
				m_NbSamplingZ = 7;
				m_NbSamplingAlpha = 7;
				m_NbSamplingBeta = 7;
				m_NbSamplingGamma = 7;
				m_ReductionCoeff = 0.5;
				m_NbIterations = 7;
			}
		};

		// _parameters will be destroyed with the instance MultiParameter2D3DRegistration
		MultiParameter2D3DRegistration(Parameters *_parameters);
		MultiParameter2D3DRegistration(const qString &_fileName);
		inline virtual ~MultiParameter2D3DRegistration(void){
			SAFE_DELETE_UNIQUE(m_Parameters);
		}

		void Apply(const q::PtList &_2dCatheter, const q::PtList &_3dVessels);

		inline BaseParameters* GetBaseParameters(void){
			return static_cast<BaseParameters*>(m_Parameters);
		}

		inline Base2D3DRegistration* Copy(void) const{
			Parameters *param = Q_NEW Parameters(*m_Parameters);
			return Q_NEW MultiParameter2D3DRegistration(param);
		}

	public:
		// input
		Parameters *m_Parameters;

		// output
	};
END_Q_NAMESPACE

#endif
