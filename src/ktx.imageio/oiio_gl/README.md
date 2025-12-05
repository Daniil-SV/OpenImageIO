## This folder contains auto generated headers from [KTX Software](https://github.com/KhronosGroup/KTX-Software/tree/v4.4.2/lib)  

Headers:
- [formatsize.h](formatsize.h)
- [gl_format.h](gl_format.h)
- [vk2gl.h](vk2gl.h)
- [vkformat_enum.h](vkformat_enum.h)
- [vkFormat2glFormat.inl](vkFormat2glFormat.inl)
- [vkFormat2glInternalFormat.inl](vkFormat2glInternalFormat.inl)
- [vkFormat2glType.inl](vkFormat2glType.inl)

## Notes:
- [Since libktx does not provide these important headers](https://github.com/KhronosGroup/KTX-Software/issues/1020), when updating libktx version, it is advisable to copy the above-mentioned files to this folder
- [glcorearb.h](GL/glcorearb.h) was written by hand to fill gaps in `vkFormat2glInternalFormat.inl` instead of copying the large original file to the repository