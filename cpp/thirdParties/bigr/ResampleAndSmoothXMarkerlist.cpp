//------------------------------------------------------------------------------
//! The ML module class ResampleAndSmoothXMarkerlist.
/*!
// \file    mlResampleAndSmoothXMarkerlist.cpp
// \author  Michiel Schaap and Coert Metz
// \date    2006-10-13
//
*/
//------------------------------------------------------------------------------
#include "ResampleAndSmoothXMarkerlist.h"
#include "splineFitter.h"
#include "common/util/File.h"

#include "float.h"

USING_Q_NAMESPACE

// smoothing
const q::qf32 ResampleAndSmoothXMarkerlist::DEFAULT_SIGMA = 1.f;
const q::qf32 ResampleAndSmoothXMarkerlist::DEFAULT_PRECISION = 5.f;
// sampling
const q::qbool ResampleAndSmoothXMarkerlist::DEFAULT_FIT_SPLINE = Q_FALSE;
const q::qbool ResampleAndSmoothXMarkerlist::DEFAULT_SAMPLE_EQUIDISTANT = Q_TRUE;
const q::qf32 ResampleAndSmoothXMarkerlist::DEFAULT_SAMPLE_DISTANCE = 1.f;
const q::qs32 ResampleAndSmoothXMarkerlist::DEFAULT_NUMBER_OF_SPLINE_POINTS = -1;
const q::qbool ResampleAndSmoothXMarkerlist::DEFAULT_CLOSED_SPLINE = Q_FALSE;
// options
const q::qbool ResampleAndSmoothXMarkerlist::DEFAULT_INTERPOLATE_VECTORS = Q_FALSE;
const q::qbool ResampleAndSmoothXMarkerlist::DEFAULT_KEEP_INPUT_POINTS = Q_FALSE;
const q::qbool ResampleAndSmoothXMarkerlist::DEFAULT_SPLIT_ON_TYPE = Q_FALSE;
const q::qbool ResampleAndSmoothXMarkerlist::DEFAULT_SPLIT_ON_TIME = Q_FALSE;
const q::qbool ResampleAndSmoothXMarkerlist::DEFAULT_SPLIT_ON_CONSECUTIVE_DISTANCE = Q_FALSE;
const q::qbool ResampleAndSmoothXMarkerlist::DEFAULT_ORDER_MARKERS = Q_FALSE;
const q::qf32 ResampleAndSmoothXMarkerlist::DEFAULT_DISTANCE_THRESHOLD = 1.f;


ResampleAndSmoothXMarkerlist::ResampleAndSmoothXMarkerlist(const qString &_fileName){
	m_Parameters = Q_NEW Parameters();
	
	qFile *file = NULL;
	try{
		qu32 err = qFOpen(file, _fileName.c_str(), "rb");
		if(err == 0){
			throw gDefaultException;
		}

		/*qu32 iErr =*/ FileReadF32(file, m_Parameters->m_Sigma);
		/*iErr =*/ FileReadF32(file, m_Parameters->m_Precision);
		/*iErr =*/ FileReadBool(file, m_Parameters->m_FitSpline);
		/*iErr =*/ FileReadBool(file, m_Parameters->m_SampleEquidistant);
		/*iErr =*/ FileReadF32(file, m_Parameters->m_SampleDistance);
		/*iErr =*/ FileReadS32(file, m_Parameters->m_NumberOfSplinePoints);
		/*iErr =*/ FileReadBool(file, m_Parameters->m_ClosedSpline);
		/*iErr =*/ FileReadBool(file, m_Parameters->m_InterpolateVectors);
		/*iErr =*/ FileReadBool(file, m_Parameters->m_KeepInputPoints);
		/*iErr =*/ FileReadBool(file, m_Parameters->m_SplitOnType);
		/*iErr =*/ FileReadBool(file, m_Parameters->m_SplitOnTime);
		/*iErr =*/ FileReadBool(file, m_Parameters->m_SplitOnConsecutiveDistance);
		/*iErr =*/ FileReadBool(file, m_Parameters->m_OrderMarkers);
		/*iErr =*/ FileReadF32(file, m_Parameters->m_DistanceThreshold);
	}
	catch(q::qDefaultException){
		q::qPrint("ResampleAndSmoothXMarkerlist::ResampleAndSmoothXMarkerlist Error during loading\n");
		qFClose(file);
		throw gDefaultException;
	}

	qFClose(file);
}

void ResampleAndSmoothXMarkerlist::Apply(const XMarkerList &_markerListInput){
	// Check for valid combination of equidistant and number of splinepoints settings
	if(m_Parameters->m_SampleEquidistant == Q_TRUE && m_Parameters->m_NumberOfSplinePoints < 1){
		m_Parameters->m_NumberOfSplinePoints = -1;
	}
	else if(m_Parameters->m_NumberOfSplinePoints < 0){
		m_Parameters->m_NumberOfSplinePoints = -1;
	}

	m_MarkerListOutput.clear();

	const XMarkerList *xml = &_markerListInput;
	if(xml->empty() == Q_FALSE){
		std::vector<XMarkerList> xmls;
		const qbool splitOnType = m_Parameters->m_SplitOnType;
		const qbool splitOnTime = m_Parameters->m_SplitOnTime;
		const bool splitOnDistance = m_Parameters->m_SplitOnConsecutiveDistance;
		const float distanceThreshold = m_Parameters->m_DistanceThreshold;
		if((splitOnType || splitOnTime || splitOnDistance) && !xml->empty()){
			qs64 markertype = (*xml)[0].type;
			qf64 timepoint = (*xml)[0].pos[4];
			Vector3 lastPos = xml->front().pos.getVec3();
			XMarkerList dummy;
			for(qu32 i = 0; i < xml->size(); ++i){
				if ((splitOnType && (*xml)[i].type != markertype && !dummy.empty()) ||
					(splitOnTime && (*xml)[i].pos[4] != timepoint && !dummy.empty()) ||
					(splitOnDistance && ((*xml)[i].pos.getVec3() - lastPos).length() > distanceThreshold)) {
					xmls.push_back(dummy);
					dummy.clear();
				}
				dummy.push_back((*xml)[i]);
				markertype = (*xml)[i].type;
				timepoint = (*xml)[i].pos[4];
				lastPos = (*xml)[i].pos.getVec3();
			}

			if (!dummy.empty()){
				xmls.push_back(dummy);
			}
		}
		else{
			xmls.push_back(*xml);
		}

		for(qsize_t i = 0; i < xmls.size(); ++i){
			XMarkerList &inputXMarkerList = xmls[i];

			// Check for markerlist size
			const qsize_t nrMarkers = inputXMarkerList.size();

			if (nrMarkers>1){
				// Get settings
				const qf32 sigma = m_Parameters->m_Sigma;
				const qf32 precision = m_Parameters->m_Precision;
				const qf32 sampleDistance = m_Parameters->m_SampleDistance;
				const qbool orderMarkers = m_Parameters->m_OrderMarkers;

				// Sort markers according to  euclidean distance if option checked
				XMarkerList sortedMarkers;
				if (orderMarkers){
					SplineFitter::sortMarkers(inputXMarkerList, sortedMarkers, m_Parameters->m_ClosedSpline);
				}
				else{
					// Just copy markers to sortedlist
					for(qsize_t j = 0; j < inputXMarkerList.size(); ++j){
						sortedMarkers.appendItem(inputXMarkerList[j]);
					}
				}

				XMarkerList blurredMarkers;
				if (sigma > 0 && precision > 0 && nrMarkers > 2){
					// Smooth markerlist
					SplineFitter::blurPath (sortedMarkers, blurredMarkers, sigma, precision);
				}
				else{
					// Just copy markers to blurredlist
					for(qsize_t j = 0; j < sortedMarkers.size(); ++j){
						blurredMarkers.appendItem(sortedMarkers[j]);
					}
				}

				// Convert blurredMarkers to vector<Vector3>
				std::vector<Vector3> blurredPoints;
				for(qsize_t j = 0; j < blurredMarkers.size(); ++j){
					blurredPoints.push_back(blurredMarkers[j].pos.getVec3());
				}

				XMarkerList tempOutputMarkers;
				std::vector<Vector3> tempOutputPoints;
				if(m_Parameters->m_FitSpline){
					// Fit 3th order spline through sorted markers
					// and return nrSplinePoints line segments per spline segment
					// (a spline segment is a section between input markers)
					std::vector<lineParam> centerSplineLines;

					SplineFitter::pointsToSpline(blurredPoints, 
						m_Parameters->m_NumberOfSplinePoints == -1 ? -1 : m_Parameters->m_NumberOfSplinePoints + 1, 
						sampleDistance, 
						centerSplineLines,
						m_Parameters->m_KeepInputPoints && m_Parameters->m_SampleEquidistant,
						m_Parameters->m_ClosedSpline
					);

					if(m_Parameters->m_KeepInputPoints || !m_Parameters->m_SampleEquidistant){
						SplineFitter::splineToPoints( centerSplineLines, tempOutputPoints, m_Parameters->m_ClosedSpline);
					}
					else{
						// Sample with equidistance along the line segments on the spline
						SplineFitter::centerlineSpline equiDistSp;
						SplineFitter::sampleEquiDistant(centerSplineLines, sampleDistance, equiDistSp);

						// Output a markers at the start of the equidistance sampled line 
						// segments
						SplineFitter::splineToPoints(  equiDistSp, tempOutputPoints, m_Parameters->m_ClosedSpline);
					}

					// Create vector map containing all vector elements with distances
					// along spline
					vectorElementsType vecs;
					createVectorMap(inputXMarkerList, tempOutputPoints, vecs);

					// Convert to XMarkerList
					for(std::vector<Vector3>::const_iterator it=tempOutputPoints.begin(); it!=tempOutputPoints.end(); ++it){
						tempOutputMarkers.appendItem(XMarker(*it));
					}

					// Interpolate vector elements
					interpolateVectors(tempOutputMarkers, vecs);
				}
				else{
					// Create vector map containing all vector elements with distances
					// along spline
					vectorElementsType vecs;
					createVectorMap(inputXMarkerList, blurredPoints, vecs);

					// Interpolate vector elements
					interpolateVectors (blurredMarkers, vecs);

					// No spline fitting, copy blurredmarkers to outputXMarkerList
					for(qsize_t j = 0; j < blurredMarkers.size(); ++j){
						tempOutputMarkers.appendItem(blurredMarkers[j]);
					}
				}

				for(qsize_t i = 0; i < tempOutputMarkers.size(); ++i){
					m_MarkerListOutput.appendItem(XMarker(tempOutputMarkers[i]));
					m_MarkerListOutput.back().type = inputXMarkerList.front().type;
					m_MarkerListOutput.back().pos[4] = inputXMarkerList.front().pos[4];
				}
			}
		}
	}
}

void ResampleAndSmoothXMarkerlist::createVectorMap(const XMarkerList & points, const std::vector<Vector3> & spline, vectorElementsType & vecs){
	// Empty table
	vecs.clear();

	// Check all points for vector elements
	for(qsize_t j = 0; j < points.size(); ++j){
		qf64 vecLength = (points[j].vec).length();
		// If length vector > 0, find closest spline segment
		if (vecLength > 0){
			qf32 minDist = FLT_MAX;
			//qs32   minIndex = -1;
			qf32 minPosition = FLT_MAX;
			qf32 position = 0.0f;
			for (qsize_t i=1; i<spline.size(); ++i){
				Vector3 cur  = spline[i];
				Vector3 prev = spline[i-1];
				lineParam lp;
				lp.ax = (cur[0]-prev[0]);
				lp.ay = (cur[1]-prev[1]);
				lp.az = (cur[2]-prev[2]);
				lp.bx = prev[0];
				lp.by = prev[1];
				lp.bz = prev[2];
				qf32 length = static_cast<qf32>((cur-prev).length());
				qf32 dist = static_cast<qf32>(SplineFitter::distanceToLine(lp, points[j].pos.getVec3(), false));
				if (dist < minDist) {
					minDist = dist;
					qf32 lengthOnLine = static_cast<qf32>(SplineFitter::positionOnLine(lp, points[j].pos.getVec3())*length);
					minPosition = position + lengthOnLine;
					//minIndex = i;
				}

				minDist = MIN(dist, minDist);
				position += length;
			}

			if (minDist==FLT_MAX) {
				//std::cout << "Segment not found!" << std::endl;
			} else {
				vecs.insert(std::make_pair(minPosition,points[j].vec));
			}
		}
	}
}

void ResampleAndSmoothXMarkerlist::interpolateVectors(XMarkerList & markerlist, const vectorElementsType & vecs){
	// Interpolate vectors
	if (m_Parameters->m_InterpolateVectors && vecs.size() > 1){
		qf64 position = 0;
		Vector6 lastPosition = markerlist.front().pos;
		for(qsize_t i = 1; i < markerlist.size(); ++i){
			position += (lastPosition - markerlist[i].pos).length();
			// Find between which vectors we need to interpolate
			for (vectorElementsType::const_iterator itV = vecs.begin(); itV != --(vecs.end()); ++itV){
				vectorElementsType::const_iterator itVnext = itV;
				itVnext++;
				if (position >= itV->first - SFMARGIN && position <= itVnext->first + SFMARGIN){
					// We need to interpolate between itV and itV+1
					double relPos = (position - itV->first) / (itVnext->first - itV->first);
					markerlist[i].vec = itV->second + (itVnext->second - itV->second) * relPos;
				}
			}
			lastPosition = markerlist[i].pos;
		}
	}
}
