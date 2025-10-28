cmake_minimum_required(VERSION 3.16)
project (SPIRE)
set (CMAKE_CXX_STANDARD 11) # needed for Mac OS X

#add an option to build the gui or only terminal appliaction
option(BUILD_QT_GUI "Builds the GUI (QT) instead of terminal interface" ON)

#add an option to build the gui or only terminal appliaction
option(BUILD_COMPUTE_CHANNEL_PROPORTIONS "Builds a tool to compute volume channel proportions" OFF)

#add an option to build the gui or only terminal appliaction
option(BUILD_BATCH_TOOL "Builds a tool to generate batches of projections" OFF)

#source files independent of interface
SET(SRC_BASE ./src/img_out.cpp ./src/surface_projection.cpp ./src/distance_transform.cpp ./src/homotopic_thinning.cpp ./src/iterable_voxel.cpp ./src/percolation_analysis.cpp ./src/batch_lib.cpp ./src/level_sets.cpp ./src/img_manip.cpp)

#the source files for the QT gui
SET(SRC_QT_GUI ./src/qt_gui.cpp ./src/qt_main.cpp ./src/qt_sp.cpp ./src/slice_orientation_visualisation.cpp ./src/drawme.cpp)

#
# external libraries
#
SET(IML_DIR ./ext_libs/iml-1.0.5/src/)
SET(SRC_IML ${IML_DIR}/nullspace.c ${IML_DIR}/basisop.c ${IML_DIR}/certsolve.c ${IML_DIR}/error.c ${IML_DIR}/latreduce.c ${IML_DIR}/memalloc.c ${IML_DIR}/mtrans.c ${IML_DIR}/nonsysolve.c ${IML_DIR}/nullspace.c ${IML_DIR}/padiclift.c ${IML_DIR}/reconsolu.c ${IML_DIR}/RNSop.c )

include_directories(./ext_libs/iml-1.0.5/src)

if(BUILD_QT_GUI)
  #build the gui, bring in all necessary libraries

  set(CMAKE_AUTOMOC ON)
  set(CMAKE_AUTORCC ON)
  set(CMAKE_AUTOUIC ON)

  add_executable(SPIRE ${SRC_BASE} ${SRC_QT_GUI} ${SRC_IML} resources.qrc)

  find_package(Qt5 COMPONENTS Widgets Gui Svg REQUIRED)
  find_library(gmp gmp)

  SET(BLA_VENDOR OpenBLAS)
  find_package(BLAS)  
  target_link_libraries(SPIRE BLAS::BLAS gmp)


  #check if we have CGAL
  find_package(CGAL REQUIRED)    
  if( CGAL_FOUND )
    add_compile_definitions(USE_CGAL)
    target_link_libraries(SPIRE CGAL::CGAL)
  endif()

  find_package(PNG REQUIRED)
  if( PNG_FOUND )
    add_compile_definitions(HAVE_PNG)
    target_link_libraries(SPIRE PNG::PNG)
    target_include_directories(SPIRE PRIVATE PNG_INCLUDE_DIRS)
  endif()
    
  target_link_libraries(SPIRE Qt5::Widgets Qt5::Gui Qt5::Svg)
endif(BUILD_QT_GUI)


if(BUILD_COMPUTE_CHANNEL_PROPORTIONS)

  add_executable(SPIRE_get_channel_proportions ${SRC_BASE} ${SRC_IML} ./src/get_channel_proportions.cpp)

  find_library(gmp gmp)
  find_package(BLAS)    
  target_link_libraries(SPIRE_get_channel_proportions  BLAS::BLAS gmp)  
endif(BUILD_COMPUTE_CHANNEL_PROPORTIONS)


if( BUILD_BATCH_TOOL )

  add_executable(SPIRE_batch_tool ${SRC_BASE} ${SRC_IML} ./src/batch_tool.cpp resources.qrc)

  find_library(gmp gmp)
  find_package(BLAS)  
  find_package(CGAL REQUIRED)
  find_package(PNG REQUIRED)
  target_link_libraries(SPIRE_batch_tool BLAS::BLAS gmp)  

  if( PNG_FOUND )
    add_compile_definitions(HAVE_PNG)
    target_link_libraries(SPIRE_batch_tool PNG::PNG)
    target_include_directories(SPIRE_batch_tool PRIVATE PNG_INCLUDE_DIRS)
  endif()  

  if( CGAL_FOUND )
    add_compile_definitions(USE_CGAL)
    target_link_libraries(SPIRE_batch_tool CGAL::CGAL)
  endif()

endif( BUILD_BATCH_TOOL )
