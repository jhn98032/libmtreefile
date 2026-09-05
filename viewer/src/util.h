/*
 * Copyright (C) 2014-2018 Johan Henriksson.
 * All rights reserved.
 *
 * This software may be modified and distributed under the terms
 * of the BSD license.  See the LICENSE file for details.
 */

#ifndef FILE__UTIL_H
#define FILE__UTIL_H

#include <QString>
#include <QPointF>


#define MIN(a,b) ((a)<(b))
#define MAX(a,b) ((a)>(b))

//#define stringToCStr(str) str.toAscii().constData()
#define stringToCStr(str) qPrintable(str)

QString getFileTitle(QString filename);
QString getFilenamePart(QString fullPath);
void dividePath(QString fullPath, QString *filename, QString *folderPath);
QString getExtensionPart(QString filename);

quint8 hexStringToU8(const char *str);
long long stringToLongLong(const char* str);
long long stringToLongLong(QString str);
QString longLongToHexString(long long num);

QString simplifyPath(QString path);


const char *charToDesc(char c);

#include <math.h>



inline double degreesToRadians(double degrees) {
    return degrees/(180.0 / M_PI);
}
inline double radiansToDegrees(double radians) {
    return radians * (180.0 / M_PI);
}


void cartesianToPolar(QPointF pos, double *angle, double *r);
void polarToCartesian(double angle, double r, QPointF *pos);
double radSanitize(double rad);

double milToMm(double m);

#endif // FILE__UTIL_H

