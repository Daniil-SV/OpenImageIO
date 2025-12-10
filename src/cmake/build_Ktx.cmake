# Copyright Contributors to the OpenImageIO project.
# SPDX-License-Identifier: Apache-2.0
# https://github.com/AcademySoftwareFoundation/OpenImageIO

######################################################################
# Ktx by hand!
######################################################################

set_cache (Ktx_BUILD_VERSION 4.4.2 "KTX Software version for local builds")
set (Ktx_GIT_REPOSITORY "https://github.com/KhronosGroup/KTX-Software.git")
set (Ktx_GIT_TAG "v${Ktx_BUILD_VERSION}")
set_cache (Ktx_BUILD_SHARED_LIBS OFF 
           DOC "Should execute a local KTX build; if necessary, build shared libraries" ADVANCED)

string (MAKE_C_IDENTIFIER ${Ktx_BUILD_VERSION} Ktx_VERSION_IDENT)

build_dependency_with_cmake(Ktx
    VERSION         ${Ktx_BUILD_VERSION}
    GIT_REPOSITORY  ${Ktx_GIT_REPOSITORY}
    GIT_TAG         ${Ktx_GIT_TAG}
    CMAKE_ARGS
        -D BUILD_SHARED_LIBS=${Ktx_BUILD_SHARED_LIBS}
        -D CMAKE_POSITION_INDEPENDENT_CODE=ON
        -D KTX_FEATURE_TESTS=OFF
        -D KTX_FEATURE_TESTS=OFF
        -D KTX_FEATURE_VK_UPLOAD=OFF
        -D KTX_FEATURE_GL_UPLOAD=OFF
        -D KTX_FEATURE_TOOLS=OFF
        -D CMAKE_INSTALL_LIBDIR=lib
    )

# Set some things up that we'll need for a subsequent find_package to work
set (Ktx_ROOT ${Ktx_LOCAL_INSTALL_DIR})
set (Ktx_DIR ${Ktx_LOCAL_INSTALL_DIR})

# Signal to caller that we need to find again at the installed location
find_package (Ktx ${Ktx_BUILD_VERSION} EXACT CONFIG REQUIRED)

if (Ktx_BUILD_SHARED_LIBS)
    install_local_dependency_libs (Ktx Ktx)
else()
    add_compile_definitions(KHRONOS_STATIC)
endif ()
