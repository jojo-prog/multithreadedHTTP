# HTTP Request/Response Capture - Setup Guide

This guide explains how to use the HTTP capture script to generate test data for your HTTP parser.

## ⚠️ Important

The first attempt captured **HTTPS traffic only** (CONNECT requests), which is why responses were empty. **HTTP traffic is readable, but most modern sites use HTTPS.**

The solution: Use the included `http_test_server.py` to generate controlled HTTP test data!

## Quick Start (5 minutes)

### Terminal 1: Start the proxy server
```bash
python3 http_capture.py
```

Output:
```
HTTP Proxy Server started on 127.0.0.1:8080
Configure your browser proxy to: http://127.0.0.1:8080
For testing, run in another terminal:
  python3 http_test_server.py
Then visit: http://localhost:9090 (through the proxy)
```

### Terminal 2: Start the test HTTP server
```bash
python3 http_test_server.py
```

Output:
```
HTTP Test Server running on http://localhost:9090
Configure your proxy first, then visit: http://localhost:9090
```

### Terminal 3: Configure browser and test

**Firefox:**
1. Preferences → Network Settings
2. Manual proxy: `127.0.0.1:8080` (HTTP only)
3. Visit: `http://localhost:9090`

**Chrome (command line):**
```bash
google-chrome --proxy-server="http://127.0.0.1:8080" &
# Then visit http://localhost:9090
```

### Click the test links on the page!
- Home
- Test Page
- API Data
- 404 Error
- Submit Form
- Fetch Request

### Check the captured data
```bash
# Stop proxy with Ctrl+C
head http_requests.txt
head http_responses.txt
```

## What You'll Capture

### http_requests.txt - Example
```
[2026-03-19 16:30:45] Request #1
GET / HTTP/1.1
Host: localhost:9090
User-Agent: Mozilla/5.0 (X11; Ubuntu; Linux x86_64; rv:148.0)
Accept: text/html,application/xhtml+xml,application/xml;q=0.9
Accept-Language: en-US,en;q=0.5
Connection: keep-alive

================================================================================

[2026-03-19 16:30:46] Request #2
GET /test HTTP/1.1
Host: localhost:9090

...
```

### http_responses.txt - Example
```
[2026-03-19 16:30:45] Response #1
HTTP/1.1 200 OK
Server: SimpleHTTP/0.6 Python/3.x.x
Content-Type: text/html
Content-Length: 850

<!DOCTYPE html>
<html>
<head>
    <title>HTTP Test Server</title>
    ...
</head>
</html>

================================================================================

[2026-03-19 16:30:46] Response #2
HTTP/1.1 200 OK
Content-Type: text/plain
Content-Length: 24

Hello from test endpoint!

================================================================================
```

## File Format

Each file is organized with:

1. **Header**: `[YYYY-MM-DD HH:MM:SS] Request/Response #N`
2. **Content**: The actual HTTP message (headers + body)
3. **Separator**: Line of 80 `=` characters
4. **Blank line**: For readability

This makes it easy to parse:
```python
messages = content.split('='*80)
for msg in messages:
    if msg.strip():
        lines = msg.strip().split('\n')
        header = lines[0]  # [timestamp] Request/Response #N
        data = '\n'.join(lines[1:])
        # Process message...
```

## Troubleshooting

| Problem | Solution |
|---------|----------|
| Empty responses | Use HTTP (port 9090) not HTTPS |
| No traffic captured | Ensure proxy is configured in browser |
| "Address in use" | Change port in http_capture.py |
| Proxy error displayed in browser | Check that http_test_server.py is running |
| Can't connect to proxy | Make sure you're using `http://` not `https://` |

## Advanced: Custom Test Data

Edit `http_test_server.py` to add more endpoints:

```python
def do_GET(self):
    if self.path == '/custom':
        response = "Custom response data"
        self.send_response(200)
        self.send_header('Content-Type', 'text/plain')
        self.send_header('Content-Length', len(response))
        self.end_headers()
        self.wfile.write(response.encode())
```

## What the Scripts Do

### http_capture.py
- Runs proxy on port 8080
- Forwards HTTP requests to target server
- Logs requests before forwarding
- Logs responses before sending to browser
- Skips HTTPS CONNECT requests (can't see encrypted content)

### http_test_server.py
- Simple HTTP server on port 9090
- Provides multiple endpoints for testing
- Generates various HTTP response types
- Small, controllable test data

## Next Steps

1. ✅ Capture real HTTP traffic using the test server
2. ✅ Parse the captured files with your HTTP parser
3. ✅ Test different response types (HTML, JSON, plain text)
4. ✅ Test error responses (404, 500, etc.)
5. ✅ Create more complex messages in http_test_server.py

Happy testing! 🎉
