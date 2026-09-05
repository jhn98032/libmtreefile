#ifndef FILE__FILE_H
#define FILE__FILE_H


#include <QString>
#include <QVector>



namespace mtreefile
{




class GenericFile
{
public:
    GenericFile() {};
    virtual ~GenericFile() {};


    virtual int read(void *buff, int len) = 0;
    virtual int write(const void *buff, int len) = 0;

    virtual off_t currentPos() = 0;
    virtual int seekTo(off_t pos) = 0;

};


class PhysicalFile : public GenericFile
{
public:
    PhysicalFile();
    virtual ~PhysicalFile() ;

    int open(QString filename, bool readOnly = true);
    int openReadOnly(QString filename) { return open(filename, true); };

    int read(void *buff, int len);
    int write(const void *buff, int len);

    off_t currentPos();
    int seekTo(off_t pos);

public:
    int m_fd;
};


class BufferFile : public GenericFile
{
public:
    BufferFile();
    virtual ~BufferFile();

    int open(QVector <uint8_t> buffer);

    int read(void *buff, int len);
    int write(const void *buff, int len);

    off_t currentPos();
    int seekTo(off_t pos);

public:
    QVector <uint8_t> m_buffer;
    off_t m_pos;
};


}; // namespace mtreefile


#endif // FILE__FILE_H

