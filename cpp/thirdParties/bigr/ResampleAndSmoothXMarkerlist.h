//----------------------------------------------------------------------------------
//! The ML module class ResampleAndSmoothXMarkerlist.
/*!
// \file    mlResampleAndSmoothXMarkerlist.h
// \author  Michiel Schaap and Coert Metz
// \date    2006-10-13
//
*/
//----------------------------------------------------------------------------------
#ifndef __mlResampleAndSmoothXMarkerlist_H
#define __mlResampleAndSmoothXMarkerlist_H
#include "common/structure/PtList.h"
#include "common/maths/Vector.h"

#include <map>

USING_Q_NAMESPACE

#ifndef _XMARKERLIST_DEFINED
#define _XMARKERLIST_DEFINED
typedef PtList XMarkerList;
typedef Pt XMarker;
#endif

//#undef min
//#undef max

// Typedef for map to store vectors in
typedef std::map<qf32, Vector3> vectorElementsType;

class ResampleAndSmoothXMarkerlist
{
public:
	struct Parameters{
		// smoothing
		qf32 m_Sigma;
		qf32 m_Precision;
		// sampling
		qbool m_FitSpline;
		qbool m_SampleEquidistant;
		qf32 m_SampleDistance; // m_SampleDistance is used only if FitSpline is True
		qs32 m_NumberOfSplinePoints;
		qbool m_ClosedSpline;
		// options
		qbool m_InterpolateVectors;
		qbool m_KeepInputPoints;
		qbool m_SplitOnType;
		qbool m_SplitOnTime;
		qbool m_SplitOnConsecutiveDistance;
		qbool m_OrderMarkers;
		qf32 m_DistanceThreshold;

		Parameters() : m_Sigma(ResampleAndSmoothXMarkerlist::DEFAULT_SIGMA)
					 , m_Precision(ResampleAndSmoothXMarkerlist::DEFAULT_PRECISION)
					 , m_FitSpline(ResampleAndSmoothXMarkerlist::DEFAULT_FIT_SPLINE)
					 , m_SampleEquidistant(ResampleAndSmoothXMarkerlist::DEFAULT_SAMPLE_EQUIDISTANT)
					 , m_SampleDistance(ResampleAndSmoothXMarkerlist::DEFAULT_SAMPLE_DISTANCE)
					 , m_NumberOfSplinePoints(ResampleAndSmoothXMarkerlist::DEFAULT_NUMBER_OF_SPLINE_POINTS)
					 , m_ClosedSpline(ResampleAndSmoothXMarkerlist::DEFAULT_CLOSED_SPLINE)
					 , m_InterpolateVectors(ResampleAndSmoothXMarkerlist::DEFAULT_INTERPOLATE_VECTORS)
					 , m_KeepInputPoints(ResampleAndSmoothXMarkerlist::DEFAULT_KEEP_INPUT_POINTS)
					 , m_SplitOnType(ResampleAndSmoothXMarkerlist::DEFAULT_SPLIT_ON_TYPE)
					 , m_SplitOnTime(ResampleAndSmoothXMarkerlist::DEFAULT_SPLIT_ON_TIME)
					 , m_SplitOnConsecutiveDistance(ResampleAndSmoothXMarkerlist::DEFAULT_SPLIT_ON_CONSECUTIVE_DISTANCE)
					 , m_OrderMarkers(ResampleAndSmoothXMarkerlist::DEFAULT_ORDER_MARKERS)
					 , m_DistanceThreshold(ResampleAndSmoothXMarkerlist::DEFAULT_DISTANCE_THRESHOLD){
		}
	};

	// smoothing
	Q_DLL static const q::qf32 DEFAULT_SIGMA;
	Q_DLL static const q::qf32 DEFAULT_PRECISION;
	// sampling
	Q_DLL static const q::qbool DEFAULT_FIT_SPLINE;
	Q_DLL static const q::qbool DEFAULT_SAMPLE_EQUIDISTANT;
	Q_DLL static const q::qf32 DEFAULT_SAMPLE_DISTANCE;
	Q_DLL static const q::qs32 DEFAULT_NUMBER_OF_SPLINE_POINTS;
	Q_DLL static const q::qbool DEFAULT_CLOSED_SPLINE;
	// options
	Q_DLL static const q::qbool DEFAULT_INTERPOLATE_VECTORS;
	Q_DLL static const q::qbool DEFAULT_KEEP_INPUT_POINTS;
	Q_DLL static const q::qbool DEFAULT_SPLIT_ON_TYPE;
	Q_DLL static const q::qbool DEFAULT_SPLIT_ON_TIME;
	Q_DLL static const q::qbool DEFAULT_SPLIT_ON_CONSECUTIVE_DISTANCE;
	Q_DLL static const q::qbool DEFAULT_ORDER_MARKERS;
	Q_DLL static const q::qf32 DEFAULT_DISTANCE_THRESHOLD;

	// _parameters will be destroyed with the instance ResampleAndSmoothXMarkerlist
	ResampleAndSmoothXMarkerlist(Parameters *_parameters) : m_Parameters(_parameters){
		qAssert(_parameters != NULL);
	}
	Q_DLL ResampleAndSmoothXMarkerlist(const qString &_fileName);

	~ResampleAndSmoothXMarkerlist(void){
		SAFE_DELETE_UNIQUE(m_Parameters);
	}

	Q_DLL void Apply(const XMarkerList &_markerListInput);

	// no input
	Parameters *m_Parameters;

	// output
	XMarkerList m_MarkerListOutput;

private:
	// Method to interpolate vector elements of markers
	void interpolateVectors(XMarkerList &_markerlist, const vectorElementsType &_vecs);
	// Method to create vector elements map
	void createVectorMap(const XMarkerList &_points, const std::vector<Vector3> &_spline, vectorElementsType &_vecs);
};

#endif
