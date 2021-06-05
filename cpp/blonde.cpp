#include "sourcekitd.h"
#include <dlfcn.h>
#include <unordered_map>
#include <nan.h>

#include <unistd.h>

#define UID(symbol, string) uids. symbol = sourcekit::uid_get_from_cstr(string)
#define SPECIES(key, id) species.insert({sourcekit::uid_get_from_cstr(key), id})

#define SYMBOL(library, name, type) name = \
    reinterpret_cast< type >(dlsym( library , "sourcekitd_" #name)); \
    if (name == nullptr) \
    { \
        return error::sourcekit_symbol_not_found; \
    }

namespace sourcekit 
{
    enum struct error 
    {
        none, 
        swift_not_found, 
        swiftenv_not_found, 
        swiftenv_error, 
        sourcekit_not_found, 
        sourcekit_symbol_not_found
    };
    
    sourcekitd_uid_t        (*uid_get_from_cstr)            (const char *string);
    
    void                    (*initialize)                   (void);
    void                    (*shutdown)                     (void);
    
    void                    (*request_release)              (sourcekitd_object_t object);
    sourcekitd_object_t     (*request_dictionary_create)    (const sourcekitd_uid_t *keys, const sourcekitd_object_t *values, size_t count);
    void                    (*request_dictionary_set_string)(sourcekitd_object_t dict, sourcekitd_uid_t key, const char *string);
    void                    (*request_dictionary_set_int64) (sourcekitd_object_t dict, sourcekitd_uid_t key, int64_t val);
    void                    (*request_dictionary_set_uid)   (sourcekitd_object_t dict, sourcekitd_uid_t key, sourcekitd_uid_t uid);
    
    void                    (*response_dispose)             (sourcekitd_response_t obj);
    bool                    (*response_is_error)            (sourcekitd_response_t obj);
    sourcekitd_variant_t    (*response_get_value)           (sourcekitd_response_t resp);
    sourcekitd_variant_t    (*variant_dictionary_get_value) (sourcekitd_variant_t dict, sourcekitd_uid_t key);
    int64_t                 (*variant_dictionary_get_int64) (sourcekitd_variant_t dict, sourcekitd_uid_t key);
    sourcekitd_uid_t        (*variant_dictionary_get_uid)   (sourcekitd_variant_t dict, sourcekitd_uid_t key);
    
    size_t                  (*variant_array_get_count)      (sourcekitd_variant_t array);
    sourcekitd_variant_t    (*variant_array_get_value)      (sourcekitd_variant_t array, size_t index);
    
    sourcekitd_response_t   (*send_request_sync)            (sourcekitd_object_t req);
    
    error load_library(void** library) 
    {
        // locate swift installation with swiftenv 
        #define SWIFT_PATH_LENGTH_MAX 256
        char path[SWIFT_PATH_LENGTH_MAX];
        
        char const* const home = getenv("HOME");
        if (home == nullptr) 
        {
            return error::swift_not_found;
        }
        
        int count;
        count = snprintf(path, SWIFT_PATH_LENGTH_MAX, "%s/.swiftenv/versions/", home);
        if (count < 0 || count >= SWIFT_PATH_LENGTH_MAX) 
        {
            return error::swift_not_found;
        }
        
        FILE* swiftenv = popen("swiftenv version | awk '{ print $1 }'", "r");
        if (swiftenv == nullptr) 
        {
            return error::swiftenv_not_found;
        }
        
        if (fgets(path + count, SWIFT_PATH_LENGTH_MAX - count, swiftenv) == nullptr) 
        {
            return error::swiftenv_not_found;
        }
        
        int const status = pclose(swiftenv);
        if (status != 0) 
        {
            return error::swiftenv_error;
        }
        
        // move to end of string
        for (; count < SWIFT_PATH_LENGTH_MAX && path[count] != '\n'; ++count);
        count += snprintf(path + count, SWIFT_PATH_LENGTH_MAX - count, "/usr/lib/libsourcekitdInProc.so");
        if (count >= SWIFT_PATH_LENGTH_MAX) 
        {
            return error::swift_not_found;
        }
        
        *library = dlopen(path, RTLD_NOW);
        
        // FILE* descriptor = fopen("/Users/kelvin/log.txt", "a");
        // fprintf(descriptor, "%p, %d\n", library, library == nullptr);
        // fclose(descriptor);
        if (library == nullptr) 
        {
            return error::sourcekit_not_found;
        }
        
        return error::none;
        
        #undef SWIFT_PATH_LENGTH_MAX
    }
    
    error load(void) 
    {
        void* library; 
        error const status = load_library(&library);
        switch (status) 
        {
        case error::none:
            break; 
        default:
            return status;
        }
        
        SYMBOL(library, uid_get_from_cstr,              sourcekitd_uid_t        (*)(const char*))
        
        SYMBOL(library, initialize,                     void                    (*)(void))
        SYMBOL(library, shutdown,                       void                    (*)(void))
        
        SYMBOL(library, request_release,                void                    (*)(sourcekitd_object_t object))
        SYMBOL(library, request_dictionary_create,      sourcekitd_object_t     (*)(const sourcekitd_uid_t *keys, const sourcekitd_object_t *values, size_t count))
        SYMBOL(library, request_dictionary_set_string,  void                    (*)(sourcekitd_object_t dict, sourcekitd_uid_t key, const char *string))
        SYMBOL(library, request_dictionary_set_int64,   void                    (*)(sourcekitd_object_t dict, sourcekitd_uid_t key, int64_t val))
        SYMBOL(library, request_dictionary_set_uid,     void                    (*)(sourcekitd_object_t dict, sourcekitd_uid_t key, sourcekitd_uid_t uid))
        
        SYMBOL(library, response_dispose,               void                    (*)(sourcekitd_response_t obj))
        SYMBOL(library, response_is_error,              bool                    (*)(sourcekitd_response_t obj))
        SYMBOL(library, response_get_value,             sourcekitd_variant_t    (*)(sourcekitd_response_t resp))
        SYMBOL(library, variant_dictionary_get_value,   sourcekitd_variant_t    (*)(sourcekitd_variant_t dict, sourcekitd_uid_t key))
        SYMBOL(library, variant_dictionary_get_int64,   int64_t                 (*)(sourcekitd_variant_t dict, sourcekitd_uid_t key))
        SYMBOL(library, variant_dictionary_get_uid,     sourcekitd_uid_t        (*)(sourcekitd_variant_t dict, sourcekitd_uid_t key))
        
        SYMBOL(library, variant_array_get_count,        size_t                  (*)(sourcekitd_variant_t array))
        SYMBOL(library, variant_array_get_value,        sourcekitd_variant_t    (*)(sourcekitd_variant_t array, size_t index))
        
        SYMBOL(library, send_request_sync,              sourcekitd_response_t   (*)(sourcekitd_object_t req))
        return error::none;
    }
}
namespace blonde 
{
    typedef struct 
    {
        uint16_t r, c;
    } latticepoint_t;
    
    typedef struct 
    {
        latticepoint_t start, end;
        uint8_t species;
        uint8_t _;
    } token_t;
    
    sourcekitd_object_t request;
    
    struct 
    {
        sourcekitd_uid_t key_request, 
            source_request_editor_open, 
            key_name, 
            key_sourcefile, 
            key_sourcetext, 
            key_enablesyntaxmap, 
            key_enablediagnostics, 
            key_enablesubstructure, 
            key_enablesyntaxtree, 
            
            key_syntaxmap, 
            key_diagnostics, 
            
            key_kind, 
            key_offset, 
            key_length;
    } uids;
    
    /*
    struct 
    {
        sourcekitd_uid_t source_diagnostic_severity_error,
            source_diagnostic_severity_warning, 
            source_diagnostic_severity_note;
    } diagnostics;
    */
    
    // we can probably speed this up 10x by implementing a custom single-layer hashmap 
    std::unordered_map<sourcekitd_uid_t, uint8_t> species;

    void create_uids(void) 
    {
        UID(key_request,                "key.request");
        UID(source_request_editor_open, "source.request.editor.open");
        UID(key_name,                   "key.name");
        UID(key_sourcefile,             "key.sourcefile");
        UID(key_sourcetext,             "key.sourcetext");
        UID(key_enablesyntaxmap,        "key.enablesyntaxmap");
        UID(key_enablediagnostics,      "key.enablediagnostics");
        UID(key_enablesubstructure,     "key.enablesubstructure");
        UID(key_enablesyntaxtree,       "key.enablesyntaxtree");
        
        UID(key_syntaxmap,              "key.syntaxmap");
        UID(key_diagnostics,            "key.diagnostics");
        
        UID(key_kind,                   "key.kind");
        UID(key_offset,                 "key.offset");
        UID(key_length,                 "key.length");
        
        //DIAGNOSTIC(source_diagnostic_severity_error,   "source.diagnostic.severity.error");
        //DIAGNOSTIC(source_diagnostic_severity_warning, "source.diagnostic.severity.warning");
        //DIAGNOSTIC(source_diagnostic_severity_note,    "source.diagnostic.severity.note");
        
        species.clear();
        SPECIES("source.lang.swift.syntaxtype.keyword",                     0);
        SPECIES("source.lang.swift.syntaxtype.identifier",                  1);
        SPECIES("source.lang.swift.syntaxtype.typeidentifier",              2);
        
        SPECIES("source.lang.swift.syntaxtype.buildconfig.keyword",         3);
        SPECIES("source.lang.swift.syntaxtype.buildconfig.id",              4);
        SPECIES("source.lang.swift.syntaxtype.pounddirective.keyword",      5);
        
        SPECIES("source.lang.swift.syntaxtype.attribute.id",                6);
        SPECIES("source.lang.swift.syntaxtype.attribute.builtin",           7);
        
        SPECIES("source.lang.swift.syntaxtype.number",                      8);
        SPECIES("source.lang.swift.syntaxtype.string",                      9);
        SPECIES("source.lang.swift.syntaxtype.string_interpolation_anchor", 10);
        
        SPECIES("source.lang.swift.syntaxtype.comment",                     11);
        SPECIES("source.lang.swift.syntaxtype.doccomment",                  12);
        SPECIES("source.lang.swift.syntaxtype.doccomment.field",            13);
        SPECIES("source.lang.swift.syntaxtype.comment.mark",                14);
        SPECIES("source.lang.swift.syntaxtype.comment.url",                 15);
        
        SPECIES("source.lang.swift.syntaxtype.placeholder",                 16);
        SPECIES("source.lang.swift.syntaxtype.objectliteral",               17);
    }
    
    void initialize(v8::FunctionCallbackInfo<v8::Value> const& frame)
    {
        sourcekit::error const status = sourcekit::load();
        char const* message;
        switch (status) 
        {
        case sourcekit::error::none:
            sourcekit::initialize();
            
            create_uids();
            
            request = sourcekit::request_dictionary_create(nullptr, nullptr, 0);
            sourcekit::request_dictionary_set_uid  (request, uids.key_request, uids.source_request_editor_open);
            sourcekit::request_dictionary_set_int64(request, uids.key_enablesyntaxmap, 1);
            sourcekit::request_dictionary_set_int64(request, uids.key_enablediagnostics, 0);
            sourcekit::request_dictionary_set_int64(request, uids.key_enablesubstructure, 0);
            sourcekit::request_dictionary_set_int64(request, uids.key_enablesyntaxtree, 0);
            frame.GetReturnValue().SetNull();
            return; 
        
        case sourcekit::error::swift_not_found:
            message = "swift not found";
            break;
        case sourcekit::error::swiftenv_not_found:
            message = "swiftenv not found";
            break;
        case sourcekit::error::swiftenv_error:
            message = "swiftenv error";
            break;
        case sourcekit::error::sourcekit_not_found:
            message = "`libsourcekitdInProc.so` not found";
            break;
        case sourcekit::error::sourcekit_symbol_not_found:
            message = "could not load symbols from `libsourcekitdInProc.so`";
            break;
        default:
            break;
        }
        
        frame.GetReturnValue().Set(Nan::New<v8::String>(message).ToLocalChecked());
    }
    
    void highlight(v8::FunctionCallbackInfo<v8::Value> const& frame)
    {
        v8::Isolate* isolate = v8::Isolate::GetCurrent();
        
        if (frame.Length() != 1 || !frame[0] -> IsString())
        {
            isolate -> ThrowException(v8::Exception::TypeError(
                v8::String::NewFromUtf8(isolate, "invalid arguments").ToLocalChecked()));
        }
        
        // reason 85 why C++ is terrible:
        Nan::Utf8String retain(frame[0]);
        char const* const source = (char const*) *retain;
        
        sourcekit::request_dictionary_set_string(request, uids.key_name, "atomic blonde");
        sourcekit::request_dictionary_set_string(request, uids.key_sourcetext, source);
        
        sourcekitd_response_t const response = sourcekit::send_request_sync(request);
        
        if (sourcekit::response_is_error(response))
        {
            isolate -> ThrowException(v8::Exception::Error(
                v8::String::NewFromUtf8(isolate, "invalid response").ToLocalChecked()));
        }
        
        sourcekitd_variant_t  const dict     = sourcekit::response_get_value(response);
        sourcekitd_variant_t  const syntax   = sourcekit::variant_dictionary_get_value(dict, uids.key_syntaxmap);
        
        size_t const count    = sourcekit::variant_array_get_count(syntax);
        
        token_t* const tokens = new token_t[count];
        uint16_t row     = 0;
        uint64_t lastnl  = 0, // 1 past the offset of last newline character seen
                 current = 0; // current character
        for (size_t i = 0; i < count; ++i)
        {
            sourcekitd_variant_t const token = sourcekit::variant_array_get_value(syntax, i);
            uint64_t const start = sourcekit::variant_dictionary_get_int64(token, uids.key_offset),
                           count = sourcekit::variant_dictionary_get_int64(token, uids.key_length);
            
            // convert 1D indices to 2D row–column indices 
            while (current < start) 
            {
                if (source[current++] == '\n')
                {
                    ++row;
                    lastnl = current;
                }
            }
            
            tokens[i].start = {row, static_cast<uint16_t>(start - lastnl)};
            
            while (current < start + count) 
            {
                if (source[current++] == '\n')
                {
                    ++row;
                    lastnl = current;
                }
            }
            
            tokens[i].end     = {row, static_cast<uint16_t>(start + count - lastnl)};
            tokens[i].species = species[sourcekit::variant_dictionary_get_uid(token, uids.key_kind)];
        }
        
        sourcekit::response_dispose(response);
        // crashes happen when we try to use NewBuffer ???
        frame.GetReturnValue().Set(Nan::CopyBuffer((char*) tokens, count * sizeof(token_t)).ToLocalChecked());
        delete[] tokens;
    }
    
    void deinitialize(v8::FunctionCallbackInfo<v8::Value> const&)
    {
        sourcekit::request_release(request);
        sourcekit::shutdown();
    }
}

void Init(v8::Local<v8::Object> exports)
{
    NODE_SET_METHOD(exports, "initialize", blonde::initialize);
    NODE_SET_METHOD(exports, "highlight", blonde::highlight);
    NODE_SET_METHOD(exports, "deinitialize", blonde::deinitialize);
}

NODE_MODULE(blonde, Init)
