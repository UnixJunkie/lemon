#!/bin/bash -xe

# Install doc dependencies
cd $TRAVIS_BUILD_DIR
pip install --user --upgrade -r doc/requirements.txt

DOXYGEN_VER=1.8.13_1
DOXYGEN_URL="https://linuxbrew.bintray.com/bottles/doxygen-${DOXYGEN_VER}.x86_64_linux.bottle.1.tar.gz"
wget -O - "${DOXYGEN_URL}" | tar xz -C /tmp doxygen/${DOXYGEN_VER}/bin/doxygen
export PATH="/tmp/doxygen/${DOXYGEN_VER}/bin:$PATH"

cd build

# Get previous documentation
git clone https://github.com/$TRAVIS_REPO_SLUG --branch gh-pages gh-pages
rm -rf gh-pages/.git
rm -rf gh-pages/deployed*

# Build new documentation
cmake -DLEMON_BUILD_DOCS=ON .
cmake --build . --target doc_html
rm -rf doc/html/.doctrees/ doc/html/.buildinfo

# Copy documentation to the right place
if [[ "$TRAVIS_TAG" != "" ]]; then
    mv doc/html/ gh-pages/$TRAVIS_TAG
else
    rm -rf gh-pages/latest
    mv doc/html/ gh-pages/latest
fi
