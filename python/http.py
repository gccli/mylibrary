import os
import sys
import re

import ssl
import socket
import SocketServer
import BaseHTTPServer
import httplib
import urllib
import urllib2
import urlparse


class BaseHTTPRequestHandler(BaseHTTPServer.BaseHTTPRequestHandler):
    scheme = 'http'
    protocol_version = 'HTTP/1.1'
    ssl_version = ssl.PROTOCOL_SSLv23

    def setup(self):
        print 'Setup HTTP server %s (system:%s)' % (self.__class__.server_version, self.__class__.sys_version)
        self.__class__.setup = BaseHTTPServer.BaseHTTPRequestHandler.setup
        self.__class__.do_CONNECT = self.__class__.do_METHOD
        self.__class__.do_GET = self.__class__.do_METHOD
        self.__class__.do_PUT = self.__class__.do_METHOD
        self.__class__.do_POST = self.__class__.do_METHOD
        self.__class__.do_HEAD = self.__class__.do_METHOD
        self.__class__.do_DELETE = self.__class__.do_METHOD
        self.__class__.do_OPTIONS = self.__class__.do_METHOD
        self.__class__.do_PATCH = self.__class__.do_METHOD
        self.setup()    

    def do_METHOD(self):
        from pprint import pprint as pp
        content_length = int(self.headers['Content-Length']) if 'Content-Length' in self.headers else 0
        self.body = self.rfile.read(content_length) if content_length > 0 else ''
        status = 200
        print '%s "%s %s %s" %d %d' % (self.address_string(), self.command, self.path, self.protocol_version, status, len(self.body))


def main():
    server_address = ('', 8080)
    httpd = BaseHTTPServer.HTTPServer(server_address, BaseHTTPRequestHandler)
    httpd.serve_forever()

if __name__ == "__main__":
    main()
