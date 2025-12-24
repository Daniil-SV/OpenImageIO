# Copyright Contributors to the OpenImageIO project.
# SPDX-License-Identifier: Apache-2.0
# https://github.com/AcademySoftwareFoundation/OpenImageIO

######################################################################
# Ktx by hand!
######################################################################
set(KTX_VERSION 0.0.0)
set_cache (Ktx_BUILD_VERSION ${KTX_VERSION} "KTX Software version for local builds")
set (Ktx_GIT_REPOSITORY "https://github.com/Daniil-SV/KTX-Software.git")
#set (Ktx_GIT_TAG "v${KTX_VERSION}")
set (Ktx_GIT_TAG "dependency-rework")
set_cache (Ktx_BUILD_SHARED_LIBS OFF 
           DOC "Should execute a local KTX build; if necessary, build shared libraries" ADVANCED)

string (MAKE_C_IDENTIFIER ${Ktx_BUILD_VERSION} Ktx_VERSION_IDENT)
set_cache (KTX_CMAKE_C_COMPILER ${CMAKE_C_COMPILER} "ktx build C compiler override" ADVANCED)
set_cache (KTX_CMAKE_CXX_COMPILER ${CMAKE_CXX_COMPILER} "ktx build C++ compiler override" ADVANCED)

build_dependency_with_cmake(Ktx
    VERSION         ${Ktx_BUILD_VERSION}
    GIT_REPOSITORY  ${Ktx_GIT_REPOSITORY}
    GIT_TAG         ${Ktx_GIT_TAG}
    SOURCE_SUBDIR   lib/
    CMAKE_ARGS
        -D BUILD_SHARED_LIBS=${Ktx_BUILD_SHARED_LIBS}
        -D CMAKE_POSITION_INDEPENDENT_CODE=ON
        -D CMAKE_INSTALL_LIBDIR=lib
        -D CMAKE_C_COMPILER=${KTX_CMAKE_C_COMPILER}
        -D CMAKE_CXX_COMPILER=${KTX_CMAKE_CXX_COMPILER}
        -DCMAKE_PREFIX_PATH=${${PROJECT_NAME}_LOCAL_DEPS_ROOT}/dist
    )

# Set some things up that we'll need for a subsequent find_package to work
set (Ktx_ROOT ${Ktx_LOCAL_INSTALL_DIR})
set (Ktx_DIR ${Ktx_LOCAL_INSTALL_DIR})

# Signal to caller that we need to find again at the installed location
set (Ktx_REFIND TRUE)
set (Ktx_REFIND_VERSION ${Ktx_BUILD_VERSION})
set (Ktx_REFIND_ARGS CONFIG)

if (Ktx_BUILD_SHARED_LIBS)
    install_local_dependency_libs (Ktx Ktx)
else()
    add_compile_definitions(KHRONOS_STATIC)
endif ()
