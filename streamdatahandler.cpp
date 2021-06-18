/*******************************************************************************
Copyright (c) 2021, Jackson Cagle, University of Floria

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <https://www.gnu.org/licenses/>.
*********************************************************************************/

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
