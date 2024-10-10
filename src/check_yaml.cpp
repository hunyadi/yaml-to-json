#include "ryml_all.hpp"
#include "string.hpp"
#include "utf8.hpp"
#include <csetjmp>

static std::jmp_buf parse_error_handler;
static std::string error_message;

static void* parser_allocate(size_t len, void* hint, void* user_data)
{
    return std::malloc(len);
}

static void parser_free(void* mem, size_t size, void* user_data)
{
    std::free(mem);
}

static void parser_raise(const char* msg, size_t len, ryml::Location location, void* user_data)
{
    constexpr const char* fmt = "%s in YAML at line %zu column %zu offset %zu";
    int count = std::snprintf(nullptr, 0, fmt, msg, location.line, location.col, location.offset);
    if (count >= 0) {
        error_message.resize(count + 1);
        std::snprintf(error_message.data(), error_message.size(), fmt, msg, location.line, location.col, location.offset);
    } else {
        error_message.clear();
    }

    longjmp(parse_error_handler, 1);
}

extern "C"
{
    /** Checks whether a YAML string represents a valid YAML document. */
    String* check_yaml(String* in_str);
}

/** Checks whether a YAML string represents a valid YAML document. */
String* check_yaml(String* in_str)
{
    char* s = in_str->data();

    // skip start of document marker
    if (in_str->size() > 3 && s[0] == '-' && s[1] == '-' && s[2] == '-') {
        s += 3;
    }

    if (setjmp(parse_error_handler)) {
        return new String(error_message.data(), error_message.size());
    }

    // parse YAML
    ryml::Tree tree = ryml::parse_in_place(s);

    // emit JSON
    std::string json = ryml::emitrs_json<std::string>(tree);

    // check if string is valid UTF-8
    std::size_t pos;
    if (!utf8::is_valid(json, pos)) {
        std::string msg;
        constexpr const char* fmt = "invalid UTF-8 character in JSON at offset %zu";
        int count = std::snprintf(nullptr, 0, fmt, pos);
        if (count >= 0) {
            msg.resize(count + 1);
            std::snprintf(msg.data(), msg.size(), fmt, pos);
        }
        return new String(msg.data(), msg.size());
    }

    return nullptr;
}

int main(int argc, const char* argv[])
{
    ryml::set_callbacks(ryml::Callbacks(nullptr, &parser_allocate, &parser_free, &parser_raise));
    return 0;
}
