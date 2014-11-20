
set val(chan) Channel/WirelessChannel ;# channel type
set val(prop) Propagation/TwoRayGround ;# radio-propagation model: TwoRayGround/FreeSpace

set val(netif) Phy/WirelessPhy ;# network interface type
set val(mac) Mac/Simple ;# MAC type

#set val(netif)          Phy/WirelessPhy/802_15_4 ;
#set val(mac)            Mac/802_15_4

set val(ifq) Queue/DropTail/PriQueue ;# interface queue type
set val(ll) LL ;# link layer type
set val(ant) Antenna/OmniAntenna ;# antenna model
set val(ifqlen) 1000 ;# max packet in ifq
set val(nReaders) 4 ;
set val(nn) 14 ;
set val(rp) DumbAgent ;# routing protocol
#set val(rp) DSDV ;# routing protocol
set val(x) 500 ;# X dimension of topography
set val(y) 500 ;# Y dimension of topography
set val(stop) 7200 ;# time of simulation end

#Create a simulator object
set ns [new Simulator]

set f [open rfid_tests/rfid.tr w]
$ns trace-all $f

set nf [open rfid_tests/rfid.nam w]
$ns namtrace-all-wireless $nf $val(x) $val(y)
$ns use-newtrace

create-god [expr $val(nn)]

#Signal Power (5m)
#$val(netif) set Pt_ 0.28
#$val(netif) set RXThresh_ 7.64097e-06
## CC1100 radio
Phy/WirelessPhy set bandwidth_ 500e3
Antenna/OmniAntenna set Gt_  1                 ;#Transmit antenna gain  
Antenna/OmniAntenna set Gr_  1                 ;#Receive  antenna gain  
Phy/WirelessPhy     set L_   1.0               ;#System Loss Factor
Phy/WirelessPhy     set freq_ 433.92e6          ;#433 Mhz
Phy/WirelessPhy     set Pt_ 0.01     ;#Transmit power       

Phy/WirelessPhy     set CPThresh_ 10.0      ;#Collision threshold  
Phy/WirelessPhy     set CSThresh_ 2.261e-15  ;#Carrier sense power CSThresh=0.9*RxThresh, 0.5*RXThresh, 0.1*RXThresh, 0.05*RXThresh 
#50 kBaud data rate, sensitivity optimized, MDMCFG2.DEM_DCFILT_OFF=0
#(GFSK, 1% packet error rate, 20 bytes packet length, 127 kHz deviation, 540 kHz digital channel filter bandwidth)
Phy/WirelessPhy     set RxThresh_ 2.512e-15  ;#Receive power  

Agent/RfidTag       set intervalToWaitToSleepAgain_ 10;
Agent/RfidTag       set intervalToWaitSleepCommand_ 5;
#Agent/RfidTag       set numberOfCyclesForAuthenticating_ 0; # password
#Agent/RfidTag       set numberOfCyclesForEncrypting_ 0; # dummy data
Agent/RfidTag       set intervalToCalculateColdStart_ 900; # dummy data
Agent/RfidTag       set intervalToCalculateHotStart_ 120;
Agent/RfidTag       set useGPS_ 1;

Agent/RfidTag set numberOfCyclesForAuthenticating_ 22403 ;
Agent/RfidTag set numberOfCyclesForEncrypting_ 559766 ;

# key scheduling + encrypting
# Agent/RfidTag set numberOfCyclesForAuthenticating_ 22403; # AES
# Agent/RfidTag set numberOfCyclesForAuthenticating_ 271998; # DES
# Agent/RfidTag set numberOfCyclesForAuthenticating_ 11830; # KLEIN64
# Agent/RfidTag set numberOfCyclesForAuthenticating_ 8319; # TEA
# Agent/RfidTag set numberOfCyclesForAuthenticating_ 179563; # KATAN48
# Agent/RfidTag set numberOfCyclesForAuthenticating_ 11509; # HIGHT
# Agent/RfidTag set numberOfCyclesForAuthenticating_ 95389; # RC6

# Agent/RfidTag set numberOfCyclesForEncrypting_ 559766; # AES
# Agent/RfidTag set numberOfCyclesForEncrypting_ 17376003; # DES
# Agent/RfidTag set numberOfCyclesForEncrypting_ 718699; # KLEIN64
# Agent/RfidTag set numberOfCyclesForEncrypting_ 500547; # TEA
# Agent/RfidTag set numberOfCyclesForEncrypting_ 14865463; # KATAN48
# Agent/RfidTag set numberOfCyclesForEncrypting_ 413521; # HIGHT
# Agent/RfidTag set numberOfCyclesForEncrypting_ 244960; # RC6

#Open a trace file
#set nf [open out.nam w]
#$ns namtrace-all $nf

#Define a 'finish' procedure
#proc finish {} {
#        global ns nf
#        $ns flush-trace
#        close $nf
#        exec nam out.nam &
#        exit 0
#}

set topo [new Topography]
$topo load_flatgrid $val(x) $val(y)

set chan_1_ [new $val(chan)]

$ns node-config -adhocRouting $val(rp) -llType $val(ll) -macType $val(mac) -ifqType $val(ifq) -ifqLen $val(ifqlen) -antType $val(ant) -propType $val(prop) -phyType $val(netif) -channel $chan_1_ -topoInstance $topo -agentTrace ON -routerTrace ON -macTrace ON -movementTrace ON 

#-energyModel "EnergyModel" -initialEnergy 8640.0 -txPower 0.08 -rxPower 0.0855 -idlePower 0.0085 -sleepPower 1e-06

#puts $val(nn)
#
for {set i 0} {$i < $val(nn) } { incr i } {
    set n($i) [$ns node]
}


#RAMDOM TAGS LOCATION
set rng1 [new RNG]
$rng1 seed 0
set rng2 [new RNG]
$rng2 seed 0
set rng3 [new RNG]
$rng3 seed 0

#puts "[$rng2 uniform 0 20]"
#puts "[$rng3 uniform 10 1000]"
#for {set i 1} {$i < $val(nn) } { incr i } {
#      $n($i) set X_ [$rng1 uniform 8 12]
#      $n($i) set Y_ [$rng2 uniform 8 12]
#      #puts "[$rng2 uniform 0 20]"
#      $n($i) set Z_ 0.0
#}

#Create three nodes
#set n0 [$ns node]
#set n1 [$ns node]

#defining heads
#$ns at 0.0 "$n(0) label LEITOR"
#$ns at 0.0 "$n(1) label TAG"

#defining connections

for {set i 0} { $i < $val(nReaders) } { incr i } {
	set reader($i) [new Agent/RfidReader]
	$ns attach-agent $n($i) $reader($i)
}
#$ns initial_node_pos $n(0) 20

for {set i $val(nReaders)} { $i < $val(nn) } { incr i } {

	set tag($i) [new Agent/RfidTag]

	$ns attach-agent $n($i) $tag($i)

	for {set j 0} {$j < $val(nReaders) } { incr j } {
		$ns connect $reader($j) $tag($i)
	}
}

# Define node initial position in nam

set now [$ns now];

for {set i 0} {$i < $val(nReaders)} { incr i } {

	$ns initial_node_pos $n($i) 16

	set xx 0
	set yy 0

	if { $i == 0 } {
		set xx 125
		set yy 125
	} elseif { $i == 1 } {
		set xx 375
		set yy 125
	} elseif { $i == 2 } {
		set xx 125
		set yy 375
	} else {
		set xx 375
		set yy 375
	}

	$ns at $now "$n($i) set X_ $xx"
	$ns at $now "$n($i) set Y_ $yy"
	$ns at $now "$n($i) set Z_ 2"

	$ns at $now "$n($i) setdest $xx $yy 1"
}

for {set i $val(nReaders)} {$i < $val(nn)} { incr i } {
	$ns initial_node_pos $n($i) 10

	set xx [$rng1 uniform 0 $val(x)]
	set yy [$rng2 uniform 0 $val(y)]

	$ns at $now "$n($i) set X_ $xx"
	$ns at $now "$n($i) set Y_ $yy"
	$ns at $now "$n($i) set Z_ 1"

	set speed [$rng1 uniform 1 10]

	$ns at $now "$n($i) setdest $xx $yy $speed"
}

# dynamic destination setting procedure..
for {set i 0} {$i < $val(stop)} { incr i 40} {
	$ns at $i "destination"
}

for {set i 0} {$i < $val(nReaders)} { incr i } {
	$ns at 0.0 "$reader($i) start"
}

for {set i $val(nReaders)} {$i < $val(nn)} { incr i} {
	$ns at 0.0 "$tag($i) start"
}

proc destination {} {

	global val rng1 rng2 ns n

	set now [$ns now]

	for {set i $val(nReaders)} {$i < $val(nn)} { incr i } {
		set dxx [$rng1 uniform 0 $val(x)]
		set dyy [$rng2 uniform 0 $val(y)]
		$ns at $now "$n($i) setdest $dxx $dyy 1"
	}
}


$ns at $val(stop) "puts \"NS EXITING...\" ; $ns halt"

#Run the simulation
$ns run
