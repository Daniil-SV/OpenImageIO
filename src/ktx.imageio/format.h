// Copyright Contributors to the OpenImageIO project.
// SPDX-License-Identifier: Apache-2.0
// https://github.com/AcademySoftwareFoundation/OpenImageIO

#pragma once

#include <ktx.h>
#include <OpenImageIO/typedesc.h>

OIIO_NAMESPACE_BEGIN
namespace ktx {

enum class glColorspace { Linear = 0, sRGB };

struct glDataDescriptor {
    int channels_count;
    glColorspace colorspace;

    ///< Data format of the channels.
    TypeDesc::BASETYPE type;

    ///< The names of each channel, in order. Typically this will be "R",
    ///< "G", "B", "A" (alpha), "Z" (depth), or other arbitrary names.
    std::vector<std::string> channel_order;
};

/// @param glFormat
///		glFormat or glBaseInternalFormat of Khronos texture
/// @return
///		Number of color channels for gl format
int
get_format_channels(ktx_uint32_t glFormat);

glDataDescriptor
get_format_descriptor(ktx_uint32_t glFormat, ktx_uint32_t glInternalFormat);

}  // namespace ktx
OIIO_NAMESPACE_END