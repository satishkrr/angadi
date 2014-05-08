/*****************************************************************************
 * productform.cpp
 *
 * Created: 18/04/2014 by vijay
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
 * Vijay @ Dhanasekaran <vijay.kuruntham.gmail.com>
 * Selvam <vjpselvam@gmail.com>
 *****************************************************************************/

#include "productform.h"
#include "ui_productform.h"

#include <QDebug>
#include <QIntValidator>
#include <QMessageBox>
#include <QSqlRelationalDelegate>
#include <QSqlRecord>
#include <QSqlQuery>

ProductForm::ProductForm(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::ProductForm)
{
    ui->setupUi(this);

    formValidation = new FormValidation;

    dataMapper = new QDataWidgetMapper(this);
    dataMapper->setItemDelegate(new QSqlRelationalDelegate(this));
    dataMapper->setSubmitPolicy(QDataWidgetMapper::ManualSubmit);
    validCodeFlag = validNameFlag = validMrpFlag = validSalePriceFlag = validWholeSalePrice = 0;

    ui->pushButtonDelete->setEnabled(false);
//    ui->pushButtonSave->setEnabled(false);

    // Hide the errors labels at the start
    ui->labelManufacturer->hide();
    ui->comboBoxManufacturer->hide();

    ui->comboBoxManufacturer->addItem("Manufacturer 1");
    ui->comboBoxManufacturer->addItem("Manufacturer 2");
    ui->comboBoxManufacturer->addItem("Manufacturer 3");

    ui->comboBoxUnit->addItem("Nos");
    ui->comboBoxUnit->addItem("Kgs");
    ui->comboBoxUnit->addItem("Bundles");

    connect(ui->pushButtonSave,SIGNAL(clicked()),this,SLOT(save()));
    connect(ui->lineEditName,SIGNAL(textChanged(QString)),this,SLOT(onNameChanged(QString)));
    connect(ui->lineEditName,SIGNAL(returnPressed()),this,SLOT(setSignalFromProductForm()));

    connect(ui->lineEditCode,SIGNAL(editingFinished()),SLOT(codeValid()));
    connect(ui->lineEditName,SIGNAL(editingFinished()),SLOT(nameValid()));
    connect(ui->lineEditMrp,SIGNAL(editingFinished()),SLOT(mrpValid()));
    connect(ui->lineEditSalePrice,SIGNAL(editingFinished()),SLOT(salePriceValid()));
    connect(ui->lineEditWholeSalePrice,SIGNAL(editingFinished()),SLOT(wholeSalePriceValid()));

    setFieldMaxLength();
}

ProductForm::~ProductForm()
{
    delete ui;
}

//save the product form
void ProductForm::save()
{
    // Initialization of local variables
    int validError = 0;
    QString errors = "";

    // Initialization of message box
    QMessageBox msgBox;
    msgBox.setText("Validation Error in this forms. Please correct the form and resubmit it");
    msgBox.setInformativeText("");
    msgBox.setStandardButtons(QMessageBox::Ok);
    msgBox.setDefaultButton(QMessageBox::Ok);

    // validate code field
    if(!ProductForm::codeValid()){
        validError = 1;
        errors.append("\nThe Category Code field may be empty or already exist");
    }

    // Validate Name field
    if(!ProductForm::nameValid()){
        validError = 1;
        errors.append("\nThe Name field may be empty or already exist");
    }

    //Validate Mrp field
    if(!ProductForm::mrpValid()){
        validError = 1;
        errors.append("\nThe MRP field may be empty or not a number");
    }

    //Validate Sale Price field
    if(!ProductForm::salePriceValid()){
        validError = 1;
        errors.append("\nThe Sale Price field may be empty or not a number");
    }

    //Validate Whole Sale Price field
    if(!ProductForm::wholeSalePriceValid()){
        validError = 1;
        errors.append("\nThe Whole Sale Price field may be empty or not a number");
    }

    // save the form if there is no errors
    if(validError == 0){
        bool status;

        if(dataMapper->currentIndex() < 0){
            int row = productsModel->rowCount();
            productsModel->insertRows(row, 1);

            productsModel->setData(productsModel->index(row,productsModel->fieldIndex("code")),ui->lineEditCode->text());
            productsModel->setData(productsModel->index(row,productsModel->fieldIndex("name")),ui->lineEditName->text());

            QSqlQueryModel model;
            QSqlQuery query;
            query.prepare("Select id from categories where name = :category_name");
            query.bindValue(":category_name", ui->comboBoxcategoryId->currentText());
            query.exec();
            model.setQuery(query);
            QSqlRecord record = model.record(0);
            int category_id = record.value("id").toInt();
            QModelIndex idx = productsModel->index(row,4);
            productsModel->setData(idx, category_id, Qt::EditRole);
//            productsModel->setData(productsModel->index(row,productsModel->fieldIndex("manufacturer")),ui->comboBoxManufacturer->currentText());
            productsModel->setData(productsModel->index(row,productsModel->fieldIndex("unit")),ui->comboBoxUnit->currentText());
            productsModel->setData(productsModel->index(row,productsModel->fieldIndex("mrp")),ui->lineEditMrp->text());
            productsModel->setData(productsModel->index(row,productsModel->fieldIndex("sprice")),ui->lineEditSalePrice->text());
            productsModel->setData(productsModel->index(row,productsModel->fieldIndex("wholeSalePrice")),ui->lineEditWholeSalePrice->text());

            QDateTime datetime = QDateTime::currentDateTime();
            productsModel->setData(productsModel->index(row,productsModel->fieldIndex("createdDate")),datetime.toString("yyyy-MM-dd hh:mm:ss"));

            productsModel->submitAll();

            statusMsg = ui->lineEditName->text() + " saved successfully";
            emit signalStatusBar(statusMsg);
        }else{
            QDateTime datetime = QDateTime::currentDateTime();
            this->setProperty("modifiedDate", datetime);

            status = dataMapper->submit();

            if(status == true){
                productsModel->submitAll();
                statusMsg = ui->lineEditName->text() + " updated successfully";
                emit signalStatusBar(statusMsg);
            }
            emit signalUpdated();
        }

        resetDataMapper();
        clear();
        setCodeFocus();

    }else{
        msgBox.setInformativeText(errors);
        int ret = msgBox.exec();
        switch (ret) {
           case QMessageBox::Ok:
               ui->lineEditCode->setFocus();
               ui->lineEditCode->selectAll();
               break;
           default:
               // should never be reached
               break;
         }
    }
}

void ProductForm::setCodeFocus(){
    ui->lineEditCode->setFocus();
    ui->lineEditCode->selectAll();
}

void ProductForm::setNameFocus()
{
    ui->lineEditName->setFocus();
    ui->lineEditName->selectAll();
}

void ProductForm::clear(){
    foreach(QLineEdit *widget, this->findChildren<QLineEdit*>()) {
        widget->clear();
        widget->setProperty("validationError",false);
        widget->setProperty("validationSuccess",false);
        widget->setStyleSheet(styleSheet());
    }

    ui->comboBoxUnit->setCurrentIndex(0);
    ui->comboBoxcategoryId->setCurrentIndex(0);

    uninstallEventFilter();
    ui->pushButtonSave->setText("Save");
    ui->pushButtonDelete->setEnabled(false);
//    ui->pushButtonSave->setEnabled(false);
}

void ProductForm::setModel(ProductsModel *model){
    productsModel = model;
    dataMapper->setModel(productsModel);

    productsModel->relationModel(4)->select();
    ui->comboBoxcategoryId->setModel(productsModel->relationModel(4));
    ui->comboBoxcategoryId->setModelColumn(productsModel->relationModel(4)->fieldIndex("name"));

    dataMapper->addMapping(ui->lineEditCode,productsModel->fieldIndex("code"));
    dataMapper->addMapping(ui->lineEditName,productsModel->fieldIndex("name"));
    dataMapper->addMapping(ui->comboBoxcategoryId, 4);
//    dataMapper->addMapping(ui->comboBoxManufacturer,productsModel->fieldIndex("manufacturer"));
    dataMapper->addMapping(ui->comboBoxUnit,productsModel->fieldIndex("unit"));
    dataMapper->addMapping(ui->lineEditMrp,productsModel->fieldIndex("mrp"));
    dataMapper->addMapping(ui->lineEditSalePrice,productsModel->fieldIndex("sprice"));
    dataMapper->addMapping(ui->lineEditWholeSalePrice,productsModel->fieldIndex("wholeSalePrice"));

    setCodeFocus();
}

// validate the code field
bool ProductForm::codeValid(){
    bool status = false;
    QString flashMsg = "";
    ui->lineEditCode->installEventFilter(this);
    if(ui->lineEditCode->text().length() > 0){
        if (uniqueValid(ui->lineEditCode->text(),"code")){
            ui->lineEditCode->setProperty("validationError",false);
            ui->lineEditCode->setProperty("validationSuccess",true);
            ui->lineEditCode->setStyleSheet(styleSheet());
            validCodeFlag = 1;
            status = true;
        }else{
            flashMsg = "This Product Code has been already taken. Please give some other code.";
            ui->lineEditCode->setProperty("validationError",true);
            ui->lineEditCode->setProperty("validationSuccess",false);
            ui->lineEditCode->setStyleSheet(styleSheet());
            validCodeFlag = 0;
            status = false;
        }
    }else{
        flashMsg = "Product Code is empty. Please give code.";
        ui->lineEditCode->setProperty("validationError",true);
        ui->lineEditCode->setProperty("validationSuccess",false);
        ui->lineEditCode->setStyleSheet(styleSheet());
        validCodeFlag = 0;
        status = false;
    }
    ui->flashMsgUp->setText(flashMsg);
    return status;
}

// validate the name field
bool ProductForm::nameValid(){
    bool status= false;
    QString flashMsg = "";
    ui->lineEditName->installEventFilter(this);
    if(ui->lineEditName->text().length() > 0){
        if(uniqueValid(ui->lineEditName->text(),"name")){
            ui->lineEditName->setProperty("validationError",false);
            ui->lineEditName->setProperty("validationSuccess",true);
            ui->lineEditName->setStyleSheet(styleSheet());
            validNameFlag = 1;
            status= true;
        }else{
            flashMsg = "This Product Name has been already taken. Please give some other name.";
            ui->lineEditName->setProperty("validationError",true);
            ui->lineEditName->setProperty("validationSuccess",false);
            ui->lineEditName->setStyleSheet(styleSheet());
            validNameFlag = 0;
            status= false;
        }
    }else{
        flashMsg = "Product Name is empty. Please give name.";
        ui->lineEditName->setProperty("validationError",true);
        ui->lineEditName->setProperty("validationSuccess",false);
        ui->lineEditName->setStyleSheet(styleSheet());
        status= false;
        validNameFlag = 0;
    }
    ui->flashMsgUp->setText(flashMsg);
    return status;
}

//validate the mrp field
bool ProductForm::mrpValid(){
    bool status = false;
    QString flashMsg = "";
    ui->lineEditMrp->installEventFilter(this);
    if(ui->lineEditMrp->text().length() > 0){
        if(formValidation->isDouble(ui->lineEditMrp->text())){
            ui->lineEditMrp->setProperty("validationError",false);
            ui->lineEditMrp->setProperty("validationSuccess",true);
            ui->lineEditMrp->setStyleSheet(styleSheet());
            validMrpFlag = 1;
            status = true;
        }else{
            flashMsg = "Mrp is not a number. Please fix the price for this Product in number.";
            ui->lineEditMrp->setProperty("validationError",true);
            ui->lineEditMrp->setProperty("validationSuccess",false);
            ui->lineEditMrp->setStyleSheet(styleSheet());
            validMrpFlag = 0;
            status = false;
        }
    }else{
        flashMsg = "Mrp field is empty. Please fix the some price.";
        ui->lineEditMrp->setProperty("validationError",true);
        ui->lineEditMrp->setProperty("validationSuccess",false);
        ui->lineEditMrp->setStyleSheet(styleSheet());
        validMrpFlag = 0;
        status = false;
    }
    ui->flashMsgUp->setText(flashMsg);
    return status;
}

//validate the sale price field
bool ProductForm::salePriceValid(){
    bool status = false;
    QString flashMsg = "";
    ui->lineEditSalePrice->installEventFilter(this);
    if(ui->lineEditSalePrice->text().length() > 0){
        if(formValidation->isDouble(ui->lineEditSalePrice->text())){
            if(ui->lineEditSalePrice->text().toDouble() <= ui->lineEditMrp->text().toDouble()){
                ui->lineEditSalePrice->setProperty("validationError",false);
                ui->lineEditSalePrice->setProperty("validationSuccess",true);
                ui->lineEditSalePrice->setStyleSheet(styleSheet());
                validSalePriceFlag = 1;
                status = true;
            }else{
                flashMsg = "Sale Price must be less than or equal to MRP. Please check.";
                ui->lineEditSalePrice->setProperty("validationError",true);
                ui->lineEditSalePrice->setProperty("validationSuccess",false);
                ui->lineEditSalePrice->setStyleSheet(styleSheet());
                validSalePriceFlag = 0;
                status = false;
            }
        }else{
            flashMsg = "Sale Price is not a number. Please fix the sale price for this Product in number.";
            ui->lineEditSalePrice->setProperty("validationError",true);
            ui->lineEditSalePrice->setProperty("validationSuccess",false);
            ui->lineEditSalePrice->setStyleSheet(styleSheet());
            validSalePriceFlag = 0;
            status = false;
        }
    }else{
        flashMsg = "Sale Price is empty. Please fix the sale price.";
        ui->lineEditSalePrice->setProperty("validationError",true);
        ui->lineEditSalePrice->setProperty("validationSuccess",false);
        ui->lineEditSalePrice->setStyleSheet(styleSheet());
        validSalePriceFlag = 1;
        status = false;
    }
    ui->flashMsgUp->setText(flashMsg);
    return status;
}

//validate the whole sale price field
bool ProductForm::wholeSalePriceValid(){
    bool status = false;
    QString flashMsg = "";
    ui->lineEditWholeSalePrice->installEventFilter(this);
    if(ui->lineEditWholeSalePrice->text().length() > 0){
        if(formValidation->isDouble(ui->lineEditWholeSalePrice->text())){
            if(ui->lineEditWholeSalePrice->text().toDouble() <= ui->lineEditSalePrice->text().toDouble()){
                ui->lineEditWholeSalePrice->setProperty("validationError",false);
                ui->lineEditWholeSalePrice->setProperty("validationSuccess",true);
                ui->lineEditWholeSalePrice->setStyleSheet(styleSheet());
                validWholeSalePrice = 1;
                status = true;
            }else{
                flashMsg = "Whole Sale Price must be less than or equal to sale price. Please check.";
                ui->lineEditWholeSalePrice->setProperty("validationError",true);
                ui->lineEditWholeSalePrice->setProperty("validationSuccess",false);
                ui->lineEditWholeSalePrice->setStyleSheet(styleSheet());
                validWholeSalePrice = 0;
                status = false;
            }
        }else{
            flashMsg = "Whole Sale Price is not a number. Please fix the whole sale price for this Product in number.";
            ui->lineEditWholeSalePrice->setProperty("validationError",true);
            ui->lineEditWholeSalePrice->setProperty("validationSuccess",false);
            ui->lineEditWholeSalePrice->setStyleSheet(styleSheet());
            validWholeSalePrice = 0;
            status = false;
        }
    }else{
        flashMsg = "Whole Sale Price is empty. Please fix the whole sale price.";
        ui->lineEditWholeSalePrice->setProperty("validationError",true);
        ui->lineEditWholeSalePrice->setProperty("validationSuccess",false);
        ui->lineEditWholeSalePrice->setStyleSheet(styleSheet());
        validWholeSalePrice = 0;
        status = false;
    }
    ui->flashMsgUp->setText(flashMsg);
    return status;
}

bool ProductForm::uniqueValid(QString text,QString field)
{
    bool status = false;
    FormValidation formValidation;
    QString id;
    QSqlRecord cRecord;
    if(dataMapper->currentIndex() < 0){
        id = "0";

    }else{
        cRecord = productsModel->record(dataMapper->currentIndex());
        id = cRecord.value("id").toString();

    }
    int count = formValidation.uniqueValid(id, text, "products", field);
    if(count <= 0){
        status = true;
    }else{
        status = false;
    }
    return status;
}

void ProductForm::setMapperIndex(QModelIndex index)
{
    clear();
    dataMapper->setCurrentIndex(index.row());
    this->ui->pushButtonSave->setText("Update");
    ui->pushButtonDelete->setEnabled(true);
    validCodeFlag = validNameFlag = 1;
//    ui->pushButtonSave->setEnabled(false);
//    setAllValidationSuccess();
}

void ProductForm::search(QString value)
{
    QString searchValue = "code = ";
    searchValue.append(value);
    productsModel->selectRow(1);
}

void ProductForm::on_pushButtonCancel_clicked()
{
    resetDataMapper();
    clear();
    setCodeFocus();
}

void ProductForm::on_pushButtonDelete_clicked()
{
    QSqlRecord record = productsModel->record(dataMapper->currentIndex());
    statusMsg = ui->lineEditName->text() + " deleted successfully";

    QDateTime datetime = QDateTime::currentDateTime();
    QChar t_status = 'D';
    record.setValue("status", t_status);
    record.setValue("modifiedDate", datetime);
    productsModel->setRecord(dataMapper->currentIndex(),record);
    productsModel->submitAll();
    productsModel->select();

    emit signalStatusBar(statusMsg);

    on_pushButtonCancel_clicked();
}

QDateTime ProductForm::modifiedDate() const
{
    return m_modifiedDate;
}

void ProductForm::setModifiedDate(QDateTime modifiedDate)
{
    m_modifiedDate = modifiedDate;
}

void ProductForm::onNameChanged(QString str)
{
    emit signalName(str);
}

void ProductForm::setSignalFromProductForm()
{
    emit signalFromProductForm();
}

void ProductForm::setFieldMaxLength()
{
    ui->lineEditCode->setMaxLength(100);
    ui->lineEditName->setMaxLength(200);

//    ui->lineEditWholeSalePrice->setProperty("validationError",false);
//    ui->lineEditWholeSalePrice->setProperty("validationSuccess",true);
//    ui->lineEditWholeSalePrice->setStyleSheet(styleSheet());
}

void ProductForm::resetDataMapper()
{
    dataMapper = new QDataWidgetMapper(this);
    dataMapper->setSubmitPolicy(QDataWidgetMapper::ManualSubmit);
    setModel(productsModel);
}

bool ProductForm::eventFilter(QObject *obj, QEvent *event)
{
    if (obj == ui->lineEditCode){
        if (event->type() == QEvent::FocusIn)
            ProductForm::codeValid();
        return false;
    }else if (obj == ui->lineEditMrp){
        if (event->type() == QEvent::FocusIn)
            ProductForm::mrpValid();
        return false;
    }else if (obj == ui->lineEditName){
        if (event->type() == QEvent::FocusIn)
            ProductForm::nameValid();
        return false;
    }else if (obj == ui->lineEditSalePrice){
        if (event->type() == QEvent::FocusIn)
            ProductForm::salePriceValid();
        return false;
    }else if (obj == ui->lineEditWholeSalePrice){
        if (event->type() == QEvent::FocusIn)
            ProductForm::wholeSalePriceValid();
        return false;
    }
    return ProductForm::eventFilter(obj, event);
}

void ProductForm::uninstallEventFilter()
{
    ui->lineEditCode->removeEventFilter(this);
    ui->lineEditMrp->removeEventFilter(this);
    ui->lineEditName->removeEventFilter(this);
    ui->lineEditSalePrice->removeEventFilter(this);
    ui->lineEditWholeSalePrice->removeEventFilter(this);
    ui->flashMsgUp->clear();
}

void ProductForm::setAllValidationSuccess()
{
    foreach(QLineEdit *widget, this->findChildren<QLineEdit*>()) {
        widget->setProperty("validationError",false);
        widget->setProperty("validationSuccess",false);
        widget->setStyleSheet(styleSheet());
    }
}

void ProductForm::setComboSource()
{
    productsModel->relationModel(4)->select();
    ui->comboBoxcategoryId->setModel(productsModel->relationModel(4));
    ui->comboBoxcategoryId->setModelColumn(productsModel->relationModel(4)->fieldIndex("name"));
}
