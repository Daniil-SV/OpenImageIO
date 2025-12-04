# Copyright Contributors to the OpenImageIO project.
# SPDX-License-Identifier: Apache-2.0
# https://github.com/AcademySoftwareFoundation/OpenImageIO

######################################################################
# KTX by hand!
######################################################################

set_cache (KTX_BUILD_VERSION 4.4.2 "KTX Software version for local builds")
set (KTX_GIT_REPOSITORY "https://github.com/KhronosGroup/KTX-Software.git")
set (KTX_GIT_TAG "v${KTX_BUILD_VERSION}")
set_cache (KTX_BUILD_SHARED_LIBS OFF 
           DOC "Should execute a local KTX build; if necessary, build shared libraries" ADVANCED)

string (MAKE_C_IDENTIFIER ${KTX_BUILD_VERSION} KTX_VERSION_IDENT)

build_dependency_with_cmake(KTX
    VERSION         ${KTX_BUILD_VERSION}
    GIT_REPOSITORY  ${KTX_GIT_REPOSITORY}
    GIT_TAG         ${KTX_GIT_TAG}
    CMAKE_ARGS
        -D BUILD_SHARED_LIBS=${KTX_BUILD_SHARED_LIBS}
        -D KTX_FEATURE_TESTS=OFF
        -D KTX_FEATURE_TESTS=OFF
        -D KTX_FEATURE_VK_UPLOAD=OFF
        -D KTX_FEATURE_GL_UPLOAD=OFF
        -D KTX_FEATURE_TOOLS=OFF
        -D CMAKE_INSTALL_LIBDIR=lib
    )

# Set some things up that we'll need for a subsequent find_package to work
set (KTX_ROOT ${KTX_LOCAL_INSTALL_DIR})
set (KTX_DIR ${KTX_LOCAL_INSTALL_DIR})

# Signal to caller that we need to find again at the installed location
set (KTX_REFIND TRUE)
set (KTX_REFIND_VERSION ${KTX_BUILD_VERSION})
set (KTX_REFIND_ARGS CONFIG)

if (KTX_BUILD_SHARED_LIBS)
    install_local_dependency_libs (KTX KTX)
else()
    add_compile_definitions(KHRONOS_STATIC)
endif ()
