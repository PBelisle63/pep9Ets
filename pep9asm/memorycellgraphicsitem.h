// File: memorycellgraphicsitem.h
/*
    Pep9 is a virtual machine for writing machine language and assembly
    language programs.

    Copyright (C) 2009  J. Stanley Warford, Pepperdine University

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

#ifndef MEMORYCELLGRAPHICSITEM_H
#define MEMORYCELLGRAPHICSITEM_H

#include <QGraphicsItem>
#include "enu.h"
class AMemoryDevice;

int cellSize(Enu::ESymbolFormat symbolFormat);
// This is used exclusively in the memoryTracePane/memoryCellGraphicsItem
class MemoryCellGraphicsItem : public QGraphicsItem
{
public:
    MemoryCellGraphicsItem(const AMemoryDevice *memDevice, int xLoc);
    MemoryCellGraphicsItem(const AMemoryDevice *memDevice, int addr, QString sym, Enu::ESymbolFormat eSymFrmt, int xLoc, int yLoc);
    ~MemoryCellGraphicsItem() { }

    QRectF boundingRect() const;

    void updateContents(int newAddr, QString newSymbol, Enu::ESymbolFormat newFmt, int newY);
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *, QWidget *);

    static const int boxHeight;
    static const int boxWidth;
    static const int addressWidth;
    static const int symbolWidth;
    static const int bufferWidth;

    QString value;
    QColor boxColor;
    QColor boxBgColor;
    QColor textColor;
    QColor boxTextColor;
    void updateValue();
    int getAddress();
    int getNumBytes();

private:
    const AMemoryDevice *memDevice;
    int x;
    int y;
    int address;
    Enu::ESymbolFormat eSymbolFormat;
    QString symbol;
    QRectF box;

};

#endif // MEMORYCELLGRAPHICSITEM_H
