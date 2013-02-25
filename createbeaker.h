#ifndef CREATEBEAKER_H
#define CREATEBEAKER_H

#include <QDialog>

namespace Ui {
class CreateBeaker;
}

class CreateBeaker : public QDialog
{
    Q_OBJECT

public:
    explicit CreateBeaker(QWidget *parent = 0);
    ~CreateBeaker();

    QString name() const;
    QString manufacturer() const;
    bool enabled() const;

private:
    Ui::CreateBeaker *ui;
};

#endif // CREATEBEAKER_H
