#include "string.hpp"

extern "C"
{
    String* string_create(std::size_t length);

    void string_delete(String* str);

    char* string_data(String* str);

    std::size_t string_length(String* str);
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
