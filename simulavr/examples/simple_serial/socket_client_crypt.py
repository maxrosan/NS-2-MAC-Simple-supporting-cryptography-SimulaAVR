import SocketServer
import base64

class MyTCPHandler(SocketServer.StreamRequestHandler):
    """
    The RequestHandler class for our server.

    It is instantiated once per connection to the server, and must
    override the handle() method to implement communication to the
    client.
    """


    def handle(self):
        # self.request is the TCP socket connected to the client
	while True:

        	cmd = self.rfile.readline().strip()

		if len(cmd) == 0:
			break

		msg = base64.b64decode(self.rfile.readline().strip())

		print cmd, msg

        #print "{} wrote:".format(self.client_address[0])
        #self.request.sendall(self.data.upper())

if __name__ == "__main__":
    HOST, PORT = "localhost", 8080

    # Create the server, binding to localhost on port 9999
    server = SocketServer.TCPServer((HOST, PORT), MyTCPHandler)

    # Activate the server; this will keep running until you
    # interrupt the program with Ctrl-C
    server.serve_forever()


#        decoded = ['e', 'g', '\xae', '\x0b', 'o', '\xaf', '_', 'C', '\x03', 'f', 'O', 'k', '\xcf', '>', '\xf6', '\xad', '\xe1', '\x8d', '\xdd',     '\x15', '\x97', 'P', 'E', '@', '~', 'l', '\xf3', '\x86', '\xf3', 'c', '&', '\x7f']
 #       decMsg = "".join(decoded)

 #       #txpin.pushChars('E' + chr(len(txt)) + txt)

  #      txpin.pushChars('D' + chr(len(decMsg)) + decMsg)
    
   #     start = sc.GetCurrentTime()
    
    #    while True:
   #       sc.RunTimeRange(100000)    
    
    #      if len(rxpin.allCharsReceived) >= 4 and "".join(rxpin.allCharsReceived[-3:]) == "END":
     #        break
    
      #  end = sc.GetCurrentTime()
    
       # while rxpin.allCharsReceived[0] == '\x00':
        #  rxpin.allCharsReceived = rxpin.allCharsReceived[1:]
        #rxpin.allCharsReceived = rxpin.allCharsReceived[:-3]
    
       # print rxpin.allCharsReceived
       # print "Interval: s", (end - start) / (10.**9)

    ##

