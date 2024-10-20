/**
 * UTF-8 string validation
 *
 * @see https://github.com/c-util/c-utf8
**/

#pragma once
#include <string>

namespace utf8
{
    namespace detail
    {
        void utf8_verify_ascii(const char*& strp, std::size_t& lenp);
        void utf8_verify(const char*& strp, std::size_t& lenp);
    }

    /**
     * Checks if a UTF-8 string is valid.
     * @returns True if the string is valid UTF-8.
     */
    inline bool is_valid(const char* str, std::size_t len, std::size_t& pos)
    {
        const char* ptr = str;
        std::size_t cnt = len;
        detail::utf8_verify(ptr, cnt);
        if (cnt > 0) {
            pos = ptr - str;
            return false;
        } else {
            return true;
        }
    }

    /**
     * Checks if a UTF-8 string is valid.
     * @returns True if the string is valid UTF-8.
     */
    inline bool is_valid(const std::string& str, std::size_t& pos)
    {
        return is_valid(str.data(), str.size(), pos);
    }
}
