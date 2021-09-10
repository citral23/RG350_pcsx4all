#!/bin/bash

make -f Makefile.gcw0
cp pcsx4all_gcw0 pcsx4all_gcw0.bak
make -f Makefile.gcw0 clean
make -f Makefile.lepus
mv pcsx4all_gcw0.bak pcsx4all_gcw0
./make_opk_odbeta.sh
