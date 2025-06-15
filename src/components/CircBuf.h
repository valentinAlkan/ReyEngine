#pragma once
#include <stdlib.h>
#include <cstring>
#include <cstdio>
#include <mutex>
#include <memory>
#include <optional>

struct CircularBuffer{
    CircularBuffer(size_t size);
    ~CircularBuffer();
    size_t read(char* dst, size_t maxBytes);
    std::optional<char> next();
    size_t write(const char* src, size_t nBytes); //write data into this circbuf
    size_t write(std::string s, size_t nBytes=-1); //write a string into this circbuf
    size_t write(char c); //write a single char into this circbuf
    char peek(size_t at) const; //should always lock if you're gonna use
    size_t available();
    size_t spaceLeft();
    bool hasOverflowed() const;
    void clearOverFlow();
    size_t size(){return _size;}

private:
    std::unique_ptr<char[]> memory;
    size_t _size;
    size_t writePointer = 0;
    size_t readPointer = 0;
    std::mutex mtex;
    bool overflow = false;
    
};