
import sys

fileName = sys.argv[1]
logFile = open(fileName)
lineString = logFile.readline()

energy = { }
initialEnergy = { }
totalConsumption = 0.
packetsDropped = 0
packetsSent = 0
packetsReceived = 0

while len(lineString) > 0:
	args = lineString.split(" ")

	command = args[0]

	if command == "N": # N -t 0.000325 -n 18 -e 8633.629975

		node = int(args[4])
		energyValue = float(args[6])

		if node >= 4:

			if not (node in energy):
				energy[node] = energyValue
				initialEnergy[node] = energyValue
			else:
				totalConsumption = totalConsumption + (energy[node] - energyValue)
				energy[node] = energyValue

	elif command == "d":

		packetsDropped = packetsDropped + 1

	elif command == "s":
		
		packetsSent =  packetsSent + 1

	elif command == "r":
		
		packetsReceived = packetsReceived + 1

	lineString = logFile.readline()

avgConsumption = 0.

for nodeId in energy:

	avgConsumption = avgConsumption + (initialEnergy[nodeId] - energy[nodeId])

avgConsumption = avgConsumption / len(energy)

print "%f %f %d %d %d" % (totalConsumption, avgConsumption, packetsDropped, packetsSent, packetsReceived)
