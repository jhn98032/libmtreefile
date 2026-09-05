#ifndef FILE__TREEFILE_H
#define FILE__TREEFILE_H

#include <string>
#include <vector>

namespace mtreefile
{


class MTreeFileDirNode;
class MTreeFileEntryNode;

class MTreeFileNode
{
public:
    MTreeFileNode(std::string name);
    virtual ~MTreeFileNode();

public:


    virtual bool isDir() { return false; };

    virtual MTreeFileDirNode* asDir() { return nullptr; };
    virtual MTreeFileEntryNode* asEntry() { return nullptr; };

    void setName(std::string name) { m_name = name; };
    std::string getName() { return m_name; };

public:
    std::string m_name;

};

class MTreeFileEntryNode : public MTreeFileNode
{
public:
    MTreeFileEntryNode(std::string name);
    virtual ~MTreeFileEntryNode();


    enum Type { TYPE_S8, TYPE_S32, TYPE_DOUBLE, TYPE_STRING, TYPE_ARRAY };


public:



    void setS8(int8_t v) { m_type = TYPE_S8; m_valueS8 = v; };
    int getS8(int8_t *v) { *v = m_valueS8; return 0; };
    void setS32(int32_t v) { m_type = TYPE_S32; m_valueS32 = v; };
    int getS32(int32_t *v) { *v = m_valueS32; return 0; };
    void setDouble(double v) { m_type = TYPE_DOUBLE; m_valueDbl = v; };
    int getDouble(double *v) { *v = m_valueDbl; return 0; };
    void setString(std::string v) { m_type = TYPE_STRING; m_valueStr = v; };
    int getString(std::string *v);

    int getArrayVoid(char **v, int *length);
    int setArrayVoid(const void *v, int length);


    Type getType() const { return m_type; };

    virtual MTreeFileEntryNode* asEntry() { return this; };


public:
     int8_t m_valueS8;
    double m_valueDbl;
    int32_t m_valueS32;
    std::string m_valueStr;
    char *m_valueArray;
    int m_valueArrayLen; // Number of bytes of data in m_valueArray
    std::string m_desc;

    Type m_type;

};


class MTreeFileDirNode : public MTreeFileNode
{
public:
    MTreeFileDirNode(std::string name);

    virtual ~MTreeFileDirNode();


    bool hasChildren() { return !m_children.empty(); };
    void addChild(MTreeFileEntryNode* child) { m_children.push_back(child); };
    void removeChild(MTreeFileEntryNode* child);

    MTreeFileEntryNode* addChild(std::string name);
    MTreeFileDirNode* addChildDir(std::string name);

    virtual MTreeFileDirNode* asDir() { return this; };

    std::vector<MTreeFileNode*> getChildren() { return m_children; };

    virtual bool isDir() { return true; };

    std::vector<MTreeFileNode*> m_children;

};


class MTreeFile
{
public:
    MTreeFile();
    virtual ~MTreeFile();

    int load(std::string filename);
    int save(std::string filename);



    MTreeFileEntryNode *getField(std::string path);
    std::vector<MTreeFileEntryNode*> getChildren(std::string path);

    MTreeFileEntryNode *createNode(std::string path);


    MTreeFileDirNode *getRoot()  { return m_root; };

    void clear();

protected:
    int writeTree(int fd, MTreeFileNode *root);
    MTreeFileNode* loadTree(int fd);

public:
    MTreeFileDirNode *m_root;
};



}; // namespace mtreefile



#endif // FILE__TREEFILE_H

