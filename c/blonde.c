#include "sourcekitd.h"
#include <stdio.h>

#define null 0

#define UID(symbol, string) uids. symbol = sourcekitd_uid_get_from_cstr(string)

struct 
{
    sourcekitd_uid_t key_request, 
        source_request_editor_open, 
        key_name, 
        key_sourcefile, 
        key_enablesyntaxmap, 
        key_enablesubstructure, 
        key_enablesyntaxtree;
} uids;

void create_uids(void) 
{
    UID(key_request,                "key.request");
    UID(source_request_editor_open, "source.request.editor.open");
    UID(key_name,                   "key.name");
    UID(key_sourcefile,             "key.sourcefile");
    UID(key_enablesyntaxmap,        "key.enablesyntaxmap");
    UID(key_enablesubstructure,     "key.enablesubstructure");
    UID(key_enablesyntaxtree,       "key.enablesyntaxtree");
}



int main(void)
{
    create_uids();
    sourcekitd_initialize();
    
    sourcekitd_object_t request = sourcekitd_request_dictionary_create(null, null, 0);
    sourcekitd_request_dictionary_set_uid   (request, uids.key_request, uids.source_request_editor_open);
    sourcekitd_request_dictionary_set_string(request, uids.key_name, "test.swift");
    sourcekitd_request_dictionary_set_string(request, uids.key_sourcefile, "test.swift");
    sourcekitd_request_dictionary_set_int64 (request, uids.key_enablesyntaxmap, 1);
    sourcekitd_request_dictionary_set_int64 (request, uids.key_enablesubstructure, 0);
    sourcekitd_request_dictionary_set_int64 (request, uids.key_enablesyntaxtree, 0);
    
    sourcekitd_response_t response = sourcekitd_send_request_sync(request);
    sourcekitd_request_release(request);
    
    sourcekitd_response_description_dump(response);
    
    sourcekitd_response_dispose(response);
    
    sourcekitd_shutdown();
    return 0;
}
