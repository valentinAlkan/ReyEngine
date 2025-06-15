#include "CircBuf.h"
    
using namespace std;

///////////////////////////////////////////////////////////////////////////////
CircularBuffer::CircularBuffer(size_t size)
: _size(size)
{
    memory = std::make_unique<char[]>(size);
}

///////////////////////////////////////////////////////////////////////////////
CircularBuffer::~CircularBuffer(){}

///////////////////////////////////////////////////////////////////////////////
size_t CircularBuffer::available(){
    std::scoped_lock<std::mutex> lock(mtex);
    if (overflow){overflow = false; return -1;}
    if (writePointer > readPointer) return writePointer - readPointer;
    if (writePointer < readPointer) return _size - readPointer + writePointer;
    return 0;
}

///////////////////////////////////////////////////////////////////////////////
bool CircularBuffer::hasOverflowed() const {
    return overflow;
}

///////////////////////////////////////////////////////////////////////////////
void CircularBuffer::clearOverFlow(){
    std::scoped_lock<std::mutex> sl(std::mutex);
    overflow = false;
}

///////////////////////////////////////////////////////////////////////////////
char CircularBuffer::peek(size_t at) const {
//    std::scoped_lock<mutex> lock(mtex);
    //read the value at readpointer + i and wrap around if it exceeds
    // the end barrier
    size_t address = readPointer + at;
    address %= _size; //wrap around
    return memory[address];
}

///////////////////////////////////////////////////////////////////////////////
size_t CircularBuffer::read(char* dst, size_t maxBytes){
    std::scoped_lock<std::mutex> lock(mtex);
    int available = writePointer - readPointer;
    size_t toRead;
    size_t bytesRead;
    if (available > 0){
        //buffer hasn't overflowed - do a normal read
        toRead = maxBytes > available ? available : maxBytes;
        memcpy(dst, &memory[readPointer], toRead);
        bytesRead = toRead;
    }
    else if (available < 0){
        //buffer has overflowed - do a wrapped read
        size_t tailSize = _size - readPointer;
        size_t headSize = writePointer;
        toRead = maxBytes > tailSize ? tailSize : maxBytes;
        memcpy(dst, &memory[readPointer], toRead);
        bytesRead = toRead;
        //start again at 0
        size_t spaceLeft = maxBytes - bytesRead;
        toRead = spaceLeft > headSize ?  headSize : spaceLeft;
        memcpy(&dst[bytesRead], memory.get(), toRead);
        bytesRead += toRead;
    } else {
        return 0;
    }
    readPointer += bytesRead;
    readPointer %= _size;
    // printf("cbufR = %d\n", readPointer);
    return bytesRead;
}

///////////////////////////////////////////////////////////////////////////////
size_t CircularBuffer::write(const char* src, size_t nBytes){
    if (nBytes <= 0) return 0;
    std::scoped_lock<std::mutex> lock(mtex);
    if (writePointer + nBytes < _size){
        //normal write
        memcpy(&memory[writePointer], src, nBytes);
        writePointer += nBytes;
    } else {
        //split write
        if (nBytes > _size){
            //message too big; truncate it
            memcpy(memory.get(), src, _size);
            writePointer = 0; //write pointer is 0 if buffer totally full
            printf("Circular buffer overflow, discarding %d bytes of data.\n", (unsigned int)(nBytes - _size));
            return -_size;
        } else {
            //buffer is large enough to hold entire message
            int bytesToWrite = _size - writePointer;
            memcpy(&(memory[writePointer]), src, bytesToWrite);
            int bytesLeft = nBytes - bytesToWrite;
            if (bytesLeft) {
                memcpy(memory.get(), &src[bytesToWrite], bytesLeft);
            }
            writePointer = bytesLeft;
        }
    }
    return nBytes;
}

/////////////////////////////////////////////////////////////////////////////////
size_t CircularBuffer::write(std::string s, size_t nBytes) {
    if(nBytes==-1){
        //write all
        return write(s.data(), s.size());
    }
    return write(s.data(), nBytes);
}

/////////////////////////////////////////////////////////////////////////////////
size_t CircularBuffer::write(char c) {
    return write(&c, 1);
}

/////////////////////////////////////////////////////////////////////////////////
std::optional<char> CircularBuffer::next() {
    if (available() > 0){
        char c;
        read(&c, 1);
        return c;
    }
    return std::nullopt;
}