/**
 * Flexible and Economical UTF-8 Decoder
 * @see https://bjoern.hoehrmann.de/utf-8/decoder/dfa/
 */

#pragma once
#include <cstdint>
#include <cstddef>
#include <string>

namespace utf8
{
    namespace detail
    {
        static constexpr std::uint8_t utf8d[] = {
            0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, // 00..1f
            0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, // 20..3f
            0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, // 40..5f
            0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, // 60..7f
            1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9, // 80..9f
            7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7, // a0..bf
            8,8,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2, // c0..df
            0xa,0x3,0x3,0x3,0x3,0x3,0x3,0x3,0x3,0x3,0x3,0x3,0x3,0x4,0x3,0x3, // e0..ef
            0xb,0x6,0x6,0x6,0x5,0x8,0x8,0x8,0x8,0x8,0x8,0x8,0x8,0x8,0x8,0x8, // f0..ff
            0x0,0x1,0x2,0x3,0x5,0x8,0x7,0x1,0x1,0x1,0x4,0x6,0x1,0x1,0x1,0x1, // s0..s0
            1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,1,1,1,1,1,0,1,0,1,1,1,1,1,1, // s1..s2
            1,2,1,1,1,1,1,2,1,2,1,1,1,1,1,1,1,1,1,1,1,1,1,2,1,1,1,1,1,1,1,1, // s3..s4
            1,2,1,1,1,1,1,1,1,2,1,1,1,1,1,1,1,1,1,1,1,1,1,3,1,3,1,1,1,1,1,1, // s5..s6
            1,3,1,1,1,1,1,3,1,3,1,1,1,1,1,1,1,3,1,1,1,1,1,1,1,1,1,1,1,1,1,1, // s7..s8
        };

        /**
         * Decodes a byte, given the existing state and code point.
         * @returns True if a code point has been fully decoded.
         */
        inline bool decode(std::uint32_t& state, std::uint32_t& codepoint, std::uint8_t byte)
        {
            std::uint32_t type = utf8d[byte];
            codepoint = (state != 0) ? (byte & 0x3fu) | (codepoint << 6) : (0xff >> type) & byte;
            state = utf8d[256 + state * 16 + type];
            return state == 0;
        }

        /**
         * The hexadecimal digit equivalent of a value between 0 and 15.
         */
        inline char hex_digit(unsigned int value)
        {
            return value >= 10 ? 'A' + (value - 10) : '0' + value;
        }
    }

    /**
     * Checks if a UTF-8 string is valid.
     * @returns True if the string is valid UTF-8.
     */
    inline bool is_valid(const char* s)
    {
        std::uint32_t codepoint;
        std::uint32_t state = 0;

        for (; *s; ++s) {
            detail::decode(state, codepoint, static_cast<std::uint8_t>(*s));
        }

        return state == 0;  // true if the string has been well-formed
    }

    /**
     * Counts the number of code points in a UTF-8 string.
     * @returns True if the string is valid UTF-8.
     */
    inline bool code_point_count(const char* s, std::size_t& count)
    {
        std::uint32_t codepoint;
        std::uint32_t state = 0;

        for (count = 0; *s; ++s) {
            if (detail::decode(state, codepoint, static_cast<std::uint8_t>(*s))) {
                count += 1;
            }
        }

        return state == 0;  // true if the string has been well-formed
    }

    /**
     * Translates non-ASCII characters in a UTF-8 string into their JSON escape sequences.
     * @returns True if the string is valid UTF-8.
     */
    inline bool escape(const std::string& str, std::string& out)
    {
        out.reserve(str.size());

        std::uint32_t codepoint;
        std::uint32_t state = 0;

        const char* s = str.data();
        for (; *s; ++s) {
            if (detail::decode(state, codepoint, static_cast<std::uint8_t>(*s))) {
                if (codepoint < 128) {
                    out.push_back(static_cast<char>(codepoint));
                } else {
                    using detail::hex_digit;
                    out.push_back('\\');
                    out.push_back('u');
                    out.push_back(hex_digit(static_cast<char>((codepoint >> 12) & 0xf)));
                    out.push_back(hex_digit(static_cast<char>((codepoint >> 8) & 0xf)));
                    out.push_back(hex_digit(static_cast<char>((codepoint >> 4) & 0xf)));
                    out.push_back(hex_digit(static_cast<char>(codepoint & 0xf)));
                }
            }
        }

        return state == 0;  // true if the string has been well-formed
    }
}
