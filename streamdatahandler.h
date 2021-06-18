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
