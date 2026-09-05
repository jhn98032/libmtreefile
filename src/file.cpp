/*
 * libmtreefile - 
 * Copyright (C) 2026 Johan Henriksson johan[a]dexar.se
 * All rights reserved.
 *
 * This software may be modified and distributed under the terms
 * of the BSD license.  See the LICENSE file for details.
 */

#include "file.h"

#include <unistd.h>
#include <fcntl.h>


using namespace mtreefile;



PhysicalFile::PhysicalFile()
    : m_fd(0)
{
}
PhysicalFile::~PhysicalFile()
{
    if(m_fd)
        ::close(m_fd);
}




int PhysicalFile::open(QString filename, bool readOnly)
{
    if(m_fd)
        close(m_fd);

    if(readOnly)
        m_fd = ::open(qPrintable(filename), O_RDONLY);
    else
        m_fd= ::open(qPrintable(filename), O_RDWR | O_CREAT);
    if (m_fd < 0)
    {
        perror("Error opening file");
        return -1;
    }
    return 0;
}


int PhysicalFile::read(void *buff, int len)
{
    ssize_t bytesWritten = ::read(m_fd, buff, len);
    return bytesWritten;
}

int PhysicalFile::write(const void *buff, int len)
{
    ssize_t bytesWritten = ::write(m_fd,buff, len);
    return bytesWritten;
}

/**
 * @brief Returns the current file position from start of file.
 * @return Pos or -1 if an error happened.
 */
off_t PhysicalFile::currentPos()
{
    off_t endPos = ::lseek(m_fd, 0, SEEK_CUR);
    return endPos;
}

/**
 * @brief Seeks to a position
 * @param pos  The position from start of file
 * @return New pos or -1 if an error happened.
 */
int PhysicalFile::seekTo(off_t pos)
{
    off_t endPos = ::lseek(m_fd, pos, SEEK_SET);
    return endPos;

}



BufferFile::BufferFile()
    : m_pos(0)
{
}

BufferFile::~BufferFile()
{
}

int BufferFile::open(QVector <uint8_t> buffer)
{
    m_buffer = buffer;
    return 0;
}


int BufferFile::read(void *buff, int len)
{
    memcpy(buff, m_buffer.data()+m_pos, len);
    m_pos += len;
    return len;
}

int BufferFile::write(const void *buff, int len)
{
    if(m_buffer.size() < len+m_pos)
        m_buffer.resize(len+m_pos);
    memcpy(m_buffer.data()+m_pos, buff, len);
    m_pos += len;
    return len;
}


off_t BufferFile::currentPos()
{
    return m_pos;
}
int BufferFile::seekTo(off_t pos)
{
    assert(pos <= m_buffer.size());
    m_pos = pos;
    return pos;
}


