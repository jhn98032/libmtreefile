/*
 * Copyright (C) 2014-2017 Johan Henriksson.
 * All rights reserved.
 *
 * This software may be modified and distributed under the terms
 * of the BSD license.  See the LICENSE file for details.
 */

#include "aboutdialog.h"

#include <mqtutil/detectdistro.h>

#include <QProcess>

#include "version.h"
#include "log.h"
#include "config.h"

using namespace mqtutil;


AboutDialog::AboutDialog(QWidget * parent)
    : QDialog(parent)
{

    m_ui.setupUi(this);

    QString tmp = QString(PROGRAM_NAME) + " was written by Johan Henriksson";
    m_ui.label_developer->setText(tmp);

    //
    QString verStr;
    verStr = QString::asprintf("Version: v%d.%d.%d", APP_MAJOR, APP_MINOR,
                               APP_PATCH);
    m_ui.label_version->setText(verStr);

    //
    QString buildStr;
    buildStr = __DATE__;
    buildStr += " ";
    buildStr += __TIME__;
    m_ui.label_buildDate->setText("Built: " + buildStr);


    QString qtVersionStr;
    qtVersionStr = QString::asprintf("Qt: %s (compiled) / %s (running)", QT_VERSION_STR,
                                     qVersion());
    m_ui.label_qtVersion->setText(qtVersionStr);



    QString distroName;
    detectDistro(NULL, &distroName);
    QString osText = "Running on " + distroName;
    m_ui.label_os->setText(osText);

}
