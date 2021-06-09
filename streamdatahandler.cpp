#include "streamdatahandler.h"

CircularBuffer::CircularBuffer()
{

}

int CircularBuffer::initiateBuffer(int size)
{
    if (this->maxSize > 0)
    {
        return 1;
    }

    this->buffer = (int16*)malloc(sizeof(int16)*size);
    this->maxSize = size;
    return 0;
}

int CircularBuffer::addBuffer(int16 *pData, int size)
{
    // I wonder if memcpy is gonna be faster...
    for (int i = 0; i < size; i++)
    {
        buffer[(writerPointer+i) % maxSize] = pData[i];
    }
    writerPointer += size;
    if (writerPointer >= maxSize * 2) writerPointer -= maxSize;
    return 0;
}

int CircularBuffer::getBuffer(int16 *pData, int size)
{
    // Give Error. If size > maxSize
    if (size <= 0 || size > writerPointer) return 1;

    if (writerPointer < maxSize)
    {
        for (int i = 0; i < size; i++)
        {
             pData[i] = buffer[i];
        }
    }
    else
    {
        for (int i = 0; i < size; i++)
        {
             pData[i] = buffer[(writerPointer+i) % maxSize];
        }
    }
    return 0;
}

StreamDataHandler::StreamDataHandler()
{

}
