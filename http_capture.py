#!/usr/bin/env python3
"""
HTTP Request/Response Capture Script

This script captures HTTP requests and responses by running a local proxy server.
HTTP requests are saved to http_requests.txt and HTTP responses to http_responses.txt.
Each message is separated by a clear delimiter.

IMPORTANT: This captures HTTP only, not HTTPS.
For testing, use: python3 http_test_server.py
Then visit: http://localhost:9090 (through the proxy)

Usage:
    1. Terminal 1: python3 http_capture.py
    2. Terminal 2: python3 http_test_server.py
    3. Configure your browser proxy to: http://127.0.0.1:8080
    4. Visit http://localhost:9090 - requests/responses will be captured
    5. Press Ctrl+C to stop capturing
"""

import socket
import threading
import time
from urllib.parse import urlparse
from datetime import datetime

REQUEST_FILE = "http_requests.txt"
RESPONSE_FILE = "http_responses.txt"
REQUEST_SEPARATOR = "=" * 80
RESPONSE_SEPARATOR = "=" * 80
LOG_FILE = "proxy_debug.log"

def write_to_file(filename, content):
    """Append content to file"""
    try:
        with open(filename, 'a') as f:
            f.write(content)
    except Exception as e:
        print(f"Error writing to {filename}: {e}")

def clear_files():
    """Clear output files at startup"""
    try:
        open(REQUEST_FILE, 'w').close()
        open(RESPONSE_FILE, 'w').close()
        print(f"Cleared {REQUEST_FILE} and {RESPONSE_FILE}\n")
    except Exception as e:
        print(f"Error clearing files: {e}")

def extract_host_from_request(request_data):
    """Extract host from HTTP request"""
    lines = request_data.split('\r\n')
    for line in lines:
        if line.lower().startswith('host:'):
            return line.split(':', 1)[1].strip()
    return None

def handle_client(client_socket, client_addr, request_count):
    """Handle individual client connection"""
    try:
        # Receive the request from client
        request_data = b""
        while True:
            try:
                chunk = client_socket.recv(4096)
                if not chunk:
                    break
                request_data += chunk
                # Check for end of HTTP request headers
                if b'\r\n\r\n' in request_data:
                    break
            except socket.timeout:
                break

        if not request_data:
            return

        # Parse the request
        request_str = request_data.decode('utf-8', errors='ignore')
        request_lines = request_str.split('\r\n')
        request_line = request_lines[0] if request_lines else ""
        
        # Check if it's a CONNECT request (HTTPS tunnel)
        if request_line.upper().startswith('CONNECT'):
            # Send 200 success for CONNECT, but don't capture encrypted traffic
            response = b"HTTP/1.1 200 Connection Established\r\n\r\n"
            client_socket.sendall(response)
            print(f"[HTTPS TUNNEL] Skipped: {request_line[:60]}")
            return
        
        # Only process actual HTTP requests (GET, POST, etc.)
        if not any(request_line.upper().startswith(m) for m in ['GET', 'POST', 'PUT', 'DELETE', 'HEAD', 'OPTIONS', 'PATCH']):
            return

        timestamp = datetime.now().strftime("%Y-%m-%d %H:%M:%S")
        
        # Save request to file
        request_output = f"[{timestamp}] Request #{request_count}\n{request_str}\n"
        write_to_file(REQUEST_FILE, request_output)
        write_to_file(REQUEST_FILE, REQUEST_SEPARATOR + "\n\n")
        
        print(f"[{timestamp}] Captured HTTP request #{request_count}: {request_line[:60]}")
        
        # Extract target server info
        parts = request_line.split()
        if len(parts) >= 2:
            path = parts[1]
            host = extract_host_from_request(request_str)
            
            if host:
                # Remove port if present
                if ':' in host:
                    hostname, port = host.rsplit(':', 1)
                    try:
                        port = int(port)
                    except:
                        port = 80
                else:
                    hostname = host
                    port = 80
                
                # Connect to target server
                try:
                    server_socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
                    server_socket.settimeout(10)
                    server_socket.connect((hostname, port))
                    
                    # Send request to server
                    server_socket.sendall(request_data)
                    
                    # Receive response from server
                    response_data = b""
                    while True:
                        chunk = server_socket.recv(4096)
                        if not chunk:
                            break
                        response_data += chunk
                    
                    server_socket.close()
                    
                    # Save response to file
                    response_str = response_data.decode('utf-8', errors='ignore')
                    response_output = f"[{timestamp}] Response #{request_count}\n{response_str}\n"
                    write_to_file(RESPONSE_FILE, response_output)
                    write_to_file(RESPONSE_FILE, RESPONSE_SEPARATOR + "\n\n")
                    
                    # Send response back to client
                    client_socket.sendall(response_data)
                    
                    print(f"[{timestamp}] Captured HTTP response #{request_count}")
                    
                except Exception as e:
                    error_response = f"HTTP/1.1 502 Bad Gateway\r\nConnection: close\r\n\r\nProxy Error: {str(e)}"
                    client_socket.sendall(error_response.encode())
                    print(f"Error connecting to {hostname}:{port} - {e}")
            else:
                error_response = b"HTTP/1.1 400 Bad Request\r\nConnection: close\r\n\r\nNo Host header found"
                client_socket.sendall(error_response)
    
    except Exception as e:
        print(f"Error handling client: {e}")
    
    finally:
        try:
            client_socket.close()
        except:
            pass

def start_proxy_server(host='127.0.0.1', port=8080):
    """Start the HTTP proxy server"""
    server_socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    server_socket.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
    
    try:
        server_socket.bind((host, port))
        server_socket.listen(5)
        
        # Clear files at startup
        clear_files()
        
        print(f"HTTP Proxy Server started on {host}:{port}")
        print(f"Configure your browser proxy to: http://{host}:{port}")
        print(f"Requests will be saved to: {REQUEST_FILE}")
        print(f"Responses will be saved to: {RESPONSE_FILE}")
        print("\nFor testing, run in another terminal:")
        print("  python3 http_test_server.py")
        print("Then visit: http://localhost:9090 (through the proxy)\n")
        print("Press Ctrl+C to stop the proxy server\n")
        
        request_count = 0
        
        while True:
            try:
                client_socket, client_addr = server_socket.accept()
                request_count += 1
                
                # Handle each client in a separate thread
                client_thread = threading.Thread(
                    target=handle_client,
                    args=(client_socket, client_addr, request_count),
                    daemon=True
                )
                client_thread.start()
            
            except KeyboardInterrupt:
                print("\n\nProxy server stopped.")
                break
            except Exception as e:
                print(f"Error accepting connection: {e}")
    
    except Exception as e:
        print(f"Error starting proxy server: {e}")
    
    finally:
        server_socket.close()

if __name__ == "__main__":
    start_proxy_server()
