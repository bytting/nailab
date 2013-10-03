#ifndef CREATEDETECTORBEAKER_H
#define CREATEDETECTORBEAKER_H

#include <QStringList>
#include <QDialog>

namespace Ui {
class createdetectorbeaker;
}

class createdetectorbeaker : public QDialog
{
    Q_OBJECT
    
public:
    explicit createdetectorbeaker(QWidget *parent = 0);
    ~createdetectorbeaker();

    void setDetector(const QString &detector);
    void setBeakers(const QStringList &beakers);
    void setCalFilePath(const QString &path);

    QString beaker();
    QString calfile();
    
private:
    Ui::createdetectorbeaker *ui;    
    QString mPath, mDetector, mFilter;

public slots:
    void selectFileClick();
    void beakerSelected(QString beaker);
};

#endif // CREATEDETECTORBEAKER_H
