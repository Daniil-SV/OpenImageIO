#!/usr/bin/env bash

# Utility script to download and build libktx
#
# Copyright Contributors to the OpenImageIO project.
# SPDX-License-Identifier: Apache-2.0
# https://github.com/AcademySoftwareFoundation/OpenImageIO

# Exit the whole script if any command fails.
set -ex

# Repo and branch/tag/commit of ktx to download if we don't have it yet
KTX_REPO=${KTX_REPO:=https://github.com/KhronosGroup/KTX-Software.git}
KTX_VERSION=${KTX_VERSION:=v4.4.2}

# Where to put ktx repo source (default to the ext area)
KTX_SRC_DIR=${KTX_SOURCE_DIR:=${LOCAL_DEPS_DIR}/ktx}
# Temp build area
KTX_BUILD_DIR=${KTX_BUILD_DIR:=${LOCAL_DEPS_DIR}/ktx-build}
# Install area for ktx
KTX_INSTALL_DIR=${KTX_INSTALL_DIR:=${LOCAL_DEPS_DIR}/dist}

pwd
echo "libktx install dir will be: ${KTX_INSTALL_DIR}"

mkdir -p ./ext
pushd ./ext

# Clone libktx project from GitHub and build
if [[ ! -e ${KTX_SRC_DIR} ]] ; then
    echo "git clone ${KTX_REPO} ${KTX_SRC_DIR}"
    git clone ${KTX_REPO} ${KTX_SRC_DIR}
fi
cd ${KTX_SRC_DIR}


echo "git checkout ${KTX_VERSION} --force"
git checkout ${KTX_VERSION} --force

if [[ -z $DEP_DOWNLOAD_ONLY ]]; then
    time cmake -S . -B ${KTX_BUILD_DIR} -DCMAKE_BUILD_TYPE=Release \
               -DCMAKE_INSTALL_PREFIX=${KTX_INSTALL_DIR} \
               -DCMAKE_POSITION_INDEPENDENT_CODE=ON \
               -DKTX_FEATURE_TESTS=OFF \
               -DKTX_FEATURE_TESTS=OFF \
               -DKTX_FEATURE_VK_UPLOAD=OFF \
               -DKTX_FEATURE_GL_UPLOAD=OFF \
               -DKTX_FEATURE_TOOLS=OFF \
               -DBUILD_SHARED_LIBS=${KTX_BUILD_SHARED_LIBS:-ON}
    time cmake --build ${KTX_BUILD_DIR} --config Release --target install
fi

# ls -R ${KTX_INSTALL_DIR}
popd


# Set up paths. These will only affect the caller if this script is
# run with 'source' rather than in a separate shell.
export Ktx_ROOT=$KTX_INSTALL_DIR

