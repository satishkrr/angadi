/*****************************************************************************
 * productsmodel.cpp
 *
 * Created: 22/04/2014 by vijay
 *
 * Copyright 2014 ThamiZha!. All rights reserved.
 *
 * Visit www.thamizha.com for more information.
 *
 * This file is a part of ThamiZha Angadi application.
 *
 * This file may be distributed under the terms of GNU Public License version
 * 3 (GPL v3) as defined by the Free Software Foundation (FSF). A copy of the
 * license should have been included with this file, or the project in which
 * this file belongs to. You may also find the details of GPL v3 at:
 * http://www.gnu.org/licenses/gpl-3.0.txt
 *
 * If you have any questions regarding the use of this file, feel free to
 * contact the author of this file, or the owner of the project in which
 * this file belongs to.
 *
 * Authors :
 * D.Mohan Raj <mohanraj.hunk@live.com>
 *****************************************************************************/

#ifndef BILLFORM_H
#define BILLFORM_H

#include "models/billitemmodel.h"
#include "models/billmodel.h"

#include <QWidget>
#include <QDateTime>
#include <QEvent>
#include <QDataWidgetMapper>
#include <QSqlRelationalDelegate>

namespace Ui {
class BillForm;
}

class BillForm : public QWidget
{
    Q_OBJECT

public:
    explicit BillForm(QWidget *parent = 0);
    ~BillForm();
    void clear();
    void setCodeFocus();
    void setModel(BillModel *billModel, BillItemModel *billItemModel);

private:
    Ui::BillForm *ui;
    BillItemModel *billItemModel;
    BillModel *billModel;

    QDataWidgetMapper *billDataMapper;
    QDataWidgetMapper *billItemDataMapper;

private slots:
    void save();

};

#endif // BILLFORM_H
