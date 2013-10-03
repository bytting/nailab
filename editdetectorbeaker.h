#ifndef EDITDETECTORBEAKER_H
#define EDITDETECTORBEAKER_H

#include <QDialog>

namespace Ui {
class editdetectorbeaker;
}

class editdetectorbeaker : public QDialog
{
    Q_OBJECT
    
public:
    explicit editdetectorbeaker(QWidget *parent = 0);
    ~editdetectorbeaker();

    void setBeaker(QString beaker);
    void setDetector(const QString &detector);
    void setCalFilePath(const QString &path);

    QString beaker() const;
    QString calfile() const;
    
public slots:

    void selectFileClick();

private:
    Ui::editdetectorbeaker *ui;
    QString mPath, mDetector;
};

#endif // EDITDETECTORBEAKER_H
