include(ExternalProject)

SET(LIB_NAME libiniparser.so.0)
SET(LIB_LINK_NAME libiniparser.so)
#SET(HEADER src/dictionary.h src/iniparser.h)

ExternalProject_Add(iniparser_target
	URL ${CMAKE_CURRENT_SOURCE_DIR}/iniparser.tar.bz2
#	PATCH_COMMAND patch -p1 < ${CMAKE_CURRENT_SOURCE_DIR}/iniparser_make.patch
	CONFIGURE_COMMAND ""
	BUILD_IN_SOURCE 1
	BUILD_COMMAND ${ADDITIONAL_ENV_CONFIG} make libiniparser.so
	INSTALL_DIR ${OPEN_SRC_3RD_INSTALL_DIR}
	INSTALL_COMMAND mkdir -p <INSTALL_DIR>/lib && mkdir -p <INSTALL_DIR>/include/iniparser && install -m 755 <SOURCE_DIR>/${LIB_NAME} <INSTALL_DIR>/lib && install -m 644 <SOURCE_DIR>/src/dictionary.h <SOURCE_DIR>/src/iniparser.h <INSTALL_DIR>/include/iniparser && rm -f <INSTALL_DIR>/lib/${LIB_LINK_NAME} && ln -s ${LIB_NAME} <INSTALL_DIR>/lib/${LIB_LINK_NAME}
	)

