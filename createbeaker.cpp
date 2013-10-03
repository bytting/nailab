#include "createbeaker.h"
#include "ui_createbeaker.h"

CreateBeaker::CreateBeaker(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::CreateBeaker)
{
    ui->setupUi(this);
}

CreateBeaker::~CreateBeaker()
{
    delete ui;
}

QString CreateBeaker::name() const
{
    return ui->tbName->text().toUpper();
}

QString CreateBeaker::manufacturer() const
{
    return ui->tbManufacturer->text();
}

bool CreateBeaker::enabled() const
{
    return ui->cbEnabled->isChecked();
}
