option(WITH_TRIANGLE "Check to compile with triangle.c (only free for non-commercial usage), or use qhull instead (requires qhull)" ON)

if(WITH_TOOLS)
	project(grid_gridding)

	# add files in sudirectories
	set(SAGA_TOOL_CUSTOM_SOURCE_LIST
		./nn/delaunay.c
		./nn/hash.c
		./nn/istack.c
		./nn/lpi.c
		./nn/nnai.c
		./nn/nncommon.c
		./nn/nncommon-vulnerable.c
		./nn/nnpi.c
	)
	set(SAGA_TOOL_CUSTOM_HEADER_LIST
		grid_cell_polygon_coverage.h
		Interpolation.h
		Interpolation_AngularDistance.h
		Interpolation_InverseDistance.h
		Interpolation_NaturalNeighbour.h
		Interpolation_NearestNeighbour.h
		Interpolation_Shepard.h
		Interpolation_Triangulation.h
	)
	if(WITH_TRIANGLE)	# use embedded triangle.c
		list(APPEND SAGA_TOOL_CUSTOM_SOURCE_LIST nn/triangle.c)
		list(APPEND SAGA_TOOL_CUSTOM_HEADER_LIST nn/triangle.h)
        message(STATUS "building with triangle")
	endif()

	include(../../CMakePluginTemplate.cmake)	# load common tool template

	if(NOT WITH_TRIANGLE)
		# link Qhull library
		# on Ubuntu 16.04 package libqhull-dev
		find_package(Qhull REQUIRED)
        find_file(LIBQHULL_HEADER NAMES libqhull/libqhull.h)
        if(LIBQHULL_HEADER)
		    target_compile_definitions(${PROJECT_NAME} PRIVATE -DUSE_QHULL -DHAVE_LIBQHULL_QHULL_A_H)
            message(STATUS "LIBQHULL header found")
        else()
            target_compile_definitions(${PROJECT_NAME} PRIVATE -DUSE_QHULL -DHAVE_QHULL_QHULL_A_H)
            message(STATUS "QHULL header found")
        endif()
		target_include_directories(${PROJECT_NAME} PRIVATE ${Qhull_INCLUDE_DIRS})
		target_link_libraries(${PROJECT_NAME} ${Qhull_LIBRARIES})
	endif()
endif()
