/*
 * Copyright (C) 2014-2017 Johan Henriksson.
 * All rights reserved.
 *
 * This software may be modified and distributed under the terms
 * of the BSD license.  See the LICENSE file for details.
 */

#ifndef FILE__ABOUTDIALOG_H
#define FILE__ABOUTDIALOG_H

#include <QDialog>

#include "config.h"
#include "ui_aboutdialog.h"


class AboutDialog : public QDialog
{
    Q_OBJECT

public:

    AboutDialog(QWidget *parent);


private:


    Ui_AboutDialog m_ui;
    
};

#endif // FILE__ABOUTDIALOG_H

