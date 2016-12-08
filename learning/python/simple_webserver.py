#! /usr/bin/env python

import SimpleHTTPServer
import SocketServer
import BaseHTTPServer

class MyHTTPRequestHandler(BaseHTTPServer.BaseHTTPRequestHandler):
    def _set_headers(self):
        self.send_response(200)
        self.send_header('Content-type', 'text/html')
        self.end_headers()

    def do_GET(s):
        """Respond to a GET request."""
        s.send_response(200)
        s.send_header("Content-type", "text/html")
        s.end_headers()
        s.wfile.write("<html><head><title>Hello World</title></head>")
        s.wfile.write("<body><p>This is a test page.</p>")
        s.wfile.write("<p>You accessed path: %s</p>" % s.path)

        if s.headers.get('Content-Length'):
            length = int(s.headers['Content-Length'])
            request_body = s.rfile.read(length)
            s.wfile.write("<br/>The client request body is: %s" % request_body)
        s.wfile.write("<br/></body></html>")


    def do_POST(self):
        content_length = int(self.headers['Content-Length'])
        post_data = self.rfile.read(content_length)

        print post_data

        self._set_headers()
        self.wfile.write("<html><body><h1>POST!</h1></body></html>")

if __name__ == '__main__':
    server_class = BaseHTTPServer.HTTPServer
    server_address = ('', 8000)
    httpd = server_class(server_address, MyHTTPRequestHandler)

    try:
        httpd.serve_forever()
    except KeyboardInterrupt:
        pass

    httpd.server_close()
