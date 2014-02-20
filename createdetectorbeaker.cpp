#include "createdetectorbeaker.h"
#include "ui_createdetectorbeaker.h"
#include <QString>
#include <QFileDialog>

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

void createdetectorbeaker::setDetector(const QString &detector)
{
    mDetector = detector;
}

void createdetectorbeaker::setBeakers(const QStringList &beakers)
{
    ui->cbBeaker->clear();
    ui->cbBeaker->addItem("");
    ui->cbBeaker->addItems(beakers);
}

void createdetectorbeaker::setCalFilePath(const QString &path)
{
    mPath = path;
}

QString createdetectorbeaker::beaker()
{
    return ui->cbBeaker->currentText();
}

QString createdetectorbeaker::calfile()
{
    return ui->txtFilename->text();
}

void createdetectorbeaker::selectFileClick()
{    
    QString filename = QFileDialog::getOpenFileName(this, tr("Select cal file"), mPath, tr("CAL files (%1)").arg(mFilter));
    if(QFile::exists(filename))
    {
        ui->txtFilename->setText(QDir::toNativeSeparators(filename));
    }
}

void createdetectorbeaker::beakerSelected(QString beaker)
{
    mFilter = mDetector + "*.cal";
}
