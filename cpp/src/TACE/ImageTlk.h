#ifndef __IMAGE_TLK_HEADER_
#define __IMAGE_TLK_HEADER_
#include "common/util/Util.h"
#include "common/structure/PtList.h"

#ifdef USE_ITK
	#ifdef WIN32
		#pragma warning(push)
		#pragma warning (disable : 4996)
		#pragma warning (disable : 4251)
	#endif
	#include <itkImage.h>
	#include <itkRGBPixel.h>

	#ifdef WIN32
		#pragma warning(pop)
	#endif

typedef itk::RGBPixel<q::qu8> RGBPixelType;
typedef itk::Image<RGBPixelType, 2> ImageType;

ImageType::Pointer CreateImage(q::qsize_t _x, q::qsize_t _y);

void SavePNGImage(ImageType::Pointer _img, const q::qString &_fileName);

void DrawPoint(ImageType::Pointer _img, q::qs64 _x, q::qs64 _y, q::qs64 _radius, const RGBPixelType &_color);
void DrawPtList(ImageType::Pointer _img, const q::PtList &_ptl, q::qs64 _radius, const RGBPixelType &_color);

#endif

#endif
