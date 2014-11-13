
set val(chan) Channel/WirelessChannel ;# channel type
set val(prop) Propagation/TwoRayGround ;# radio-propagation model: TwoRayGround/FreeSpace
set val(netif) Phy/WirelessPhy ;# network interface type
set val(mac) Mac/Simple ;# MAC type
set val(ifq) Queue/DropTail/PriQueue ;# interface queue type
set val(ll) LL ;# link layer type
set val(ant) Antenna/OmniAntenna ;# antenna model
set val(ifqlen) 1000 ;# max packet in ifq
set val(nn) 50 ;# number of mobilenodes
set val(rp) DumbAgent ;# routing protocol
#set val(rp) DSDV ;# routing protocol
set val(x) 30 ;# X dimension of topography
set val(y) 30 ;# Y dimension of topography
set val(stop) 1000 ;# time of simulation end

#Create a simulator object
set ns [new Simulator]

set f [open rfid_tests/rfid.tr w]
$ns trace-all $f

set nf [open rfid_tests/rfid.nam w]
$ns namtrace-all-wireless $nf $val(x) $val(y)
$ns use-newtrace

create-god [expr $val(nn)]

#Signal Power (5m)
$val(netif) set Pt_ 0.28
$val(netif) set RXThresh_ 7.64097e-06

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

$ns node-config -adhocRouting $val(rp) \
	-llType $val(ll) \
	-macType $val(mac) \
	-ifqType $val(ifq) \
	-ifqLen $val(ifqlen) \
	-antType $val(ant) \
	-propType $val(prop) \
	-phyType $val(netif) \
        -channel $chan_1_ \
	-topoInstance $topo \
	-agentTrace ON \
	-routerTrace ON \
	-macTrace ON \
	-movementTrace ON \
	-energyModel "EnergyModel" \
	-initialEnergy 3000.4 \
	-txPower 0.33 \
	-rxPower 0.1 \
	-idlePower 0.05 \
	-sleepPower 0.03

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

set reader1 [new Agent/RfidReader]
$ns attach-agent $n(0) $reader1
#$ns initial_node_pos $n(0) 20

for {set i 1} {$i < $val(nn) } { incr i } {

    set tag($i) [new Agent/RfidTag]

    $ns attach-agent $n($i) $tag($i)
    $ns connect $reader1 $tag($i)
#    $ns initial_node_pos $n($i) 20

#    $ns random-motion 0 $n($i)

#    $n($i) set X_ [$rng1 uniform 0 30]
#    $n($i) set Y_ [$rng2 uniform 0 30]
    #puts "[$rng2 uniform 0 20]"
#    $n($i) set Z_ 1

#    $tag($i) set tagEPC_ [expr $i*10]
#    $tag($i) set time_ 1
#    $tag($i) set messages_ 0
#    $tag($i) set seed_ [$rng2 uniform 10 1000]
}

# Define node initial position in nam
$ns initial_node_pos $n(0) 8

$n(0) set X_ 15
$n(0) set Y_ 15
$n(0) set Z_ 2

set now [$ns now]

for {set i 1} {$i < $val(nn)} { incr i } {
	$ns initial_node_pos $n($i) 5

	set xx [$rng1 uniform 0 5]
	set yy [$rng2 uniform 0 5]

	set dxx [$rng1 uniform 0 15]
	set dyy [$rng2 uniform 0 15]

	$ns at $now "$n($i) set X_ $xx"
	$ns at $now "$n($i) set Y_ $yy"
	$ns at $now "$n($i) set Z_ 1"

	$ns at $now "$n($i) setdest $dxx $dyy 1"
}

proc destination {} {

	global val rng1 rng2 ns n

	set now [$ns now]

	for {set i 1} {$i < $val(nn)} { incr i } {
		set dxx [$rng1 uniform 0 30]
		set dyy [$rng2 uniform 0 30]
		$ns at $now "$n($i) setdest $dxx $dyy 1"
	}
}

# dynamic destination setting procedure..
for {set i 0} {$i < $val(stop)} { incr i 40} {
	$ns at $i "destination"
}
#for {set i 1} {$i < $val(stop) } { incr i 10} {
#        $ns at $i "destination"
#}


#$ns attach-agent $n(0) $reader1
#
for {set i 1} {$i < $val(nn) } { incr i } {
        $ns connect $reader1 $tag($i)
}

$ns at 0.0 "$reader1 start"

for {set i 1} {$i < $val(nn)} { incr i} {
	$ns at 0.0 "$tag($i) start"
}


$ns at $val(stop) "puts \"NS EXITING...\" ; $ns halt"

#Run the simulation
$ns run
