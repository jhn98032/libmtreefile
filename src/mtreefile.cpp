/*
 * libmtreefile - 
 * Copyright (C) 2026 Johan Henriksson johan[a]dexar.se
 * All rights reserved.
 *
 * This software may be modified and distributed under the terms
 * of the BSD license.  See the LICENSE file for details.
 */

#include "mtreefile.h"

#include <string.h>

#include <assert.h>
#include <unistd.h>
#include <stdint.h>
#include <fcntl.h>

#include <stdexcept>
#include <algorithm>


using namespace std;
using namespace mtreefile;


struct Header
{
    char magic[4]; // "TREE"
    uint32_t version; // 1
    unsigned char unused[64-8];
};

#define HEADER_MAGIC  "TREE"


MTreeFileNode::MTreeFileNode(std::string name)
    : m_name(name)
{

}

MTreeFileNode::~MTreeFileNode()
{

}


MTreeFileDirNode::MTreeFileDirNode(std::string name)
    : MTreeFileNode(name)

{

}

MTreeFileDirNode::~MTreeFileDirNode()
{
    for(MTreeFileNode*child :  m_children)
    {
        delete child;
    }

}



MTreeFileEntryNode::MTreeFileEntryNode(std::string name)
    : MTreeFileNode(name)
    ,m_valueS8(0)
    ,m_valueDbl(0.0)
    ,m_valueS32(0)
    ,m_valueArray(nullptr)
    ,m_valueArrayLen(0)
{
    m_type = Type::TYPE_S8;
}


MTreeFileEntryNode::~MTreeFileEntryNode()
{
    delete [] m_valueArray;

}


void MTreeFileDirNode::removeChild(MTreeFileEntryNode* child)
{
    auto it = std::find(m_children.begin(), m_children.end(), child);
    if (it != m_children.end())
    {
        m_children.erase(it);
        delete child;
    }

}


MTreeFileEntryNode* MTreeFileDirNode::addChild(std::string name)
{
    for(MTreeFileNode *node : m_children)
    {
        if(!node->isDir())
        {
        if(node->getName() == name)
            return node->asEntry();
        }
    }
    MTreeFileEntryNode *node = new MTreeFileEntryNode(name);
    m_children.push_back(node);
    return node;
}

MTreeFileDirNode* MTreeFileDirNode::addChildDir(std::string name)
{
    for(MTreeFileNode *node : m_children)
    {
        if(node->isDir())
        {
        if(node->getName() == name)
            return node->asDir();
        }
    }
    MTreeFileDirNode *node = new MTreeFileDirNode(name);
    m_children.push_back(node);
    return node;
}

int MTreeFileEntryNode::getString(std::string *v)
{
    std::string s;
    if(m_type == TYPE_STRING)
        s = m_valueStr;
    else if(m_type == TYPE_DOUBLE)
        s = std::to_string(m_valueDbl);
    else if(m_type == TYPE_S8)
        s = std::to_string(m_valueS8);
    else if(m_type == TYPE_S32)
        s = std::to_string(m_valueS32);
    else if(m_type == TYPE_ARRAY)
    {
        char sub[16];
        int i;
        for(i = 0;i < std::min(32,m_valueArrayLen);i++)
        {
            if(i)
                s += ",";

            char c = m_valueArray[i];
            sprintf(sub, "0x%02x", c);
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
 
  Note that the pointer memory is owned by MTreeFileEntryNode and will be freed
  when node is destroyed.
 */
int MTreeFileEntryNode::getArrayVoid(char **v, int *length)
{
    assert(v != nullptr);
    
    *length = m_valueArrayLen;
    *v = m_valueArray;
    return 0;
}



int MTreeFileEntryNode::setArrayVoid(const void *v, int length)
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



MTreeFile::MTreeFile()
{

    m_root = new MTreeFileDirNode("");


}




MTreeFile::~MTreeFile()
{
    delete m_root;

}

MTreeFileEntryNode *MTreeFile::createNode(std::string path)
{
    int startPos = 1;
    int pos;
    string dirName;
    assert(path[0] == '/');
    MTreeFileDirNode *parent = m_root;

    do
    {
        pos = path.find_first_of('/', startPos);
        if(pos >= 0)
        {
            dirName = path.substr(startPos, pos-startPos);

            startPos = pos+1;

            parent = parent->addChildDir(dirName);

        }
    }while(pos >= 0);

    //
    string entryName = path.substr(startPos);
    MTreeFileEntryNode *node = nullptr;
    node = parent->addChild(entryName);

    return node;
}

void MTreeFile::clear()
{
    delete m_root;
    m_root = nullptr;
}


static int readString(int fd, std::string *str)
{
    char c;
    *str = "";
    while(1)
    {
        ssize_t bytesWritten = ::read(fd,&c, 1);
        if(bytesWritten != 1)
            return -1;
        if(c == 0)
            return str->length()+1;
        *str += c;
    }
    return -1;
}

MTreeFileNode* MTreeFile::loadTree(int fd)
{
    int bytesRead = 0;
    char  type[4];

    bytesRead = read(fd, type, 4);
    if(bytesRead < 0)
        throw std::runtime_error("Failed to read");

    off_t endPos = lseek(fd, 0, SEEK_CUR);

    // Get length
    uint32_t len = 0;
    bytesRead = read(fd, &len, 4);
    if(bytesRead < 0)
        throw std::runtime_error("Failed to read");

    // Calculate position where the data ends
    endPos += len;

    std::string name;
    bytesRead = readString(fd, &name);
    if(bytesRead < 0)
        throw std::runtime_error("Failed to read");

    std::string desc;
    bytesRead = readString(fd, &desc);
    if(bytesRead < 0)
        throw std::runtime_error("Failed to read");

    // Get padding
    off_t curPos = lseek(fd, 0, SEEK_CUR);
    //off_t nextPos;


    MTreeFileNode *node_ = nullptr;
    if(memcmp(type,"DIR ", 4) == 0)
    {
        MTreeFileDirNode *node = new MTreeFileDirNode(name);
        node_ = node;
        while(curPos != endPos)
        {

            MTreeFileNode * child = loadTree(fd);
            node->m_children.push_back(child);

            curPos = lseek(fd, 0, SEEK_CUR);
        }
    }
    else
    {
        MTreeFileEntryNode *node = new MTreeFileEntryNode(name);
        node_ = node;
        node->m_desc = desc;
    if(memcmp(type,"DBL ", 4) == 0)
    {
        double d;
        int frc = read(fd, &d, sizeof(d));
        if(frc < 0)
            throw std::runtime_error("Failed to read");
        node->setDouble(d);
    }
    else if(memcmp(type,"S8  ", 4) == 0)
    {
        int8_t v;
        int frc = read(fd, &v, sizeof(v));
        if(frc < 0)
            throw std::runtime_error("Failed to read");
        node->setS8(v);
    }
    else if(memcmp(type,"S32 ", 4) == 0)
    {
        int32_t v;
        int frc = read(fd, &v, sizeof(v));
        if(frc < 0)
            throw std::runtime_error("Failed to read");
        node->setS32(v);
    }
    else if(memcmp(type,"STR ", 4) == 0)
    {
        int strLen = len - 4 - (name.length()+1) - (desc.length()+1);
        char *str = new char[strLen];

        //node->m_valueStr.reserve(strLen);
        //int frc = read(fd, (void*)node->m_valueStr.c_str(), strLen);
        int frc = read(fd, str, strLen);
        node->setString(str);
        delete [] str;
        if(frc < 0)
            throw std::runtime_error("Failed to read");

    }
    else if(memcmp(type,"VEC ", 4) == 0)
    {
        int strLen = len - 4 - (name.length()+1) - (desc.length()+1);
        char *v = new char[strLen];
        int frc = read(fd, v, strLen);
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
        if((int)lseek(fd, padLen, SEEK_CUR) == -1)
            throw std::runtime_error("Failed to read");
    }
    return node_;
}



int MTreeFile::load(std::string filename)
{
    //int rc = 0;
    clear();


    int fd = open(filename.c_str(), O_RDONLY);
    if (fd < 0) {
        perror("Error opening file");
        return 1;
    }


    try
    {

        Header hdr;
        ssize_t bytesWritten = ::read(fd,&hdr, sizeof(hdr));
        if(bytesWritten != sizeof(hdr))
        {
            throw std::runtime_error("Failed to seek");
        }

        if(memcmp(hdr.magic, HEADER_MAGIC, strlen(HEADER_MAGIC)) != 0)
        {
            printf("Invalid header in '%s'\n", filename.c_str());
            close(fd);
            return -2;
        }
        if(hdr.version != 1)
        {
            printf("Invalid header version in '%s'\n", filename.c_str());
            close(fd);
            return -3;
        }

        m_root = (MTreeFileDirNode*)loadTree(fd);





    }
    catch (...)
    {
        close(fd);
        throw;  // rethrow the same exception
    }
    close(fd);

    return 0;
}



int MTreeFile::writeTree(int fd, MTreeFileNode *root)
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
        MTreeFileEntryNode * nodeEntry = root->asEntry();

        switch(nodeEntry->getType())
    {
    default: assert(0);break;

    case MTreeFileEntryNode::TYPE_S8:
    {
        memcpy(type, "S8  ", 4);
        dataPtr = &nodeEntry->m_valueS8;
        dataLen = 1;
    };break;
    case MTreeFileEntryNode::TYPE_S32:
    {
        memcpy(type, "S32 ", 4);
        dataPtr = &nodeEntry->m_valueS32;
        dataLen = 4;
    };break;
    case MTreeFileEntryNode::TYPE_DOUBLE:
    {
        memcpy(type, "DBL ", 4);
        dataPtr = &nodeEntry->m_valueDbl;
        dataLen = sizeof(nodeEntry->m_valueDbl);
    };break;
    case MTreeFileEntryNode::TYPE_STRING:
    {
        memcpy(type, "STR ", 4);
        dataPtr = (const void*)nodeEntry->m_valueStr.c_str();
        dataLen = nodeEntry->m_valueStr.length()+1;
    };break;
    case MTreeFileEntryNode::TYPE_ARRAY:
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
    bytesWritten = ::write(fd,root->m_name.c_str(), root->m_name.length()+1);
    if(bytesWritten < 0)
    {
        throw std::runtime_error("Failed to write header");
    }

    // Write 'desc' field
    string desc = "";
    bytesWritten = ::write(fd,desc.c_str(), desc.length()+1);
    if(bytesWritten < 0)
    {
        throw std::runtime_error("Failed to write header");
    }

    // Write data
    bytesWritten = ::write(fd,dataPtr, dataLen);
    if(bytesWritten < 0)
    {
        throw std::runtime_error("Failed to write data");
    }

    // Write all children
    if(root->isDir())
    {
        for(MTreeFileNode* child : root->asDir()->m_children)
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

int MTreeFile::save(std::string filename)
{
    // Open file for writing (create if it doesn't exist, truncate if it does)
    int fd = open(filename.c_str(), O_WRONLY | O_CREAT | O_TRUNC, 0644);
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




#ifdef TEST_PROGRAM
using namespace std;

int main()
{
    TreeFile tf;



    return 0;
}
#endif

