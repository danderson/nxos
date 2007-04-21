#!/bin/sh

ROOT=`pwd`
SRCDIR=$ROOT/src
BUILDDIR=$ROOT/build
PREFIX=$ROOT/install

GCC_URL=http://www.gnuarm.com/gcc-4.1.1.tar.bz2
GCC_VERSION=4.1.1
GCC_DIR=gcc-$GCC_VERSION

BINUTILS_URL=http://www.gnuarm.com/binutils-2.17.tar.bz2
BINUTILS_VERSION=2.17
BINUTILS_DIR=binutils-$BINUTILS_VERSION

NEWLIB_URL=http://www.gnuarm.com/newlib-1.14.0.tar.gz
NEWLIB_VERSION=1.14.0
NEWLIB_DIR=newlib-$NEWLIB_VERSION

echo "I will build an arm-elf cross-compiler:

  Prefix: $PREFIX
  Sources: $SRCDIR
  Build files: $BUILDDIR

Press ^C now if you do NOT want to do this."
read IGNORE

#
# Helper functions.
#
ensure_source()
{
    URL=$1
    FILE=$(basename $1)

    if [ ! -e $FILE ]; then
	wget -O$FILE $URL
    fi
}

unpack_source()
{
(
    cd $SRCDIR
    ARCHIVE_SUFFIX=${1##*.}
    if [ "$ARCHIVE_SUFFIX" = "gz" ]; then
      tar zxvf $1
    elif [ "$ARCHIVE_SUFFIX" = "bz2" ]; then
      tar jxvf $1
    else
      echo "Unknown archive format for $1"
      exit 1
    fi
)
}

# Create all the directories we need.
mkdir -p $SRCDIR $BUILDDIR $PREFIX

(
cd $SRCDIR

# First grab all the source files...
ensure_source $GCC_URL
ensure_source $BINUTILS_URL
ensure_source $NEWLIB_URL

# ... And unpack the sources.
unpack_source $(basename $GCC_URL)
unpack_source $(basename $BINUTILS_URL)
unpack_source $(basename $NEWLIB_URL)
)

#
# Stage 1: Build binutils
#
(
mkdir -p $BUILDDIR/$BINUTILS_DIR
cd $BUILDDIR/$BINUTILS_DIR

$SRCDIR/$BINUTILS_DIR/configure --target=arm-elf --prefix=$PREFIX \
    --enable-interwork --enable-multilib --with-float=soft

make all install
)

# Set the PATH to include the newly built binutils
OLD_PATH=$PATH
export PATH=$PREFIX/bin:$PATH

#
# Stage 2: Patch the GCC multilib rules, then build the gcc compiler only
#
(
MULTILIB_CONFIG=$SRCDIR/$GCC_DIR/gcc/config/arm/t-arm-elf

echo "

MULTILIB_OPTIONS += mno-thumb-interwork/mthumb-interwork
MULTILIB_DIRNAMES += normal interwork

" >> $MULTILIB_CONFIG

mkdir -p $BUILDDIR/$GCC_DIR
cd $BUILDDIR/$GCC_DIR

$SRCDIR/$GCC_DIR/configure --target=arm-elf --prefix=$PREFIX \
    --enable-interwork --enable-multilib --with-float=soft \
    --enable-languages="c" --with-newlib \
    --with-headers=$SRCDIR/$NEWLIB_DIR/newlib/libc/include

make all-gcc install-gcc
)

#
# Stage 3: Build and install newlib
#
(
mkdir -p $BUILDDIR/$NEWLIB_DIR
cd $BUILDDIR/$NEWLIB_DIR

$SRCDIR/$NEWLIB_DIR/configure --target=arm-elf --prefix=$PREFIX \
    --enable-interwork --enable-multilib --with-float=soft

make all install
)

#
# Stage 4: Build and install the rest of GCC.
#
(
cd $BUILDDIR/$GCC_DIR

make all install
)

export PATH=$OLD_PATH

echo "
Build complete! Add $PREFIX/bin to your PATH to make arm-elf-gcc and friends
accessible directly.
"
