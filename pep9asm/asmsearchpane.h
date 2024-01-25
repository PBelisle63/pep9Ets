// File: searchPane.h
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

#ifndef SEARCHPANE_H
#define SEARCHPANE_H

#include <QWidget>

#include <QList>        // Retains a list of Cursor of foundPositions of the searched text.

#include <QDialog>      // Parent of this pane.

#include <QPushButton>

#include <QCheckBox>    // Ask for case sensitivity or not.

#include <QTextEdit>    // Receved by the constructor, it's the textEdit to search in.

#include <QComboBox>    // Contains edit lines for last texts searched and replaced

#include <QVBoxLayout>  // Contains all components (it's in the DialogBox).

#include <QLabel>

#include <QKeyEvent>

// Shortcut
namespace Ui {
class SearchPane;
}


class SearchPane : public QDialog {
    Q_OBJECT


public:

    // We only need the constructor to show and manage this pane.
    explicit SearchPane(QWidget *parent = nullptr,
                        QTextEdit* textEdit = nullptr,
                        QList<QString> *lastSearchList = nullptr,
                        QList<QString> *lastReplaceList = nullptr);

    void SearchPane::closeEvent(QCloseEvent *event);
    // Post : The pane is closed and the last found text selected is still shown.

private:

    QComboBox *comboLastSearchBox = new QComboBox();
    QComboBox *comboLastReplaceBox = new QComboBox();

    QList<QString> *lastSearchList;
    QList<QString> *lastReplaceList;
    // These lists will potentially be modified by this searchPane.
    // ÉTS H2024

    // Button control
    QPushButton *nextButton = new QPushButton("Next", this);
    QPushButton *previousButton = new QPushButton("Previous", this);

    QPushButton *replaceButton = new QPushButton("Replace and find next", this);
    QPushButton *replaceAllButton = new QPushButton("Replace all", this);

    QCheckBox *caseSensitiveCheckBox = new QCheckBox("Case sensitive search", this);

    QTextEdit* textEdit;
    // A text box to search in.

    QLabel *statusBar = new QLabel(this);


    // Show message at the bottom of the searchPane.

    // Remind all positions found int the text who match the 'last' search.
    QList<QTextCursor> foundPositions;
    int currentFindIndex;
    // ÉTS 2024


    /********
     *        Functions for local utilisation but need 'this' with a global access
     *        to members of SearchPane.
    **********/

    void connectAll();
    void addComponentsToLayout();

    void initComponents();

    void findPositionsAndHighlightCursors(QTextDocument* document,
                                          QString searchText,
                                          bool caseSensitive);

    void resetHighlightingExceptLast();
    void resetHighlighting();

    void updateStatusBar();
    // Show if nothing found in the statusBar or clear it.

    void updateTextEdit(QTextCursor cursor);
    // Give the focus to the parent and place his scrollBar(s).

    typedef void (SearchPane::*ActionFunction)(const QString&, const QString&);
    // To avoid code repetition for 'replace' functions.

    void handleButtonLogic(ActionFunction action);
    // 'action' is the function to call after some checks of the comboBoxs current text.


/*****************SLOTS***********************/
private slots:

    void initNextSearch();
    void onNextPressed();
    void goToNext();
    // Put the highlight on the next text find or give an
    // advice that nothing found.

    void initPrevSearch();
    void onPreviousPressed();
    void goToPrevious();
    // Put the highlight on the previous text find or give an
    // advice that nothing found.

    void replaceAndFindNext(const QString &searchText, const QString &replaceText);
    void replaceAll(const QString &searchText, const QString &replaceText);
    // If the replacement text is empty, nothing is replaced for
    // both 'replace' functions.

    void updateComboList(QComboBox* comboBox);
    // Append the text selected ine the comboBox at th lastSearchList.

};


#endif // SEARCHPANE_H
