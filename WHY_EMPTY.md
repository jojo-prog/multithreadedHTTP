# HTTP Capture - Why Responses Are Empty

## The Problem

When you visited `http://localhost:9090` through your browser, Firefox **bypassed the proxy** for localhost connections. This is a browser security feature.

Result:
- ✅ Test server received requests
- ❌ Proxy never saw those requests  
- ❌ No responses captured
- ✅ But Firefox's own detection requests DID go through proxy → captured but timed out

## The Solution

Use the **`http_test_client.py`** script instead of manual browsing. This python script makes HTTP requests **directly through the proxy**, ensuring they're captured.

## Step-by-Step

### Terminal 1: Start Proxy
```bash
python3 http_capture.py
```

Output:
```
Cleared http_requests.txt and http_responses.txt
HTTP Proxy Server started on 127.0.0.1:8080
...
Press Ctrl+C to stop the proxy server
```

### Terminal 2: Start Test Server
```bash
python3 http_test_server.py
```

Output:
```
HTTP Test Server running on 127.0.0.1:9090
...
Press Ctrl+C to stop
```

### Terminal 3: Run Test Client
```bash
python3 http_test_client.py
```

Output:
```
HTTP Test Client - Generating traffic through proxy

Proxy: http://127.0.0.1:8080
Target: http://127.0.0.1:9090

[1] Home page: GET /
    ✓ Status: 200 OK
    Response length: 850 bytes

[2] Test endpoint: GET /test
    ✓ Status: 200 OK
    Response length: 24 bytes

...
```

### Check Captured Files
```bash
# Stop the proxy (Ctrl+C in Terminal 1)
cat http_requests.txt
cat http_responses.txt
```

## Expected Output Format

### http_requests.txt
```
[2026-03-19 16:30:45] Request #1
GET http://127.0.0.1:9090/ HTTP/1.1
Host: 127.0.0.1:9090
Connection: keep-alive

================================================================================

[2026-03-19 16:30:46] Request #2
GET http://127.0.0.1:9090/test HTTP/1.1
Host: 127.0.0.1:9090

================================================================================
```

### http_responses.txt
```
[2026-03-19 16:30:45] Response #1
HTTP/1.1 200 OK
Server: SimpleHTTP/0.6 Python/3.10.x
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

## Why This Works

The test client uses Python's `http.client` to make HTTP requests **with explicit proxy settings**, ensuring:

1. ✅ All requests go through `http://127.0.0.1:8080`
2. ✅ Proxy captures requests AND responses
3. ✅ Data is saved to files with proper format
4. ✅ Fully reproducible test data

## Alternative: Browser with Proper Proxy Config

If you prefer to use a browser, Firefox has an option to NOT bypass localhost:

1. Type `about:config` in address bar
2. Search: `network.proxy.no_proxies_on`
3. Clear the value (remove "localhost, 127.0.0.1")
4. Now visit `http://127.0.0.1:9090` and it will go through proxy

But **the test client approach is simpler and more reliable**.

## What You Get

- **Real HTTP requests** (not HTTPS)
- **Real HTTP responses** with headers and body
- **Different response types**: HTML, JSON, plain text
- **Different status codes**: 200, 404
- **Different methods**: GET, POST
- **Easy to parse** with clear message separators
- **Reproducible** - same requests every time

## Now Test Your Parser

You have real, captured HTTP traffic! Use it to:

```python
def parse_http_message(text):
    """Parse a single HTTP message"""
    lines = text.strip().split('\n')
    
    # First line (request/response line)
    first_line = lines[0]
    
    # Headers until blank line
    headers = {}
    body_start = None
    for i, line in enumerate(lines[1:], 1):
        if line.strip() == '':
            body_start = i + 1
            break
        if ':' in line:
            key, val = line.split(':', 1)
            headers[key.strip()] = val.strip()
    
    # Body
    body = '\n'.join(lines[body_start:]) if body_start else ''
    
    return first_line, headers, body

# Parse requests
with open('http_requests.txt') as f:
    messages = f.read().split('='*80)
    
for msg in messages:
    if msg.strip():
        lines = msg.strip().split('\n')
        timestamp = lines[0]
        req_data = '\n'.join(lines[1:])
        
        first_line, headers, body = parse_http_message(req_data)
        print(f"Time: {timestamp}")
        print(f"Request: {first_line}")
        print(f"Headers: {headers}")
        if body:
            print(f"Body: {body[:100]}...")
        print()
```

Happy parsing! 🎉
