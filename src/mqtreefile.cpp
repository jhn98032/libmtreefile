#include "mqtreefile.h"

#include <QString>

#include <assert.h>
#include <unistd.h>
#include <stdint.h>
#include <fcntl.h>

#include <stdexcept>
#include <algorithm>
#include <memory>

#include "zipfile.h"
#include "version.h"

using namespace std;
using namespace mtreefile;





struct Header
{
    char magic[4]; // "TREE"
    uint32_t version; // 1
    unsigned char unused[64-8];
};

#define HEADER_MAGIC  "TREE"


MQTreeFileNode::MQTreeFileNode(QString name)
    : m_name(name)
{

}

MQTreeFileNode::~MQTreeFileNode()
{

}


MQTreeFileDirNode::MQTreeFileDirNode(QString name)
    : MQTreeFileNode(name)

{

}

MQTreeFileDirNode::~MQTreeFileDirNode()
{
    for(MQTreeFileNode*child :  m_children)
    {
        delete child;
    }

}



MQTreeFileEntryNode::MQTreeFileEntryNode(QString name)
    : MQTreeFileNode(name)
    ,m_valueS8(0)
    ,m_valueDbl(0.0)
    ,m_valueS32(0)
    ,m_valueArray(nullptr)
    ,m_valueArrayLen(0)
{
    m_type = Type::TYPE_S8;
}


MQTreeFileEntryNode::~MQTreeFileEntryNode()
{
    delete [] m_valueArray;

}


void MQTreeFileDirNode::removeChild(MQTreeFileEntryNode* child)
{
    auto it = std::find(m_children.begin(), m_children.end(), child);
    if (it != m_children.end())
    {
        m_children.erase(it);
        delete child;
    }

}


MQTreeFileEntryNode* MQTreeFileDirNode::addChildEntry(QString name)
{
    for(MQTreeFileNode *node : m_children)
    {
        if(!node->isDir())
        {
        if(node->getName() == name)
            return node->asEntry();
        }
    }
    MQTreeFileEntryNode *node = new MQTreeFileEntryNode(name);
    m_children.push_back(node);
    return node;
}

MQTreeFileDirNode* MQTreeFileDirNode::addChildDir(QString name)
{
    for(MQTreeFileNode *node : m_children)
    {
        if(node->isDir())
        {
        if(node->getName() == name)
            return node->asDir();
        }
    }
    MQTreeFileDirNode *node = new MQTreeFileDirNode(name);
    m_children.push_back(node);
    return node;
}



MQTreeFileEntryNode* MQTreeFileDirNode::findChildEntry(QString name)
{
    if(name.isEmpty())
        return nullptr;
    for(MQTreeFileNode *node : m_children)
    {
        if(!node->isDir())
        {
            if(node->getName() == name)
                return node->asEntry();
        }
    }
    // @TODO Parameter that starts with "." are system params
    // Some have been renamed so to handle backward compability older names
    // without "." is checked.
    if(name.startsWith("."))
        return findChildEntry(name.mid(1));

    return nullptr;
}


MQTreeFileDirNode* MQTreeFileDirNode::findChildDir(QString name)
{
    for(MQTreeFileNode *node : m_children)
    {
        if(node->isDir())
        {
            if(node->getName() == name)
                return node->asDir();
        }
    }
    return nullptr;
}

int MQTreeFileEntryNode::getS64(int64_t *v) const
{
    if(m_type == TYPE_S32)
        *v = m_valueS32;
    else
        *v = m_valueS64;
    return 0;
}


QString MQTreeFileEntryNode::getString()  const
{
    QString v;
    getString(&v);
    return v;
}



int MQTreeFileEntryNode::getString(QString *v) const
{
    QString s;
    if(m_type == TYPE_STRING)
        s = m_valueStr;
    else if(m_type == TYPE_DOUBLE)
        s = QString::number(m_valueDbl);
    else if(m_type == TYPE_S8)
        s = QString::number(m_valueS8);
    else if(m_type == TYPE_S32)
        s = QString::number(m_valueS32);
    else if(m_type == TYPE_S64)
        s = QString::number(m_valueS64);
    else if(m_type == TYPE_ARRAY)
    {
        char sub[16];
        int i;
        s = QString::asprintf("%d bytes: ", m_valueArrayLen);
        for(i = 0;i < std::min(10,m_valueArrayLen);i++)
        {
            if(i)
                s += ",";

            char c = m_valueArray[i];
            sprintf(sub, "0x%02x", (unsigned char)c);
            s += sub;
            if(c == '\n')
                s += " '\\n'";
            else if(c == '\r')
                s += " '\\r'";
            else if(c == '\t')
                s += " '\\t'";
            else if(32 <= c && c <= 126)
            {
                sprintf(sub, " '%c'", c);
                s += sub;
            }
        }

        if(i != m_valueArrayLen)
            s += ",...";
    }
    else
    {
        assert(0);
    }
    *v = s;
    return 0;
}


/**
  @brief Returns a pointer to the data contained in the node.
  @return 0 on success.
 
  Note that the pointer memory is owned by MQTreeFileEntryNode and will be freed
  when node is destroyed.
 */
int MQTreeFileEntryNode::getArrayVoid(char **v, int *length) const
{
    assert(v != nullptr);
    assert(length != nullptr);

    *length = m_valueArrayLen;
    *v = m_valueArray;
    return 0;
}



int MQTreeFileEntryNode::setArrayVoid(const void *v, int length)
{
    assert(v != nullptr || length == 0);
    assert(length >= 0);
    m_type = TYPE_ARRAY;
    delete [] m_valueArray;
    m_valueArrayLen = length;
    if(length != 0)
    {
        m_valueArray = new char[length];
        memcpy(m_valueArray, v, length);
    }
    else
        m_valueArray = new char[1];
    

    return 0;
}



MQTreeFile::MQTreeFile()
{

    m_root = new MQTreeFileDirNode("");


}




MQTreeFile::~MQTreeFile()
{
    delete m_root;

}


QVector<MQTreeFileNode*> MQTreeFile::getChildren(QString path)
{
    int startPos = 1;
    int pos;
    QString dirName;
    assert(path[0] == '/');
    MQTreeFileDirNode *parent = m_root;
    QVector<MQTreeFileNode*>  list;

    if(!parent)
        return list;
    do
    {
        pos = path.indexOf('/', startPos);
        if(pos >= 0)
        {
            dirName = path.mid(startPos, pos-startPos);

            startPos = pos+1;

            parent = parent->findChildDir(dirName);
            if(!parent)
                return list;
        }
    }while(pos >= 0);

    //
    QString entryName = path.mid(startPos);
    MQTreeFileDirNode *dir;
    if(path == "/")
        dir = m_root;
    else
        dir = parent->findChildDir(entryName);
    if(dir)
        list = dir->getChildren();
    return list;
}


MQTreeFileEntryNode *MQTreeFile::getField(QString path)
{
    int startPos = 1;
    int pos;
    QString dirName;
    assert(path[0] == '/');
    MQTreeFileDirNode *parent = m_root;

    if(!parent)
        return nullptr;
    do
    {
        pos = path.indexOf('/', startPos);
        if(pos >= 0)
        {
            dirName = path.mid(startPos, pos-startPos);

            startPos = pos+1;

            parent = parent->findChildDir(dirName);
            if(!parent)
                return nullptr;
        }
    }while(pos >= 0);

    //
    QString entryName = path.mid(startPos);
    MQTreeFileEntryNode *node = nullptr;
    node = parent->findChildEntry(entryName);

    return node;
}


/**
 * @brief Creates a field (or return a already existing one).
 * @param path      The path to the field (Eg: "/a_dir/field")
 * @return  The created (or found) field.
 */
MQTreeFileEntryNode *MQTreeFile::createNode(QString path)
{
    MQTreeFileEntryNode *node = getField(path);
    if(node)
        return node;
    int startPos = 1;
    int pos;
    QString dirName;
    assert(path[0] == '/');

    if(!m_root)
        m_root = new MQTreeFileDirNode("");

    MQTreeFileDirNode *parent = m_root;


    do
    {
        pos = path.indexOf('/', startPos);
        if(pos >= 0)
        {
            dirName = path.mid(startPos, pos-startPos);

            startPos = pos+1;

            parent = parent->addChildDir(dirName);

        }
    }while(pos >= 0);

    //
    QString entryName = path.mid(startPos);
    node = parent->addChildEntry(entryName);

    return node;
}

void MQTreeFile::clear()
{
    delete m_root;
    m_root = nullptr;
}


static int readString(GenericFile* fd, QString *str)
{
    char c;
    *str = "";
    while(1)
    {
        ssize_t bytesWritten = fd->read(&c, 1);
        if(bytesWritten != 1)
            return -1;
        if(c == 0)
            return str->length()+1;
        *str += c;
    }
    return -1;
}

MQTreeFileNode* MQTreeFile::loadTree(GenericFile* fd)
{
    int bytesRead = 0;
    char  type[4];

    bytesRead = fd->read(type, 4);
    if(bytesRead < 0)
        throw std::runtime_error("Failed to read");

    off_t endPos = fd->currentPos();

    // Get length
    uint32_t len = 0;
    bytesRead = fd->read(&len, 4);
    if(bytesRead < 0)
        throw std::runtime_error("Failed to read");

    // Calculate position where the data ends
    endPos += len;

    QString name;
    bytesRead = readString(fd, &name);
    if(bytesRead < 0)
        throw std::runtime_error("Failed to read");

    QString desc;
    bytesRead = readString(fd, &desc);
    if(bytesRead < 0)
        throw std::runtime_error("Failed to read");

    // Get padding
    off_t curPos = fd->currentPos();
    //off_t nextPos;


    MQTreeFileNode *node_ = nullptr;
    if(memcmp(type,"DIR ", 4) == 0)
    {
        MQTreeFileDirNode *node = new MQTreeFileDirNode(name);
        node_ = node;
        while(curPos != endPos)
        {

            MQTreeFileNode * child = loadTree(fd);
            node->m_children.push_back(child);

            curPos = fd->currentPos();
        }
    }
    else
    {
        MQTreeFileEntryNode *node = new MQTreeFileEntryNode(name);
        node_ = node;
        node->setDesc(desc);
    if(memcmp(type,"DBL ", 4) == 0)
    {
        double d;
        int frc = fd->read(&d, sizeof(d));
        if(frc < 0)
            throw std::runtime_error("Failed to read");
        node->setDouble(d);
    }
    else if(memcmp(type,"S8  ", 4) == 0)
    {
        int8_t v;
        int frc = fd->read(&v, sizeof(v));
        if(frc < 0)
            throw std::runtime_error("Failed to read");
        node->setS8(v);
    }
    else if(memcmp(type,"S32 ", 4) == 0)
    {
        int32_t v;
        int frc = fd->read(&v, sizeof(v));
        if(frc < 0)
            throw std::runtime_error("Failed to read");
        node->setS32(v);
    }
    else if(memcmp(type,"S64 ", 4) == 0)
    {
        int64_t v;
        int frc = fd->read(&v, sizeof(v));
        if(frc < 0)
            throw std::runtime_error("Failed to read");
        node->setS64(v);
    }
    else if(memcmp(type,"STR ", 4) == 0)
    {
        int strLen = len - 4 - (name.length()+1) - (desc.length()+1);
        char *str = new char[strLen];

        //node->m_valueStr.reserve(strLen);
        //int frc = read(fd, (void*)node->m_valueStr.c_str(), strLen);
        int frc = fd->read(str, strLen);
        node->setString(std::string(str));
        delete [] str;
        if(frc < 0)
            throw std::runtime_error("Failed to read");

    }
    else if(memcmp(type,"VEC ", 4) == 0)
    {
        int strLen = len - 4 - (name.length()+1) - (desc.length()+1);
        char *v = new char[strLen];
        int frc = fd->read(v, strLen);
        if(frc < 0)
            throw std::runtime_error("Failed to read");
        node->setArrayVoid(v, strLen);
        delete [] v;
    }
    else
    {
        assert(0);
    }
    }

    // Skip padding
    int padLen = 4-(endPos&0x3);
    if(padLen != 4)
    {
        if(fd->seekTo(fd->currentPos() + padLen) == -1)
            throw std::runtime_error("Failed to read");
    }
    return node_;
}

QString MQTreeFile::getFieldString(QString path, QString defaultValue)
{
    QString v = defaultValue;
    MQTreeFileEntryNode *node = getField(path);
    if(node)
        node->getString(&v);
    return v;
}


/**
 * @brief Set a string field.
 * @param path      The path to the field (Eg: "/a_dir/field")
 */
void MQTreeFile::setFieldString(QString path,QString v)
{
    createNode(path)->setString(v);
}



/**
 * @brief Returns a S8 field.
 * @param path      The path to the field (Eg: "/a_dir/field")
 */
int8_t MQTreeFile::getFieldS8(QString path, int8_t defaultValue)
{
    int8_t v = defaultValue;
    MQTreeFileEntryNode *node = getField(path);
    if(node)
        node->getS8(&v);
    return v;
}


/**
 * @brief Set a S8 field.
 * @param path      The path to the field (Eg: "/a_dir/field")
 */
void MQTreeFile::setFieldS8(QString path, int8_t v)
{
    createNode(path)->setS8(v);
}




/**
 * @brief Returns a S32 field.
 * @param path      The path to the field (Eg: "/a_dir/field")
 */
int32_t MQTreeFile::getFieldS32(QString path, int32_t defaultValue)
{
    int32_t v = defaultValue;
    MQTreeFileEntryNode *node = getField(path);
    if(node)
        node->getS32(&v);
    return v;
}


/**
 * @brief Set a S32 field.
 * @param path      The path to the field (Eg: "/a_dir/field")
 */
void MQTreeFile::setFieldS32(QString path, int32_t v)
{
    createNode(path)->setS32(v);
}


/**
 * @brief Returns a S64 field.
 * @param path      The path to the field (Eg: "/a_dir/field")
 */
int64_t MQTreeFile::getFieldS64(QString path, int64_t defaultValue)
{
    int64_t v = defaultValue;
    MQTreeFileEntryNode *node = getField(path);
    if(node)
        node->getS64(&v);
    return v;
}


/**
 * @brief Set a S64 field.
 * @param path      The path to the field (Eg: "/a_dir/field")
 */
void MQTreeFile::setFieldS64(QString path, int64_t v)
{
    createNode(path)->setS64(v);
}


/**
 * @brief Returns a S32 field.
 * @param path      The path to the field (Eg: "/a_dir/field")
 */
int MQTreeFile::getFieldBuffer(QString path, char **v, int *length)
{
    MQTreeFileEntryNode *node = getField(path);
    assert(v != nullptr);
    assert(length != nullptr);
    *length = 0;
    if(!node)
        return -1;

    return node->getArrayVoid(v, length);
}


/**
 * @brief Set a S32 field.
 * @param path      The path to the field (Eg: "/a_dir/field")
 */
void MQTreeFile::setFieldBuffer(QString path, const void *v, int length)
{
    createNode(path)->setArrayVoid(v, length);
}


double MQTreeFile::getFieldDouble(QString path, double defaultValue)
{
    double v = defaultValue;
    MQTreeFileEntryNode *node = getField(path);
    if(node)
        node->getDouble(&v);
    return v;
}


/**
 * @brief Set a double field.
 * @param path      The path to the field (Eg: "/a_dir/field")
 */
void MQTreeFile::setFieldDouble(QString path, double v)
{
    createNode(path)->setDouble(v);
}




int MQTreeFile::load(QString filename)
{
    //int rc = 0;
    clear();

    std::unique_ptr<GenericFile> fd;
    if(ZipFile::isZipFile(filename))
    {
        ZipFile zip;
        if(zip.load(filename))
        {
            return -1;
        }


        QStringList entries = zip.getEntries();
        if(entries.size() == 0)
        {
            return -2;
        }

        QVector<uint8_t> buf;
        if(zip.readEntry(qPrintable(entries[0]), &buf))
            return -3;


        BufferFile *buffFile = new BufferFile();
        fd.reset(buffFile);
        buffFile->open(buf);
    }
    else
    {
        PhysicalFile *physFile = new PhysicalFile();
        fd.reset(physFile);
        if(physFile->openReadOnly(filename))
        {
            perror("Error opening file");
            return -1;
        }
    }

    try
    {

        Header hdr;
        ssize_t bytesWritten = fd->read(&hdr, sizeof(hdr));
        if(bytesWritten != sizeof(hdr))
        {
            throw std::runtime_error("Failed to seek");
        }

        if(memcmp(hdr.magic, HEADER_MAGIC, strlen(HEADER_MAGIC)) != 0)
        {
            printf("Invalid header in '%s'\n", qPrintable(filename));
            return -2;
        }
        if(hdr.version != 1)
        {
            printf("Invalid header version in '%s'\n", qPrintable(filename));
            return -3;
        }

        m_root = (MQTreeFileDirNode*)loadTree(fd.get());





    }
    catch (...)
    {
        throw;  // rethrow the same exception
    }

    return 0;
}



int MQTreeFile::writeTree(int fd, MQTreeFileNode *root)
{
    char type[4] = {'?','?','?','?'};
    const void *dataPtr = nullptr;
    int dataLen = 0;

    off_t startPos = lseek(fd, 0, SEEK_CUR);
    if((int)startPos == -1)
    {
        throw std::runtime_error("Failed to seek");
    }

    // Get type and data pointer
    if(root->isDir())
    {
        if(!root->asDir()->m_children.empty())
        {
            memcpy(type, "DIR ", 4);
            dataLen = 0;
        }

    }
    else
    {
        MQTreeFileEntryNode * nodeEntry = root->asEntry();

        switch(nodeEntry->getType())
    {
    default: assert(0);break;

    case MQTreeFileEntryNode::TYPE_S8:
    {
        memcpy(type, "S8  ", 4);
        dataPtr = &nodeEntry->m_valueS8;
        dataLen = 1;
    };break;
    case MQTreeFileEntryNode::TYPE_S32:
    {
        memcpy(type, "S32 ", 4);
        dataPtr = &nodeEntry->m_valueS32;
        dataLen = 4;
    };break;
    case MQTreeFileEntryNode::TYPE_S64:
    {
        memcpy(type, "S64 ", 4);
        dataPtr = &nodeEntry->m_valueS64;
        dataLen = 8;
    };break;
    case MQTreeFileEntryNode::TYPE_DOUBLE:
    {
        memcpy(type, "DBL ", 4);
        dataPtr = &nodeEntry->m_valueDbl;
        dataLen = sizeof(nodeEntry->m_valueDbl);
    };break;
    case MQTreeFileEntryNode::TYPE_STRING:
    {
        memcpy(type, "STR ", 4);
     };break;
    case MQTreeFileEntryNode::TYPE_ARRAY:
    {
        memcpy(type, "VEC ", 4);
        dataPtr = nodeEntry->m_valueArray;
        dataLen = nodeEntry->m_valueArrayLen;
    };break;
    };
    }


    // Write 'type' field
    ssize_t bytesWritten = ::write(fd,type, sizeof(type));
    if(bytesWritten < 0)
    {
        throw std::runtime_error("Failed to write header");
    }

    // Skip 'length' field
    off_t newPos = lseek(fd, 4, SEEK_CUR);
    if((int)newPos == -1)
    {
        throw std::runtime_error("Failed to seek");
    }
    off_t lengthFieldPos = newPos-4;

    // Write 'name' field
    bytesWritten = ::write(fd,qPrintable(root->m_name), root->m_name.length()+1);
    if(bytesWritten < 0)
    {
        throw std::runtime_error("Failed to write header");
    }

    // Write 'desc' field
    QString desc = "";
    bytesWritten = ::write(fd,qPrintable(desc), desc.length()+1);
    if(bytesWritten < 0)
    {
        throw std::runtime_error("Failed to write header");
    }

    // Write data
    if(!root->isDir())
    {
        MQTreeFileEntryNode * nodeEntry = root->asEntry();
        if(nodeEntry->m_type == MQTreeFileEntryNode::TYPE_STRING)
        {
            dataLen = nodeEntry->m_valueStr.length()+1;
            bytesWritten = ::write(fd,qPrintable(nodeEntry->m_valueStr), dataLen);
        }
        else
        {
            assert(dataPtr != nullptr);
            bytesWritten = ::write(fd,dataPtr, dataLen);
        }
        if(bytesWritten < 0)
        {
            throw std::runtime_error("Failed to write data");
        }
    }


    // Write all children
    if(root->isDir())
    {
        for(MQTreeFileNode* child : root->asDir()->m_children)
    {
        int rc = writeTree(fd, child);
        if(rc)
            return rc;
    }
    }
    off_t endPos = lseek(fd, 0, SEEK_CUR);


    // Go back and write the size of the entry
    if(lseek( fd, lengthFieldPos, SEEK_SET) < 0)
    {
        throw std::runtime_error("Failed to seek");
    }
    uint32_t lenEntry = endPos-startPos-4;
    if(::write(fd, &lenEntry, sizeof(lenEntry)) < 0)
    {
        throw std::runtime_error("Failed to seek");
    }

    // Go back to end of entry
    if(lseek( fd, endPos, SEEK_SET) < 0)
    {
        throw std::runtime_error("Failed to seek");
    }

    // Add extra padding
    int padLen = 4-(endPos&0x3);
    if(padLen != 4)
    {
        char padding[4] = {0, 0,0,0};
        bytesWritten = ::write(fd,padding, padLen);
        if(bytesWritten < 0)
        {
            throw std::runtime_error("Failed to write header");
        }
    }

    return 0;
}

int MQTreeFile::save(QString filename)
{
    // Open file for writing (create if it doesn't exist, truncate if it does)
    int fd = open(qPrintable(filename), O_WRONLY | O_CREAT | O_TRUNC, 0644);
    if (fd < 0) {
        throw std::runtime_error("Failed to seek");
        return 1;
    }

    int rc = -1;
    try
    {
        Header hdr;
        memset(&hdr,0,sizeof(hdr));
        memcpy(hdr.magic, HEADER_MAGIC, strlen(HEADER_MAGIC));
        hdr.version = 1;
        ssize_t bytesWritten = ::write(fd,&hdr, sizeof(hdr));
        if(bytesWritten < 0)
        {
            throw std::runtime_error("Failed to seek");
        }

        rc = writeTree(fd, m_root);
    }
    catch (...) {
        close(fd);
        throw;  // rethrow the same exception
    }
    close(fd);
    return rc;
}


#define STRINGIZE(v)   #v
#define STR(v)    STRINGIZE(v)
namespace mtreefile {

/**
 * @brief Returns the version number of the library
 */
const char *getMTreeFileVersion()
{
    return STR(LIB_MAJOR) "." STR(LIB_MINOR) "." STR(LIB_PATCH);
}

};


#ifdef TEST_PROGRAM
using namespace std;

int main()
{
    TreeFile tf;



    return 0;
}
#endif

