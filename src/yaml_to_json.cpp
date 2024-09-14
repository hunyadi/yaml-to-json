#include "ryml_all.hpp"
#include "string.hpp"

extern "C"
{
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
        if (s[0] == '-' && s[1] == '-' && s[2] == '-') {
            s += 3;
        }

        // parse YAML
        ryml::Tree tree = ryml::parse_in_place(s);

        // emit JSON
        auto json = ryml::emitrs_json<std::string>(tree);

        // construct result
        String* out_str = new String(json.size());
        out_str->assign(json.data(), json.size());
        return out_str;
    }
}
