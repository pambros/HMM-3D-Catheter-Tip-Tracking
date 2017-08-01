#ifndef __IMAGE_ITK_HEADER_
#define __IMAGE_ITK_HEADER_
#include "common/util/Util.h"
#include "common/structure/PtList.h"

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

BEGIN_Q_NAMESPACE

typedef itk::RGBPixel<q::qu8> RGBPixelType;
typedef itk::Image<RGBPixelType, 2> ImageType;

Q_DLL ImageType::Pointer CreateImage(q::qsize_t _x, q::qsize_t _y);

Q_DLL void SavePNGImage(ImageType::Pointer _img, const q::qString &_fileName);

void DrawPoint(ImageType::Pointer _img, q::qs64 _x, q::qs64 _y, q::qs64 _radius, const RGBPixelType &_color);
void DrawLine(ImageType::Pointer _img, q::qu32 _x1, q::qu32 _y1, q::qu32 _x2, q::qu32 _y2, const RGBPixelType &_color);
Q_DLL void DrawRect(ImageType::Pointer _img, q::qu32 _x1, q::qu32 _y1, q::qu32 _x2, q::qu32 _y2, const RGBPixelType &_color);
void DrawPtList(ImageType::Pointer _img, const q::PtList &_ptl, q::qs64 _radius, const RGBPixelType &_color);

END_Q_NAMESPACE

#endif
