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
    QString presetType1;
    QString presetType1Value;
    QString presetType1StartChannel;
    QString presetType1EndChannel;
    QString presetType2;
    QString presetType2Value;
};

#endif // SAMPLEINPUT_H
