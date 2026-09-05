/*
 * Copyright (C) 2014-2018 Johan Henriksson.
 * All rights reserved.
 *
 * This software may be modified and distributed under the terms
 * of the BSD license.  See the LICENSE file for details.
 */

#include "util.h"

#include <assert.h>
#include <QString>
#include <QFile>
#include <QProcess>
#include <QMap>


const char *
charToDesc(char c)
{
    static char desc[2];
    desc[0] = c;
    desc[1] = '\0';
    if (c == '\t')
        return "<TAB>";
    if (c == '\n')
        return "<NL>";
    if (c == '\r')
        return "<CR>";
    if (c == '.')
        return ". (dot)";
    if (c == ',')
        return ", (comma)";
    if (c == '|')
        return "| (bar)";


    return desc;
}


/**
 * @brief Returns the filetitle of a file.
 * @param the filename (Ex: "/a_path/test.bmp").
 * @return the filetitle  (Eg: "test")
 */
QString
getFileTitle(QString filename)
{
    filename = getFilenamePart(filename);
    int lastPos = filename.lastIndexOf('.');
    if (lastPos == -1)
        return filename;
    else
        return filename.left(lastPos);
}

/**
 * @brief Divides a path into a filename and a path.
 *
 * Example: dividePath("/dir/filename.ext") => "/dir", "filename.ext".
 */
void
dividePath(QString fullPath, QString * filename, QString * folderPath)
{
    int divPos = fullPath.lastIndexOf('/');
    if (divPos > 0)
    {
        if (filename)
            *filename = fullPath.mid(divPos + 1);
        if (folderPath)
            *folderPath = fullPath.left(divPos);
    }
    else
    {
        if (filename)
            *filename = fullPath;
    }
}

/**
 * @brief Returns the filename of a path.
 *
 * Example: getFilenamePart("/dir/filename.ext") => "filename.ext".
 */
QString
getFilenamePart(QString fullPath)
{
    QString filename;
    dividePath(fullPath, &filename, NULL);
    return filename;
}


/**
 * @brief Returns the extension of a file.
 * @return The extension excluding the dot (Eg: "txt").
 */
QString
getExtensionPart(QString filename)
{
    int idx = filename.lastIndexOf('.');
    if (idx == -1)
        return QString("");
    return filename.mid(idx + 1);
}



long long
stringToLongLong(const char *str)
{
    unsigned long long num = 0;
    QString str2 = str;
    str2.replace('_', "");
    num = str2.toLongLong(0, 0);

    return num;
}


QString
longLongToHexString(long long num)
{
    QString newStr;
    QString str;
#if QT_VERSION >= QT_VERSION_CHECK(5,5,0)
    str = QString::asprintf("%llx", num);
#else
    str.sprintf("%llx", num);
#endif
    if (num != 0)
    {
        while (str.length() % 4 != 0)
            str = "0" + str;


        for (int i = str.length() - 1; i >= 0; i--)
        {
            newStr += str[str.length() - i - 1];
            if (i % 4 == 0 && i != 0)
                newStr += "_";
        }
        str = newStr;
    }
    return "0x" + str;
}


static QString
priv_simplifySubPath(QString path)
{
    QString out;

    if (path.startsWith('/'))
        return simplifyPath(path.mid(1));
    if (path.startsWith("./"))
        return simplifyPath(path.mid(2));

    QString first;
    QString rest;

    int piv = path.indexOf('/');
    if (piv == -1)
        return path;
    else
    {
        first = path.left(piv);
        rest = path.mid(piv + 1);
        rest = priv_simplifySubPath(rest);
        if (rest.isEmpty())
            path = first;
        else
            path = first + "/" + rest;
    }
    return path;
}


/**
 * @brief Simplifies a path by removing unnecessary seperators.
 *
 * Eg: simplifyPath("./a///path/") => "./a/path".
 */
QString
simplifyPath(QString path)
{
    QString out;
    if (path.startsWith("./"))
        out = "./" + priv_simplifySubPath(path.mid(2));
    else if (path.startsWith('/'))
        out = '/' + priv_simplifySubPath(path.mid(1));
    else
        out = priv_simplifySubPath(path);
    return out;
}

/**
*  @brief Converts a hex two byte string to a unsigned char.
*/
quint8
hexStringToU8(const char *str)
{
    quint8 d = 0;
    char c1 = str[0];
    char c2 = str[1];

    // Upper byte
    if ('0' <= c1 && c1 <= '9')
        d = c1 - '0';
    else if ('a' <= c1 && c1 <= 'f')
        d = 0xa + (c1 - 'a');
    else if ('A' <= c1 && c1 <= 'F')
        d = 0xa + (c1 - 'A');
    else                        // invalid character
    {
        assert(0);
        return 0;
    }
    d = d << 4;

    // Lower byte
    if ('0' <= c2 && c2 <= '9')
        d += c2 - '0';
    else if ('a' <= c2 && c2 <= 'f')
        d += 0xa + (c2 - 'a');
    else if ('A' <= c2 && c2 <= 'F')
        d += 0xa + (c2 - 'A');
    else                        // invalid character?
    {
        assert(0);
        d = d >> 4;
    }

    return d;
}

long long
stringToLongLong(QString str)
{
    return stringToLongLong(stringToCStr(str));
}



double radSanitize(double rad)
{
    while(rad < 0.0)
        rad += 2.0*M_PI;
    while(rad > 2.0*M_PI)
        rad -= 2.0*M_PI;
    return rad;
}

void polarToCartesian(double angle, double r, QPointF *pos)
{
    pos->setX(r*cos(angle));
    pos->setY(r*sin(angle));
}

void cartesianToPolar(QPointF pos, double *angle, double *r)
{
    if(r)
        *r = sqrt( (pos.x()*pos.x()) + (pos.y()*pos.y()));

    if(angle)
    {
        *angle = ::atan2(pos.y(), pos.x());
         while((*angle) < 0.0)
            (*angle) += 2*M_PI;

    }
}


double milToMm(double m)
{
    return m * 25.4 / 1000.0;
}


#ifdef NEVER
void testFuncs()
{
    printf("%12x -> '%s'\n", 0, stringToCStr(longLongToHexString(0)));
    printf("%12x -> '%s'\n", 0x1, stringToCStr(longLongToHexString(0x1)));
    printf("%12x -> '%s'\n", 0x123, stringToCStr(longLongToHexString(0x123)));
    printf("%12x -> '%s'\n", 0x1234, stringToCStr(longLongToHexString(0x1234)));
    printf("%12x -> '%s'\n", 0x1234567, stringToCStr(longLongToHexString(0x1234567)));
    printf("%12llx -> '%s'\n", 0x12345678ULL, stringToCStr(longLongToHexString(0x12345678ULL)));
    printf("%12llx -> '%s'\n", 0x123456789abcULL, stringToCStr(longLongToHexString(0x123456789abcULL)));
}
#endif

