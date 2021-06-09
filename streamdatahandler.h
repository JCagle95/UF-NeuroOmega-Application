#ifndef STREAMDATAHANDLER_H
#define STREAMDATAHANDLER_H

#include "AOTypes.h"

class CircularBuffer
{
public:
    CircularBuffer();
    int initiateBuffer(int size);
    int addBuffer(int16 *pData, int size);
    int getBuffer(int16 *pData, int size);

private:
    int16 *buffer;

    int readerPointer = 0;
    int writerPointer = 0;
    int maxSize = 0;
};


class StreamDataHandler
{
public:
    StreamDataHandler();
};

#endif // STREAMDATAHANDLER_H
