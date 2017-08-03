#ifndef __BASE_2D_3D_REGISTRATION_HEADER_
#define __BASE_2D_3D_REGISTRATION_HEADER_
#include "common/util/Util.h"
#include "common/maths/Matrix.h"
#include "common/structure/PtList.h"
#include "thirdParties/fastDelegate/FastDelegate.h"
#include "common/structure/Vessels.h"
#include "common/structure/VesselsStateTransitionProbabilities.h"

BEGIN_Q_NAMESPACE
	// Coordinate system definition
	// 3d world (in mm)
	// axe x -> move the sagittal plane from left to right
	// axe y -> move the transver plane from head to feet
	// axe z -> move the coronal plane from nose to back
	//
	// 2d x-ray images (in mm)
	// axe x -> move the sagittal plane from left to right
	// axe y -> move the transver plane from head to feet
	//
	// 3d world to 2d x-ray images
	// P3d = [p3dx, p3dy, p3dz, 1]^t
	// MworldToCarm -> transformation from the standard carm position to its current position
	// McarmProjection -> projection matrix of the c-arm
	// P2d = McarmProjection.MworldToCarm.P3d
	// 
	// We try to find a rigid transformation of the 3d blood vessel model to fit with the 2d catheter
	// P2d = McarmProjection.MrigidTransform3D.MworldToCarm.P3d
	// Precisely with MrigidTransform3D = Mtrans.Mrot
	// P2d = McarmProjection.Mtrans.MisoToCarm.Mrot.McarmToIso.MworldToCarm.P3d
	class Base2D3DRegistration{
	public:
		typedef fastdelegate::FastDelegate1<const qf64*, qf64> MetricCallback;
		typedef fastdelegate::FastDelegate2<const qf64*, qu32, qf64> MultiThreadMetricCallback;

		enum DISTANCE_ENUM{
			DISTANCE_SQR = 0,
			DISTANCE_ABSOLUTE = 1,
			DISTANCE_EUCLIDEAN,
			DISTANCE_COUNT
		};

		enum METRIC_ENUM{
			METRIC_FROM_CATHETER = 0,
			METRIC_IMPROVED_FROM_CATHETER = 1,
			METRIC_COUNT
		};

		enum RADIUS_METRIC_ENUM{
			RADIUS_METRIC_FCOST1 = 0,
			RADIUS_METRIC_FCOST2 = 1,
			RADIUS_METRIC_FCOST3,
			RADIUS_METRIC_COUNT
		};

		struct BaseParameters{
			Matrix44 m_CArmProjection;
			Matrix44 m_WorldToCArm;
			Matrix44 m_IsoCenterToCArm;
			Matrix44 m_TransformInWorldCS;

			qu32 m_DistanceUsed; // DISTANCE_ENUM
			qu32 m_MetricUsed; // METRIC_ENUM
			qf64 m_Dmax; // in mm (between 10 and 150mm)
			qbool m_UseWeight;
			qf64 m_WeightSigma; // sigma (between 20 and 100 with a catheter size in average of 200mm) of the gaussian  W(x) = lambda + (1.0 - lambda)*e(-x^2/(2*sigma^2))
			qf64 m_WeightLambda; // lambda (something between 0 and 0.5)
			qu32 m_RadiusMetric; // RADIUS_METRIC_ENUM
			qf64 m_RadiusAlpha1;
			qf64 m_RadiusAlpha2;
			qbool m_UseLimits; // limit is not used with brute force (no need)
			qf64 m_LimitX; // in mm
			qf64 m_LimitY; // in mm
			qf64 m_LimitZ; // in mm
			qf64 m_LimitAlpha; // in degree
			qf64 m_LimitBeta; // in degree
			qf64 m_LimitGamma; // in degree
			
			BaseParameters(void){
				m_CArmProjection = Matrix44::getIdentity();
				m_WorldToCArm = Matrix44::getIdentity();
				m_IsoCenterToCArm = Matrix44::getIdentity();
				m_TransformInWorldCS = Matrix44::getIdentity();
				m_DistanceUsed = DISTANCE_EUCLIDEAN;
				m_MetricUsed = METRIC_FROM_CATHETER;
				m_Dmax = 10;
				m_UseWeight = Q_FALSE;
				m_WeightSigma = 10;
				m_WeightLambda = 0.25;
				m_RadiusMetric = RADIUS_METRIC_FCOST1;
				m_RadiusAlpha1 = 2.;
				m_RadiusAlpha2 = 0.85;
				m_UseLimits = Q_FALSE;
				m_LimitX = 100.;
				m_LimitY = 100.;
				m_LimitZ = 100.;
				m_LimitAlpha = 7.5;
				m_LimitBeta = 7.5;
				m_LimitGamma = 7.5;
			}
		};

		virtual ~Base2D3DRegistration(void){
		}

		virtual void Apply(const q::PtList &_2dCatheter, const q::PtList &_3dVessels) = 0;

		virtual BaseParameters* GetBaseParameters(void) = 0;
		virtual Base2D3DRegistration* Copy(void) const = 0;

	protected:
		qu32 LoadParameters(qFile *_file);
		void PreApply(const q::PtList &_2dCatheter, const q::PtList &_3dVessels, qu32 _nbProcUsed = 1);
		void PostApply(const q::Matrix44 &_mat, qu32 _nbProcUsed = 1);
		void Init(void);

		#define DDM_HEADER
			#include "DefineDistanceMetric.h"
		#undef DDM_HEADER

	//public: // debug
		q::Matrix44 GetRigidTransform(q::qf64 _x, q::qf64 _y, q::qf64 _z
									, q::qf64 _alpha, q::qf64 _beta, q::qf64 _gamma);

		// temporary
		Matrix44 m_TransformToCArm;
	protected:
		Matrix44 m_CArmToIsoCenter;
		
		Matrix44 m_CArmToSupposedTip;
		Matrix44 m_SupposedTipToCArm;

		qu32 m_Nb2dPts;
		Vector2 *m_2dCatheterTmp;
		qf64 *m_PrecomputedWeightCurve;
		qu32 m_Nb3dPts;
		Vector4 *m_3dVesselsTmp;

		qf64 *m_3dVesselsRadius;

		// temporary for single thread
		Vector2 *m_Pt3dTo2d;
		// temporary for multi thread
		Vector2 **m_Pt3dTo2dThread;

	public:
		// input
		qbool m_ComputeRigidTransformInSupposedTipSpace; // in this case, we need to set m_WorldToSupposedTip, the transform from the world space to the supposed tip space
		Matrix44 m_WorldToSupposedTip;

		// output
		Matrix44 m_RigidTransform3DInWorldCS;
		qf64 m_FittingScore;
	};
END_Q_NAMESPACE

#endif
