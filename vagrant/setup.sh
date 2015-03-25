#!/usr/bin/env bash

echo "deb http://llvm.org/apt/trusty/ llvm-toolchain-trusty-3.5 main" > /etc/apt/sources.list.d/llvm.org.list
echo "deb-src http://llvm.org/apt/trusty/ llvm-toolchain-trusty-3.5 main" >> /etc/apt/sources.list.d/llvm.org.list

wget -O - http://llvm.org/apt/llvm-snapshot.gpg.key|sudo apt-key add -

apt-get update

apt-get install -y --force-yes llvm-3.5-dev

apt-get install -y bison flex libboost-all-dev libyaml-cpp-dev g++ cmake libedit-dev

ln -s /usr/share /usr/lib/llvm-3.5/share
ln -s /usr/share/llvm-3.5 /usr/share/llvm

mkdir /opt/monicelli/build
cd /opt/monicelli/build

cmake ..

make install
