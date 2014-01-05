//
//  LogDialog.cpp
//  interface
//
//  Created by Stojce Slavkovski on 12/12/13.
//  Copyright (c) 2013 High Fidelity, Inc. All rights reserved.
//

#include <QDesktopWidget>
#include <QTextBlock>
#include <QtGui>

#include "SharedUtil.h"
#include "ui/LogDialog.h"

const int TOP_BAR_HEIGHT = 46;
const int INITIAL_WIDTH = 720;
const int MINIMAL_WIDTH = 570;
const int ELEMENT_MARGIN = 7;
const int ELEMENT_HEIGHT = 32;
const int SEARCH_BUTTON_LEFT = 25;
const int SEARCH_BUTTON_WIDTH = 20;
const int SEARCH_TEXT_WIDTH = 240;
const int CHECKBOX_MARGIN = 12;
const int CHECKBOX_WIDTH = 140;
const int REVEAL_BUTTON_WIDTH = 122;
const float INITIAL_HEIGHT_RATIO = 0.6f;
const QString HIGHLIGHT_COLOR = "#3366CC";

int qTextCursorMeta = qRegisterMetaType<QTextCursor>("QTextCursor");
int qTextBlockMeta = qRegisterMetaType<QTextBlock>("QTextBlock");

LogDialog::LogDialog(QWidget* parent, AbstractLoggerInterface* logger) : QDialog(parent, Qt::Dialog) {

    _logger = logger;
    setWindowTitle("Log");
    setAttribute(Qt::WA_DeleteOnClose);

    switchToResourcesParentIfRequired();
    QFile styleSheet("resources/styles/log_dialog.qss");
    if (styleSheet.open(QIODevice::ReadOnly)) {
        setStyleSheet(styleSheet.readAll());
    }

    initControls();

    QDesktopWidget desktop;
    QRect screen = desktop.screenGeometry();
    resize(INITIAL_WIDTH, static_cast<int>(screen.height() * INITIAL_HEIGHT_RATIO));
    move(screen.center() - rect().center());
    setMinimumWidth(MINIMAL_WIDTH);
}

LogDialog::~LogDialog() {
    deleteLater();
}

void LogDialog::initControls() {

    int left;
    _searchButton = new QPushButton(this);
    // set object name for css styling
    _searchButton->setObjectName("searchButton");
    left = SEARCH_BUTTON_LEFT;
    _searchButton->setGeometry(left, ELEMENT_MARGIN, SEARCH_BUTTON_WIDTH, ELEMENT_HEIGHT);
    left += SEARCH_BUTTON_WIDTH;
    _searchButton->show();
    connect(_searchButton, SIGNAL(clicked()), SLOT(handleSearchButton()));

    _searchTextBox = new QLineEdit(this);
    // disable blue outline in Mac
    _searchTextBox->setAttribute(Qt::WA_MacShowFocusRect, false);
    _searchTextBox->setGeometry(left, ELEMENT_MARGIN, SEARCH_TEXT_WIDTH, ELEMENT_HEIGHT);
    left += SEARCH_TEXT_WIDTH + CHECKBOX_MARGIN;
    _searchTextBox->show();
    connect(_searchTextBox, SIGNAL(textChanged(QString)), SLOT(handleSearchTextChanged(QString)));

    _extraDebuggingBox = new QCheckBox("Extra debugging", this);
    _extraDebuggingBox->setGeometry(left, ELEMENT_MARGIN, CHECKBOX_WIDTH, ELEMENT_HEIGHT);
    if (_logger->extraDebugging()) {
        _extraDebuggingBox->setCheckState(Qt::Checked);
    }
    _extraDebuggingBox->show();
    connect(_extraDebuggingBox, SIGNAL(stateChanged(int)), SLOT(handleExtraDebuggingCheckbox(int)));

    _revealLogButton = new QPushButton("Reveal log file", this);
    // set object name for css styling
    _revealLogButton->setObjectName("revealLogButton");
    _revealLogButton->show();
    connect(_revealLogButton, SIGNAL(clicked()), SLOT(handleRevealButton()));

    _logTextBox = new QPlainTextEdit(this);
    _logTextBox->setReadOnly(true);
    _logTextBox->show();
    _highlighter = new KeywordHighlighter(_logTextBox->document());

}

void LogDialog::showEvent(QShowEvent*)  {
    connect(_logger, SIGNAL(logReceived(QString)), this, SLOT(appendLogLine(QString)));
    showLogData();
}

void LogDialog::resizeEvent(QResizeEvent*) {
    _logTextBox->setGeometry(0, TOP_BAR_HEIGHT, width(), height() - TOP_BAR_HEIGHT);
    _revealLogButton->setGeometry(width() - ELEMENT_MARGIN - REVEAL_BUTTON_WIDTH,
                                  ELEMENT_MARGIN,
                                  REVEAL_BUTTON_WIDTH,
                                  ELEMENT_HEIGHT);
}

void LogDialog::appendLogLine(QString logLine) {
    if (isVisible()) {
        pthread_mutex_lock(& _mutex);
        if (logLine.contains(_searchTerm, Qt::CaseInsensitive)) {
            _logTextBox->appendPlainText(logLine.simplified());
        }
        pthread_mutex_unlock(& _mutex);
        _logTextBox->ensureCursorVisible();
    }
}

void LogDialog::handleSearchButton() {
    _searchTextBox->setFocus();
}

void LogDialog::handleRevealButton() {
    _logger->locateLog();
}

void LogDialog::handleExtraDebuggingCheckbox(const int state) {
    _logger->setExtraDebugging(state != 0);
}

void LogDialog::handleSearchTextChanged(const QString searchText) {
    _searchTerm = searchText;
    _highlighter->keyword = searchText;
    showLogData();
}

void LogDialog::showLogData() {
    _logTextBox->clear();
    pthread_mutex_lock(& _mutex);
    QStringList _logData = _logger->getLogData();
    for (int i = 0; i < _logData.size(); ++i) {
        appendLogLine(_logData[i]);
    }

    pthread_mutex_unlock(& _mutex);
}

KeywordHighlighter::KeywordHighlighter(QTextDocument *parent) : QSyntaxHighlighter(parent), keywordFormat() {
    keywordFormat.setForeground(QColor(HIGHLIGHT_COLOR));
}

void KeywordHighlighter::highlightBlock(const QString &text) {

    if (keyword.isNull() || keyword.isEmpty()) {
        return;
    }

    int index = text.indexOf(keyword, 0, Qt::CaseInsensitive);
    int length = keyword.length();

    while (index >= 0) {
        setFormat(index, length, keywordFormat);
        index = text.indexOf(keyword, index + length, Qt::CaseInsensitive);
    }
}