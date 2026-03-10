#define TEST
#include <http.h>
#include <string.h>
#include <stdlib.h>
int main()
{
    HttpMessage httpmessage = {
        .type = REQUEST,
        .start_line = &(RequestLine){ GET, "/index.html", 11, 1, 1 },
        .header = (Header[]){
            {
                .name = HOST,
                .content = (char*[]){"localhost"},
                .content_len = 1
            }
        },
        .num_header = 1,
        .body = "Hallo Welt!",
        .body_len = 11
    };
    size_t size = 4096;
    char* message = malloc(size);
    buildMessage(&message, &httpmessage, &size);
    printf("Build: %s\n", message);
    HttpMessage* http_m = (HttpMessage*) malloc(sizeof(HttpMessage));
    int result = parseMessage(http_m,message);
    memset(message,0,size);
    buildMessage(&message,http_m,&size);
    printf("Recreated: %s\n", message);
    freeMessage(http_m);
    free(message);
    return 0;
}
