#!/bin/bash

if [ "x$1" == "xrun_out" ]; then
	./ns rfid_tests/rfid_test.tcl
elif [ "x$1" == "xrun" ]; then
	./ns rfid_tests/rfid_test.tcl > rfid_tests/out.txt 2>&1
elif [ "x$1" == "xcompile" ]; then
	make clean; make -j32 > /dev/null 2>&1; make
elif [ "x$1" == "xall" ]; then

	rm rfid_tests/log/*

	for i in `seq 1 10`; do

		./run.sh test

	done;

elif [ "x$1" == "xtest" ]; then

	timestamp=$(date +%s)
	
	numberOfNodes=( "14" "54" "104" )
	algorithms=( "NONE" "AES" "DES" "KLEIN" "TEA" "KATAN" "HIGHT" "AES" "FAKE" )
	cyclesToAuthenticate=( "0" "22403" "271998" "11830" "8319" "179563" "11509" "95389" "10")
	cyclesToEncrypt=( "0" "559766" "17376003" "718699" "500547" "14865463" "413521" "244960" "10" )

	sed -i "s/Agent\/RfidTag set useGPS_ .*/Agent\/RfidTag set useGPS_ 1;/" rfid_tests/rfid_test.tcl

	sed -i 's/$ns node-config .*/$ns node-config -adhocRouting $val(rp) -llType $val(ll) -macType $val(mac) -ifqType $val(ifq) -ifqLen $val(ifqlen) -antType $val(ant) -propType $val(prop) -phyType $val(netif) -channel $chan_1_ -topoInstance $topo -agentTrace ON -routerTrace ON -macTrace ON -movementTrace ON -energyModel "EnergyModel" -initialEnergy 8640.0 -txPower 0.08 -rxPower 0.0855 -idlePower 0.0085 -sleepPower 1e-06/' rfid_tests/rfid_test.tcl

	
	for n in "${numberOfNodes[@]}"; do

		rm rfid_tests/*.tr
		rm rfid_tests/*.nam
		rm rfid.txt

		i=0
		for alg in "${algorithms[@]}"; do

			echo "${algorithms[$i]} $n ${cyclesToAuthenticate[$i]} ${cyclesToEncrypt[$i]}"

			i=$(($i+1))


			sed -i "s/^set val(nn).*/set val(nn) ${n} ;/" rfid_tests/rfid_test.tcl
			sed -i "s/Agent\/RfidTag set numberOfCyclesForAuthenticating_ .*/Agent\/RfidTag set numberOfCyclesForAuthenticating_ ${cyclesToAuthenticate[$i]} ;/" rfid_tests/rfid_test.tcl
			sed -i "s/Agent\/RfidTag set numberOfCyclesForEncrypting_ .*/Agent\/RfidTag set numberOfCyclesForEncrypting_ ${cyclesToEncrypt[$i]} ;/" rfid_tests/rfid_test.tcl

			./run.sh run

			#cp rfid.txt rfid_tests/log/rfid_${n}_${alg}_${timestamp}.txt
			#cp rfid_tests/rfid.tr rfid_tests/log/rfid_${n}_${alg}_${timestamp}.tr
			#cp rfid_tests/rfid.nam rfid_tests/log/rfid_${n}_${alg}_${timestamp}.nam

			cp rfid_tests/rfid_test.tcl rfid_tests/log/config_${n}_${alg}.txt

			pypy rfid_tests/graph.py rfid.txt $n >> rfid_tests/log/recognized_${n}_${alg}.txt
			pypy rfid_tests/trace.py rfid_tests/rfid.tr >> rfid_tests/log/trace_${n}_${alg}.txt

		done;

	done;

	#GPS TEST

	defaultNumberOfNodes=104

	sed -i "s/set val(nn).*/set val(nn) ${defaultNumberOfNodes} ;/" rfid_tests/rfid_test.tcl
	sed -i "s/Agent\/RfidTag set numberOfCyclesForAuthenticating_ .*/Agent\/RfidTag set numberOfCyclesForAuthenticating_ 0 ;/" rfid_tests/rfid_test.tcl
	sed -i "s/Agent\/RfidTag set numberOfCyclesForEncrypting_ .*/Agent\/RfidTag set numberOfCyclesForEncrypting_ 0 ;/" rfid_tests/rfid_test.tcl
	sed -i "s/Agent\/RfidTag set useGPS_ .*/Agent\/RfidTag set useGPS_ 0;/" rfid_tests/rfid_test.tcl

	#cp rfid.txt rfid_tests/log/rfid_without_gps_${timestamp}.txt
	#cp rfid_tests/rfid.tr rfid_tests/log/rfid_without_gps_${timestamp}.tr
	#cp rfid_tests/rfid.nam rfid_tests/log/rfid_without_gps_${timestamp}.nam

	./run.sh run

	pypy rfid_tests/graph.py rfid.txt >> rfid_tests/log/recognized_gps.txt
	pypy rfid_tests/trace.py rfid_tests/rfid.tr >> rfid_tests/log/trace_gps.txt

	cp rfid_tests/rfid_test.tcl rfid_tests/log/config_gps.txt

	# WITHOUT ENERGY MODEL

	defaultAlgorithm="AES"
	defaultNumberOfNodes=104

	sed -i "s/set val(nn).*/set val(nn) ${defaultNumberOfNodes} ;/" rfid_tests/rfid_test.tcl
	sed -i "s/Agent\/RfidTag set useGPS_ .*/Agent\/RfidTag set useGPS_ 1;/" rfid_tests/rfid_test.tcl
	sed -i "s/Agent\/RfidTag set numberOfCyclesForAuthenticating_ .*/Agent\/RfidTag set numberOfCyclesForAuthenticating_ 22403 ;/" rfid_tests/rfid_test.tcl
	sed -i "s/Agent\/RfidTag set numberOfCyclesForEncrypting_ .*/Agent\/RfidTag set numberOfCyclesForEncrypting_ 559766 ;/" rfid_tests/rfid_test.tcl
	sed -i 's/$ns node-config .*/$ns node-config -adhocRouting $val(rp) -llType $val(ll) -macType $val(mac) -ifqType $val(ifq) -ifqLen $val(ifqlen) -antType $val(ant) -propType $val(prop) -phyType $val(netif) -channel $chan_1_ -topoInstance $topo -agentTrace ON -routerTrace ON -macTrace ON -movementTrace ON/' rfid_tests/rfid_test.tcl

	./run.sh run

	#cp rfid.txt rfid_tests/log/rfid_${defaultNumberOfNodes}_${defaultAlgorithm}_without_energymodel_${timestamp}.txt
	#cp rfid_tests/rfid.tr rfid_tests/rfid_${defaultNumberOfNodes}_${defaultAlgorithm}_without_energymodel_${timestamp}.tr
	#cp rfid_tests/rfid.nam rfid_tests/rfid_${defaultNumberOfNodes}_${defaultAlgorithm}_without_energymodel_${timestamp}.nam

	pypy rfid_tests/graph.py rfid.txt >> rfid_tests/log/recognized_without_energy.txt
	pypy rfid_tests/trace.py rfid_tests/rfid.tr >> rfid_tests/log/trace_without_energy.txt

	cp rfid_tests/rfid_test.tcl rfid_tests/log/config_without_energy.txt

fi;
