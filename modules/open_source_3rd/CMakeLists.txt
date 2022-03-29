# We need to add the sub-directory of each project follows their dependency
# because we use ExternalProject_Add function to build the open source project.

INCLUDE(${CMAKE_MODULE_DIR}/ConditionalAddSubDir.cmake)

ADD_SUBDIRECTORY(iniparser)
ADD_SUBDIRECTORY(fdk-aac)
ADD_SUBDIRECTORY(g711)
ADD_SUBDIRECTORY(g726)

IF(GAMR_SUPPORT_ENABLE)
	ADD_SUBDIRECTORY(opencore-amr)
ENDIF()
