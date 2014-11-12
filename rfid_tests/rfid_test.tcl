
set val(chan) Channel/WirelessChannel ;# channel type
set val(prop) Propagation/TwoRayGround ;# radio-propagation model: TwoRayGround/FreeSpace
set val(netif) Phy/WirelessPhy ;# network interface type
set val(mac) Mac/Simple ;# MAC type
set val(ifq) Queue/DropTail/PriQueue ;# interface queue type
set val(ll) LL ;# link layer type
set val(ant) Antenna/OmniAntenna ;# antenna model
set val(ifqlen) 1000 ;# max packet in ifq
set val(nn) 4 ;# number of mobilenodes
set val(rp) DumbAgent ;# routing protocol
#set val(rp) DSDV ;# routing protocol
set val(x) 30 ;# X dimension of topography
set val(y) 30 ;# Y dimension of topography
set val(stop) 60 ;# time of simulation end

#Create a simulator object
set ns [new Simulator]

set f [open rfid.tr w]
$ns trace-all $f

set nf [open rfid.nam w]
$ns namtrace-all-wireless $nf $val(x) $val(y)
$ns use-newtrace

create-god [expr $val(nn)]

#Signal Power (5m)
$val(netif) set Pt_ 0.28
$val(netif) set RXThresh_ 7.64097e-06

#Open a trace file
set nf [open out.nam w]
$ns namtrace-all $nf

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
	-routerTrace OFF \
	-macTrace OFF \
	-movementTrace OFF

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

for {set i 1} {$i < $val(nn) } { incr i } {

    set tag($i) [new Agent/RfidTag]

    $ns attach-agent $n($i) $tag($i)
    $ns connect $reader1 $tag($i)
#    $tag($i) set tagEPC_ [expr $i*10]
#    $tag($i) set time_ 1
#    $tag($i) set messages_ 0
#    $tag($i) set seed_ [$rng2 uniform 10 1000]
}

#$ns attach-agent $n(0) $reader1
#
for {set i 1} {$i < $val(nn) } { incr i } {
        $ns connect $reader1 $tag($i)
}

$ns at 0.1 "$reader1 start"

#Run the simulation
$ns run
$ns at 30.0 "finish"
