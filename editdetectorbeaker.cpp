#include "editdetectorbeaker.h"
#include "ui_editdetectorbeaker.h"
#include <QString>
#include <QFileDialog>

editdetectorbeaker::editdetectorbeaker(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::editdetectorbeaker)
{
    ui->setupUi(this);
}

editdetectorbeaker::~editdetectorbeaker()
{
    delete ui;
}

void editdetectorbeaker::setBeaker(QString beaker)
{
    ui->lblBeaker->setText(beaker);
}

void editdetectorbeaker::setDetector(const QString &detector)
{
    mDetector = detector;
}

void editdetectorbeaker::setCalFilePath(const QString &path)
{
    mPath = path;
}

QString editdetectorbeaker::beaker() const
{
    return ui->lblBeaker->text();
}

QString editdetectorbeaker::calfile() const
{
    return ui->txtFilename->text();
}

void editdetectorbeaker::selectFileClick()
{
    QString filter = mDetector + beaker() + "*.cal";
    QString filename = QFileDialog::getOpenFileName(this, tr("Select cal file"), mPath, tr("CAL files (%1)").arg(filter));
    if(QFile::exists(filename))
    {
        ui->txtFilename->setText(filename);
    }
}
