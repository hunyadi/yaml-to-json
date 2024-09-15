#pragma once
#include <cstddef>

/** A fixed-length string that facilitates data interchange over a C interface. */
struct String
{
    /** Creates a string with uninitialized content. */
    String(std::size_t length)
    {
        _chars = new char[length + 1];
        _chars[length] = 0;
        _length = length;
    }

    /** Deallocates a string. */
    ~String()
    {
        delete[] _chars;
    }

    /** Points to the internal character array. */
    const char* data() const
    {
        return _chars;
    }

    /** Points to the internal character array. */
    char* data()
    {
        return _chars;
    }

    /** Returns a character in the string. */
    char operator[](std::size_t index) const
    {
        return _chars[index];
    }

    /** Accesses a character in the string. */
    char& operator[](std::size_t index)
    {
        return _chars[index];
    }

    /** Returns the length of the string. */
    std::size_t size() const
    {
        return _length;
    }

    /** Populates the string with data. */
    void assign(const char* data, std::size_t size)
    {
        memcpy(_chars, data, std::min(_length, size));
        _chars[size] = 0;  // NUL terminator
    }

private:
    char* _chars;
    std::size_t _length;
};
