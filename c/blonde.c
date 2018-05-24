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
        key_enablesyntaxtree, 
        
        key_syntaxmap;
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
    
    UID(key_syntaxmap,              "key.syntaxmap");
}

sourcekitd_object_t request;

void blonde_init(void)
{
    create_uids();
    sourcekitd_initialize();
    
    request = sourcekitd_request_dictionary_create(null, null, 0);
    sourcekitd_request_dictionary_set_uid   (request, uids.key_request, uids.source_request_editor_open);
    sourcekitd_request_dictionary_set_int64 (request, uids.key_enablesyntaxmap, 1);
    sourcekitd_request_dictionary_set_int64 (request, uids.key_enablesubstructure, 0);
    sourcekitd_request_dictionary_set_int64 (request, uids.key_enablesyntaxtree, 0);    
}

void blonde_set_sourcefile(char const* const sourcefile)
{
    sourcekitd_request_dictionary_set_string(request, uids.key_name, sourcefile);
    sourcekitd_request_dictionary_set_string(request, uids.key_sourcefile, sourcefile);
}

void blonde_highlight(void)
{
    sourcekitd_response_t const response = sourcekitd_send_request_sync(request);
    sourcekitd_variant_t  const dict     = sourcekitd_response_get_value(response);
    sourcekitd_variant_t  const array    = sourcekitd_variant_dictionary_get_value(dict, uids.key_syntaxmap);
    
    size_t const count = sourcekitd_variant_array_get_count(array);
    
    sourcekitd_variant_description_dump(array);
    
    sourcekitd_response_dispose(response);
}

void blonde_deinit(void)
{
    sourcekitd_request_release(request);
    sourcekitd_shutdown();
}

int main(void)
{
    blonde_init();
    blonde_set_sourcefile("test.swift");
    blonde_highlight();
    blonde_deinit();
    return 0;
}
