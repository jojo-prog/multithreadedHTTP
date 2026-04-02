#ifndef HTTP_H
#define HTTP_H
#include <stdio.h>
#include <stddef.h>
#include <stdbool.h>

#define CRLF "\r\n"
#define MISSINGSTARTL 1
#define BRKHEADER 2
#define BRKSTATUSL 3
#define BRKRL 3
#define MISSINGEND 4

typedef enum {
    INVALID,
    OPTIONS,
    GET,
    HEAD,
    POST,
    PUT,
    DELETE,
    TRACE,
    CONNECT
} Methods;

typedef enum {
    NOTFOUND,
    // general
    CC, // Cache-Control
    CON, // Connection
    DATE,
    PRAGMA,
    TRAILER,
    TE, // Transfer-Encoding
    UPGRADE,
    VIA,
    WARN, // Warning
    // request
    ACC, // Accept
    ACCCHAR, // Accept-Charset
    ACCENC, // Accept-Encoding
    ACCLAN, // Accept-Language
    AUTH, // Authorization
    EXPECT, 
    FROM,
    HOST,
    IFMATCH,
    IFMS, // If-Modified-Since
    IFNM, // If-None-Match
    IFRANGE,
    IFUMS, // If-Unmodified-Since
    MAXF, // Max-Forwards
    PAUTH, // Proxy-Authorization
    RANGE,
    REFERER,
    USERAGENT,
    // resonse header
    ACCR, // Accept-Range
    AGE,
    ETAG,
    LOC, // Location
    RETRYAFTER,
    SERVER,
    VARY,
    WWWAUTH, // WWW-Authenticate
    // entiy header
    ALLOW,
    CENC, // Content-Encoding
    CLAN, //Content-Language
    CLEN, // Content-Length
    CLOC, // Content-Location
    CMD5, // Content-MD5
    CRANGE, // Content-Range
    CTYPE, // Content-Type
    EXPIRES,
    LASTMOD, // Last-Modifed

} HeaderFieldName;

typedef enum {
    C100,
 C101,
 C200,
 C201,
 C202,
 C203,
 C204,
 C205,
 C206,
 C300,
 C301,
 C302,
 C303,
 C304,
 C305,
 C307,
 C400,
 C401,
 C402,
 C403,
 C404,
 C405,
 C406,
 C407,
 C408,
 C409,
 C410,
 C411,
 C412,
 C413,
 C414,
 C415,
 C416,
 C417,
 C500,
 C501,
 C502,
 C503,
 C504,
 C505
} StatusCode;

typedef struct {
    HeaderFieldName name;
    char** content;
    size_t content_len;
} Header;

typedef struct {
    Methods method;
    char* uri;
    size_t uri_len;
    unsigned int v_major;
    unsigned int v_minor;

} RequestLine;

typedef struct {
    unsigned int v_major;
    unsigned int v_minor;

    char status_code[3];
    char* reason_phrase;
    size_t phrase_len;

} StatusLine;

typedef enum {
    REQUEST,
    RESPONSE
} MessageType;


typedef struct {
    MessageType type;
    void* start_line; // if type REQUEST cast to requestline if type RESPONSE parse to StatusLine
    Header** header;
    size_t num_header;
    char* body;
    size_t body_len;
} HttpMessage;

int parseMessage(HttpMessage* httpmessage, char* message);

int buildMessage(char** message_str, HttpMessage* message, size_t *len);

void printMessage(HttpMessage* httpmessage);

void freeMessage(HttpMessage* message);

extern StatusCode code;

#ifdef TEST
    char* getNextToken(char **str, const char delim[2]);
    char* str_trim(char** str);
    bool isEqual(HttpMessage* message1, HttpMessage* message2);
#endif // 0

#endif // !HTTP_H