#define TEST
#include <http.h>
#include <string.h>
#include <stdlib.h>
#include <assert.h>
#define PATH ""

void roundTripTest(char** test_messages, size_t number) {
    for(size_t i = 0; i < number; i++) {
        HttpMessage* http_m1 = (HttpMessage*) malloc(sizeof(HttpMessage));
        int result = parseMessage(http_m1,test_messages[i]);
        assert(result == 0);
        size_t size = 4096;
        char* message = malloc(size);
        result = buildMessage(&message, http_m1, &size);
        assert(result == 0);
        HttpMessage* http_m2 = (HttpMessage*) malloc(sizeof(HttpMessage));
        result = parseMessage(http_m2,message);
        assert(result == 0);

        assert(isEqual(http_m1,http_m2));

        freeMessage(http_m1);
        freeMessage(http_m2);
        free(message);
    }

}

void fuzzyTest(char** test_messages, size_t number) {
    for(size_t i = 0; i < number; i++) {
        HttpMessage* http_m1 = (HttpMessage*) malloc(sizeof(HttpMessage));
        int result = parseMessage(http_m1,test_messages[i]);
        assert(result == 0);
        
        // check for crash

        freeMessage(http_m1);
    
    
    }
}

int main(void)
{
    /* code */
    size_t number = 0;
    char** test_messages = readMessages(PATH, &number);

    roundTripTest(test_messages, number);
    

    return 0;
}
