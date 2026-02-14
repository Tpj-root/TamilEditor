#ifndef SUGGESTIONENGINE_H
#define SUGGESTIONENGINE_H

#include <QObject>
#include <QStringList>

class SuggestionEngine : public QObject
{
    Q_OBJECT
public:
    explicit SuggestionEngine(QObject *parent = nullptr);

    void setWordList(const QStringList &words);
    QStringList suggest(const QString &prefix) const;

private:
    QStringList wordList;
};

#endif // SUGGESTIONENGINE_H