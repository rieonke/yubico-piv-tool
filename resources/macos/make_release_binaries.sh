#!/bin/bash
# Script to produce an OS X binaries
# This script has to be run from the source directory
if [ "$#" -ne 3 ]; then
    echo "This script build the binaries to be installed on a MacOS. This script should be run from the main project directory"
    echo ""
    echo "      Usage: ./resources/macos/make_release_binaries.sh <Release version> <SO version> <Value of CMAKE_INSTALL_PREFIX>"
    echo "";
    echo "Release version               : Full yubico-piv-tool version, tex 2.1.0"
    echo "SO version                    : The version of the ykpiv library, tex 2"
    echo "Value of CMAKE_INSTALL_PREFIX : The value of the CMAKE_INSTALL_PREFIX, tex /usr/local. Can be displayed by running 'cmake -L | grep CMAKE_INSTALL_PREFIX'"
    exit 0
fi

VERSION=$1 # Full yubico-piv-tool version, tex 2.1.0
SO_VERSION=$2
CMAKE_INSTALL_PREFIX=$3 # The value of the CMAKE_INSTALL_PREFIX, tex /usr/local. Can be displayed by running "cmake -L | grep CMAKE_INSTALL_PREFIX"

echo "Release version : $VERSION"
echo "SO version: $SO_VERSION"
echo "CMAKE_INSTALL_PREFIX: $CMAKE_INSTALL_PREFIX"
echo "Working directory: $PWD"

set -x

PACKAGE=yubico-piv-tool
CFLAGS="-mmacosx-version-min=10.6"

SOURCE_DIR=$PWD
MAC_DIR=$SOURCE_DIR/resources/macos
PKG_DIR=$MAC_DIR/pkgtmp
INSTALL_DIR=$PKG_DIR/install
FINAL_INSTALL_DIR=$INSTALL_DIR/$CMAKE_INSTALL_PREFIX
BUILD_DIR=$PKG_DIR/build
LICENSE_DIR=$PKG_DIR/licenses
BREW_DIR=/opt/homebrew/opt


# Create missing directories
rm -rf $PKG_DIR
mkdir -p $PKG_DIR $INSTALL_DIR $BUILD_DIR $LICENSE_DIR $FINAL_INSTALL_DIR

# Build yubico-piv-tool and install it in $INSTALL_DIR
cd $BUILD_DIR
CFLAGS=$CFLAGS CMAKE_OSX_ARCHITECTURES=arm64 PKG_CONFIG_PATH=$BREW_DIR/openssl/lib/pkgconfig cmake $SOURCE_DIR -DCMAKE_BUILD_TYPE=Release
make
env DESTDIR="$INSTALL_DIR" make install;

# Fix paths
cp $BREW_DIR/openssl/lib/libcrypto.1.1.dylib $FINAL_INSTALL_DIR/lib
chmod u+w $FINAL_INSTALL_DIR/lib/libcrypto.1.1.dylib

ls $FINAL_INSTALL_DIR/lib

install_name_tool -id "@loader_path/libcrypto.1.1.dylib" "$FINAL_INSTALL_DIR/lib/libcrypto.1.1.dylib"
install_name_tool -id "@loader_path/libykpiv.$SO_VERSION.dylib" "$FINAL_INSTALL_DIR/lib/libykpiv.$SO_VERSION.dylib"
install_name_tool -id "@loader_path/libykcs11.$SO_VERSION.dylib" "$FINAL_INSTALL_DIR/lib/libykcs11.$SO_VERSION.dylib"

install_name_tool -add_rpath "@loader_path/../lib" "$FINAL_INSTALL_DIR/lib/libykpiv.$SO_VERSION.dylib"
install_name_tool -add_rpath "@loader_path/../lib" "$FINAL_INSTALL_DIR/lib/libykcs11.$SO_VERSION.dylib"
install_name_tool -add_rpath "@loader_path/../lib" "$FINAL_INSTALL_DIR/bin/yubico-piv-tool"

install_name_tool -change "$BREW_DIR/openssl@1.1/lib/libcrypto.1.1.dylib" "@loader_path/libcrypto.1.1.dylib" "$FINAL_INSTALL_DIR/lib/libykpiv.$SO_VERSION.dylib"

install_name_tool -change "$FINAL_INSTALL_DIR/lib/libykpiv.$SO_VERSION.dylib" "@loader_path/../lib/libykpiv.$SO_VERSION.dylib" "$FINAL_INSTALL_DIR/lib/libykcs11.$SO_VERSION.dylib"
install_name_tool -change "$BREW_DIR/openssl@1.1/lib/libcrypto.1.1.dylib" "@loader_path/libcrypto.1.1.dylib" "$FINAL_INSTALL_DIR/lib/libykcs11.$SO_VERSION.dylib"

install_name_tool -change "$BREW_DIR/openssl@1.1/lib/libcrypto.1.1.dylib" "@loader_path/../lib/libcrypto.1.1.dylib" "$FINAL_INSTALL_DIR/bin/yubico-piv-tool"
install_name_tool -change "$FINAL_INSTALL_DIR/lib/libykpiv.$SO_VERSION.dylib" "@executable_path/../lib/libykpiv.$SO_VERSION.dylib" "$FINAL_INSTALL_DIR/bin/yubico-piv-tool"


if otool -L $FINAL_INSTALL_DIR/lib/*.dylib $FINAL_INSTALL_DIR/bin/* | grep '$FINAL_INSTALL_DIR' | grep -q compatibility; then
	echo "something is incorrectly linked!";
	exit 1;
fi

otool -L $FINAL_INSTALL_DIR/lib/*.dylib
otool -L $FINAL_INSTALL_DIR/bin/*

# Copy yubico-piv-tool and openssl licenses and move the whole lisenses directory under FINALINSTALL_DIR.
cd $SOURCE_DIR
cp COPYING $LICENSE_DIR/$PACKAGE.txt
cp $BREW_DIR/openssl/LICENSE $LICENSE_DIR/openssl.txt
mv $LICENSE_DIR $FINAL_INSTALL_DIR/

cd $INSTALL_DIR
zip -r $MAC_DIR/$PACKAGE-$VERSION-mac.zip .

cd $MAC_DIR
rm -rf $PKG_DIR