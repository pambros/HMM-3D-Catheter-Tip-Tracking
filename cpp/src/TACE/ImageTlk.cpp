#include "ImageTlk.h"

#ifdef USE_ITK
	#ifdef WIN32
		#pragma warning(push)
		#pragma warning (disable : 4996)
		#pragma warning (disable : 4251)
	#endif
	#include <itkImageFileWriter.h>
	#ifdef WIN32
		#pragma warning(pop)
	#endif

USING_Q_NAMESPACE
using namespace std;

typedef itk::ImageFileWriter<ImageType> WriterType;

ImageType::Pointer CreateImage(qsize_t _x, qsize_t _y){
	ImageType::RegionType region;
	ImageType::IndexType start;
	start[0] = 0;
	start[1] = 0;

	ImageType::SizeType size;
	size[0] = _x;
	size[1] = _y;

	region.SetSize(size);
	region.SetIndex(start);

	ImageType::Pointer img = ImageType::New();
	img->SetRegions(region);
	img->Allocate();

	RGBPixelType* buffer = img->GetBufferPointer();
	memset(buffer, 0, sizeof(RGBPixelType)*_x*_y);

	return img;
}

void SavePNGImage(ImageType::Pointer _img, const qString &_fileName){
	WriterType::Pointer writer = WriterType::New();
	writer->SetFileName(_fileName.c_str());
	writer->SetInput(_img);
	writer->Update();
}

void DrawPoint(ImageType::Pointer _img, qs64 _x, qs64 _y, qs64 _radius, const RGBPixelType &_color){
	ImageType::RegionType region = _img->GetLargestPossibleRegion();
	ImageType::SizeType size = region.GetSize();

	for(qs64 r = _y - _radius; r <= _y + _radius; ++r){
		for(qs64 c = _x - _radius; c <= _x + _radius; ++c){
			if(r > 0 && r < static_cast<qs64>(size[0]) && c > 0 && c < static_cast<qs64>(size[1])){
				ImageType::IndexType pixelIndex;
				pixelIndex[0] = c;
				pixelIndex[1] = r;
				_img->SetPixel(pixelIndex, _color);
			}
		}
	}
}

void DrawPtList(ImageType::Pointer _img, const PtList &_ptl, qs64 _radius, const RGBPixelType &_color){
	for(qsize_t i = 0; i < _ptl.size(); ++i){
		DrawPoint(_img, static_cast<qs64>(_ptl[i].pos[PT_X]), static_cast<qs64>(_ptl[i].pos[PT_Y]), _radius, _color);
	}
}

#endif
