//----------------------------------------------------------------------------
// \file    splineFitter.h
// \author  Michiel Schaap
// \date    2006-10-13
//
// Class to fit a spline to a series of markers
//
//----------------------------------------------------------------------------
#include "splineFitter.h"

//const float_ SplineFitter::MARGIN=10E-3f;

#include <list>
#include "math.h"

#include "float.h"
#define ML_DOUBLE_MAX     DBL_MAX

USING_Q_NAMESPACE

typedef Vector3 Vector3f;

// Find position of point on line with parameters lp
double_ SplineFitter::positionOnLine(const lineParam & lp, const Vector3 & point) {
  const double_ x = point[0];
  const double_ y = point[1];
  const double_ z = point[2];

  return (lp.ax * (x - lp.bx) + lp.ay*(y - lp.by) + lp.az*(z - lp.bz)) /
         (lp.ax * lp.ax + lp.ay * lp.ay + lp.az * lp.az);
}

// Compute minimal distance from point to line with parameters lp
// If outSideSegMeas = true, a distance will always be returned.
// If outSideSegMeas = false, only a distance will be returned if the line
//   through point perpendicular to the line intersects the line, otherwise
//   ML_DOUBLE_MAX will be returned.
double_ SplineFitter::distanceToLine(const lineParam & lp, const Vector3 & point, const bool outSideSegMeas = true) {
  const double_ x = point[0];
  const double_ y = point[1];
  const double_ z = point[2];

  double_ t = SplineFitter::positionOnLine(lp, point);

  if(t < 0-SFMARGIN) {
    if(!outSideSegMeas)
      return ML_DOUBLE_MAX;
    t = 0;
  }

  if(t > 1+SFMARGIN) {
    if(!outSideSegMeas)
      return ML_DOUBLE_MAX;
    t = 1;
  }

  const double_ lx = lp.bx + t * lp.ax;
  const double_ ly = lp.by + t * lp.ay;
  const double_ lz = lp.bz + t * lp.az;

  const double_ dist = sqrt( (lx - x)*(lx - x) + 
                            (ly - y)*(ly - y) +
                            (lz - z)*(lz - z));

  return dist;
}

double_ SplineFitter::distanceBetweenTwoLines(const lineParam& lp1, const lineParam& lp2) {
  Vector3 r1(lp1.bx, lp1.by, lp1.bz);
  Vector3 r2(lp2.bx, lp2.by, lp2.bz);

  Vector3 a1(lp1.ax, lp1.ay, lp1.az);
  a1.normalize();
  Vector3 e1 = a1;
  Vector3 a2(lp2.ax, lp2.ay, lp2.az);
  a2.normalize();
  Vector3 e2 = a2;

  Vector3 r12 = r2 - r1;

  double_ divValue = (1 - (e1.dot(e2)) * (e1.dot(e2)));

  if(divValue != 0) {
    double_ lambda0 =  (r12.dot(e1) - r12.dot(e2)*(e1.dot(e2)) ) / divValue;

    double_ tOnLine1 = lambda0 / a1.length();

    if(tOnLine1 < 0)
      tOnLine1 = 0;

    if(tOnLine1 > 1)
      tOnLine1 = 1;

    Vector3 dPos = r1 + tOnLine1 * a1;

    Vector3 pos(dPos[0],dPos[1],dPos[2]);

    return distanceToLine(lp2, pos, false);
  } else { // Lines parallel
    Vector3 p11 = r1;
    Vector3 p12 = r1 + a1;

    Vector3 p21 = r2;
    Vector3 p22 = r2 + a2;

    double_ dist1 = (p11 - p21).length();
    double_ dist2 = (p11 - p22).length();
    double_ dist3 = (p12 - p21).length();
    double_ dist4 = (p12 - p22).length();

    double_ minDist = dist1;
    if(dist2 < minDist)
      minDist = dist2;
    if(dist3 < minDist)
      minDist = dist3;
    if(dist4 < minDist)
      minDist = dist4;

    return minDist;
  }
}


void SplineFitter::addLines(  std::vector<lineParam>& lineParams, 
                              const float_ a[3], 
                              const float_ b[3], 
                              const float_ c[3], 
                              const float_ d[3], 
                              int_ nrLines,
                              const float_ sampleDistance,
                              bool useTwoPasses) {
  if(nrLines == -1) {
    float_ distStartEnd = 0;
    for(int_ i = 0; i < 3; i++) {
      const float_ compDif = a[i] + b[i] + c[i];
      distStartEnd += compDif*compDif;
    }
    distStartEnd  = sqrt(distStartEnd);

    nrLines = static_cast<int_>( (distStartEnd / sampleDistance) + 1 );
  }

  if (useTwoPasses) {
    float_ distanceOverCurve = 0;
    for(int_ p = 0; p < nrLines; p ++) {
      const float_ t1 = (float_)p / nrLines;
      const float_ t2 = (float_)(p+1) / nrLines;

      float_ sqrDist = 0;

      for(int_ component = 0; component < 3; component++) {
        const float_ cx1 = a[component] * t1 * t1 * t1 + b[component] * t1 * t1 + c[component] * t1 + d[component];
        const float_ cx2 = a[component] * t2 * t2 * t2 + b[component] * t2 * t2 + c[component] * t2 + d[component];

        sqrDist += (cx2 - cx1) * (cx2 - cx1);
      }
      distanceOverCurve += sqrt(sqrDist);
    }

    // Calculate more accurately the number of lines 
    nrLines = static_cast<int_>( distanceOverCurve / sampleDistance + 1 );
  }

  for (int_ p=0; p < nrLines; p ++) {
    lineParam lp;
    const float_ t1 = (float_)p / nrLines;
    const float_ t2 = (float_)(p+1) / nrLines;


    for(int_ component = 0; component < 3; component++) {
      const float_ cx1 = a[component] * t1 * t1 * t1 + b[component] * t1 * t1 + c[component] * t1 + d[component];
      const float_ cx2 = a[component] * t2 * t2 * t2 + b[component] * t2 * t2 + c[component] * t2 + d[component];

      switch(component) {
        case 0:
          lp.bx = cx1;
          lp.ax = cx2-cx1;
          break;
        case 1:
          lp.by = cx1;
          lp.ay = cx2-cx1;
          break;
        case 2:
          lp.bz = cx1;
          lp.az = cx2-cx1;
          break;
      }
    }

    lineParams.push_back(lp);
  }
}


void SplineFitter::sampleEquiDistant(const centerlineSpline& inputLines,
                                     const float_ sampleDistance,
                                     centerlineSpline& outputLines) {
  if(!inputLines.empty()) {
    centerlineSpline copyInputLines(inputLines);
    std::copy(inputLines.begin(), inputLines.end(), copyInputLines.begin());

    //Last point
    lineParam lastLp;
    lastLp.bx = (inputLines.end() - 1)->bx + (inputLines.end() - 1)->ax;
    lastLp.by = (inputLines.end() - 1)->by + (inputLines.end() - 1)->ay;
    lastLp.bz = (inputLines.end() - 1)->bz + (inputLines.end() - 1)->az;

    copyInputLines.push_back(lastLp);

    outputLines.clear();
    //Calculate distance from each point to each spline in the reference 
    centerlineSpline::const_iterator cpIt = copyInputLines.begin();

    Vector3f currentPoint(cpIt->bx, cpIt->by, cpIt->bz);
    Vector3f prevPoint = currentPoint;
    Vector3f prevDir;


    for(; cpIt != copyInputLines.end(); ) {
      float_ distToPrev = 0;
      //Calculate position of next point
      Vector3f prevPointInSeg = prevPoint;
      while((distToPrev < sampleDistance) && (cpIt != copyInputLines.end()))
      {
        currentPoint = Vector3f(cpIt->bx, cpIt->by, cpIt->bz);
        distToPrev  += (currentPoint - prevPointInSeg).length();

        if(distToPrev > sampleDistance)
        {
          const float_ tooFar = (distToPrev - sampleDistance);
          currentPoint -= (tooFar / (prevDir.length())) * prevDir;
          distToPrev += sampleDistance;
        }
        else
        {
          prevDir = Vector3f(cpIt->ax, cpIt->ay, cpIt->az);
          ++cpIt;
        }

        prevPointInSeg = currentPoint;
      }

      lineParam lp;
      lp.ax = (currentPoint[0] - prevPoint[0]);
      lp.ay = (currentPoint[1] - prevPoint[1]);
      lp.az = (currentPoint[2] - prevPoint[2]);
      lp.bx = prevPoint[0];
      lp.by = prevPoint[1];
      lp.bz = prevPoint[2];

      outputLines.push_back(lp);

      prevPoint = currentPoint;
    }
  }
}

void SplineFitter::resampleSpline(const centerlineSpline& inputLines,
                                  const double_ nrInputLinesPerOutputLine,
                                  centerlineSpline& outputLines,
                                  const int_ numOutputPoints) 
{
  outputLines.clear();
  int_ num=0;
  for(double_ inIndex=0; inIndex<inputLines.size(); inIndex+=nrInputLinesPerOutputLine) {
    // Interpolate between lineParams
    const lineParam prev = inputLines[int_(inIndex)];
    const double_ fraction = inIndex - int_(inIndex);

    lineParam n;
    n.bx = prev.bx + prev.ax * fraction;
    n.by = prev.by + prev.ay * fraction;
    n.bz = prev.bz + prev.az * fraction;

    if (num<numOutputPoints) {
      // Out new line param to output
      outputLines.push_back(n);
    }
    num++;
  }

  // Set b parameters
  for (uint_ i=0; i<outputLines.size()-1; ++i) {
    outputLines[i].ax=outputLines[i+1].bx-outputLines[i].bx;
    outputLines[i].ay=outputLines[i+1].by-outputLines[i].by;
    outputLines[i].az=outputLines[i+1].bz-outputLines[i].bz;
  }
  // Set last b parameters
  outputLines[outputLines.size()-1].ax=inputLines[inputLines.size()-1].bx-outputLines[outputLines.size()-1].bx;
  outputLines[outputLines.size()-1].ay=inputLines[inputLines.size()-1].by-outputLines[outputLines.size()-1].by;
  outputLines[outputLines.size()-1].az=inputLines[inputLines.size()-1].bz-outputLines[outputLines.size()-1].bz;
}

void SplineFitter::splineToPoints( const centerlineSpline& inputSpline,
                                   std::vector<Vector3> & outputPoints,
                                   const bool closed,
                                   std::vector<Vector3> * outputVec) {
  outputPoints.clear();
  if (outputVec!=NULL) outputVec->clear();
  if (!inputSpline.empty()) {
    //Output points
    std::vector<lineParam>::const_iterator splineIt = inputSpline.begin();
    for(;splineIt != inputSpline.end(); ++splineIt) {
      const Vector3 outPoint (splineIt->bx, splineIt->by, splineIt->bz);
      outputPoints.push_back(outPoint);
      if (outputVec!=NULL) {
        const Vector3 outVec (splineIt->ax, splineIt->ay, splineIt->az);
        outputVec->push_back(outVec);
      }
    }

    --splineIt;

    if (!closed) {
      const Vector3 outPoint (splineIt->bx+splineIt->ax, splineIt->by+splineIt->ay, splineIt->bz+splineIt->az);
      outputPoints.push_back(outPoint);
      if (outputVec!=NULL) {
        const Vector3 outVec (splineIt->ax, splineIt->ay, splineIt->az);
        outputVec->push_back(outVec);
      }
    }
  }
}

void SplineFitter::pointsToSpline( const std::vector<Vector3> & inputPoints,
                                   int_ nrSplinePoints,
                                   float_ sampleDistance,
                                   centerlineSpline& outputSpline,
                                   bool useTwoPasses,
                                   bool closed,
                                   bool outputSplineParams,
                                   splineType *outputSplinePars) {
  // Empty output spline
  outputSpline.clear();
  int_ maxPos = int_ (inputPoints.size()) - 2;

  if(closed)
    maxPos = int_ (inputPoints.size()) -1;

  for(int_ i = 0; i <= maxPos; i++) {
    // Get markers at i-1, i, i+1, i+2
    int_ iPrev = static_cast<int_>((i + inputPoints.size() - 1) % inputPoints.size());
    int_ iNext1 = static_cast<int_>((i + inputPoints.size() + 1) % inputPoints.size());
    int_ iNext2 = static_cast<int_>((i + inputPoints.size() + 2) % inputPoints.size());

    Vector3 pos0 (inputPoints[iPrev]);
    const Vector3 pos1 (inputPoints[i]);
    const Vector3 pos2 (inputPoints[iNext1]);
    Vector3 pos3 (inputPoints[iNext2]);

    // Check if marker i-1 exists
    if(!closed) {
      //if (i > 0) {
      if (i == 0) {
        pos0 = inputPoints[i];
      }

      // Check if marker i+2 exists
      if (i == int_ (inputPoints.size()) - 2) {
        pos3 = inputPoints[i+1];
      }
    }

    float_ a[3], b[3], c[3], d[3];
    splineParam sp;
    for(qu8 component=0; component < 3; component++) {
      const float_ x0 = pos0[component];
      const float_ x1 = pos1[component];
      const float_ x2 = pos2[component];
      const float_ x3 = pos3[component];

      c[component] = (x2 - x0) / 2.0;
      d[component] = x1;
      a[component] = ((x3- x1)/2.0 + c[component] - 2.0 * (x2 - x1));
      b[component] = (x2 - x1) - c[component] - a[component];

      sp.a[component]=a[component];
      sp.b[component]=b[component];
      sp.c[component]=c[component];
      sp.d[component]=d[component];
    }
    if (outputSplineParams) {
      outputSplinePars->push_back(sp);
    }

    SplineFitter::addLines(outputSpline, a, b, c, d, nrSplinePoints, sampleDistance, useTwoPasses);
  }
}

// XMarkerList wrapper function for sortPoints
void SplineFitter::sortMarkers(const XMarkerList & inputMarkers, 
                               XMarkerList & sortedMarkers,
                               bool closed) {
  // ImageVector of Vector3f for input and output points
  std::vector<Vector3> inputPoints, outputPoints;
  // Convert XMarkerList to vector<Vector3f>
  for(qu32 i = 0; i < inputMarkers.getSize(); ++i){
	inputPoints.push_back(inputMarkers[i].pos.getVec3());
  }
  // Sort points
  sortPoints(inputPoints,outputPoints,closed);
  // Convert result back to XMarkerList format
  sortedMarkers.clear();
  for (std::vector<Vector3>::const_iterator it=outputPoints.begin(); it!=outputPoints.end(); ++it) {
    const XMarker marker (*it);
    sortedMarkers.appendItem(marker);
  }
}
void SplineFitter::sortPoints (const std::vector<Vector3> & inputPoints,
                               std::vector<Vector3> & outputPoints,
                               bool closed) {
  std::list<int_> sortedInputIndices;

  if(!closed) {
    // Minimal distance found between two markers
    double_ minDist = ML_DOUBLE_MAX;

    // Find first two points (combination of points that are Euclidian closest)
    uint_ minId1 = 0, minId2 = 1;
    for(uint_ i = 0; i < inputPoints.size(); i++) {
      for(uint_ j = i+1; j < inputPoints.size(); j++) {
        const Vector3 point1 = inputPoints[i];
        const Vector3 point2 = inputPoints[j];
        double_ dist = (point1-point2).lengthSquared();
        if(dist < minDist) {
          minDist = dist;
          minId1 = i;
          minId2 = j;
        }
      }
    }

    // Remaining point to process: all except the pair of point that
    // are closest to each other.
    std::list<int_> remaining;
    for (uint_ i=0; i<inputPoints.size(); i++) {
      if(i != minId1 && i != minId2)
        remaining.push_back(i);
    }

    // Insert into sorted list
    sortedInputIndices.push_back(minId1);
    sortedInputIndices.push_back(minId2);

    while (!remaining.empty()) {
      // Find closest to front and back of current list
      double_ distmin = ML_DOUBLE_MAX;

      // Iterator to last point
      std::list<int_>::iterator minIt = remaining.end();

      bool attachFront = true;
      // Test all remaining points
      for (std::list<int_>::iterator  s = remaining.begin();
                                     s != remaining.end(); ++s) {
        const Vector3 point = inputPoints[*s];
        const Vector3 front = inputPoints[sortedInputIndices.front()];
        const Vector3 back = inputPoints[sortedInputIndices.back()];
        
        // If closer to front...
        const double_ frontdist = (point-front).lengthSquared();
        if(frontdist < distmin) {
          distmin = frontdist;
          minIt = s;
          attachFront = true;
        }

        // If closer to rear...
        const double_ reardist = (point-back).lengthSquared();
        if(reardist < distmin) {
          distmin = reardist;
          minIt = s;
          attachFront = false;
        }
      }

      if(minIt != remaining.end()) {
        if(attachFront) {
          sortedInputIndices.push_front(*minIt);
        } else {
          sortedInputIndices.push_back(*minIt);
        }
        // Remove point from list
        remaining.erase(minIt);
      }
    }
  } else { // For closed splines...
    // Maximal distance found between two markers
    double_ maxDist = 0;
    // Find first two points
    uint_ maxId1 = 0, maxId2 = 1;
    for(uint_ i=0; i<inputPoints.size(); i++) {
      for(uint_ j = i+1; j < inputPoints.size(); j++) {
        const Vector3 pos1 = inputPoints[i];
        const Vector3 pos2 = inputPoints[j];
        const double_ dist = (pos1-pos2).lengthSquared();
        if(dist > maxDist) {
          maxDist = dist;
          maxId1 = i;
          maxId2 = j;
        }
      }
    }

    Vector3 pos1 = inputPoints[maxId1];
    Vector3 pos2 = inputPoints[maxId2];
    maxDist = 0;
    uint_ maxId3 = 0;
    for(uint_ i = 0; i < inputPoints.size(); i++) {
      if (i!=maxId1 && i!=maxId2) {
        const Vector3 pos = inputPoints[i];
        const double_ dist=(pos1-pos).lengthSquared()+(pos2-pos).lengthSquared();
        if(dist > maxDist) {
          maxDist = dist;
          maxId3 = i;
        }
      }
    }

    std::list<int_> remaining;
    for(uint_ i=0; i<inputPoints.size(); i++) {
      if (i!=maxId1 && i!=maxId2 && i!=maxId3) {
        remaining.push_back(i);
      }
    }

    // Insert into sorted list
    sortedInputIndices.push_back(maxId1);
    sortedInputIndices.push_back(maxId2);
    sortedInputIndices.push_back(maxId3);

    while (!remaining.empty()) {
      //Find point from remaining with smallest insert distance
      double_ angleDiffMin = ML_DOUBLE_MAX;

      std::list<int_>::iterator insertBefore = sortedInputIndices.end();
      std::list<int_>::iterator insertAfter = sortedInputIndices.end();
      std::list<int_>::iterator insertRem = remaining.end();

      for (std::list<int_>::iterator  s = remaining.begin();
        s != remaining.end(); ++s) {
        Vector3 sPos = inputPoints[*s];

        std::list<int_>::iterator endIt = sortedInputIndices.end();
        --endIt;
        Vector3 prevPos = inputPoints[*endIt];
        --endIt;
        Vector3 prevPos2 = inputPoints[*endIt];

        for (std::list<int_>::iterator  q = sortedInputIndices.begin();
                                       q != sortedInputIndices.end(); ++q) {
          std::list<int_>::iterator curIt = q;
          std::list<int_>::iterator nextIt = ++curIt;
          if(nextIt == sortedInputIndices.end())
            nextIt = sortedInputIndices.begin();
        
          Vector3 curPos = inputPoints[*q];
          Vector3 nextPos = inputPoints[*nextIt];

          Vector3 vecA = (prevPos - prevPos2);
          vecA.normalize();
          Vector3 vecB = (sPos - prevPos);
          vecB.normalize();
          Vector3 vecC = (curPos - sPos);
          vecC.normalize();
          Vector3 vecD = (nextPos - curPos);
          vecD.normalize();

          Vector3 vecE = (nextPos - prevPos);
          vecE.normalize();

          //Dotproduct before insert
          double_ angleBefore = acos(vecA.dot(vecE));
          angleBefore+= acos(vecE.dot(vecD));

          //Dotproduct after insert
          double_ angleAfter = acos(vecA.dot(vecB));
          angleAfter += acos(vecB.dot(vecC));
          angleAfter += acos(vecC.dot(vecD));

          double_ angleDiff = angleAfter - angleBefore;
          if(angleDiff < angleDiffMin)
          {
            angleDiffMin = angleDiff;
            insertBefore = q;
            insertRem = s;
          }

          prevPos2 = prevPos;
          prevPos = curPos;
        }
      }

      if(insertBefore != sortedInputIndices.end()) {
        sortedInputIndices.insert(insertBefore, *insertRem);
        remaining.erase(insertRem);
      } else {
        remaining.clear();
      }
    }
  }

  // Check order of markers (reverse or not)
  std::list<int_>::iterator minIt = sortedInputIndices.begin();
  int_ index = 0;
  int_ indexRev = static_cast<int_>(sortedInputIndices.size() - 1);
  int_ dif = 0;
  int_ difRev = 0;
  while(minIt != sortedInputIndices.end())
  {
    dif    += abs(*minIt- index);
    difRev += abs(*minIt- indexRev);
    ++minIt;
    ++index;
    --indexRev;
  }

  // Output markers in correct order
  outputPoints.clear();
  if(dif < difRev) {
    std::list<int_>::iterator iIt = sortedInputIndices.begin();
    while(iIt != sortedInputIndices.end()) {
      outputPoints.push_back(inputPoints[*iIt]);
      ++iIt;
    }
  } else {
    std::list<int_>::iterator iIt = sortedInputIndices.end();
    while(iIt != sortedInputIndices.begin()) {
      --iIt;
      outputPoints.push_back(inputPoints[*iIt]);
    }
  }
}

// Wrapper function for XMarkerList input and output
void SplineFitter::fitSpline(const XMarkerList & markersIn, 
                             XMarkerList & outputList,
                             const int_ nrSplinePoints,
                             const float_ sampleDistance,
                             const bool equidistant,
                             const int_ markerType,
                             const bool setVec,
                             const bool orderMarkers,
                             const bool closed,
                             const int_ numOutputPoints) {
  // ImageVector of Vector3f for input and output points
  std::vector<Vector3> inputPoints, outputPoints, outputVecs;
  // Convert XMarkerList to vector<Vector3f>

  for(qu32 i = 0; i < markersIn.getSize(); ++i){
	inputPoints.push_back(markersIn[i].pos.getVec3());
  }
  // Fit spline
  fitSpline(inputPoints, outputPoints, nrSplinePoints, equidistant, sampleDistance, &outputVecs, orderMarkers, closed, numOutputPoints);
  // Convert result back to XMarkerList format
  outputList.clear();
  for (std::vector<Vector3>::const_iterator it=outputPoints.begin(), itVec=outputVecs.begin(); it!=outputPoints.end(); ++it, ++itVec) {
    XMarker marker (*it);
    if (setVec) {
      marker.vec=*itVec;
    }
    marker.type=markerType;
    outputList.appendItem(marker);
  }
}

// Real splinefitter function, working on Vector3 input
void SplineFitter::fitSpline(const std::vector<Vector3> & pointsIn,
                             std::vector<Vector3> & pointsOut,
                             const int_   nrSplinePoints,
                             const bool  equidistant,
                             const float_ sampleDistance,
                             std::vector<Vector3> * vecElements,
                             const bool  orderPoints,
                             const bool  closed,
                             const int_   numOutputPoints) {
  if (pointsIn.size() > 1) {
    // Sort points on Euclidian distance
    std::vector< Vector3 > sortedPoints;
    
    if (orderPoints) {
      // Really sort points
      sortPoints(pointsIn, sortedPoints, closed);
    } else {
      // Just copy points to sortedlist
      std::copy(pointsIn.begin(), pointsIn.end(), std::back_inserter(sortedPoints));
    }

    std::vector<lineParam> centerSplineLines;
    pointsToSpline(sortedPoints, nrSplinePoints, sampleDistance, centerSplineLines, false, closed );

    if(numOutputPoints < 0)
    {
      centerlineSpline equiDistSp;
      if(equidistant)
      {
        // Sample with equidistant along the line segments on the spline
        sampleEquiDistant(centerSplineLines, sampleDistance, equiDistSp);
      }
      else
      {
        equiDistSp.resize(centerSplineLines.size());
        std::copy(centerSplineLines.begin(), centerSplineLines.end(), equiDistSp.begin());
      }

      // Output points
      splineToPoints (equiDistSp, pointsOut, closed, vecElements);
    }
    else
    {
      // Resample XMarkerList
      double_ step = (float_(pointsIn.size()-1)*nrSplinePoints)/(numOutputPoints-1);
      if (closed) {
        step = (float_(pointsIn.size())*nrSplinePoints)/numOutputPoints;
      }
      centerlineSpline resampledSpline;
      resampleSpline( centerSplineLines, step, resampledSpline, numOutputPoints-!closed);
      splineToPoints( resampledSpline, pointsOut, closed, vecElements);
    }
  }
  else
    pointsOut.clear();
}

// Blur a path with a Gaussian kernel
void SplineFitter::blurPath( const XMarkerList & markersIn, 
                             XMarkerList & outputList,
                             const float_ sigma,
                             const float_ precision) {
  // Define squareroot of 2*pi
  const float_ sqrt2pi = sqrt(2 * 3.14159265358);

  // Empty output list
  outputList.clear();

  // Get number of input markers
  int_ nrMarkers = static_cast<int_>(markersIn.size());

  if(nrMarkers > 2) {
    // Compute cumulative distances across path, from startpoint
    // Cumulative distance from endpoint = cumulative distance last
    // point from startpoint - cumulative distance current point
    // from startpoint
    std::vector<float_> cumulativeDistance (markersIn.size(), 0.0f);
    for (int_ i=1; i<nrMarkers; ++i) {
      Vector6 pos = markersIn[i].pos;
      Vector6 lastPos = markersIn[i-1].pos;
      cumulativeDistance[i] = (lastPos-pos).length() + cumulativeDistance[i-1];
    }

    // Distance threshold depends on sigma kernel and precision
    float_ distThreshold = sigma * precision;

    // Blur every position
    for (int_ i=0; i<int_(markersIn.size()); ++i) {
      // Create output marker
      XMarker markerOut = markersIn[i];
      if (sigma > 0.0001) {
        // Sigma not too small, output modified position

        // Check distanceThreshold
        float_ curDistThreshold = distThreshold;
        if (curDistThreshold > cumulativeDistance[i]) {
          // Distance to start marker is smaller than kernel size
          curDistThreshold = cumulativeDistance[i];
        }
        float_ cumDistToEnd = *(cumulativeDistance.end()-1)-cumulativeDistance[i];
        if (curDistThreshold > cumDistToEnd) {
          // Distance to end marker is smaller than kernel size
          curDistThreshold = cumDistToEnd;
        }

        // Now we know the real kernelsize (curDistThreshold)
        // Walk over positions

        // Maintain sum of values (for every dimension) and sum of weights
        std::vector<float_> sumValues (3, 0.0f);
        float_ sumWeights = 0.0f;

        // First walk to the left
        float_ cumDist  = 0.0f;
        int_   curIndex = i;
        while (cumDist <= curDistThreshold && curIndex>=0) {
          // Compute gaussian weight
          float_ weight = 1.0/(sqrt2pi * sigma) * exp(-(cumDist*cumDist)/(2*sigma*sigma));

          // Add values
          sumValues[0] += weight*markersIn[curIndex].pos[0];
          sumValues[1] += weight*markersIn[curIndex].pos[1];
          sumValues[2] += weight*markersIn[curIndex].pos[2];
          sumWeights += weight;

          // Compute distance to left marker
          if (curIndex>0) {
            cumDist += (markersIn[curIndex].pos-markersIn[curIndex-1].pos).length();
          }
          curIndex--;
        }

        if (i+1 < nrMarkers) {
          curIndex = i+1;
          cumDist  = (markersIn[i].pos-markersIn[curIndex].pos).length();
          while (cumDist <= curDistThreshold && curIndex < nrMarkers) {
            // Compute gaussian weight
            float_ weight = 1.0/(sqrt2pi * sigma) * exp(-(cumDist*cumDist)/(2*sigma*sigma));

            // Add values
            sumValues[0] += weight*markersIn[curIndex].pos[0];
            sumValues[1] += weight*markersIn[curIndex].pos[1];
            sumValues[2] += weight*markersIn[curIndex].pos[2];
            sumWeights += weight;

            // Compute distance to right marker
            if (curIndex+1 < nrMarkers) {
              cumDist += (markersIn[curIndex].pos-markersIn[curIndex+1].pos).length();
            }
            curIndex++;
          }
        }

        markerOut.pos[0] = sumValues[0] / sumWeights;
        markerOut.pos[1] = sumValues[1] / sumWeights;
        markerOut.pos[2] = sumValues[2] / sumWeights;
      }
      // Add smoothed marker to output
      outputList.appendItem(markerOut);
    }
  }
}

// Compute intersection between a plane and a line and return distance
double_ SplineFitter::intersectionPlaneAndLine( const lineParam& plane,
                                               const lineParam& line) {
  const Vector3 planePos(plane.bx, plane.by, plane.bz);
  Vector3 N (plane.ax, plane.ay, plane.az);
  N.normalize();

  const Vector3 P1      (line.bx, line.by, line.bz);
  const Vector3 lineDir (line.ax, line.ay, line.az);
  const Vector3 P2        = P1 + lineDir;
  const double_ divValue  = N.dot(P2 - P1);

  if(divValue == 0) {
    return positionOnLine (line, planePos);
  } else {
    double_ mu = N.dot(planePos - P1) / divValue;

    if(mu < 0)
      return ML_DOUBLE_MAX;

    if(mu > 1)
      return ML_DOUBLE_MAX;

    return mu;
  }
}
