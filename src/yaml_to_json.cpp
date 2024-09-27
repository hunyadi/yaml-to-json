#include "ryml_all.hpp"
#include "string.hpp"
#include "utf8.hpp"
#include <csetjmp>

static std::jmp_buf parse_error_handler;

static void* parser_allocate(size_t len, void* hint, void* user_data)
{
    return std::malloc(len);
}

static void parser_free(void* mem, size_t size, void* user_data)
{
    std::free(mem);
}

static void parser_raise(const char* msg, size_t msg_len, ryml::Location location, void* user_data)
{
    longjmp(parse_error_handler, 0);
}

extern "C"
{
    String* string_create(std::size_t length);

    void string_delete(String* str);

    char* string_data(String* str);

    std::size_t string_length(String* str);

    /** An identity transform for testing purposes. */
    String* identity(String* in_str);

    /** Converts a YAML string into a JSON string. */
    String* transform(String* in_str);
}

String* string_create(std::size_t length)
{
    return new String(length);
}

void string_delete(String* str)
{
    delete str;
}

char* string_data(String* str)
{
    return str->data();
}

std::size_t string_length(String* str)
{
    return str->size();
}

/** An identity transform for testing purposes. */
String* identity(String* in_str)
{
    String* out_str = new String(in_str->size());
    out_str->assign(in_str->data(), in_str->size());
    return out_str;
}

/** Converts a YAML string into a JSON string. */
String* transform(String* in_str)
{
    char* s = in_str->data();

    // skip start of document marker
    if (in_str->size() > 3 && s[0] == '-' && s[1] == '-' && s[2] == '-') {
        s += 3;
    }

    if (setjmp(parse_error_handler))
    {
        return nullptr;
    }

    // parse YAML
    ryml::Tree tree = ryml::parse_in_place(s);

    // emit JSON
    std::string json = ryml::emitrs_json<std::string>(tree);

    // check if string is valid UTF-8
    if (!utf8::is_valid(json)) {
        return nullptr;
    }

    // construct result
    String* out_str = new String(json.size());
    out_str->assign(json.data(), json.size());
    return out_str;
}

int main(int argc, const char* argv[])
{
    ryml::set_callbacks(ryml::Callbacks(nullptr, &parser_allocate, &parser_free, &parser_raise));
    return 0;
}
