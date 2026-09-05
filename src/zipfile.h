/*
 * libmtreefile - 
 * Copyright (C) 2026 Johan Henriksson johan[a]dexar.se
 * All rights reserved.
 *
 * This software may be modified and distributed under the terms
 * of the BSD license.  See the LICENSE file for details.
 */

#ifndef FILE__ZIPFILE_H
#define FILE__ZIPFILE_H

#include <QString>
#include <QVector>
#include <QStringList>


#include <zip.h>


namespace mtreefile
{



class ZipFile
{
public:
    ZipFile();
    virtual ~ZipFile();


    static bool isZipFile(QString filename);

    QStringList getEntries();

    int load(QString filename);
    int readEntry(const char* entryName, QVector<uint8_t>* out);

public:

    zip_t* za;
};



}; // namespace mtreefile




#endif // FILE__ZIPFILE_H

