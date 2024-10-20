/**
 * Convert YAML to JSON with Wasm
 *
 * Copyright 2024, Levente Hunyadi
 *
 * @see https://github.com/hunyadi/yaml-to-json
**/

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
    longjmp(parse_error_handler, 1);
}

extern "C"
{
    /** Converts a YAML string into a JSON string. */
    String* transform_yaml(String* in_str);
}

/** Converts a YAML string into a JSON string. */
String* transform_yaml(String* in_str)
{
    char* s = in_str->data();

    // skip start of document marker
    if (in_str->size() > 3 && s[0] == '-' && s[1] == '-' && s[2] == '-') {
        s += 3;
    }

    if (setjmp(parse_error_handler)) {
        return nullptr;
    }

    // parse YAML
    ryml::Tree tree = ryml::parse_in_place(s);

    // emit JSON
    std::string json = ryml::emitrs_json<std::string>(tree);

    // check if string is valid UTF-8
    std::size_t pos;
    if (!utf8::is_valid(json, pos)) {
        return nullptr;
    }

    // construct result
    return new String(json.data(), json.size());
}

int main(int argc, const char* argv[])
{
    ryml::set_callbacks(ryml::Callbacks(nullptr, &parser_allocate, &parser_free, &parser_raise));
    return 0;
}
