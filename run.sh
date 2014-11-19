#!/bin/sh

if [ "x$1" == "xrun_out" ]; then
	./ns rfid_tests/rfid_test.tcl
elif [ "x$1" == "xrun" ]; then
	./ns rfid_tests/rfid_test.tcl > rfid_tests/out.txt 2>&1
elif [ "x$1" == "xcompile" ]; then
	make clean; make -j32 > /dev/null 2>&1; make
elif [ "x$1" == "xtest" ]; then
	
	numberOfNodes=( "14" "54" "104" "204" )
	algorithms=( "NONE" "AES" "DES" "KLEIN" "TEA" "KATAN" "HIGHT" "RC6" )
	cyclesToAuthenticate=( "0" "22403" "271998" "11830" "8319" "179563" "11509" "95389" )
	cyclesToEncrypt=( "0" "559766" "17376003" "718699" "500547" "14865463" "413521" "244960" )

	set -i "s/-movementTrace/	-movementTrace ON \\/"
	sed -i "s/-energyModel/	-energyModel \"EnergyModel\" \\/" rfid_tests/rfid_test.tcl
	sed -i "s/-initialEnergy/	-initialEnergy 8640.0 \\/" rfid_tests/rfid_test.tcl
	sed -i "s/-txPower/	-txPower 0.08 \\/" rfid_tests/rfid_test.tcl
	sed -i "s/-rxPower/	-rxPower 0.0855 \\/" rfid_tests/rfid_test.tcl
	sed -i "s/-idlePower/	-idlePower 0.0085 \\/" rfid_tests/rfid_test.tcl
	sed -i "s/-sleepPower/	-sleepPower 1e-06/" rfid_tests/rfid_test.tcl

	
	for n in "${numberOfNodes[@]}"; do

		rm rfid_tests/*.tr

		i=0
		for alg in "${algorithms[@]}"; do

			echo "${algorithms[$i]} $n ${cyclesToAuthenticate[$i]} ${cyclesToEncrypt[$i]}"

			i=$(($i+1))


			sed -i "s/^set val(nn).*/set val(nn) ${n} ;/" rfid_tests/rfid_test.tcl
			set -i "s/Agent\/RfidTag set numberOfCyclesForAuthenticating_ .*/Agent\/RfidTag set numberOfCyclesForAuthenticating_ ${cyclesToAuthenticate[$i]} ;/" rfid_tests/rfid_test.tcl
			set -i "s/Agent\/RfidTag set numberOfCyclesForEncrypting_ .*/Agent\/RfidTag set numberOfCyclesForEncrypting_ ${cyclesToAuthenticate[$i]} ;/" rfid_tests/rfid_test.tcl

			#./ns rfid_tests/rfid_test.tcl > rfid_tests/out_$n.txt 2>&1

			cp rfid_tests/rfid.tr rfid_tests/rfid_${n}_${alg}.tr
			cp rfid_tests/rfid.nam rfid_tests/rfid_${n}_${alg}.nam

		done;

		set -i "s/-movementTrace/	-movementTrace ON"
		sed -i "s/-energyModel/#	-energyModel \"EnergyModel\" \\/" rfid_tests/rfid_test.tcl
		sed -i "s/-initialEnergy/#	-initialEnergy 8640.0 \\/" rfid_tests/rfid_test.tcl
		sed -i "s/-txPower/#	-txPower 0.08 \\/" rfid_tests/rfid_test.tcl
		sed -i "s/-rxPower/#	-rxPower 0.0855 \\/" rfid_tests/rfid_test.tcl
		sed -i "s/-idlePower/#	-idlePower 0.0085 \\/" rfid_tests/rfid_test.tcl
		sed -i "s/-sleepPower/#	-sleepPower 1e-06/" rfid_tests/rfid_test.tcl


	done;

fi;
