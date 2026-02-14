#ifndef ENGLISHTOTAMILCONVERTER_H
#define ENGLISHTOTAMILCONVERTER_H

#include <QObject>
#include <QMap>
#include <QString>

class EnglishToTamilConverter : public QObject
{
    Q_OBJECT
public:
    explicit EnglishToTamilConverter(QObject *parent = nullptr);

    QString convert(const QString &roman) const;
    void addMapping(const QString &phoneme, const QString &tamilChar);

private:
    QMap<QString, QString> phonemeMap;
    int maxKeyLength;
};

#endif // ENGLISHTOTAMILCONVERTER_H