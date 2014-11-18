#!/bin/sh

if [ "x$1" == "xrun_out" ]; then
	./ns rfid_tests/rfid_test.tcl
elif [ "x$1" == "xrun" ]; then
	./ns rfid_tests/rfid_test.tcl > rfid_tests/out.txt 2>&1
elif [ "x$1" == "xcompile" ]; then
	make clean; make -j32 > /dev/null 2>&1; make
fi;
