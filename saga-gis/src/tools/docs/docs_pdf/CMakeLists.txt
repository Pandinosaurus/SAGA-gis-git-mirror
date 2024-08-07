option(WITH_TOOLS_HPDF "Check to create build targets for Haru PDF tools" ON)

if(WITH_TOOLS_HPDF)
	if(MSVC AND DEPS_FROM_ENVARS)
		if("${CMAKE_GENERATOR_PLATFORM}" STREQUAL "Win32")
			set(LIB4SAGA $ENV{HARU_32})
		else()
			set(LIB4SAGA $ENV{HARU})
		endif()
		find_path   (HPDF_INCLUDE NAMES hpdf.h       PATHS "${LIB4SAGA}/include")
		find_library(HPDF_LIBRARY NAMES hpdf libhpdf PATHS "${LIB4SAGA}")
		mark_as_advanced(HPDF_INCLUDE HPDF_LIBRARY)
		include(FindPackageHandleStandardArgs)
		FIND_PACKAGE_HANDLE_STANDARD_ARGS(HPDF REQUIRED_VARS HPDF_LIBRARY HPDF_INCLUDE)
	endif()
	if(NOT HPDF_FOUND)
		find_package(HPDF QUIET)	# ubuntu package libhpdf-dev
	endif()
	if(HPDF_FOUND)
		project(docs_pdf)
		include(../../CMakePluginTemplate.cmake)	# load common tool template
		target_include_directories(${PROJECT_NAME} PRIVATE "${HPDF_INCLUDE}")
		target_link_libraries(${PROJECT_NAME} "${HPDF_LIBRARY}")
	endif()
endif()
