#include "createdetectorbeaker.h"
#include "ui_createdetectorbeaker.h"

createdetectorbeaker::createdetectorbeaker(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::createdetectorbeaker)
{
    ui->setupUi(this);    
}

createdetectorbeaker::~createdetectorbeaker()
{
    delete ui;
}

void createdetectorbeaker::setBeakers(const QStringList &beakers)
{
    ui->cbBeaker->clear();
    ui->cbBeaker->addItems(beakers);
}

QString createdetectorbeaker::beaker()
{
    return ui->cbBeaker->currentText();
}

QString createdetectorbeaker::calfile()
{
    return ui->txtFilename->text();
}
