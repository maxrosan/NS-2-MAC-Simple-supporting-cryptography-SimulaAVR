
import sys

fileName = sys.argv[1]

logFile = open(fileName)
lineString = logFile.readline()

numberOfCollectionsResponse = 0
numberOfSleeps = 0

while len(lineString) > 0:

	args = lineString.split(" ")

	time = float(args[0])
	command = args[1]

	if command == "response_collect":
		numberOfCollectionsResponse = numberOfCollectionsResponse + 1

	if command == "sleep":
		numberOfSleeps = numberOfSleeps + 1

	lineString = logFile.readline()


print "%f %d" % ( float(numberOfSleeps) / numberOfCollectionsResponse,  numberOfCollectionsResponse )
