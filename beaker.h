#ifndef BEAKER_H
#define BEAKER_H

#include <QString>

struct Beaker
{
    Beaker() : name(""), manufacturer(""), enabled(false) {}
    Beaker(const QString& name, const QString& manufacturer, bool enabled)
        : name(name), manufacturer(manufacturer), enabled(enabled) {}

    QString name;
    QString manufacturer;
    bool enabled;
};

#endif // BEAKER_H
