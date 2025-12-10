# Copyright Contributors to the OpenImageIO project.
# SPDX-License-Identifier: Apache-2.0
# https://github.com/AcademySoftwareFoundation/OpenImageIO

if (Ktx_FOUND)
    add_oiio_plugin (ktxinput.cpp stream.cpp format.cpp
                     LINK_LIBRARIES KTX::ktx
                     DEFINITIONS "USE_KTX=1")
else ()
    message (STATUS "KTX plugin will not be built")
endif()
