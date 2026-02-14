#include "mainwindow.h"
#include <QSplitter>
#include <QVBoxLayout>
#include <QStatusBar>
#include <QMenuBar>
#include <QMenu>
#include <QAction>
#include <QFileDialog>
#include <QMessageBox>
#include <QTextStream>
#include <QCompleter>
#include <QStringListModel>
#include <QStandardItemModel>
#include <QStandardItem>
#include <QTextCursor>
#include <QRegularExpression>
#include <QAbstractItemView>
#include <QKeyEvent>          // for key handling
#include <QCoreApplication>   // for sendEvent
#include <QDebug>
#include "CustomSuggestionWords.h"


MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent), updatingCompleter(false)
{
    converter = new EnglishToTamilConverter(this);
    suggestionEngine = new SuggestionEngine(this);

    // // Suggestion word list
    // suggestionWordList << "aa" << "aam" << "aai" << "ii"
    //                    << "uu" << "ee" << "oo" << "ai" << "au"
    //                    << "aaiiuu";

    // suggestionEngine->setWordList(suggestionWordList);


    // Suggestion word list 2.0
    // #include "CustomSuggestionWords.h"
    suggestionWordList = CustomSuggestionWords::getList();
    suggestionEngine->setWordList(suggestionWordList);


    // GUI
    setupUi();
    setupCompleter();
    setupBottomSuggestionList();

    // Install event filter for keyboard navigation in completer
    leftEdit->installEventFilter(this);   // <-- NEW

    // Connections
    connect(leftEdit, &QTextEdit::textChanged,
            this, &MainWindow::onLeftTextChanged);
    connect(leftEdit, &QTextEdit::cursorPositionChanged,
            this, &MainWindow::onCursorPositionChanged);
    connect(completer, QOverload<const QString&>::of(&QCompleter::activated),
            this, &MainWindow::insertSuggestion);

    onLeftTextChanged();
    statusBar()->showMessage(tr("Ready"));
}

MainWindow::~MainWindow() {}

// ------------------------------------------------------------
// Event Filter – Keyboard navigation for completer popup
// ------------------------------------------------------------
bool MainWindow::eventFilter(QObject *obj, QEvent *event)
{
    if (obj == leftEdit && event->type() == QEvent::KeyPress) {
        QKeyEvent *keyEvent = static_cast<QKeyEvent*>(event);
        if (completer && completer->popup() && completer->popup()->isVisible()) {
            switch (keyEvent->key()) {
            case Qt::Key_Up:
            case Qt::Key_Down:
            case Qt::Key_Enter:
            case Qt::Key_Return:
            case Qt::Key_Tab:
            case Qt::Key_Backtab:
                // Forward the event to the completer popup
                QCoreApplication::sendEvent(completer->popup(), event);
                return true;   // consume event, prevent default handling
            case Qt::Key_Escape:
                completer->popup()->hide();
                return true;
            default:
                break;
            }
        }
    }
    return QMainWindow::eventFilter(obj, event);
}

// ------------------------------------------------------------
// GUI Setup
// ------------------------------------------------------------
void MainWindow::setupUi()
{
    setWindowTitle(tr("Tamil Phonetic Editor"));
    resize(900, 700);

    QSplitter *mainSplitter = new QSplitter(Qt::Vertical, this);
    QSplitter *editorSplitter = new QSplitter(Qt::Horizontal, mainSplitter);

    leftEdit = new QTextEdit(editorSplitter);
    rightEdit = new QTextEdit(editorSplitter);
    editorSplitter->addWidget(leftEdit);
    editorSplitter->addWidget(rightEdit);
    editorSplitter->setStretchFactor(0, 1);
    editorSplitter->setStretchFactor(1, 1);

    // Bottom suggestion list
    suggestionList = new QListWidget(mainSplitter);
    suggestionList->setMaximumHeight(150);
    suggestionList->setSelectionMode(QAbstractItemView::SingleSelection);

    mainSplitter->addWidget(editorSplitter);
    mainSplitter->addWidget(suggestionList);
    mainSplitter->setStretchFactor(0, 1);
    mainSplitter->setStretchFactor(1, 0);

    setCentralWidget(mainSplitter);

    QFont font("Arial", 12);
    leftEdit->setFont(font);
    rightEdit->setFont(font);
    rightEdit->setReadOnly(true);
    rightEdit->setStyleSheet("QTextEdit { selection-background-color: yellow; }");

    // File menu
    QMenu *fileMenu = menuBar()->addMenu(tr("&File"));
    QAction *saveLeftAction = fileMenu->addAction(tr("&Save Left..."));
    QAction *saveRightAction = fileMenu->addAction(tr("Sa&ve Right..."));
    QAction *saveBothAction = fileMenu->addAction(tr("Save &Both..."));

    connect(saveLeftAction, &QAction::triggered, this, &MainWindow::saveLeft);
    connect(saveRightAction, &QAction::triggered, this, &MainWindow::saveRight);
    connect(saveBothAction, &QAction::triggered, this, &MainWindow::saveBoth);
}

// ------------------------------------------------------------
// Popup Completer (shows "word [Tamil]")
// ------------------------------------------------------------
void MainWindow::setupCompleter()
{
    completerModel = new QStandardItemModel(this);

    for (const QString &word : suggestionWordList) {
        QStandardItem *item = new QStandardItem();
        QString tamil = romanToTamil(word);
        QString displayText = QString("%1 [%2]").arg(word, tamil);
        item->setText(displayText);
        item->setData(word, Qt::UserRole);   // plain roman word for completion
        completerModel->appendRow(item);
    }

    completer = new QCompleter(this);
    completer->setModel(completerModel);
    completer->setCompletionRole(Qt::UserRole);
    completer->setCaseSensitivity(Qt::CaseInsensitive);
    completer->setCompletionMode(QCompleter::PopupCompletion);
    completer->setWidget(leftEdit);   // required for popup positioning
}

// ------------------------------------------------------------
// Bottom Suggestion List
// ------------------------------------------------------------
void MainWindow::setupBottomSuggestionList()
{
    suggestionList->clear();
    connect(suggestionList, &QListWidget::itemClicked,
            this, &MainWindow::onBottomSuggestionClicked);
}

void MainWindow::updateBottomSuggestions(const QString &prefix)
{
    suggestionList->clear();
    if (prefix.isEmpty())
        return;

    QStringList matches = suggestionEngine->suggest(prefix);
    for (const QString &word : matches) {
        QString tamil = romanToTamil(word);
        QString display = QString("%1 [%2]").arg(word, tamil);
        suggestionList->addItem(display);
    }
}

// ------------------------------------------------------------
// Convert Roman word to Tamil (via converter)
// ------------------------------------------------------------
QString MainWindow::romanToTamil(const QString &roman) const
{
    return converter->convert(roman);
}

// ------------------------------------------------------------
// Slot: left text changed → update right side
// ------------------------------------------------------------
void MainWindow::onLeftTextChanged()
{
    QString leftText = leftEdit->toPlainText();
    QString tamilText = converter->convert(leftText);
    rightEdit->setPlainText(tamilText);
}

// ------------------------------------------------------------
// Slot: cursor moved → show popup completer + update bottom list
// ------------------------------------------------------------
void MainWindow::onCursorPositionChanged()
{
    if (updatingCompleter) return;
    if (!completer) return;

    QTextCursor cursor = leftEdit->textCursor();
    QString text = leftEdit->toPlainText();
    int pos = cursor.position();

    QRegularExpression wordRegex("\\b\\w+\\b");
    QRegularExpressionMatchIterator it = wordRegex.globalMatch(text);
    QString currentWord;
    while (it.hasNext()) {
        QRegularExpressionMatch match = it.next();
        if (match.capturedStart() <= pos && match.capturedEnd() >= pos) {
            currentWord = match.captured();
            break;
        }
    }

    // --- Popup completer ---
    if (!currentWord.isEmpty()) {
        completer->setCompletionPrefix(currentWord);
        if (completer->completionCount() > 0) {
            completer->complete();
        } else {
            if (completer->popup()) completer->popup()->hide();
        }
    } else {
        if (completer->popup()) completer->popup()->hide();
    }

    // --- Bottom suggestion list ---
    updateBottomSuggestions(currentWord);
}

// ------------------------------------------------------------
// Insert suggestion from popup completer
// ------------------------------------------------------------
void MainWindow::insertSuggestion(const QString &suggestion)
{
    updatingCompleter = true;

    QTextCursor cursor = leftEdit->textCursor();
    QString text = leftEdit->toPlainText();
    int pos = cursor.position();

    QRegularExpression wordRegex("\\b\\w+\\b");
    QRegularExpressionMatchIterator it = wordRegex.globalMatch(text);
    bool replaced = false;
    while (it.hasNext()) {
        QRegularExpressionMatch match = it.next();
        if (match.capturedStart() <= pos && match.capturedEnd() >= pos) {
            cursor.setPosition(match.capturedStart());
            cursor.setPosition(match.capturedEnd(), QTextCursor::KeepAnchor);
            cursor.insertText(suggestion);
            replaced = true;
            break;
        }
    }

    if (!replaced) {
        cursor.insertText(suggestion);
    }

    updatingCompleter = false;
    onLeftTextChanged();
}

// ------------------------------------------------------------
// Insert suggestion from bottom list (click)
// ------------------------------------------------------------
void MainWindow::onBottomSuggestionClicked(QListWidgetItem *item)
{
    QString display = item->text();
    QString roman = display.left(display.indexOf('[')).trimmed();

    QTextCursor cursor = leftEdit->textCursor();
    cursor.insertText(roman);
}

// ------------------------------------------------------------
// File save operations (unchanged)
// ------------------------------------------------------------
void MainWindow::saveLeft()
{
    QString fileName = QFileDialog::getSaveFileName(this,
        tr("Save Left Editor Content"), "", tr("Text Files (*.txt);;All Files (*)"));
    if (fileName.isEmpty()) return;

    QFile file(fileName);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QMessageBox::warning(this, tr("Error"),
            tr("Cannot write file %1:\n%2.").arg(fileName).arg(file.errorString()));
        return;
    }

    QTextStream out(&file);
    out << leftEdit->toPlainText();
    file.close();
    statusBar()->showMessage(tr("Left editor saved to %1").arg(fileName), 3000);
}

void MainWindow::saveRight()
{
    QString fileName = QFileDialog::getSaveFileName(this,
        tr("Save Right Editor Content"), "", tr("Text Files (*.txt);;All Files (*)"));
    if (fileName.isEmpty()) return;

    QFile file(fileName);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QMessageBox::warning(this, tr("Error"),
            tr("Cannot write file %1:\n%2.").arg(fileName).arg(file.errorString()));
        return;
    }

    QTextStream out(&file);
    out << rightEdit->toPlainText();
    file.close();
    statusBar()->showMessage(tr("Right editor saved to %1").arg(fileName), 3000);
}

void MainWindow::saveBoth()
{
    saveLeft();
    saveRight();
}