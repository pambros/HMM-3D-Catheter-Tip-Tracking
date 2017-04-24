//----------------------------------------------------------------------------
// \file    splineFitter.h
// \author  Michiel Schaap
// \date    2006-10-13
//
// Class to fit a spline to a series of markers
//
//----------------------------------------------------------------------------
#ifndef __splineFitter_H
#define __splineFitter_H
#include "common/maths/Matrix.h"

#include <vector>

#include "common/structure/PtList.h"

USING_Q_NAMESPACE

#ifndef _XMARKERLIST_DEFINED
#define _XMARKERLIST_DEFINED
typedef PtList XMarkerList;
typedef Pt XMarker;
#endif

typedef qf64 float_;
//typedef qf32 float_; // get the same result as mevislab 2.4, but warnings
typedef qf64 double_;
typedef qs32 int_;
typedef qu32 uint_;

#define SFMARGIN 10E-3

class lineParam
{
  public: 
    lineParam() {}
    lineParam(const Vector3& p1, const Vector3& p2)
    {
      bx = static_cast<float_>( p1[0] );
      by = static_cast<float_>( p1[1] );
      bz = static_cast<float_>( p1[2] );
      ax = static_cast<float_>( p2[0] - p1[0] );
      ay = static_cast<float_>( p2[1] - p1[1] );
      az = static_cast<float_>( p2[2] - p1[2] );
      r1 = 0.0f;
      r2 = 0.0f;
      d1 = 0.0f;
      d2 = 0.0f;
    }
    
    float_ ax,bx,ay,by,az,bz;
    float_ r1,r2;
    float_ d1,d2;
};

class splineParam {
public:
  splineParam() {}
  float_ a[3], b[3], c[3], d[3];
};

class SplineFitter {
  public:
    typedef std::vector<splineParam> splineType;
    typedef std::vector<lineParam> centerlineSpline;
    typedef std::pair<XMarkerList, centerlineSpline> centerline;

    //static const float_ MARGIN;

    // Wrapper function for XMarkerList input and output
    static void fitSpline(const XMarkerList & markersIn, 
                          XMarkerList & outputList,
                          const int_ nrSplinePoints,
                          const float_ sampleDistance,
                          const bool equidistant,
                          const int_ markerType=0,
                          const bool setVec=false,
                          const bool orderMarkers=true,
                          const bool closed=false,
                          const int_ numOutputPoints=-1);

    // Real splinefitter function, working on Vector3f input
    static void fitSpline(const std::vector<Vector3> & pointsIn,       // Input points
                          std::vector<Vector3> & pointsOut,            // Output points
                          const int_   nrSplinePoints=1,             // Number of spline points to add between two input points
                          const bool  equidistant=false,            // Enable/disable equidistant sampling (nrSplinePoints ignored if true)
                          const float_ sampleDistance=1,             // Sample distance for equidistant sampling
                          std::vector<Vector3> * vecElements = NULL,   // Output vector elements (NULL is no output)
                          const bool  orderPoints=false,            // Enable/disable to order points on Euclidian length before fitting the spline
                          const bool  closed=false,                 // Enable/disable closed spline fitting
                          const int_ numOutputPoints=-1);            // Enable resampling of points along centerline

    static double_ positionOnLine(const lineParam & lp,
                                const Vector3 & point);

    static double_ distanceToLine(const lineParam& lp, 
                                const Vector3& point,
                                const bool outSideSegMeas);

    static double_ distanceBetweenTwoLines(const lineParam& lp1, 
                                         const lineParam& lp2);

    static double_ intersectionPlaneAndLine(const lineParam& plane, 
                                          const lineParam& lp2);

    static void addLines (std::vector<lineParam>& lineParams, 
                          const float_ a[3], 
                          const float_ b[3], 
                          const float_ c[3], 
                          const float_ d[3], 
                          int_ nrLines,
                          const float_ sampleDistance,
                          bool useTwoPasses = false);

    static void sampleEquiDistant(const centerlineSpline& inputLines,
                                  const float_ sampleDistance,
                                  centerlineSpline& outputLines);

    static void resampleSpline(const centerlineSpline& inputLines,
                               const double_ nrInputLinesPerOutputLine,
                               centerlineSpline& outputLines,
                               const int_ numOutputPoints);

    static void splineToPoints(const centerlineSpline& inputSpline,
                               std::vector<Vector3> & outputPoints,
                               const bool closed,
                               std::vector<Vector3> * outputVec=NULL);

    static void pointsToSpline (const std::vector<Vector3> & inputPoints,
                                int_ nrSplinePoints,
                                float_ sampleDistance,
                                centerlineSpline& outputSpline,
                                bool useTwoPasses = false,
                                bool closed=false,
                                bool outputSplineParams=false,
                                splineType *outputSplinePars=NULL);

    // XMarkerList wrapper function for sortPoints
    static void sortMarkers (const XMarkerList & inputMarkers,
                             XMarkerList & sortedMarkers,
                             bool closed = false);
    
    // Real point sorting function: sorts points on Euclidian length
    static void sortPoints (const std::vector<Vector3> & inputPoints,
                            std::vector<Vector3> & outputPoints,
                            bool closed = false);

    // Smooth the path using a Gaussian kernel for smoothing
    static void blurPath ( const XMarkerList & markersIn,
                           XMarkerList & outputList,
                           const float_ sigma,
                           const float_ precision);
};

#endif
