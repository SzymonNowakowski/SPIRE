cmake_minimum_required(VERSION 3.16)
project (SPIRE)
set (CMAKE_CXX_STANDARD 11)

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

# Prefer a system/OpenBLAS cblas.h; fall back to IML's local header
find_path(CBLAS_INCLUDE_DIR
        NAMES cblas.h
        PATH_SUFFIXES openblas
)

if(BUILD_QT_GUI)
  #build the gui, bring in all necessary libraries

  set(CMAKE_AUTOMOC ON)
  set(CMAKE_AUTORCC ON)
  set(CMAKE_AUTOUIC ON)

  add_executable(SPIRE ${SRC_BASE} ${SRC_QT_GUI} ${SRC_IML} resources.qrc)

  # Ensure include priority: system/OpenBLAS first, then IML fallback
  if(CBLAS_INCLUDE_DIR)
    target_include_directories(SPIRE PRIVATE ${CBLAS_INCLUDE_DIR})
  endif()
  target_include_directories(SPIRE PRIVATE ${IML_DIR})

  # Qt6: QSvgWidget lives in SvgWidgets in Qt6
  find_package(Qt6 COMPONENTS Core Gui Widgets Svg SvgWidgets REQUIRED)

  SET(BLA_VENDOR OpenBLAS)
  find_package(BLAS)
  # remove gmp from link to avoid LNK1104; CGAL pulls GMP/MPFR transitively if needed
  target_link_libraries(SPIRE BLAS::BLAS)

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

  # add SvgWidgets here (Qt6 requirement for QSvgWidget)
  target_link_libraries(SPIRE Qt6::Widgets Qt6::Gui Qt6::Svg Qt6::SvgWidgets)

  # ========== Windows deployment (MinGW DLLs + Qt) ==========
  if(WIN32 AND MINGW)
    # Get the compiler's bin directory
    get_filename_component(COMPILER_DIR ${CMAKE_CXX_COMPILER} DIRECTORY)

    # List of required MinGW runtime DLLs
    set(MINGW_DLLS
            libstdc++-6.dll
            libgcc_s_seh-1.dll
            libwinpthread-1.dll
    )

    # Copy each DLL to the output directory after build
    foreach(DLL ${MINGW_DLLS})
      add_custom_command(TARGET SPIRE POST_BUILD
              COMMAND ${CMAKE_COMMAND} -E copy_if_different
              "${COMPILER_DIR}/${DLL}"
              $<TARGET_FILE_DIR:SPIRE>
              COMMENT "Copying ${DLL} to output directory"
      )
    endforeach()
  endif()

  # Windows: Run windeployqt to copy Qt DLLs and plugins
  if(WIN32)
    find_program(WINDEPLOYQT_EXECUTABLE windeployqt HINTS "${Qt6_DIR}/../../../bin")

    if(WINDEPLOYQT_EXECUTABLE)
      add_custom_command(TARGET SPIRE POST_BUILD
              COMMAND ${WINDEPLOYQT_EXECUTABLE}
              --no-compiler-runtime
              $<TARGET_FILE:SPIRE>
              COMMENT "Running windeployqt..."
      )
    endif()
  endif()
  # ========== End Windows deployment ==========
endif(BUILD_QT_GUI)

if(BUILD_COMPUTE_CHANNEL_PROPORTIONS)
  add_executable(SPIRE_get_channel_proportions ${SRC_BASE} ${SRC_IML} ./src/get_channel_proportions.cpp)

  # Ensure include priority: system/OpenBLAS first, then IML fallback
  if(CBLAS_INCLUDE_DIR)
    target_include_directories(SPIRE_get_channel_proportions PRIVATE ${CBLAS_INCLUDE_DIR})
  endif()
  target_include_directories(SPIRE_get_channel_proportions PRIVATE ${IML_DIR})

  # keep BLAS; drop gmp (not needed here, avoids unresolved manual gmp.lib)
  find_package(BLAS)
  target_link_libraries(SPIRE_get_channel_proportions BLAS::BLAS)
endif(BUILD_COMPUTE_CHANNEL_PROPORTIONS)


if( BUILD_BATCH_TOOL )

  add_executable(SPIRE_batch_tool ${SRC_BASE} ${SRC_IML} ./src/batch_tool.cpp resources.qrc)

  # Ensure include priority: system/OpenBLAS first, then IML fallback
  if(CBLAS_INCLUDE_DIR)
    target_include_directories(SPIRE_batch_tool PRIVATE ${CBLAS_INCLUDE_DIR})
  endif()
  target_include_directories(SPIRE_batch_tool PRIVATE ${IML_DIR})

  # keep exactly as before re: BLAS/CGAL/PNG; just drop gmp to avoid manual .lib
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
  # ========== Windows deployment for batch tool ==========
  if(WIN32 AND MINGW)
    # Get the compiler's bin directory
    get_filename_component(COMPILER_DIR ${CMAKE_CXX_COMPILER} DIRECTORY)

    # List of required MinGW runtime DLLs
    set(MINGW_DLLS
            libstdc++-6.dll
            libgcc_s_seh-1.dll
            libwinpthread-1.dll
    )

    # Copy each DLL to the output directory after build
    foreach(DLL ${MINGW_DLLS})
      add_custom_command(TARGET SPIRE_batch_tool POST_BUILD
              COMMAND ${CMAKE_COMMAND} -E copy_if_different
              "${COMPILER_DIR}/${DLL}"
              $<TARGET_FILE_DIR:SPIRE_batch_tool>
              COMMENT "Copying ${DLL} to batch tool output directory"
      )
    endforeach()
  endif()
  # ========== End Windows deployment for batch tool ==========

endif( BUILD_BATCH_TOOL )
