cmake_minimum_required(VERSION 2.8)

project(Vatics-adlr064)

IF(CMAKE_BUILD_TYPE  STREQUAL  "Debug")
    IF(CMAKE_BUILD_TYPE  STREQUAL  "RelWithDebInfo")
        MESSGAE("======== Release Mode ========")
        set(CMAKE_C_FLAGS  "${CMAKE_C_FLAGS}   -std=gnu99  -O3  -Wall -Wextra -Werror -pedantic -pthread")
        set(CMAKE_CXX_GLAGS  "${CMAKE_CXX_FLAGS} -std=c++11 -O3 -Wall -Wextra -Werror -pedantic -pthread")
        add_definitions("-DDEBUG")
        set(DEBUG_ENABLE  true)

    ELSE(CMAKE_BUILD_TYPE STREQUAL "RelWithDebInfo")
        MESSGAE("======== Release Mode ========")
        set(CMAKE_C_FLAGS  "${CMAKE_C_FLAGS}   -std=gnu99  -O3  -Wall -Wextra -Werror -pedantic -pthread")
        set(CMAKE_CXX_GLAGS  "${CMAKE_CXX_FLAGS} -std=c++11 -O3 -Wall -Wextra -Werror -pedantic -pthread")
        add_definitions("-DNDEBUG")
        set(DEBUG_ENABLE  false)
    ENDIF(CMAKE_BUILD_TYPE STREQUAL "RelWithDebInfo")
ENDIF(CMAKE_BUILD_TYPE STREQUAL "Debug")

set(VIENNA_PLATFORM  true CACHE BOOL "target to Vienna platform")
set(PLATFORM_DIR vienna)
set(MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/modules")
set(MODULE_LIB "${MODULE_PATH}/libs/${PLATFORM_DIR}")
set(MODULE_OPEN_SRC_3RD_INSTALL_DIR "${MODULE_PATH}/install_temp_open_src_3rd")
set(VTCS_SW_DEV_ROOT  "${CMAKE_CURRENT_SOURCE_DIR}/vtcs_root_${PLATFORM_DIR}")

IF(CMAKE_CROSSCOMPILING)
    set(HOST_CONFIG  --host=arm-linux)
    # This is for CMake > 2.8.11
    IF (${CMAKE_VERSION} VERSION_GREATER 2.8.11)
        GET_FILENAME_COMPONENT(MY_TOOLCHAIN_DIR ${CMAKE_C_COMPILER} DIRECTORY)
    # This is for CMake <= 2.8.11
    ELSE()
        GET_FILENAME_COMPONENT(MY_TOOLCHAIN_DIR  ${CMAKE_C_COPILER} PATH)
    ENDIF()
    set(ADDITIONAL_ENV_CONFIG  CC=${CMAKE_C_COMPILER}  CXX=${CMAKE_CXX_COMPILER}  LD=${CMAKE_LINKER} NM=${CMAKE_NM} AR=${CMAKE_AR} RANLIB=${CMAKE_RANLIB} STRIP=${CMAKE_STRIP} OBJDUMP=${CMAKE_OBJDUMP}  PKG_CONFIG=${MY_TOOLCHAIN_DIR}/pkg-config)
ENDIF(CMAKE_CROSSCOMPILING)

include_directories(${MODULE_PATH}  ${MODULE_OPEN_SRC_3RD_INSTALL_DIR}/include ${VTCS_SW_DEV_ROOT}/include  ${VTCS_SW_DEV_ROOT}/include/vmf   ${CMAKE_CURRENT_SOURCE_DIR}/include ${CMAKE_CURRENT_SOURCE_DIR}/pi_mobile/axclib ${CMAKE_CURRENT_SOURCE_DIR}/sqlite3_arm/include)
link_directories(${MODULE_LIB}  ${MODULE_OPEN_SRC_3RD_INSTALL_DIR}/lib  ${VTCS_SW_DEV_ROOT}/lib  ${CMAKE_CURRENT_SOURCE_DIR}/lib)
set(EXECUTABLE_OUTPUT_PATH  ${CMAKE_CURRENT_SOURCE_DIR}/bin)
set(SRC_ ${CMAKE_CURRENT_SOURCE_DIR}/src)
 

## only thermal
#set(SRC_LIST ${SRC_}/adlr064.cpp ${SRC_}/dbm.cpp  ${SRC_}/main.cpp  ${SRC_}/net.cpp ${SRC_}/spi.cpp ${SRC_}/ulis_encode.cpp )
#list(APPEND LINK_LIST axclib sqlite3)
#set(TARGET_NAME OnlyThermal)
#add_executable(${TARGET_NAME} ${SRC_LIST})
#target_link_libraries(${TARGET_NAME}  ${LINK_LIST})


## with imx307 vision
set(SRC_LIST ${SRC_}/adlr064.cpp ${SRC_}/dbm.cpp  ${SRC_}/main.cpp  ${SRC_}/net.cpp ${SRC_}/spi.cpp ${SRC_}/ulis_encode.cpp ${SRC_}/imx307.cpp)
list(APPEND LINK_LIST axclib sqlite3 syncringbuffer msgbroker)
set(TARGET_NAME Vision_Thermal)
add_executable(${TARGET_NAME} ${SRC_LIST})
target_link_libraries(${TARGET_NAME}  ${LINK_LIST})




#set(SRC_LIST ${CMAKE_CURRENT_SOURCE_DIR}/src/VaticsFirmwork.cpp)
#list(APPEND LINK_LIST vmf membroker msgbroker syncringbuffer)
#set(TARGET_NAME Vatics)
#add_executable(${TARGET_NAME} ${SRC_LIST})
#target_link_libraries(${TARGET_NAME}  ${LINK_LIST})
