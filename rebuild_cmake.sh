#!/bin/bash


BUILD_TYPE="Release"
PWD='pwd'
PWD_PATH=${PWD}
SDK_TYPE="SDK_IPCAMERA"

PLATFORM_DIR="ade_vienna"
PLATFORM_TYPE=SCHUBERT
CMAKE_TOOLCHAIN_FILE=CrossCompile_toolchain.cmake

BUILD_DIR_PREFIX="build_"
BUILD_DIR=${BUILD_DIR_PREFIX}${PLATFORM_DIR}


bd_info()
{
    echo "####################################"
    echo "##  ${PLATFORM_DIR} [${BD_STAGE}]  $1##"
    echo "####################################"
}


check_error()
{
    if [ ! "$?" = "0" ];  then
    echo  ""
    echo  "!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!"
    echo  "!  ${PLATFORM_DIR}   [${BD_STAGE}] build  errot at :  'date'!"
    echo  "!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!"
    echo  ""
    fi
}


build()
{
    BD_STAGE=$1
    #cd ./$1
    mkdir  -p  ${BUILD_DIR}
    cd ${BUILD_DIR}
    bd_info "Begin to cmake ..."
    cmake .. -DCMAKE_BUILD_TYPE==${BUILD_TYPE}  \
                      -DPLATFORM_TYPE=${PLATFORM_TYPE}  \
                      -DSDK_TYPE=${SDK_TYPE}  \
                      -DCMAKE_TOOLCHAIN_FILE=./${CMAKE_TOOLCHAIN_FILE}
    
    check_error
    bd_info   "Begin to  make. ..."
    make  -j4
    check_error

    if [ ! "$2" = "NO-INSTALL" ];  then
        bd_info "Begin to make install"
        make install 
        check_error
    fi
}


rm -fr ade_vienna/bin/${PLATFORM_DIR}
rm -fr ade_vienna/build_${PLATFORM_DIR}

build  "test" "NO-INSTALL"

mv $(pwd)/../bin/Vision_Thermal  /mnt/nfs_share

