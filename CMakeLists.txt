CMAKE_MINIMUM_REQUIRED( VERSION 2.8 )
SET( MODULE_NAME xtestx )
GET_FILENAME_COMPONENT( CONFIGURATION_PROJECT ${CMAKE_BINARY_DIR} NAME )

####################################################################
#                                                                  #
#                        i586                                      #
#                                                                  #
####################################################################
IF( CONFIGURATION_PROJECT STREQUAL "i586" )
	MESSAGE( "=== i586 ===" )
	ADD_DEFINITIONS( -DDEBUG_I586  -Di586)

	# Variabili da Settare
	SET( ARCHITETTURA                    i386 )
	SET( PREFIX_CROSS_COMPILER           "" )
	SET( CMAKE_CXX_COMPILER              ${PREFIX_CROSS_COMPILER}c++ )
	SET( CMAKE_C_COMPILER                ${PREFIX_CROSS_COMPILER}gcc )
	SET( CMAKE_LINKER                    ${PREFIX_CROSS_COMPILER}ld )

	PROJECT( ${MODULE_NAME} )
	SET( CMAKE_INSTALL_PREFIX           ${PROJECT_SOURCE_DIR}/release/i586 )
	SET( BASE_DEVELOP                   "" )
	
	SET( CMAKE_CXX_FLAGS "-DCAN_OVER_ETHERNET -DDRIVE_CAN_NAME='FlexCAN'")
	
ENDIF ()


####################################################################
#                                                                  #
#                        ARM- IMX285                               #
#                                                                  #
####################################################################
IF( CONFIGURATION_PROJECT STREQUAL "arm-imx285.release" )
	MESSAGE( "=== arm-imx285.release ===" )
	ADD_DEFINITIONS( -DRELEASE_IMX285 -D__KERNEL_STRICT_NAMES -DIMX285 )
	SET ( imx285_COMUNE Y )
ELSEIF( CONFIGURATION_PROJECT STREQUAL "arm-imx285.debug" )
	MESSAGE( "=== arm-imx285.debug ===" )
	ADD_DEFINITIONS( -D_DEBUG -DDEBUG_IMX285 -D__KERNEL_STRICT_NAMES -DIMX285 --debug )
	SET ( imx285_COMUNE Y )
ENDIF ()

IF( imx285_COMUNE )
####################################################################
#                                                                  #
#                        ARM9                                      #
#                                                                  #
####################################################################
	# Variabili da Settare
	SET( ARCHITETTURA                    arm )
	SET( PREFIX_CROSS_COMPILER           "arm-SK5783-linux-gnueabi-" )
	SET( CMAKE_CXX_COMPILER              ${PREFIX_CROSS_COMPILER}c++ )
	SET( CMAKE_C_COMPILER                ${PREFIX_CROSS_COMPILER}gcc )
	SET( CMAKE_LINKER                    ${PREFIX_CROSS_COMPILER}ld )

	PROJECT( ${MODULE_NAME} )
	SET( CMAKE_INSTALL_PREFIX 		/home/dinex/Cdk/SK5783.imx285/trunk/cdk/rootfs )
	SET( BASE_DEVELOP 			/home/dinex/Cdk/SK5783.imx285/trunk/cdk/opt )

#	SET( KERNEL_VERSION "2.6.35" )	
#	SET( KERNEL_DIR ${BASE_DEVELOP}-linux-${KERNEL_VERSION} )

ENDIF ()

####################################################################
#                                                                  #
#                        PARTE GENERALE                            #
#                                                                  #
####################################################################

SET( CMAKE_CXX_FLAGS -std=gnu++11 )
# SET( CMAKE_EXE_LINKER_FLAGS "")
SET( LINK_LIBRARIES pthread rt dl)
ADD_DEFINITIONS( -D_DISABLE_ELPP_ASSERT -Wall -Wextra -fno-strict-aliasing )
FIND_PACKAGE (Threads)

INCLUDE_DIRECTORIES(
	${PROJECT_SOURCE_DIR}/src
	${PROJECT_SOURCE_DIR}/src/lib
)

MESSAGE(  ${BASE_DEVELOP}/${LINUX_VERSION_DIR}/include )

SET( xtestx_SRCS
    src/lib/configfile.cpp
	src/lib/epoll_fds_mgr.cpp
	src/lib/fileutility.cpp
	src/lib/timer_pool.cpp
	src/lib/sock_server.cpp
	src/lib/syssettings.cpp
	src/lib/typedumpers.cpp
    src/lib/version.c
	src/main.cpp
	src/appl.cpp
	src/appl2.cpp
)

SET( xtestx_INCS
    src/lib/configfile.hpp
	src/lib/easylogging++.hpp
	src/lib/logging.hpp
	src/lib/asciibin.hpp
	src/lib/epoll_fds_mgr.hpp
	src/lib/fileutility.hpp
	src/lib/sock_server.hpp
    src/lib/syssettings.h
    src/lib/timer_pool.hpp
	src/lib/typedumpers.hpp
	src/appl.hpp
	src/appl2.hpp
    src/applConfigFile.hpp
)

ADD_EXECUTABLE( xtestx ${xtestx_SRCS} )
TARGET_LINK_LIBRARIES( xtestx ${LINK_LIBRARIES} )
INSTALL( TARGETS ${MODULE_NAME} DESTINATION home/dinex/bin )
	