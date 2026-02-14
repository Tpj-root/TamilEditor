#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTextEdit>
#include <QListWidget>
#include <QCompleter>
#include <QStandardItemModel>
#include "englishtotamilconverter.h"
#include "suggestionengine.h"

QT_BEGIN_NAMESPACE
class QTextEdit;
class QListWidget;
class QCompleter;
class QStandardItemModel;
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT
public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

protected:
    bool eventFilter(QObject *obj, QEvent *event) override;   // <-- NEW

private slots:
    void onLeftTextChanged();
    void onCursorPositionChanged();
    void insertSuggestion(const QString &suggestion);
    void onBottomSuggestionClicked(QListWidgetItem *item);
    void saveLeft();
    void saveRight();
    void saveBoth();

private:
    void setupUi();
    void setupCompleter();
    void setupBottomSuggestionList();
    void updateBottomSuggestions(const QString &prefix);
    QString romanToTamil(const QString &roman) const;

    QTextEdit *leftEdit;
    QTextEdit *rightEdit;
    QListWidget *suggestionList;
    QCompleter *completer;
    QStandardItemModel *completerModel;
    EnglishToTamilConverter *converter;
    SuggestionEngine *suggestionEngine;
    QStringList suggestionWordList;
    bool updatingCompleter;
};

#endif // MAINWINDOW_H
