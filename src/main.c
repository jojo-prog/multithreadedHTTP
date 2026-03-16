#define TEST
#include <http.h>
#include <string.h>
#include <stdlib.h>
int main()
{
    char* m = "  \t \n Hallo    \n\r  ";
    str_trim(&m);
    printf("%s\n", m);

    HttpMessage httpmessage = {
        .type = REQUEST,
        .start_line = &(RequestLine){ GET, "/index.html", 11, 1, 1 },
        .header = (Header*[]){
            &(Header){
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
    printf("Build:\n %s\n", message);
    HttpMessage* http_m = (HttpMessage*) malloc(sizeof(HttpMessage));
    int result = parseMessage(http_m,message);
    char* message2 = malloc(4096);
    buildMessage(&message2,http_m,&size);
    printf("Recreated:\n %s\n", message2);

    if (isEqual(&httpmessage,http_m)) {
        printf("Messages are equal");
    } else {
        printf("Something wrong!");
    }

    freeMessage(http_m);
    free(message);
    free(message2);
    return 0;
}
