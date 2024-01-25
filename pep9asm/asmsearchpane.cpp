
// File: searchPane.cpp
/*
    Pep8-1 is a virtual machine for writing machine language and assembly
    language programs.

    Copyright (C) 2023  P. Bélisle, École de Technologie Supérieure (Éts)
                                    (Québec, Canada)
    *** with the help of ChatGpt4

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/
#include "searchpane.h"
#include <QScrollBar>
#include <QRect>
#include <QMessageBox>
#include <QLineEdit>
#include <iostream>

#include "sourcecodepane.h"

#define SIZE_EDIT_LINE 300

// searchpane.cpp
SearchPane::SearchPane(QWidget *parent,
                       QTextEdit* textEdit,
                       QList<QString> *lastSearchList,
                       QList<QString> *lastReplaceList) : QDialog(parent){


    this->textEdit = textEdit;
    this->textEdit->ensureCursorVisible();

    this->lastSearchList = lastSearchList;
    this->lastReplaceList = lastReplaceList;

    // The list 'foundPositions' is already an instance of QList.
    this->currentFindIndex = 0;

    setWindowFlags(windowFlags() | Qt::WindowMaximizeButtonHint);
    setWindowFlags(windowFlags() | Qt::WindowMinimizeButtonHint);

    initComponents();
}


static void loadComboBox(QComboBox* comboBox,
                         QList<QString> *lastList);

void SearchPane::updateComboList(QComboBox* comboBox){

    QString text = comboBox->currentText();

    if(!this->lastSearchList->contains(text)){

        this->lastSearchList->append(text);

        comboBox->addItem(text);
    }
}

static void initComboBox(QComboBox* comboBox){

    comboBox->setMinimumWidth(SIZE_EDIT_LINE);
    comboBox->setEditable(true);
}


void SearchPane::addComponentsToLayout(){

    // Create boxLayout and add components to it,
    QBoxLayout *layout = new QVBoxLayout(this);

    layout->addWidget(new QLabel("  "));

    layout->addWidget(new QLabel("Search : "));
    layout->addWidget(comboLastSearchBox);

    layout->addWidget(new QLabel("Replace : "));
    layout->addWidget(comboLastReplaceBox);

    layout->addWidget(nextButton);
    layout->addWidget(previousButton);

    layout->addWidget(replaceButton);
    layout->addWidget(replaceAllButton);

    layout->addWidget(caseSensitiveCheckBox);

    layout->addWidget(statusBar);

}

// II used this good practive to keep out the 'initComponents' code from the constructor.
// if eventuelly we would like to 'initComponents' again, it's easy (by exemple).
// ÉTS H2024
void SearchPane::initComponents(){

    /**** Components are declare by order of apperance in the pane. ****/

    this->comboLastSearchBox = new QComboBox(this);
    this->comboLastReplaceBox = new QComboBox(this);

    initComboBox(this->comboLastSearchBox);
    initComboBox(this->comboLastReplaceBox);

    addComponentsToLayout();

    loadComboBox(this->comboLastSearchBox, this->lastSearchList);
    loadComboBox(this->comboLastReplaceBox, this->lastReplaceList);

    connectAll();

    // Focus on searched edit text.
    this->comboLastSearchBox->lineEdit()->setFocus();


}

void SearchPane::connectAll(){

    connect(nextButton, &QPushButton::clicked, this, &SearchPane::goToNext);
    connect(previousButton, &QPushButton::clicked, this, &SearchPane::goToPrevious);
    // Connect directly at the right slot at the click.

    connect(replaceButton, &QPushButton::clicked, this, [this]() {

        handleButtonLogic(&SearchPane::replaceAndFindNext);
    });

    connect(replaceAllButton, &QPushButton::clicked, this, [this]() {

        handleButtonLogic(&SearchPane::replaceAll);

    });

    // Nothing to connect for the checkBox.


}
static void loadComboBox(QComboBox* comboBox, QList<QString> *lastList){

    int size = lastList->size();

    if(comboBox->count() == 0){

        // The first one lets empty.
        comboBox->addItem("");
    }

    if(size){

        for(int i = 0; i < size; i++){

            comboBox->addItem(lastList->at(i));
        }
    }
}

void SearchPane::handleButtonLogic(ActionFunction action) {


    if (this->comboLastSearchBox->currentText().isEmpty()) {

        this->statusBar->setText("You must put some in the search box");
        this->comboLastSearchBox->lineEdit()->setFocus();
    }
    else if (this->comboLastReplaceBox->currentText().isEmpty()) {

        this->statusBar->setText("You must put some in the replace box");
        this->comboLastReplaceBox->lineEdit()->setFocus();
    }
    else {
        updateComboList(this->comboLastSearchBox);
        updateComboList(this->comboLastReplaceBox);

        // We know here that at least goToNext() was called before.
        (this->*action)(this->comboLastSearchBox->currentText(),
                        this->comboLastReplaceBox->currentText());
    }
}


QTextDocument::FindFlags getOptions(bool caseSensitive){

    QTextDocument::FindFlags options;

    if (caseSensitive) {

        options |= QTextDocument::FindCaseSensitively;
    }

    return options;
}

// Used to highlight the text who correspond to 'searchText' with
// case sensitive considération.  It stores cursors in 'foundPositions' list
// by the same occasion.
void  SearchPane::findPositionsAndHighlightCursors(QTextDocument* document,
                                                  QString searchText,
                                                  bool caseSensitive){


    QTextCursor highlightCursor(document);
    QTextCursor cursor(document);

    this->foundPositions.clear();


    // Start an edit  block for cursors encontered ho matches the search string.
    cursor.beginEditBlock();{

        QTextCharFormat plainFormat(highlightCursor.charFormat());
        QTextCharFormat colorFormat = plainFormat;
        colorFormat.setBackground(Qt::yellow);

        // Simple linear search.
        while (!highlightCursor.isNull() && !highlightCursor.atEnd()) {

            // getOtions is to care about case sensitivity.
            highlightCursor = document->find(searchText,
                                             highlightCursor,
                                             getOptions(caseSensitive));

            if (!highlightCursor.isNull()) {

                highlightCursor.setCharFormat(colorFormat);
                this->foundPositions.append(highlightCursor);
            }
        }      
    }cursor.endEditBlock();

}


/**************SLOTS*****************/
void SearchPane::goToNext() {

    updateComboList(this->comboLastSearchBox);

    initNextSearch();

    if(this->currentFindIndex > -1){

        QTextCharFormat fmt;

        // Put the index at the next cursor int the list and obtain it.
        QTextCursor newCursor = this->foundPositions.at(this->currentFindIndex);

        newCursor.beginEditBlock();{

            fmt.setBackground(Qt::green);
            newCursor.mergeCharFormat(fmt);
            this->textEdit->setTextCursor(newCursor);

        }newCursor.endEditBlock();

        updateTextEdit(newCursor);
    }


}

void SearchPane::initNextSearch(){

    // Make sure the list is about the last LineEdit text and
    // the currentfindIndex well placed.
    onNextPressed();

    updateStatusBar();
}

int getNearestNextCursor(QTextEdit *textEdit, QList<QTextCursor> *foundPositions);

void SearchPane::onNextPressed() {

    resetHighlighting();

    // Get the document from the textEdit.
    QTextDocument *document = this->textEdit->document();

    // Charge the list, found positions of text ans highlight them.
    this->findPositionsAndHighlightCursors(document,
                                           this->comboLastSearchBox->currentText(),
                                           this->caseSensitiveCheckBox->isChecked());

    // Move the currentFindIndex at the nearest 'text' of the actuel position of cursor.
    this->currentFindIndex = getNearestNextCursor(this->textEdit,
                                                  &this->foundPositions);
}

int getNearestNextCursor(QTextEdit *textEdit, QList<QTextCursor> *foundPositions){

    int size = foundPositions->size();
    int currentFindIndex = -1;

    if(size){

        currentFindIndex = 0;

        QTextCursor cursor = textEdit->textCursor();

        int posCur = cursor.position();

        // Check with the first one to start
        QTextCursor currentCursor = foundPositions->at(0);

        // If currentCursor.position() > posCur then it's the right cursor
        // at position 0, nothing to do.
        if(currentCursor.position() <= posCur){

            do{

                currentFindIndex++;

                if(currentFindIndex < size){

                    currentCursor = foundPositions->at(currentFindIndex);
                }

            } while(currentFindIndex < size && currentCursor.position() <= posCur);

            // Circular
            if(currentFindIndex == size){

                currentFindIndex = 0;
            }

        }


    }

    return currentFindIndex;
}

void SearchPane::updateTextEdit(QTextCursor cursor){

    QRect cursorRect = textEdit->cursorRect(cursor);

    int cursorY = cursorRect.center().y();
    int centerY = textEdit->viewport()->height() / 2;
    int cursorX = cursorRect.center().x();

    QScrollBar *scrollBar = this->textEdit->verticalScrollBar();
    scrollBar->setValue(scrollBar->value() + cursorY - centerY);

    scrollBar = textEdit->horizontalScrollBar();
    int newValue = qMax(0, cursorX - (textEdit->viewport()->width() / 2));
    scrollBar->setValue(newValue);

    scrollBar = nullptr;

    this->textEdit->activateWindow();
}

void SearchPane::updateStatusBar(){

    // Nothing to do for an empty lineEdit text.
    if (this->foundPositions.isEmpty()) {

        this->statusBar->setText("Nothing found");
    }
    else {

        this->statusBar->clear();

    }
}

// Go to the previous occur of the highlighted text in the document.
void SearchPane::goToPrevious() {

    updateComboList(this->comboLastReplaceBox);

    initPrevSearch();

    if(this->currentFindIndex > -1){

        // Reset the actual text in yellow
        QTextCharFormat fmt;

        // Get the previous text ans put it green.
        QTextCursor cursor = this->foundPositions.at(this->currentFindIndex);

        cursor.beginEditBlock();{

            fmt.setBackground(Qt::green);
            cursor.setCharFormat(fmt);
            this->textEdit->setTextCursor(cursor);

        }cursor.endEditBlock();

        updateTextEdit(cursor);
    }
}

void SearchPane::initPrevSearch(){

    // Make sure the list is about the last lineEdit text and
    // the currentfindIndex well placed.
    onPreviousPressed();

    updateStatusBar();
}

int getNearestPrevCursor(QTextEdit *textEdit, const QList<QTextCursor> *foundPositions);

void SearchPane::onPreviousPressed() {

    resetHighlighting();

    // Get the document from the textEdit.
    QTextDocument *document = this->textEdit->document();

    // Charge the list, found positions of text and highlight them.
    this->findPositionsAndHighlightCursors(document,
                                           this->comboLastSearchBox->currentText(),
                                           this->caseSensitiveCheckBox->isChecked());

    this->currentFindIndex = getNearestPrevCursor(this->textEdit,
                                                  &this->foundPositions);

}

int getNearestPrevCursor(QTextEdit *textEdit, const QList<QTextCursor> *foundPositions){


    int size = foundPositions->size();
    int currentFindIndex = - 1;

    if(size){

        currentFindIndex = size - 1;

        QTextCursor cursor = textEdit->textCursor();

        int posCur = cursor.position();

        // Check with the last one to start
        QTextCursor currentCursor = foundPositions->at(size - 1);


        // If currentCursor.position() > posCur then it's the right cursor
        // at position size - 1, nothing to do.
        if(currentCursor.position() >= posCur){

            do{

                currentFindIndex--;

                if(currentFindIndex >= 0){

                    currentCursor = foundPositions->at(currentFindIndex);
                }

            } while(currentFindIndex >= 0 && currentCursor.position() >= posCur);

            // Circular
            if(currentFindIndex < 0){

                currentFindIndex = size - 1;
            }

        }
    }

    return currentFindIndex;
}

// We know here that this->foundPositions, searchText and replaceText are not empty.
void SearchPane::replaceAndFindNext(const QString &searchText,
                                    const QString &replaceText) {

    goToNext();

    if(this->currentFindIndex > -1){

        QTextCursor cursor = this->foundPositions.at(this->currentFindIndex);

        cursor.beginEditBlock();{

            if (!cursor.isNull()){

                cursor.removeSelectedText();
                cursor.insertText(replaceText);
            }

        }cursor.endEditBlock();

        goToNext();

        updateTextEdit(cursor);


    }
}

void SearchPane::replaceAll(const QString &searchText, const QString &replaceText) {

    QTextDocument* document = this->textEdit->document();

    QTextCursor cursor(document);

    cursor.beginEditBlock();{

        int size = this->lastSearchList->size();

        cursor.setPosition(0);

        // Replace all cursors in the foundPositions cursor list
        // who comes from the last goToNext() or goToPrevious call.
        for(int i = 0; i < size; i++){

            replaceAndFindNext(searchText, replaceText);
        }

        QMessageBox::information(this, "Replacement done", "All \"" +
                                       searchText + "\" has been replaced.");

    }cursor.endEditBlock();



}

void SearchPane::resetHighlightingExceptLast() {

    resetHighlighting();

    QTextCursor currentCursor = this->textEdit->textCursor();

    currentCursor.beginEditBlock();{

        QTextCharFormat highlightFormat;
        highlightFormat.setBackground(Qt::green);

        currentCursor.mergeCharFormat(highlightFormat);

    }currentCursor.endEditBlock();

}

void SearchPane::resetHighlighting() {

    QTextCursor cursor(this->textEdit->document());

    cursor.beginEditBlock();{

        cursor.select(QTextCursor::Document);

        QTextCharFormat plainFormat;
        plainFormat.setBackground(Qt::white);

        cursor.setCharFormat(plainFormat);

    }cursor.endEditBlock();
}

void SearchPane::closeEvent(QCloseEvent *event) {

    resetHighlightingExceptLast();

    this->textEdit->activateWindow();
    this->textEdit->setFocus();
    this->textEdit->update();


    QDialog::closeEvent(event);
}
