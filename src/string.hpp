#pragma once
#include <cstddef>
#include <cstring>

/** A fixed-length string that facilitates data interchange over a C interface. */
struct String
{
    /** Creates a string with the given content. */
    String(const char* data)
        : _length(std::strlen(data))
    {
        _chars = new char[_length + 1];
        set(data, _length);
    }

    /** Creates a string with the given content. */
    String(const char* data, std::size_t length)
        : _length(length)
    {
        _chars = new char[length + 1];
        set(data, length);
    }

    /** Creates a string with uninitialized content. */
    explicit String(std::size_t length)
        : _length(length)
    {
        _chars = new char[length + 1];
        _chars[length] = 0;
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
        set(data, size < _length ? size : _length);
    }

private:
    /** Populates the string with data. */
    void set(const char* data, std::size_t size)
    {
        memcpy(_chars, data, size);
        _chars[size] = 0;  // NUL terminator
    }

private:
    char* _chars;
    std::size_t _length;
};
