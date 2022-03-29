#
# audio_server_mmap
#
SET(SRC_LIST audio_server_mmap.cpp audio_setting.cpp)
SET(LINK_LIST atk_audio_utils_mmap asound syncringbuffer membroker msgbroker atk_encoder fdk-aacdec g711sdec g726sdec)

# hw aac4
LIST(APPEND LINK_LIST vmf) 

INCLUDE_DIRECTORIES(${CMAKE_CURRENT_SOURCE_DIR} ${MODULE_OPEN_SRC_3RD_INSTALL_DIR}/include)
LINK_DIRECTORIES(${MODULE_OPEN_SRC_3RD_INSTALL_DIR}/lib)

SET(TARGET_NAME audio_server_mmap)
ADD_EXECUTABLE(${TARGET_NAME} ${SRC_LIST})
TARGET_LINK_LIBRARIES(${TARGET_NAME} ${LINK_LIST})

#
# audio_client_mmap
#
SET(SRC_LIST audio_client_mmap.cpp audio_setting.cpp)
SET(LINK_LIST atk_audio_utils_mmap asound syncringbuffer membroker msgbroker atk_encoder fdk-aacdec g711sdec g726sdec)

# hw aac4
LIST(APPEND LINK_LIST vmf) 

INCLUDE_DIRECTORIES(${CMAKE_CURRENT_SOURCE_DIR} ${MODULE_OPEN_SRC_3RD_INSTALL_DIR}/include)
LINK_DIRECTORIES(${MODULE_OPEN_SRC_3RD_INSTALL_DIR}/lib)

SET(TARGET_NAME audio_client_mmap)
ADD_EXECUTABLE(${TARGET_NAME} ${SRC_LIST})
TARGET_LINK_LIBRARIES(${TARGET_NAME} ${LINK_LIST})
