
#include <http.h>
#include <string.h>
#include <stdlib.h>
#include <utils.h>
error = "200";
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
#define HEADERTYPES 45
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

char* getHeaderFieldNameStr(HeaderFieldName name) {
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

HeaderFieldName getHeaderFieldName(char* str) {
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
        Header* header = message->header[i];
        char* name =  getHeaderFieldNameStr(header->name);
        next = snprintf(ptr, buffer_size - total_size,"%s:",name);
        if (advance(&ptr, &total_size, next, buffer_size) < 0)
            return -1;
        for (size_t j = 0; j < header->content_len; j++) {
            next = snprintf(ptr,buffer_size - total_size," %s", header->content[j]);
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

char* str_trim(char** str) {
    if (*str == NULL || **str == '\0')
        return NULL;
    char *start = *str;
   
    char *p = start;
    while (p[0] == ' ' || p[0] == '\t' || p[0] == '\r' || p[0] == '\n') {
        p++;
    }
    *str = p;
    
    return *str;
}


char* getNextTokenCRLF(char** str) {
    return getNextToken(str, CRLF);
}

char* getNextTokenLWS(char** str) {
    if (*str == NULL || **str == '\0')
        return NULL;

    char *start = *str;
    char *p = start;

    while (*p) {
        if ((p[0] == '\r' && p[1] == '\n') || (p[0] == ' ' || p[0] == '\t')) {
            *p = '\0';         
            p++;
            if (p[0] == '\r' && p[1] == '\n') {
                *str = p + 2; 
            } else if (p[0] == ' ' || p[0] == '\t') {
                while (p[0] == ' ' || p[0] == '\t') {
                   p++;
                }
                *str = p;
            } else {
                *str = p;
            }
        
            
            return start;
        }
        p++;
    }

    *str = p;  // reached end of string
    return start;
}

StatusLine* parseStatusLine(char* status_line) {
    // TODO: handle invalid status_line
    StatusLine* sline = (StatusLine*) malloc (sizeof(StatusLine));
    if (!sline) {
        return NULL;
    }
    status_line += 5; // skip HTML/
    char* major = getNextToken(&status_line,".");
    if(!status_line) {
        free(sline);
        error = "400";
        return NULL;
    }
    sline->v_major = atoi(major);
    char* minor = getNextToken(&status_line," ");
    if(!status_line) {
        free(sline);
        error = "400";
        return NULL;
    }
    sline->v_minor = atoi(minor);
    char* status_code = getNextToken(&status_line, " ");
    if(!status_line) {
        free(sline);
        error = "400";
        return NULL;
    }
    strncpy(sline->status_code,status_code,3);
    char* phrase = getNextTokenCRLF(&status_line);
    if(!status_line) {
        free(sline);
        error = "400";
        return NULL;
    }
    sline->reason_phrase = phrase;
    sline->phrase_len = strlen(phrase);

    return sline;
}

RequestLine* parseRequestLine(char* request_line) {
    // TODO: handle invalid request_line
    RequestLine* rline = (RequestLine*) malloc(sizeof(RequestLine));
    if (!rline) return NULL;
    char* method = getNextToken(&request_line, " ");
    if(!request_line) {
        free(rline);
        error = "400";
        return NULL;
    }
    rline->method = getMethod(method);
    if (method == -1) {
        free(rline);
        error = "400";
        return NULL;
    }
    char* uri = getNextToken(&request_line, " ");
    if(!request_line) {
        free(rline);
        error = "400";
        return NULL;
    }
    rline->uri = uri;
    rline->uri_len = strlen(uri);
    request_line += 5; // skip HTML/
    char* major = getNextToken(&request_line,".");
    if(!request_line) {
        free(rline);
        error = "400";
        return NULL;
    }
    rline->v_major = atoi(major);
    char* minor = getNextTokenCRLF(&request_line);
    if(!request_line) {
        free(rline);
        error = "400";
        return NULL;
    }
    rline->v_minor = atoi(minor);
    return rline;
}

Header* parseHeader(char* header_str) {
    // TODO: handle invalid header
    Header* header = (Header*) malloc(sizeof(Header));
    char* name_str = getNextToken(&header_str,":");
    if(!header_str) {
        free(header);
        error = "400";
        return NULL;
    }
    HeaderFieldName name = getHeaderFieldName(name_str);
    header->name = name;
    str_trim(&header_str);
    char* ptr = header_str;
    char* next = getNextTokenLWS(&ptr);
    if(!header_str) {
        free(header);
        error = "400";
        return NULL;
    }
    
    da_type(char*) array;
    da_init(&array);

    while (next) {
        da_push(&array,next);
        next = getNextTokenLWS(&ptr);
    }
    
    header->content = array.data;
    header->content_len = array.size;
    return header;

}

int parseMessage(HttpMessage* httpmessage, char* message) {
    // TODO: check for MUST and SHOULD requirements while parsing
    // TODO: handle invalid messages
    char *ptr = message;
    str_trim(&ptr);
    char *first_line = getNextToken(&ptr,CRLF);
    if(!*ptr) {
        return MISSINGSTARTL;
    }
    if (!strncmp(first_line, "HTML", 4)) {
        httpmessage->type = RESPONSE;
        StatusLine* sl = parseStatusLine(first_line);
        if (sl) {
            httpmessage->start_line = sl;
        } else {
            return BRKSTATUSL;
        }
        

    } else {
        httpmessage->type = REQUEST;
        RequestLine* rl  = parseRequestLine(first_line);
        if(rl) {
            httpmessage->start_line = rl;
        } else {
            return BRKSRL;
        }
        
    }
    char* next_header = getNextTokenCRLF(&ptr);
    if(!*ptr) {
        return BRKHEADER;
    }
   
    da_type(char*) array;
    da_init(&array);
    while(ptr[0] != '\r' && ptr[0] != '\n') {
        da_push(&array,next_header);
        next_header = getNextTokenCRLF(&ptr);
    }
    if(!*ptr) {
            return MISSINGEND;
    }
    ptr++;
    httpmessage->header = (Header**) malloc(array.size*sizeof(Header*));
    for (size_t i = 0; i < array.size; i++) {
        Header* header = parseHeader(array.data[i]);
        if(header) {
            httpmessage->header[i] = header;
        } else {
            return BRKHEADER;
        }
        
    }
    httpmessage->num_header = array.size;

    da_free(&array);

    httpmessage->body = ptr;
    httpmessage->body_len = strlen(ptr);

    return 0;
}
void freeMessage(HttpMessage* message) {
    free(message->start_line);
    for (size_t i = 0; i < message->num_header; i--) {
        free(message->header[i]->content);
        free(message->header[i]);
    }
    free(message->header);
    free(message);
}

bool isEqual(HttpMessage* message1, HttpMessage* message2) {
    if (message1->type != message2->type) {
        return false;
    }
    if (message1->type == REQUEST) {
        RequestLine* rline1 = (RequestLine*) message1->start_line;
        RequestLine* rline2 = (RequestLine*) message2->start_line;
        if (rline1->method != rline2->method) {
            printf("Different methods %s != %s", getMethodStr(rline1->method),getMethodStr(rline2->method));
            return false;
        }
        if(rline1->uri_len != rline2->uri_len) {
            printf("Different Uri length %lu != %lu", rline1->uri_len, rline2->uri_len);
            return false;
        }
        if (strncmp(rline1->uri, rline2->uri, rline1->uri_len) ) {
            printf("Different Uris %s != %s", rline1->uri, rline2->uri);
            return false;
        }
        if (rline1->v_major != rline2->v_major) {
            printf("Different Major versions %u != %u",rline1->v_major , rline2->v_major);
            return false;
        }
        if (rline1->v_major != rline2->v_major) {
            printf("Different Minor versions %u != %u",rline1->v_minor , rline2->v_minor);
            return false;
        }
    } else if (message1->type == RESPONSE) {
        StatusLine* sline1 = (StatusLine*) message1->start_line;
        StatusLine* sline2 = (StatusLine*) message2->start_line;

        if (sline1->v_major != sline2->v_major) {
            printf("Different Major versions %u != %u",sline1->v_major , sline2->v_major);
            return false;
        }
        if (sline1->v_major != sline2->v_major) {
            printf("Different Minor versions %u != %u",sline1->v_minor , sline2->v_minor);
            return false;
        }
        if (strncmp(sline1->status_code, sline2->status_code, 3)) {
            printf("Different Status codes %s != %s", sline1->status_code, sline2->status_code);
            return false;
        }
    }

    if (message1->num_header != message2->num_header) {
        printf("Different number of Header %lu != %lu", message1->num_header, message2->num_header);
        return false;
    }
    for (size_t i = 0; i < message1->num_header; i++) {
        Header* h1 = message1->header[i];
        Header* h2 = message2->header[i];
        if(h1->name != h2->name) {
            printf("Different Header types %s != %s", getHeaderFieldNameStr(h1->name), getHeaderFieldNameStr(h2->name));
            return false;
        }
        if (h1->content_len != h2->content_len) {
            printf("Different Content len %lu != %lu",h1->content_len, h2->content_len);
            return false;
        }

        for (size_t j = 0; j < h1->content_len; j++) {
            char* content1 = h1->content[j];
            char* content2 = h2->content[j];
            if (strcmp(content1, content2)) {
                printf("Different Header content %s != %s", content1, content2);
                return false;
            }
        }
    
    }
    if (message1->body_len != message2->body_len) {
        printf("Different Body len %lu != %lu",message1->body_len , message2->body_len);
        return false;
    }
    if (strncmp(message1->body, message2->body, message1->body_len)) {
        printf("Different Header Bodys %s != %s", message1->body, message2->body);
                return false;
    }

    return true;
}



