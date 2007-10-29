#!/bin/sh

export PATH=`pwd`/tvm/slinker:$PATH

if [ ! -d "tvm" ]; then
  echo "No TVM available, checking out..."
  svn co http://svn.transterpreter.org/transterpreter/trunk tvm
fi

(cd tvm

 if [ ! -f "configure" ]; then
   echo "No configure present, running bootstrap"
   ./bootstrap
 fi

 if [ ! -f "Makefile" ]; then
   echo "No makefile present, running configuring..."
   ./scripts/nxos-crossconfigure.sh
 fi

 make)
