
#define NOB_IMPLEMENTATION
#include "nob.h"
#define TEST
#include <http.h>
#include <utils.h>
#include <string.h>
#include <stdlib.h>
#include <assert.h>

typedef struct test
    {
        char** items;
        size_t count;
        size_t capacity;
    } Messages;

#define PATH_REQUESTS "/home/jojo/dev/multithreadedHTTP/src/http_requests.txt"
#define PATH_RESPONSES "/home/jojo/dev/multithreadedHTTP/src/http_responses.txt"


Messages readMessages(const char* path) {
    String_Builder sb = {0};
    Messages message_array = {0};
    

    if(nob_read_entire_file(path,&sb)){
        String_View sv = nob_sb_to_sv(sb);
        String_View line = nob_sv_chop_by_delim(&sv,'\n');
        String_Builder message = {0};
        while(*line.data != '\0') {
            if(sv_starts_with(line,(String_View){1,"["})) line = nob_sv_chop_by_delim(&sv,'\n');
            
            if(nob_sv_eq(line, (Nob_String_View) {80,"================================================================================"})) {
                nob_sb_append_null(&message);
                nob_da_append(&message_array,message.items);
               // printf("%s\n", message.items);
                memset(&message,0,sizeof(String_Builder));

            } else {
                nob_sb_append_cstr(&message,nob_temp_sv_to_cstr(line));
                nob_sb_append(&message,'\n');
            }
            line = nob_sv_chop_by_delim(&sv,'\n');
            
        }
    }


    return message_array;
}

void roundTripTest() {
    Messages test_messages = readMessages(PATH_REQUESTS);
    for(size_t i = 0; i < test_messages.count; i++) {
        HttpMessage* http_m1 = (HttpMessage*) malloc(sizeof(HttpMessage));
        int result = parseMessage(http_m1,test_messages.items[i]);
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
    nob_da_free(test_messages);
}


void fuzzyTest() {
    Messages test_messages = readMessages(PATH_REQUESTS);
    for(size_t i = 0; i < test_messages.count; i++) {

        // random mutation

        // parse
        printf("%s\n", test_messages.items[i]);
        HttpMessage* http_m1 = (HttpMessage*) malloc(sizeof(HttpMessage));
        int result = parseMessage(http_m1,test_messages.items[i]);
        assert(result == 0);
        
        // check for crash

        freeMessage(http_m1);
    
    
    }
    nob_da_free(test_messages);
}

void diffTest() {
    Messages test_messages = readMessages(PATH_REQUESTS);
    for(size_t i = 0; i < test_messages.count; i++) {

        // my parser
        
        HttpMessage* http_m1 = (HttpMessage*) malloc(sizeof(HttpMessage));
        int result = parseMessage(http_m1,test_messages.items[i]);
        assert(result == 0);
        
        // different parser

        // check for differences

        freeMessage(http_m1);
    
    
    }
    nob_da_free(test_messages);
}



int main(void)
{
    /* code */

    roundTripTest();
    fuzzyTest();
    diffTest();

    
    
    return 0;
}
