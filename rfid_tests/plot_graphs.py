
import numpy as np
import matplotlib.pyplot as plt
import sys
import scipy as sp
import scipy.stats

# example data
x = [ ]
y = [ ]
# example error bar values that vary with x-position
error = [  ]

algorithms = [ "NONE", "AES", "DES", "KLEIN", "TEA", "KATAN", "HIGHT", "RC6" ]

def mean_confidence_interval(data, confidence=0.95):
    a = 1.0*np.array(data)
    n = len(a)
    m, se = np.mean(a), scipy.stats.sem(a)
    h = se * sp.stats.t._ppf((1+confidence)/2., n-1)
    return m, h

folder = sys.argv[1]

bars = { }
error = { }

nodes = [ 14, 54, 104 ]

N = len(nodes)
width = 0.03       # the width of the bars
ind = np.arange(N) * width

fig, ax = plt.subplots()

rects = []

for algorithm in algorithms:

	bars[algorithm] = [ ]
	error[algorithm] = [ ]

	for numberOfNodes in nodes:

		fileName = folder + "/trace_" + str(numberOfNodes) + "_" + algorithm + ".txt"
		logFile = open(fileName)
		lineString = logFile.readline()

		values = [ ]

		while len(lineString) > 0:
			line = lineString.split(" ")
			values.append(float(line[1])) # consumption
			lineString = logFile.readline()

		(mean, errorValue) = mean_confidence_interval(values)
		bars[algorithm].append(mean)
		error[algorithm].append(errorValue)		

	print bars[algorithm], error[algorithm], ind

	rect = ax.bar(ind, bars[algorithm], width, color='r', yerr=error[algorithm])
	rects.append(rect)

	ind = ind + width * (len(nodes) + 1)


ax.set_ylabel('Consumo de energia (J)')
ax.set_title('Consumo de energia por cifra')
ind = np.arange(len(algorithms)) * width * (len(nodes) + 1) + width*2
ax.set_xticks(ind)
ax.set_xticklabels( algorithms )

#ax.legend( rects, algorithms )
#ax.legend( (rects1[0], rects2[0]), ('Men', 'Women') )

def autolabel(rects):
    # attach some text labels
    for rect in rects:
        height = rect.get_height()
        ax.text(rect.get_x()+rect.get_width()/2., 1.05*height, '%d'%int(height),
                ha='center', va='bottom')

plt.savefig('rfid_tests/consumo_cifra.png')


#############

barsNone = []
errorNone = []
widthNone = 0.10       # the width of the bars
indNone = np.arange(len(nodes)) * (width * 2)
figNone, axNone = plt.subplots()

for numberOfNodes in nodes:


	fileName = folder + "/recognized_" + str(numberOfNodes) + "_NONE.txt"
	logFile = open(fileName)
	lineString = logFile.readline()

	values = [ ]

	while len(lineString) > 0:
		line = lineString.split(" ")
		values.append(float(line[0])) # consumption
		lineString = logFile.readline()

	(mean, errorValue) = mean_confidence_interval(values)
	print (mean, errorValue)

	barsNone.append(mean)
	errorNone.append(errorValue)

rectNone = axNone.bar(indNone, barsNone, width, color='y', yerr=errorNone)

axNone.set_ylabel('Taxa de tags reconhecidas')
axNone.set_title('Reconhecimento de tags')
axNone.set_xticks(indNone + width/2.)
axNone.set_xticklabels( ( '10 tags', '50 tags', '100 tags') )

plt.savefig('rfid_tests/reconhecimento_tags.png')

####

barsGPS = []
errorGPS = []
widthGPS = 0.10       # the width of the bars
indGPS = np.arange(2) * (width * 2)
figGPS, axGPS = plt.subplots()

for fileName in [ folder + "/trace_gps.txt", folder + "/trace_104_AES.txt"]:

	logFile = open(fileName)
	lineString = logFile.readline()

	values = [ ]

	while len(lineString) > 0:
		line = lineString.split(" ")
		values.append(float(line[1])) # consumption
		lineString = logFile.readline()

	(mean, errorValue) = mean_confidence_interval(values)
	print (mean, errorValue)

	barsGPS.append(mean)
	errorGPS.append(errorValue)

rectNone = axGPS.bar(indGPS, barsGPS, width, color='b', ecolor='y', yerr=errorGPS)

axGPS.set_ylabel('Consumo de energia (J)')
axGPS.set_title('Consumo de energia com e sem GPS')
axGPS.set_xticks(indGPS + width/2.)
axGPS.set_xticklabels( ( 'sem GPS', 'com GPS' ) )

plt.savefig('rfid_tests/consumo_gps.png')

#N = 5
#menMeans = (20, 35, 30, 35, 27)
#menStd =   (2, 3, 4, 1, 2)

#ind = np.arange(N)  # the x locations for the groups

#fig, ax = plt.subplots()
#rects1 = ax.bar(ind, menMeans, width, color='r', yerr=menStd)

#womenMeans = (25, 32, 34, 20, 25)
#womenStd =   (3, 5, 2, 3, 3)
#rects2 = ax.bar(ind+width, womenMeans, width, color='y', yerr=womenStd)

# add some text for labels, title and axes ticks
#ax.set_ylabel('Scores')
#ax.set_title('Scores by group and gender')
#ax.set_xticks(ind+width)
#ax.set_xticklabels( algorithms )

#ax.legend( (rects1[0], rects2[0]), ('Men', 'Women') )

#def autolabel(rects):
    # attach some text labels
#    for rect in rects:
#        height = rect.get_height()
#        ax.text(rect.get_x()+rect.get_width()/2., 1.05*height, '%d'%int(height),
#                ha='center', va='bottom')

#autolabel(rects1)
#autolabel(rects2)

#fig, ax0 = plt.subplots(nrows=1, sharex=True)
#ax0.errorbar(x, y, yerr=error, fmt='-o')
#ax0.set_title('variable, symmetric error')

#plt.show()
