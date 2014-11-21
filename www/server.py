#!/usr/bin/python
import BaseHTTPServer, SimpleHTTPServer
import CGIHTTPServer
import ssl

#import SimpleHTTPServer
#import SocketServer
#from BaseHTTPServer import BaseHTTPRequestHandler, HTTPServer

#httpd = BaseHTTPServer.HTTPServer(("0", 4443), CGIHTTPServer.CGIHTTPRequestHandler) #SimpleHTTPServer.SimpleHTTPRequestHandler)
#httpd = HTTPServer(("0", 4443), CGIHTTPRequestHandler) #SimpleHTTPServer.SimpleHTTPRequestHandler)
httpd = BaseHTTPServer.HTTPServer(("0", 4443), CGIHTTPServer.CGIHTTPRequestHandler)
#httpd.socket = ssl.wrap_socket(httpd.socket, certfile='localhost.pem', server_side=True)
httpd.serve_forever()
