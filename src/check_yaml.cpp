#include "ryml_all.hpp"
#include "string.hpp"
#include "utf8.hpp"
#include <csetjmp>

static std::jmp_buf parse_error_handler;
static std::string message;

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
    constexpr const char* fmt = "%s at line %zu column %zu";
    int count = std::snprintf(nullptr, 0, fmt, msg, location.line, location.col);
    if (count > 0) {
        message.resize(count + 1);
        std::snprintf(message.data(), message.size(), fmt, msg, location.line, location.col);
    } else {
        message.clear();
    }

    longjmp(parse_error_handler, 1);
}

extern "C"
{
    /** Checks whether a YAML string represents a valid YAML document. */
    String* check_yaml(String* in_str);
}

static bool visit_node(ryml::Tree& tree, ryml::id_type id)
{
    if (tree.is_keyval(id)) {
        if (!utf8::is_valid(tree.key(id).data())) {
            return false;
        }
        if (!utf8::is_valid(tree.val(id).data())) {
            return false;
        }
    } else if (tree.is_val(id)) {
        if (!utf8::is_valid(tree.val(id).data())) {
            return false;
        }
    } else if (tree.is_container(id)) {
        if (tree.has_key(id)) {
            if (!utf8::is_valid(tree.key(id).data())) {
                return false;
            }
        }
    }
    for (ryml::id_type ich = tree.first_child(id); ich != ryml::NONE; ich = tree.next_sibling(ich)) {
        if (!visit_node(tree, ich)) {
            return false;
        }
    }
    return true;
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
        return new String(message.data(), message.size());
    }

    // parse YAML
    ryml::Tree tree = ryml::parse_in_place(s);

    // emit JSON
    std::string json = ryml::emitrs_json<std::string>(tree);

    // check if string is valid UTF-8
    if (!utf8::is_valid(json)) {
        return new String("invalid UTF-8 sequence in YAML");
    }

    return nullptr;
}

int main(int argc, const char* argv[])
{
    ryml::set_callbacks(ryml::Callbacks(nullptr, &parser_allocate, &parser_free, &parser_raise));
    return 0;
}
