#include "englishtotamilconverter.h"
#include "TamilPhonemeMap.h"


EnglishToTamilConverter::EnglishToTamilConverter(QObject *parent)
    : QObject(parent), maxKeyLength(0)
{

    // New update
    // No touching constructor again
    phonemeMap = TamilPhonemeMap::getMap();


    for (auto it = phonemeMap.constBegin(); it != phonemeMap.constEnd(); ++it) {
        if (it.key().length() > maxKeyLength)
            maxKeyLength = it.key().length();
    }
}

QString EnglishToTamilConverter::convert(const QString &roman) const
{
    QString result;
    int i = 0;
    int n = roman.length();

    while (i < n) {
        bool matched = false;
        // try longest possible match first
        for (int len = maxKeyLength; len >= 1; --len) {
            if (i + len <= n) {
                QString key = roman.mid(i, len);
                if (phonemeMap.contains(key)) {
                    result += phonemeMap.value(key);
                    i += len;
                    matched = true;
                    break;
                }
            }
        }
        if (!matched) {
            // no mapping – keep original character
            result += roman[i];
            ++i;
        }
    }
    return result;
}

void EnglishToTamilConverter::addMapping(const QString &phoneme, const QString &tamilChar)
{
    phonemeMap[phoneme] = tamilChar;
    if (phoneme.length() > maxKeyLength)
        maxKeyLength = phoneme.length();
}