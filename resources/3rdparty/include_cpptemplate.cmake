string(REPLACE " " ";" CMAKE_CXX_FLAGS_AS_LIST ${CMAKE_CXX_FLAGS})
if(CMAKE_BUILD_TYPE MATCHES DEBUG)
	string(REPLACE " " ";" CMAKE_CXX_FLAGS_DEBUG_AS_LIST ${CMAKE_CXX_FLAGS_DEBUG})
    list(APPEND CMAKE_CXX_FLAGS_AS_LIST ${CMAKE_CXX_FLAGS_DEBUG_AS_LIST})
else()
	string(REPLACE " " ";" CMAKE_CXX_FLAGS_RELEASE_AS_LIST ${CMAKE_CXX_FLAGS_RELEASE})
    list(APPEND CMAKE_CXX_FLAGS_AS_LIST ${CMAKE_CXX_FLAGS_RELEASE_AS_LIST})
endif()
set(CPPTEMPLATE_LIB_DIR ${STORM_3RDPARTY_BINARY_DIR}/cpptemplate)
ExternalProject_Add(
        cpptemplate
        DOWNLOAD_COMMAND ""
        SOURCE_DIR ${STORM_3RDPARTY_SOURCE_DIR}/cpptemplate
        PREFIX ${STORM_3RDPARTY_BINARY_DIR}/cpptemplate
        CONFIGURE_COMMAND ""
        BUILD_COMMAND ${CMAKE_CXX_COMPILER} -c ${CMAKE_CXX_FLAGS_AS_LIST} ${CMAKE_CXX_FLAGS_RELEASE_AS_LIST} -fPIC -I${Boost_INCLUDE_DIRS} ${STORM_3RDPARTY_SOURCE_DIR}/cpptemplate/cpptempl.cpp -o ${CPPTEMPLATE_LIB_DIR}/cpptemplate${STATIC_EXT}
        INSTALL_COMMAND ""
        BUILD_IN_SOURCE 0
        LOG_BUILD ON
        BUILD_BYPRODUCTS ${CPPTEMPLATE_LIB_DIR}/cpptemplate${DYNAMIC_EXT} ${CPPTEMPLATE_LIB_DIR}/cpptemplate${STATIC_EXT}
)

set(CPPTEMPLATE_INCLUDE_DIR ${STORM_3RDPARTY_SOURCE_DIR}/cpptemplate)
set(CPPTEMPLATE_STATIC_LIBRARY ${STORM_3RDPARTY_BINARY_DIR}/cpptemplate/cpptemplate${STATIC_EXT})
add_dependencies(resources cpptemplate)

message(STATUS "Storm - Linking with cpptemplate.")
add_imported_library(cpptempl STATIC ${CPPTEMPLATE_STATIC_LIBRARY} ${CPPTEMPLATE_INCLUDE_DIR})
list(APPEND STORM_DEP_TARGETS cpptempl_STATIC)