#include "suggestionengine.h"

SuggestionEngine::SuggestionEngine(QObject *parent) : QObject(parent)
{
}

void SuggestionEngine::setWordList(const QStringList &words)
{
    wordList = words;
    wordList.sort();
}

QStringList SuggestionEngine::suggest(const QString &prefix) const
{
    QStringList result;
    for (const QString &word : wordList) {
        if (word.startsWith(prefix, Qt::CaseInsensitive))
            result << word;
    }
    return result;
}