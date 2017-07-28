cmake_minimum_required(VERSION 2.8)

project(TACE)

set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} -DQ_DEBUG")
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -DQ_DEBUG")

set(EXECUTABLE_OUTPUT_PATH ${CMAKE_BUILD_TYPE})

set(TACELIB_PATH "")
include_directories("/")

add_definitions(-DNEED_FILE)
add_definitions(-DNEED_MATRIX)
add_definitions(-DNEED_QUATERNION)
add_definitions(-DNEED_LOG_FUNCTIONS)

set(THIRD_PARTY_SOURCES)

option(USE_MULTITHREADING "Use multithreading" ON)
set(NB_THREADS 8 CACHE INTEGER "Number of threads")
if(USE_MULTITHREADING)
	add_definitions(-DUSE_MULTITHREADING)
	add_definitions(-DNB_THREADS=${NB_THREADS})
endif(USE_MULTITHREADING)

option(USE_AVX "Use AVX instructions" OFF)
if(MSVC)
	if(USE_AVX)
		add_definitions(-DUSE_AVX)
		ADD_DEFINITIONS(/arch:AVX)
	endif(USE_AVX)
	# ADD_DEFINITIONS(/Qvec-report:1)
	# ADD_DEFINITIONS(/Qvec-report:2)
	# ADD_DEFINITIONS(/GL)
	# set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} /LTCG")
endif(MSVC)

# option(USE_GDCM "Use GDCM library" OFF)
# if(USE_GDCM)
	# find_package(GDCM REQUIRED)
	# include(${GDCM_USE_FILE})
	# add_definitions( -DUSE_GDCM )
	# set(EXTRA_LIBS ${EXTRA_LIBS} gdcmCommon gdcmIOD)
# endif(USE_GDCM)

option(USE_ITK "Use ITK library" ON)
option(USE_VXL "Use VXL library" OFF)
if(USE_ITK)
	find_package(ITK REQUIRED)
	include(${ITK_USE_FILE})
	add_definitions(-DUSE_ITK)
else(USE_ITK)
	if(USE_VXL)
		find_package(VXL QUIET)
		if(VXL_FOUND)
			include(${VXL_CMAKE_DIR}/UseVXL.cmake)
			set(EXTRA_LIBS ${EXTRA_LIBS} vnl vnl_algo)
			add_definitions(-DUSE_VXL)
		endif(VXL_FOUND)
	endif(USE_VXL)
endif(USE_ITK)

option(USE_ZLIB "Use ZLIB library" OFF)
if(USE_ZLIB)
	find_package(ZLIB REQUIRED)
	include_directories(${ZLIB_INCLUDE_DIR})
	add_definitions(-DUSE_ZLIB)
	set(EXTRA_LIBS ${EXTRA_LIBS} ${ZLIB_LIBRARY})

	list(APPEND THIRD_PARTY_SOURCES
	${TACELIB_PATH}thirdParties/minizip/ioapi.c
	${TACELIB_PATH}thirdParties/minizip/ioapi.h
	${TACELIB_PATH}thirdParties/minizip/unzip.c
	${TACELIB_PATH}thirdParties/minizip/unzip.h)
endif(USE_ZLIB)

list(APPEND THIRD_PARTY_SOURCES
	${TRLIB_PATH}thirdParties/bigr/ResampleAndSmoothXMarkerlist.cpp
	${TRLIB_PATH}thirdParties/bigr/ResampleAndSmoothXMarkerlist.h
	${TRLIB_PATH}thirdParties/bigr/SplineFitter.cpp
	${TRLIB_PATH}thirdParties/bigr/SplineFitter.h
	
	${TRLIB_PATH}thirdParties/fastDelegate/FastDelegate.h
	
	${TRLIB_PATH}thirdParties/mevislab/mlLinearAlgebraTools.h
	
	${TRLIB_PATH}thirdParties/numpy/npy_math.h	
)

add_library(
	# TACELib STATIC
	TACELib SHARED
	
	${TACELIB_PATH}common/maths/Maths.h
	${TACELIB_PATH}common/maths/Matrix.cpp
	${TACELIB_PATH}common/maths/Matrix.h
	${TACELIB_PATH}common/maths/Quaternion.h
	${TACELIB_PATH}common/maths/Vector.h
	
	${TACELIB_PATH}common/medical/HMM2D3D.cpp
	${TACELIB_PATH}common/medical/HMM2D3D.h
	${TACELIB_PATH}common/medical/Fluoro3draParams.h
	${TACELIB_PATH}common/medical/Object2D3D.cpp
	${TACELIB_PATH}common/medical/Object2D3D.h
	${TACELIB_PATH}common/medical/Registration2D3D.cpp
	${TACELIB_PATH}common/medical/Registration2D3D.h
	${TACELIB_PATH}common/medical/ShapeSimilarity2D3D.cpp
	${TACELIB_PATH}common/medical/ShapeSimilarity2D3D.h
	
	${TACELIB_PATH}common/optimization/MultiParameterSearch.cpp
	${TACELIB_PATH}common/optimization/MultiParameterSearch.h
	
	${TACELIB_PATH}common/registration/Base2D3DRegistration.cpp
	${TACELIB_PATH}common/registration/Base2D3DRegistration.h
	${TACELIB_PATH}common/registration/CallDistanceMetric.h
	${TACELIB_PATH}common/registration/DefineDistanceMetric.h
	${TACELIB_PATH}common/registration/MultiParameter2D3DRegistration.cpp
	${TACELIB_PATH}common/registration/MultiParameter2D3DRegistration.h
	${TACELIB_PATH}common/registration/Powell2D3DRegistration.cpp
	${TACELIB_PATH}common/registration/Powell2D3DRegistration.h
	${TACELIB_PATH}common/registration/ShapeSimilarity.cpp
	${TACELIB_PATH}common/registration/ShapeSimilarity.h
	
	${TACELIB_PATH}common/structure/PtList.cpp
	${TACELIB_PATH}common/structure/PtList.h
	${TACELIB_PATH}common/structure/Vessels.cpp
	${TACELIB_PATH}common/structure/Vessels.h
	${TACELIB_PATH}common/structure/VesselsStateTransitionProbabilities.cpp
	${TACELIB_PATH}common/structure/VesselsStateTransitionProbabilities.h
	
	${TACELIB_PATH}common/util/File.cpp
	${TACELIB_PATH}common/util/File.h
	${TACELIB_PATH}common/util/LowLevel.cpp
	${TACELIB_PATH}common/util/LowLevel.h
	${TACELIB_PATH}common/util/Thread.h
	${TACELIB_PATH}common/util/Util.cpp
	${TACELIB_PATH}common/util/Util.h
	${TACELIB_PATH}common/util/UtilTime.cpp
	${TACELIB_PATH}common/util/UtilTime.h
	
	${THIRD_PARTY_SOURCES}
)

# set_target_properties(TACELib PROPERTIES COMPILE_FLAGS "-DQ_STATIC=1")
set_target_properties(TACELib PROPERTIES COMPILE_FLAGS "-DQ_SHARED=1 -DQ_DLL_EXPORTS=1")

target_link_libraries(
	TACELib
	${ITK_LIBRARIES}
	${EXTRA_LIBS}
)

add_executable(
	TACE
	src/TACE/ImageTlk.cpp
	src/TACE/ImageTlk.h
	src/TACE/main.cpp
	src/TACE/Registration.cpp
	src/TACE/Registration.h
	src/TACE/ResampleAndSmoothPtList.cpp
	src/TACE/ResampleAndSmoothPtList.h
	src/TACE/ResampleAndSmoothVessels.cpp
	src/TACE/ResampleAndSmoothVessels.h
	src/TACE/TransformPtList.cpp
	src/TACE/TransformPtList.h
	src/TACE/TransformVessels.cpp
	src/TACE/TransformVessels.h
)

target_link_libraries(
	TACE
	TACELib
)