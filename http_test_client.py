#!/usr/bin/env python3
"""
HTTP Test Client - Makes requests through the proxy

This script generates HTTP traffic by making requests to the test server
through the proxy, ensuring all traffic is captured.

Usage:
    1. Start proxy: python3 http_capture.py
    2. Start test server: python3 http_test_server.py
    3. Run this: python3 http_test_client.py
"""

import http.client
import time
from datetime import datetime

PROXY_HOST = "127.0.0.1"
PROXY_PORT = 8080
TARGET_HOST = "127.0.0.1"
TARGET_PORT = 9090

def make_request(method, path, headers=None, body=None):
    """Make HTTP request through proxy"""
    try:
        # Connect to proxy
        conn = http.client.HTTPConnection(PROXY_HOST, PROXY_PORT, timeout=5)
        
        # Build absolute URL
        url = f"http://{TARGET_HOST}:{TARGET_PORT}{path}"
        
        # Make request through proxy
        if headers is None:
            headers = {}
        headers['Host'] = f"{TARGET_HOST}:{TARGET_PORT}"
        
        conn.request(method, url, body, headers)
        response = conn.getresponse()
        data = response.read()
        conn.close()
        
        return response.status, response.reason, data.decode('utf-8', errors='ignore')
    except Exception as e:
        return None, str(e), ""

def main():
    print("HTTP Test Client - Generating traffic through proxy\n")
    print(f"Proxy: http://{PROXY_HOST}:{PROXY_PORT}")
    print(f"Target: http://{TARGET_HOST}:{TARGET_PORT}\n")
    
    tests = [
        ("GET", "/", None, "Home page"),
        ("GET", "/test", None, "Test endpoint"),
        ("GET", "/api/data", None, "API endpoint (JSON)"),
        ("GET", "/nonexistent", None, "404 Not Found"),
        ("POST", "/api/data", "key=value&data=test", "POST request"),
    ]
    
    for i, (method, path, body, description) in enumerate(tests, 1):
        print(f"[{i}] {description}: {method} {path}")
        
        status, reason, response = make_request(method, path, body=body)
        
        if status:
            print(f"    ✓ Status: {status} {reason}")
            print(f"    Response length: {len(response)} bytes")
        else:
            print(f"    ✗ Error: {reason}")
        
        print()
        time.sleep(0.5)  # Small delay between requests
    
    print("\nTesting complete! Check http_requests.txt and http_responses.txt")

if __name__ == "__main__":
    main()
