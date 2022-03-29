include(ExternalProject)

ExternalProject_Add(opencore-amr_target
	URL ${CMAKE_CURRENT_SOURCE_DIR}/opencore-amr-0.1.3.tar.gz
	CONFIGURE_COMMAND ${ADDITIONAL_ENV_CONFIG} ./configure --host=arm-linux --prefix=<INSTALL_DIR>
	BUILD_IN_SOURCE 1
	BUILD_COMMAND ${ADDITIONAL_ENV_CONFIG} make install 
	INSTALL_DIR ${OPEN_SRC_3RD_INSTALL_DIR}
	TEST_COMMAND ${CMAKE_STRIP} ${OPEN_SRC_3RD_INSTALL_DIR}/lib/libopencore-amrnb.so.0.0.3
	INSTALL_COMMAND rm -rf <INSTALL_DIR>/include/opencore/ && rm -rf <INSTALL_DIR>/include/opencore-amrwb && mv <INSTALL_DIR>/include/opencore-amrnb/ <INSTALL_DIR>/include/opencore/ && rm -rf <INSTALL_DIR>/lib/pkgconfig		
	)

