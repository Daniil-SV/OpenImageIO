#!/usr/bin/env bash

# Utility script to download and build libktx
#
# Copyright Contributors to the OpenImageIO project.
# SPDX-License-Identifier: Apache-2.0
# https://github.com/AcademySoftwareFoundation/OpenImageIO

# Exit the whole script if any command fails.
set -ex

# Repo and branch/tag/commit of libpng to download if we don't have it yet
LIBKTX_REPO=${LIBKTX_REPO:=https://github.com/KhronosGroup/KTX-Software.git}
KTX_VERSION=${KTX_VERSION:=v4.4.2}

# Where to put libpng repo source (default to the ext area)
LIBKTX_SRC_DIR=${LIBKTX_SRC_DIR:=${PWD}/ext/libktx}
# Temp build area (default to a build/ subdir under source)
LIBKTX_BUILD_DIR=${LIBKTX_BUILD_DIR:=${LIBKTX_SRC_DIR}/build}
# Install area for libpng (default to ext/dist)
LIBKTX_INSTALL_DIR=${LIBKTX_INSTALL_DIR:=${PWD}/ext/dist}
#LIBKTX_CONFIG_OPTS=${LIBKTX_CONFIG_OPTS:=}

pwd
echo "libktx install dir will be: ${LIBKTX_INSTALL_DIR}"

mkdir -p ./ext
pushd ./ext

# Clone libktx project from GitHub and build
if [[ ! -e ${LIBKTX_SRC_DIR} ]] ; then
    echo "git clone ${LIBKTX_REPO} ${LIBKTX_SRC_DIR}"
    git clone ${LIBKTX_REPO} ${LIBKTX_SRC_DIR}
fi
cd ${LIBKTX_SRC_DIR}


echo "git checkout ${KTX_VERSION} --force"
git checkout ${KTX_VERSION} --force

if [[ -z $DEP_DOWNLOAD_ONLY ]]; then
    time cmake -S . -B ${LIBKTX_BUILD_DIR} -DCMAKE_BUILD_TYPE=Release \
               -DCMAKE_INSTALL_PREFIX=${LIBKTX_INSTALL_DIR} \
               -DCMAKE_POSITION_INDEPENDENT_CODE=ON \
               -DKTX_FEATURE_TESTS=OFF \
               -DKTX_FEATURE_TESTS=OFF \
               -DKTX_FEATURE_VK_UPLOAD=OFF \
               -DKTX_FEATURE_GL_UPLOAD=OFF \
               -DKTX_FEATURE_TOOLS=OFF \
               ${LIBKTX_CONFIG_OPTS}
    time cmake --build ${LIBKTX_BUILD_DIR} --config Release --target install
fi

# ls -R ${LIBKTX_INSTALL_DIR}
popd


# Set up paths. These will only affect the caller if this script is
# run with 'source' rather than in a separate shell.
export KTX_ROOT=$LIBKTX_INSTALL_DIR

