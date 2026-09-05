/*
 * libmtreefile - 
 * Copyright (C) 2026 Johan Henriksson johan[a]dexar.se
 * All rights reserved.
 *
 * This software may be modified and distributed under the terms
 * of the BSD license.  See the LICENSE file for details.
 */

#ifndef FILE__QTREEFILE_H
#define FILE__QTREEFILE_H

#include <QString>
#include <QVector>

#include <string>

#include "file.h"

namespace mtreefile
{




class MQTreeFileDirNode;
class MQTreeFileEntryNode;
class MQTreeFile;

class MQTreeFileNode
{
public:
    MQTreeFileNode(QString name);
    virtual ~MQTreeFileNode();

public:


    virtual bool isDir() const = 0;

    virtual MQTreeFileDirNode* asDir() { return nullptr; }
    virtual MQTreeFileEntryNode* asEntry() { return nullptr; }

    void setName(QString name) { m_name = name; }
    QString getName() const { return m_name; }

private:
    QString m_name;


    friend MQTreeFile;
};

class MQTreeFileEntryNode : public MQTreeFileNode
{
public:
    MQTreeFileEntryNode(QString name);
    virtual ~MQTreeFileEntryNode();


    enum Type { TYPE_S8, TYPE_S32, TYPE_S64, TYPE_DOUBLE, TYPE_STRING, TYPE_ARRAY };


public:

    void setDesc(QString desc) { m_desc = desc; }

    void setS8(int8_t v) { m_type = TYPE_S8; m_valueS8 = v; }
    int getS8(int8_t *v) const { *v = m_valueS8; return 0; }
    void setS32(int32_t v) { m_type = TYPE_S32; m_valueS32 = v; }
    int getS32(int32_t *v)  const { *v = m_valueS32; return 0; }
    void setS64(int64_t v) { m_type = TYPE_S64; m_valueS64 = v; }
    int getS64(int64_t *v)  const;
    void setDouble(double v) { m_type = TYPE_DOUBLE; m_valueDbl = v; }
    int getDouble(double *v) const { *v = m_valueDbl; return 0; }
    void setString(QString v) { m_type = TYPE_STRING; m_valueStr = v; }
    void setString(std::string v) { m_type = TYPE_STRING; m_valueStr = QString::fromStdString(v); }
    void setString(const char *v) { m_type = TYPE_STRING; m_valueStr = v; }
    int getString(QString *v) const;
    QString getString() const;

    int getArrayVoid(char **v, int *length) const;
    int setArrayVoid(const void *v, int length);


    Type getType() const { return m_type; }

    virtual MQTreeFileEntryNode* asEntry() { return this; }

    virtual bool isDir() const { return false; }

private:
    int64_t m_valueS64;
    int8_t m_valueS8;
    double m_valueDbl;
    int32_t m_valueS32;
    QString m_valueStr;
    char *m_valueArray;
    int m_valueArrayLen; // Number of bytes of data in m_valueArray
    QString m_desc;

    Type m_type;


    friend MQTreeFile;
};


class MQTreeFileDirNode : public MQTreeFileNode
{
public:
    MQTreeFileDirNode(QString name);

    virtual ~MQTreeFileDirNode();


    bool hasChildren() { return !m_children.empty(); }
    void addChild(MQTreeFileEntryNode* child) { m_children.push_back(child); }
    void removeChild(MQTreeFileEntryNode* child);

    MQTreeFileEntryNode* addChildEntry(QString name);
    MQTreeFileDirNode* addChildDir(QString name);
    MQTreeFileDirNode* findChildDir(QString name);
    MQTreeFileEntryNode* findChildEntry(QString name);

    virtual MQTreeFileDirNode* asDir() { return this; }

    QVector<MQTreeFileNode*> getChildren() { return m_children; }

    virtual bool isDir() const { return true; }

    QVector<MQTreeFileNode*> m_children;

};


class MQTreeFile
{
public:
    MQTreeFile();
    virtual ~MQTreeFile();

    int load(QString filename);
    int save(QString filename);

    QString getFieldString(QString path, QString defaultValue = "");
    void setFieldString(QString path,QString v);

    int8_t getFieldS8(QString path, int8_t defaultValue = 0);
    void setFieldS8(QString path, int8_t v);

    int32_t getFieldS32(QString path, int32_t defaultValue = 0);
    void setFieldS32(QString path, int32_t v);

    int64_t getFieldS64(QString path, int64_t defaultValue = 0);
    void setFieldS64(QString path, int64_t v);

    double getFieldDouble(QString path, double defaultValue = 0.0);
    void setFieldDouble(QString path, double v);

    void setFieldBuffer(QString path, const void *v, int length);
    int getFieldBuffer(QString path, char **v, int *length);



    MQTreeFileEntryNode *getField(QString path);
    QVector<MQTreeFileNode*> getChildren(QString path);

    MQTreeFileEntryNode *createNode(QString path);


    MQTreeFileDirNode *getRoot()  { return m_root; }

    void clear();

protected:
    int writeTree(int fd, MQTreeFileNode *root);
    MQTreeFileNode* loadTree(mtreefile::GenericFile *fd);

public:
    MQTreeFileDirNode *m_root;

};



const char *getMTreeFileVersion();



}; // namespace mtreefile





#endif // FILE__TREEFILE_H

