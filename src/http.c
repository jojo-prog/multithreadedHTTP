#include <http.h>
#include <string.h>
#include <stdlib.h>
#define METHODSNUM 7
const struct {char* str; Methods m;} methods[7] = {
    {"OPTIONS", OPTIONS},
    {"GET", GET},
    {"HEAD", HEAD},
    {"POST",POST},
    {"PUT", DELETE},
    {"TRACE", TRACE},
    {"CONNECT", CONNECT}
};
#define HEADERTYPES 49
const struct {HeaderFieldName name; char* str; } field_names[] = {
    {CC, "Cache-Control"},
    {CON, "Connection"},
    {DATE, "Date"},
    {PRAGMA, "Pragma"},
    {TRAILER, "Trailer"},
    {TE, "Transfer-Encoding"},
    {UPGRADE, "Upgrade"},
    {VIA, "Via"},
    {WARN, "Warning"},
    // request
    {ACC, "Accept"},
    {ACCCHAR,  "Accept-Charset"},
    {ACCENC, "Accept-Encoding"},
    {ACCLAN,  "Accept-Language"},
    {AUTH, "Authorization"},
    {EXPECT,  "Expect"},
    {FROM, "From"},
    {HOST, "Host"},
    {IFMATCH, "If-Match"},
    {IFMS, "If-Modified-Since"},
    {IFNM, "If-None-Match"},
    {IFRANGE, "If-Range"},
    {IFUMS, "If-Unmodified-Since"},
    {MAXF, "Max-Forwards"},
    {PAUTH, "Proxy-Authorization"},
    {RANGE, "Range"},
    {REFERER, "Referer"},
    {TE, "TE"},
    {USERAGENT, "User-Agent"},
    // resonse header
    {ACCR, "Accept-Range"},
    {AGE, "Age"},
    {ETAG, "ETag"},
    {LOC, "Location"},
    {RETRYAFTER, "Retry-After"},
    {SERVER, "Server"},
    {VARY, "Vary"},
    {WWWAUTH, "WWW-Authenticate"},
    // entiy header
    {ALLOW, "Allow"},
    {CENC, "Content-Encoding"},
    {CLAN, "Content-Language"},
    {CLEN, "Content-Length"},
    {CLOC, "Content-Location"},
    {CMD5, "Content-MD5"},
    {CRANGE, "Content-Range"},
    {CTYPE, "Content-Type"},
    {EXPIRES, "Expires"},
    {LASTMOD, "Last-Modified"}
};

const char* status_codes[][2] = {
 {"100", "Continue"},
 {"101", "Switching Protocols"},
 {"200", "OK"},
 {"201", "Created"},
 {"202", "Accepted"},
 {"203", "Non-Authoritative Information"},
 {"204", "No Content"},
 {"205", "Reset Content"},
 {"206", "Partial Content"},
 {"300", "Multiple Choices"},
 {"301", "Moved Permanently"},
 {"302", "Found"},
 {"303", "See Other"},
 {"304", "Not Modified"},
 {"305", "Use Proxy"},
 {"307", "Temporary Redirect"},
 {"400", "Bad Request"},
 {"401", "Unauthorized"},
 {"402", "Payment Required"},
 {"403", "Forbidden"},
 {"404", "Not Found"},
 {"405", "Method Not Allowed"},
 {"406", "Not Acceptable"},
 {"407", "Proxy Authentication Required"},
 {"408", "Request Time-out"},
 {"409", "Conflict"},
 {"410", "Gone"},
 {"411", "Length Required"},
 {"412", "Precondition Failed"},
 {"413", "Request Entity Too Large"},
 {"414", "Request-URI Too Large"},
 {"415", "Unsupported Media Type"},
 {"416", "Requested range not satisfiable"},
 {"417", "Expectation Failed"},
 {"500", "Internal Server Error"},
 {"501", "Not Implemented"},
 {"502", "Bad Gateway"},
 {"503", "Service Unavailable"},
 {"504", "Gateway Time-out"},
 {"505", "HTTP Version not supported"}
};

char* getMethodStr(Methods method) {
    for (int i = 0; i < METHODSNUM; i++) {
        if (methods[i].m == method) {
            return methods[i].str;
        }
    }
   return NULL;
}

Methods getMethod(char* str) {
    for (int i = 0; i < METHODSNUM; i++) {
        if (!strcmp(methods[i].str ,str)) {
            return methods[i].m;
        }
    }
    return -1;
}

char* getHeaderFileNameStr(HeaderFieldName name) {
    for (int i = 0; i < HEADERTYPES; i++) {
        if (field_names[i].name == name) {
            return field_names[i].str;
        }
    }
   return NULL;
}

int getStatusCode(const char code[3]) {
    for (int i = 0; i < 40; i++) {
        if (!strncmp(status_codes[i][0],code,3)) {
            return i;
        }
    }
    return -1;
}

HeaderFieldName getHeaderFileName(char* str) {
    for (int i = 0; i < HEADERTYPES; i++) {
        if (!strcmp(field_names[i].str ,str)) {
            return field_names[i].name;
        }
    }
    return -1;
}

int advance(char **ptr, size_t *current, size_t next, size_t max) {
    if (*current + next >= max)
        return -1;

    *ptr += next;
    *current += next;
    return 0;
}

// len used to forward how big the message
int buildMessage(char** message_str, HttpMessage* message, size_t *len) {
    size_t total_size = 0;
    size_t buffer_size = *len;
    char* ptr = *message_str;
    // TODO: Predict message size
    if (message->type == REQUEST) {
       
        RequestLine* rline = (RequestLine*) message->start_line;
        char* method = getMethodStr(rline->method);
        printf("Uri in Build %s\n", rline->uri);
        size_t rline_size = snprintf(ptr, buffer_size - total_size, "%s %s HTTP/%u.%u" CRLF, method, rline->uri,rline->v_major, rline->v_minor);
        
        
        if (advance(&ptr, &total_size, rline_size, buffer_size) < 0)
            return -1;

    } else if (message->type == RESPONSE) {
        StatusLine* sline = (StatusLine*) message->start_line;
        size_t sline_size = snprintf(ptr, buffer_size - total_size, "HTTP/%u.%u %s %s"CRLF, sline->v_major, sline->v_minor, sline->status_code, sline->reason_phrase);

        if (advance(&ptr, &total_size, sline_size, buffer_size) < 0)
            return -1;
    }
    
    int next;
    for (size_t i = 0; i < message->num_header; i++) {
        Header header = message->header[i];
        char* name =  getHeaderFileNameStr(header.name);
        next = snprintf(ptr, buffer_size - total_size,"%s:",name);
        if (advance(&ptr, &total_size, next, buffer_size) < 0)
            return -1;
        for (size_t j = 0; j < header.content_len; j++) {
            next = snprintf(ptr,buffer_size - total_size," %s", header.content[i]);
            if (advance(&ptr, &total_size, next, buffer_size) < 0)
            return -1;
        }
        next = snprintf(ptr,buffer_size - total_size,CRLF);
        if (advance(&ptr, &total_size, next, buffer_size) < 0)
            return -1;
    }

    next = snprintf(ptr,buffer_size - total_size,CRLF);
    if (advance(&ptr, &total_size, next, buffer_size) < 0)
            return -1;
    next = snprintf(ptr,buffer_size-total_size,"%s",message->body);

    if (advance(&ptr, &total_size, next, buffer_size) < 0)
            return -1;

    *len = total_size;
    
    return 0;

}

char* getNextToken(char **str, const char delim[2]) {
    if (*str == NULL || **str == '\0')
        return NULL;

    char *start = *str;
    char *p = start;

    while (*p) {
        if (p[0] == delim[0] && (p[1] == delim[1] || !delim[1])) {
            *p = '\0';         
                  
            if (!delim[1]) {
                *str = p + 1;
            } else {
                *str = p + 2; 
            }
            return start;
        }
        p++;
    }

    *str = p;  // reached end of string
    return start;
}

StatusLine* parseStatusLine(char* status_line) {
    StatusLine* sline = (StatusLine*) malloc (sizeof(StatusLine));
    status_line += 5; // skip HTML/
    char* major = getNextToken(&status_line,".");
    sline->v_major = atoi(major);
    char* minor = getNextToken(&status_line," ");
    sline->v_minor = atoi(minor);
    char* status_code = getNextToken(&status_line, " ");
    strncpy(sline->status_code,status_code,3);
    char* phrase = getNextToken(&status_line, CRLF);
    sline->reason_phrase = phrase;
    sline->phrase_len = strlen(phrase);

    return sline;
}

RequestLine* parseRequestLine(char* request_line) {
    RequestLine* rline = (RequestLine*) malloc(sizeof(RequestLine));
    char* method = getNextToken(&request_line, " ");
    rline->method = getMethod(method);
    char* uri = getNextToken(&request_line, " ");
    strcpy(rline->uri, uri);
    rline->uri_len = strlen(uri);
    request_line += 5; // skip HTML/
    char* major = getNextToken(&request_line,".");
    rline->v_major = atoi(major);
    char* minor = getNextToken(&request_line,CRLF);
    rline->v_minor = atoi(minor);
    return rline;
}

int parseMessage(HttpMessage* httpmessage, char* message) {
    // TODO: check for MUST and SHOULD requirements while parsing
    char *ptr = message;
    char *first_line = getNextToken(&ptr,CRLF);
    if (!strncmp(first_line, "HTML", 4)) {
        httpmessage->type = RESPONSE;
        httpmessage->start_line = parseStatusLine(first_line);

    } else {
        httpmessage->type = REQUEST;
        httpmessage->start_line = parseRequestLine(first_line);
    }


    return 0;
}
void freeMessage(HttpMessage* message) {
    free(message->start_line);
    free(message);
}



