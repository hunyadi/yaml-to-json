/**
 * UTF-8 string validation
 *
 * @see https://github.com/c-util/c-utf8
**/

#include "utf8.hpp"
#include <cstddef>
#include <cstdlib>
#include <cstdint>

/* The following constants are truncated on 32-bit machines */
constexpr std::size_t C_UTF8_ASCII_MASK = static_cast<std::size_t>(UINT64_C(0x8080808080808080));
constexpr std::size_t C_UTF8_ASCII_SUB = static_cast<std::size_t>(UINT64_C(0x0101010101010101));

static inline const char* c_align_to(const char* _val, std::size_t _to) {
    return reinterpret_cast<const char*>((reinterpret_cast<std::intptr_t>(_val) + _to - 1) & ~(_to - 1));
}

static inline uint8_t c_load_8(const char* memory, std::size_t offset) {
    return reinterpret_cast<const uint8_t*>(memory)[offset];
}

static inline uint64_t c_load_64le_aligned(const void* memory, std::size_t offset) {
    const uint8_t* m = reinterpret_cast<const uint8_t*>(__builtin_assume_aligned(reinterpret_cast<const uint8_t*>(memory) + offset, 8, 0));
    return
        ((uint64_t)m[0] << 0) | ((uint64_t)m[1] << 8) |
        ((uint64_t)m[2] << 16) | ((uint64_t)m[3] << 24) |
        ((uint64_t)m[4] << 32) | ((uint64_t)m[5] << 40) |
        ((uint64_t)m[6] << 48) | ((uint64_t)m[7] << 56);
}

static inline int utf8_word_is_ascii(std::size_t word) {
    /* True unless any byte is NULL or has the MSB set. */
    return ((((word - C_UTF8_ASCII_SUB) | word) & C_UTF8_ASCII_MASK) == 0);
}

/**
 * Verifies that a string is ASCII encoded.
 *
 * @param strp Reference to string to verify.
 * @param lenp Reference to length of string.
 *
 * Up to the first `lenp` bytes of the string pointed to by `strp` is
 * verified to be ASCII encoded, and `strp` and `lenp` are updated to
 * point to the first non-ASCII character or the first NUL byte of the
 * string, and the remaining number of bytes of the string,
 * respectively.
 */
void utf8::detail::utf8_verify_ascii(const char*& strp, std::size_t& lenp) {
    const char* str = strp;
    std::size_t len = lenp;

    while (len > 0 && c_load_8(str, 0) < 128) {
        if (c_align_to(str, sizeof(std::size_t)) == str) {
            // if the string is aligned to a word boundary, scan two words at a time for NUL or non-ASCII characters;
            // we do two words at a time to take advantage of the compiler being able to use SIMD instructions
            // where available
            while (len >= 2 * sizeof(std::size_t)) {
                if (!utf8_word_is_ascii(c_load_64le_aligned(str, 0)) || !utf8_word_is_ascii(c_load_64le_aligned(str, sizeof(std::size_t)))) {
                    break;
                }

                str += 2 * sizeof(std::size_t);
                len -= 2 * sizeof(std::size_t);
            }


            // find the actual end of the ASCII-portion of the string
            while (len > 0 && c_load_8(str, 0) < 128) {
                if (c_load_8(str, 0) == 0x00) [[unlikely]] {
                    goto out;
                }
                ++str;
                --len;
            }
        } else {
            // the string was not aligned, scan one character at a time until it is
            if (c_load_8(str, 0) == 0x00) [[unlikely]] {
                goto out;
            }
            ++str;
            --len;
        }
    }

out:
    strp = str;
    if (lenp) {
        lenp = len;
    }
}

#define C_UTF8_CHAR_IS_TAIL(_x) (((_x) & 0xC0) == 0x80)

/**
 * Verifies that a string is UTF-8 encoded.
 *
 * @param strp Reference to string to verify.
 * @param lenp Reference to length of string.
 *
 * Up to the first `lenp` bytes of the string pointed to by `strp` is
 * verified to be UTF-8 encoded, and `strp` and `lenp` are updated to
 * point to the first non-UTF-8 character or the first NUL byte of the
 * string, and the remaining number of bytes of the string,
 * respectively.
 */
void utf8::detail::utf8_verify(const char*& strp, std::size_t& lenp) {
    const char* str = strp;
    std::size_t len = lenp;

    // See Unicode 10.0.0, Chapter 3, Section D92
    while (len > 0) {
        switch (c_load_8(str, 0)) {
        case 0x00:
            goto out;
        case 0x01 ... 0x7F:
            // special case and optimize the ASCII case
            utf8_verify_ascii(str, len);

            break;
        case 0xC2 ... 0xDF:
            if (len < 2) [[unlikely]] {
                goto out;
            }
            if (!C_UTF8_CHAR_IS_TAIL(c_load_8(str, 1))) [[unlikely]] {
                goto out;
            }

            str += 2;
            len -= 2;

            break;
        case 0xE0:
            if (len < 3) [[unlikely]] {
                goto out;
            }
            if (c_load_8(str, 1) < 0xA0 || c_load_8(str, 1) > 0xBF) [[unlikely]] {
                goto out;
            }
            if (!C_UTF8_CHAR_IS_TAIL(c_load_8(str, 2))) [[unlikely]] {
                goto out;
            }

            str += 3;
            len -= 3;

            break;
        case 0xE1 ... 0xEC:
            if (len < 3) [[unlikely]] {
                goto out;
            }
            if (!C_UTF8_CHAR_IS_TAIL(c_load_8(str, 1))) [[unlikely]] {
                goto out;
            }
            if (!C_UTF8_CHAR_IS_TAIL(c_load_8(str, 2))) [[unlikely]] {
                goto out;
            }

            str += 3;
            len -= 3;

            break;
        case 0xED:
            if (len < 3) [[unlikely]] {
                goto out;
            }
            if (c_load_8(str, 1) < 0x80 || c_load_8(str, 1) > 0x9F) [[unlikely]] {
                goto out;
            }
            if (!C_UTF8_CHAR_IS_TAIL(c_load_8(str, 2))) [[unlikely]] {
                goto out;
            }

            str += 3;
            len -= 3;

            break;
        case 0xEE ... 0xEF:
            if (len < 3) [[unlikely]] {
                goto out;
            }
            if (!C_UTF8_CHAR_IS_TAIL(c_load_8(str, 1))) [[unlikely]] {
                goto out;
            }
            if (!C_UTF8_CHAR_IS_TAIL(c_load_8(str, 2))) [[unlikely]] {
                goto out;
            }

            str += 3;
            len -= 3;

            break;
        case 0xF0:
            if (len < 4) [[unlikely]] {
                goto out;
            }
            if (c_load_8(str, 1) < 0x90 || c_load_8(str, 1) > 0xBF) [[unlikely]] {
                goto out;
            }
            if (!C_UTF8_CHAR_IS_TAIL(c_load_8(str, 2))) [[unlikely]] {
                goto out;
            }
            if (!C_UTF8_CHAR_IS_TAIL(c_load_8(str, 3))) [[unlikely]] {
                goto out;
            }

            str += 4;
            len -= 4;

            break;
        case 0xF1 ... 0xF3:
            if (len < 4) [[unlikely]] {
                goto out;
            }
            if (!C_UTF8_CHAR_IS_TAIL(c_load_8(str, 1))) [[unlikely]] {
                goto out;
            }
            if (!C_UTF8_CHAR_IS_TAIL(c_load_8(str, 2))) [[unlikely]] {
                goto out;
            }
            if (!C_UTF8_CHAR_IS_TAIL(c_load_8(str, 3))) [[unlikely]] {
                goto out;
            }

            str += 4;
            len -= 4;

            break;
        case 0xF4:
            if (len < 4) [[unlikely]] {
                goto out;
            }
            if (c_load_8(str, 1) < 0x80 || c_load_8(str, 1) > 0x8F) [[unlikely]] {
                goto out;
            }
            if (!C_UTF8_CHAR_IS_TAIL(c_load_8(str, 2))) [[unlikely]] {
                goto out;
            }
            if (!C_UTF8_CHAR_IS_TAIL(c_load_8(str, 3))) [[unlikely]] {
                goto out;
            }

            str += 4;
            len -= 4;

            break;
        default:
            goto out;
        }
    }

out:
    strp = str;
    if (lenp) {
        lenp = len;
    }
}
