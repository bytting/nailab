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

    void setBeakers(const QStringList &beakers);

    QString beaker();
    QString calfile();
    
private:
    Ui::createdetectorbeaker *ui;    
};

#endif // CREATEDETECTORBEAKER_H
