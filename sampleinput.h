#ifndef SAMPLEINPUT_H
#define SAMPLEINPUT_H

#include <QString>

struct SampleInput
{
    QString detector;
    QString title;
    QString username;
    QString description;
    QString specterref;
    QString ID;
    QString type;
    QString quantity;
    QString quantityError;
    QString units;
    QString geometry;
    QString builduptype;
    QString startTime;
    QString endTime;
    QString randomError;
    QString systematicError;
};

#endif // SAMPLEINPUT_H
