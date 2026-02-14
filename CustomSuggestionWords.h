#ifndef CUSTOMSUGGESTIONWORDS_H
#define CUSTOMSUGGESTIONWORDS_H

#include <QStringList>
#include "TamilPhonemeMap.h"

class CustomSuggestionWords
{
public:
    static QStringList getList()
    {
        QStringList list;

        QMap<QString, QString> map = TamilPhonemeMap::getMap();

        for (auto it = map.constBegin(); it != map.constEnd(); ++it) {
            list << it.key();   // only English keys
        }

        return list;
    }
};

#endif // CUSTOMSUGGESTIONWORDS_H

