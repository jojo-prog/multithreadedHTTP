#!/usr/bin/env python3
"""
Simple HTTP Test Server for capturing HTTP traffic

This creates a basic HTTP server that responds to requests.
Use this with the http_capture.py proxy to generate real HTTP traffic.

Usage:
    python3 http_test_server.py
    
Then configure your browser to use the proxy (http://127.0.0.1:8080)
and visit http://localhost:9090 (HTTP only, not HTTPS)
"""

import http.server
import socketserver
import threading

PORT = 9090

class SimpleHTTPRequestHandler(http.server.SimpleHTTPRequestHandler):
    def do_GET(self):
        """Handle GET requests"""
        if self.path == '/':
            self.send_response(200)
            self.send_header('Content-Type', 'text/html')
            self.send_header('Content-Length', len(self.html_content()))
            self.end_headers()
            self.wfile.write(self.html_content().encode())
        elif self.path == '/test':
            response = "Hello from test endpoint!"
            self.send_response(200)
            self.send_header('Content-Type', 'text/plain')
            self.send_header('Content-Length', len(response))
            self.end_headers()
            self.wfile.write(response.encode())
        elif self.path == '/api/data':
            response = '{"status": "ok", "message": "This is JSON data"}'
            self.send_response(200)
            self.send_header('Content-Type', 'application/json')
            self.send_header('Content-Length', len(response))
            self.end_headers()
            self.wfile.write(response.encode())
        else:
            self.send_response(404)
            self.send_header('Content-Type', 'text/plain')
            self.end_headers()
            self.wfile.write(b'Not Found')
    
    def do_POST(self):
        """Handle POST requests"""
        content_length = int(self.headers.get('Content-Length', 0))
        body = self.rfile.read(content_length)
        
        response = f"Received {len(body)} bytes of POST data"
        self.send_response(200)
        self.send_header('Content-Type', 'text/plain')
        self.send_header('Content-Length', len(response))
        self.end_headers()
        self.wfile.write(response.encode())
    
    def html_content(self):
        return """<!DOCTYPE html>
<html>
<head>
    <title>HTTP Test Server</title>
    <style>
        body { font-family: Arial; margin: 20px; }
        .section { margin: 20px 0; }
        a { color: blue; margin-right: 15px; }
        button { padding: 10px 20px; margin: 5px; }
    </style>
</head>
<body>
    <h1>HTTP Test Server</h1>
    <p>This server generates HTTP traffic for testing your parser.</p>
    
    <div class="section">
        <h2>Navigation Links</h2>
        <p>
            <a href="/">Home</a>
            <a href="/test">Test Page</a>
            <a href="/api/data">API Data</a>
            <a href="/nonexistent">404 Error</a>
        </p>
    </div>
    
    <div class="section">
        <h2>POST Form</h2>
        <form method="POST" action="/api/data">
            <input type="text" name="query" placeholder="Enter text" />
            <button type="submit">Submit</button>
        </form>
    </div>
    
    <div class="section">
        <h2>JavaScript Requests</h2>
        <button onclick="fetchRequest()">Fetch Request</button>
        <p id="result"></p>
    </div>
    
    <script>
        function fetchRequest() {
            fetch('/test')
                .then(r => r.text())
                .then(data => document.getElementById('result').textContent = data)
                .catch(e => console.error(e));
        }
    </script>
</body>
</html>
"""

def start_server():
    handler = SimpleHTTPRequestHandler
    with socketserver.TCPServer(("127.0.0.1", PORT), handler) as httpd:
        print(f"\nHTTP Test Server running on http://127.0.0.1:{PORT}")
        print(f"\nIMPORTANT: To capture traffic, visit through proxy:")
        print(f"  http://127.0.0.1:{PORT}  (NOT localhost)")
        print(f"\nOr configure this in /etc/hosts:")
        print(f"  127.0.0.1  testserver.local")
        print(f"Then visit:")
        print(f"  http://testserver.local:{PORT}\n")
        print("In Firefox:")
        print("  1. Preferences → Network Settings")
        print("  2. Manual proxy: 127.0.0.1:8080")
        print("  3. NO LOCALHOST BYPASS - important for localhost too!")
        print("Press Ctrl+C to stop\n")
        try:
            httpd.serve_forever()
        except KeyboardInterrupt:
            print("\nServer stopped")

if __name__ == "__main__":
    start_server()
