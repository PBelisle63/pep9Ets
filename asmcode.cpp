// File: code.cpp
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
#include "asmcode.h"
#include "asmargument.h"
#include "pep.h"
#include "isaasm.h"
#include <QRegExp>
#include <QDebug>

// appendObjectCode
void UnaryInstruction::appendObjectCode(QList<int> &objectCode)
{
    if ((Pep::burnCount == 0) || ((Pep::burnCount == 1) && (memAddress >= Pep::romStartAddress))) {
        objectCode.append(Pep::opCodeMap.value(mnemonic));
    }
}

void NonUnaryInstruction::appendObjectCode(QList<int> &objectCode)
{
    if ((Pep::burnCount == 0) || ((Pep::burnCount == 1) && (memAddress >= Pep::romStartAddress))) {
        int instructionSpecifier = Pep::opCodeMap.value(mnemonic);
        if (Pep::addrModeRequiredMap.value(mnemonic)) {
            instructionSpecifier += Pep::aaaAddressField(addressingMode);
        }
        else {
            instructionSpecifier += Pep::aAddressField(addressingMode);
        }
        objectCode.append(instructionSpecifier);
        int operandSpecifier = argument->getArgumentValue();
        objectCode.append(operandSpecifier / 256);
        objectCode.append(operandSpecifier % 256);
    }
}

void DotAddrss::appendObjectCode(QList<int> &objectCode)
{
    if ((Pep::burnCount == 0) || ((Pep::burnCount == 1) && (memAddress >= Pep::romStartAddress))) {
        int symbolValue = Pep::symbolTable.value(argument->getArgumentString());
        objectCode.append(symbolValue / 256);
        objectCode.append(symbolValue % 256);
    }
}

void DotAlign::appendObjectCode(QList<int> &objectCode)
{
    if ((Pep::burnCount == 0) || ((Pep::burnCount == 1) && (memAddress >= Pep::romStartAddress))) {
        for (int i = 0; i < numBytesGenerated->getArgumentValue(); i++) {
            objectCode.append(0);
        }
    }
}

void DotAscii::appendObjectCode(QList<int> &objectCode)
{
    if ((Pep::burnCount == 0) || ((Pep::burnCount == 1) && (memAddress >= Pep::romStartAddress))) {
        int value;
        QString str = argument->getArgumentString();
        str.remove(0, 1); // Remove the leftmost double quote.
        str.chop(1); // Remove the rightmost double quote.
        while (str.length() > 0) {
            IsaAsm::unquotedStringToInt(str, value);
            objectCode.append(value);
        }
    }
}

void DotBlock::appendObjectCode(QList<int> &objectCode)
{
    if ((Pep::burnCount == 0) || ((Pep::burnCount == 1) && (memAddress >= Pep::romStartAddress))) {
        for (int i = 0; i < argument->getArgumentValue(); i++) {
            objectCode.append(0);
        }
    }
}

void DotBurn::appendObjectCode(QList<int> &)
{
    // Does not generate code.
}

void DotByte::appendObjectCode(QList<int> &objectCode)
{
    if ((Pep::burnCount == 0) || ((Pep::burnCount == 1) && (memAddress >= Pep::romStartAddress))) {
        objectCode.append(argument->getArgumentValue());
    }
}

void DotEnd::appendObjectCode(QList<int> &)
{
    // Does not generate code.
}

void DotEquate::appendObjectCode(QList<int> &)
{
    // Does not generate code.
}

void DotWord::appendObjectCode(QList<int> &objectCode)
{
    if ((Pep::burnCount == 0) || ((Pep::burnCount == 1) && (memAddress >= Pep::romStartAddress))) {
        int value = argument->getArgumentValue();
        objectCode.append(value / 256);
        objectCode.append(value % 256);
    }
}

void CommentOnly::appendObjectCode(QList<int> &)
{
    // Does not generate code.
}

void BlankLine::appendObjectCode(QList<int> &)
{
    // Does not generate code.
}

void UnaryInstruction::appendSourceLine(QStringList &assemblerListingList, QStringList &listingTraceList, QList<bool> &hasCheckBox)
{
    QString memStr = QString("%1").arg(memAddress, 4, 16, QLatin1Char('0')).toUpper();
    QString codeStr = QString("%1").arg(Pep::opCodeMap.value(mnemonic), 2, 16, QLatin1Char('0')).toUpper();
    if ((Pep::burnCount == 1) && (memAddress < Pep::romStartAddress)) {
        codeStr = "  ";
    }
    QString symbolStr = symbolDef;
    if (symbolStr.length() > 0) {
        symbolStr.append(":");
    }
    QString mnemonStr = Pep::enumToMnemonMap.value(mnemonic);
    QString lineStr = QString("%1%2%3%4%5")
                      .arg(memStr, -6, QLatin1Char(' '))
                      .arg(codeStr, -7, QLatin1Char(' '))
                      .arg(symbolStr, -9, QLatin1Char(' '))
                      .arg(mnemonStr, -8, QLatin1Char(' '))
                      .arg("            " + comment);
    Pep::memAddrssToAssemblerListing->insert(memAddress, assemblerListingList.size());
    Pep::listingRowChecked->insert(assemblerListingList.size(), Qt::Unchecked);
    assemblerListingList.append(lineStr);
    listingTraceList.append(lineStr);
    hasCheckBox.append(true);
}

void NonUnaryInstruction::appendSourceLine(QStringList &assemblerListingList, QStringList &listingTraceList, QList<bool> &hasCheckBox)
{
    QString memStr = QString("%1").arg(memAddress, 4, 16, QLatin1Char('0')).toUpper();
    int temp = Pep::opCodeMap.value(mnemonic);
    temp += Pep::addrModeRequiredMap.value(mnemonic) ? Pep::aaaAddressField(addressingMode) : Pep::aAddressField(addressingMode);
    QString codeStr = QString("%1").arg(temp, 2, 16, QLatin1Char('0')).toUpper();
    QString oprndNumStr = QString("%1").arg(argument->getArgumentValue(), 4, 16, QLatin1Char('0')).toUpper();
    if ((Pep::burnCount == 1) && (memAddress < Pep::romStartAddress)) {
        codeStr = "  ";
        oprndNumStr = "    ";
    }
    QString symbolStr = symbolDef;
    if (symbolStr.length() > 0) {
        symbolStr.append(":");
    }
    QString mnemonStr = Pep::enumToMnemonMap.value(mnemonic);
    QString oprndStr = argument->getArgumentString();
    if (Pep::addrModeRequiredMap.value(mnemonic)) {
        oprndStr.append("," + Pep::intToAddrMode(addressingMode));
    }
    else if (addressingMode == Enu::EAddrMode::X) {
        oprndStr.append("," + Pep::intToAddrMode(addressingMode));
    }
    QString lineStr = QString("%1%2%3%4%5%6%7")
                      .arg(memStr, -6, QLatin1Char(' '))
                      .arg(codeStr, -2)
                      .arg(oprndNumStr, -5, QLatin1Char(' '))
                      .arg(symbolStr, -9, QLatin1Char(' '))
                      .arg(mnemonStr, -8, QLatin1Char(' '))
                      .arg(oprndStr, -12)
                      .arg(comment);
    Pep::memAddrssToAssemblerListing->insert(memAddress, assemblerListingList.size());
    Pep::listingRowChecked->insert(assemblerListingList.size(), Qt::Unchecked);
    assemblerListingList.append(lineStr);
    listingTraceList.append(lineStr);
    hasCheckBox.append(true);
}

void DotAddrss::appendSourceLine(QStringList &assemblerListingList, QStringList &listingTraceList, QList<bool> &hasCheckBox)
{
    QString memStr = QString("%1").arg(memAddress, 4, 16, QLatin1Char('0')).toUpper();
    int symbolValue = Pep::symbolTable.value(argument->getArgumentString());
    QString codeStr = QString("%1").arg(symbolValue, 4, 16, QLatin1Char('0')).toUpper();
    if ((Pep::burnCount == 1) && (memAddress < Pep::romStartAddress)) {
        codeStr = "    ";
    }
    QString symbolStr = symbolDef;
    if (symbolStr.length() > 0) {
        symbolStr.append(":");
    }
    QString dotStr = ".ADDRSS";
    QString oprndStr = argument->getArgumentString();
    QString lineStr = QString("%1%2%3%4%5%6")
                      .arg(memStr, -6, QLatin1Char(' '))
                      .arg(codeStr, -7, QLatin1Char(' '))
                      .arg(symbolStr, -9, QLatin1Char(' '))
                      .arg(dotStr, -8, QLatin1Char(' '))
                      .arg(oprndStr, -12)
                      .arg(comment);
    assemblerListingList.append(lineStr);
    listingTraceList.append(lineStr);
    hasCheckBox.append(false);
}

void DotAlign::appendSourceLine(QStringList &assemblerListingList, QStringList &listingTraceList, QList<bool> &hasCheckBox)
{
    int numBytes = numBytesGenerated->getArgumentValue();
    QString memStr = numBytes == 0 ? "      " : QString("%1").arg(memAddress, 4, 16, QLatin1Char('0')).toUpper();
    QString codeStr = "";
    while ((numBytes > 0) && (codeStr.length() < 6)) {
        codeStr.append("00");
        numBytes--;
    }
    if ((Pep::burnCount == 1) && (memAddress < Pep::romStartAddress)) {
        codeStr = "      ";
    }
    QString symbolStr = symbolDef;
    if (symbolStr.length() > 0) {
        symbolStr.append(":");
    }
    QString dotStr = ".ALIGN";
    QString oprndStr = argument->getArgumentString();
    QString lineStr = QString("%1%2%3%4%5%6")
                      .arg(memStr, -6, QLatin1Char(' '))
                      .arg(codeStr, -7, QLatin1Char(' '))
                      .arg(symbolStr, -9, QLatin1Char(' '))
                      .arg(dotStr, -8, QLatin1Char(' '))
                      .arg(oprndStr, -12)
                      .arg(comment);
    assemblerListingList.append(lineStr);
    listingTraceList.append(lineStr);
    hasCheckBox.append(false);
    if ((Pep::burnCount == 0) || ((Pep::burnCount == 1) && (memAddress >= Pep::romStartAddress))) {
        while (numBytes > 0) {
            codeStr = "";
            while ((numBytes > 0) && (codeStr.length() < 6)) {
                codeStr.append("00");
                numBytes--;
            }
            lineStr = QString("      %1").arg(codeStr, -7, QLatin1Char(' '));
            assemblerListingList.append(lineStr);
            listingTraceList.append(lineStr);
            hasCheckBox.append(false);
        }
    }
}

void DotAscii::appendSourceLine(QStringList &assemblerListingList, QStringList &listingTraceList, QList<bool> &hasCheckBox)
{
    QString memStr = QString("%1").arg(memAddress, 4, 16, QLatin1Char('0')).toUpper();
    QString str = argument->getArgumentString();
    str.remove(0, 1); // Remove the leftmost double quote.
    str.chop(1); // Remove the rightmost double quote.
    int value;
    QString codeStr = "";
    while ((str.length() > 0) && (codeStr.length() < 6)) {
        IsaAsm::unquotedStringToInt(str, value);
        codeStr.append(QString("%1").arg(value, 2, 16, QLatin1Char('0')).toUpper());
    }
    if ((Pep::burnCount == 1) && (memAddress < Pep::romStartAddress)) {
        codeStr = "      ";
    }
    QString symbolStr = symbolDef;
    if (symbolStr.length() > 0) {
        symbolStr.append(":");
    }
    QString dotStr = ".ASCII";
    QString oprndStr = argument->getArgumentString();
    QString lineStr = QString("%1%2%3%4%5%6")
                      .arg(memStr, -6, QLatin1Char(' '))
                      .arg(codeStr, -7, QLatin1Char(' '))
                      .arg(symbolStr, -9, QLatin1Char(' '))
                      .arg(dotStr, -8, QLatin1Char(' '))
                      .arg(oprndStr, -12)
                      .arg(comment);
    assemblerListingList.append(lineStr);
    listingTraceList.append(lineStr);
    hasCheckBox.append(false);
    if ((Pep::burnCount == 0) || ((Pep::burnCount == 1) && (memAddress >= Pep::romStartAddress))) {
        while (str.length() > 0) {
            codeStr = "";
            while ((str.length() > 0) && (codeStr.length() < 6)) {
                IsaAsm::unquotedStringToInt(str, value);
                codeStr.append(QString("%1").arg(value, 2, 16, QLatin1Char('0')).toUpper());
            }
            lineStr = QString("      %1").arg(codeStr, -7, QLatin1Char(' '));
            assemblerListingList.append(lineStr);
            listingTraceList.append(lineStr);
            hasCheckBox.append(false);
        }
    }
}

void DotBlock::appendSourceLine(QStringList &assemblerListingList, QStringList &listingTraceList, QList<bool> &hasCheckBox)
{
    QString memStr = QString("%1").arg(memAddress, 4, 16, QLatin1Char('0')).toUpper();
    int numBytes = argument->getArgumentValue();
    QString codeStr = "";
    while ((numBytes > 0) && (codeStr.length() < 6)) {
        codeStr.append("00");
        numBytes--;
    }
    if ((Pep::burnCount == 1) && (memAddress < Pep::romStartAddress)) {
        codeStr = "      ";
    }
    QString symbolStr = symbolDef;
    if (symbolStr.length() > 0) {
        symbolStr.append(":");
    }
    QString dotStr = ".BLOCK";
    QString oprndStr = argument->getArgumentString();
    QString lineStr = QString("%1%2%3%4%5%6")
                      .arg(memStr, -6, QLatin1Char(' '))
                      .arg(codeStr, -7, QLatin1Char(' '))
                      .arg(symbolStr, -9, QLatin1Char(' '))
                      .arg(dotStr, -8, QLatin1Char(' '))
                      .arg(oprndStr, -12)
                      .arg(comment);
    assemblerListingList.append(lineStr);
    listingTraceList.append(lineStr);
    hasCheckBox.append(false);
    if ((Pep::burnCount == 0) || ((Pep::burnCount == 1) && (memAddress >= Pep::romStartAddress))) {
        while (numBytes > 0) {
            codeStr = "";
            while ((numBytes > 0) && (codeStr.length() < 6)) {
                codeStr.append("00");
                numBytes--;
            }
            lineStr = QString("      %1").arg(codeStr, -7, QLatin1Char(' '));
            assemblerListingList.append(lineStr);
            listingTraceList.append(lineStr);
            hasCheckBox.append(false);
        }
    }
}

void DotBurn::appendSourceLine(QStringList &assemblerListingList, QStringList &listingTraceList, QList<bool> &hasCheckBox)
{
    QString memStr = QString("%1").arg(memAddress, 4, 16, QLatin1Char('0')).toUpper();
    QString symbolStr = symbolDef;
    if (symbolStr.length() > 0) {
        symbolStr.append(":");
    }
    QString dotStr = ".BURN";
    QString oprndStr = argument->getArgumentString();
    QString lineStr = QString("%1       %2%3%4%5")
                      .arg(memStr, -6, QLatin1Char(' '))
                      .arg(symbolStr, -9, QLatin1Char(' '))
                      .arg(dotStr, -8, QLatin1Char(' '))
                      .arg(oprndStr, -12)
                      .arg(comment);
    assemblerListingList.append(lineStr);
    listingTraceList.append(lineStr);
    hasCheckBox.append(false);
}

void DotByte::appendSourceLine(QStringList &assemblerListingList, QStringList &listingTraceList, QList<bool> &hasCheckBox)
{
    QString memStr = QString("%1").arg(memAddress, 4, 16, QLatin1Char('0')).toUpper();
    QString codeStr = QString("%1").arg(argument->getArgumentValue(), 2, 16, QLatin1Char('0')).toUpper();
    if ((Pep::burnCount == 1) && (memAddress < Pep::romStartAddress)) {
        codeStr = "  ";
    }
    QString symbolStr = symbolDef;
    if (symbolStr.length() > 0) {
        symbolStr.append(":");
    }
    QString dotStr = ".BYTE";
    QString oprndStr = argument->getArgumentString();
    if (oprndStr.startsWith("0x")) {
        oprndStr.remove(2, 2); // Display only the last two hex characters
    }
    QString lineStr = QString("%1%2%3%4%5%6")
                      .arg(memStr, -6, QLatin1Char(' '))
                      .arg(codeStr, -7, QLatin1Char(' '))
                      .arg(symbolStr, -9, QLatin1Char(' '))
                      .arg(dotStr, -8, QLatin1Char(' '))
                      .arg(oprndStr, -12)
                      .arg(comment);
    assemblerListingList.append(lineStr);
    listingTraceList.append(lineStr);
    hasCheckBox.append(false);
}

void DotEnd::appendSourceLine(QStringList &assemblerListingList, QStringList &listingTraceList, QList<bool> &hasCheckBox)
{
    QString memStr = QString("%1").arg(memAddress, 4, 16, QLatin1Char('0')).toUpper();
    QString symbolStr = symbolDef;
    if (symbolStr.length() > 0) {
        symbolStr.append(":");
    }
    QString dotStr = ".END";
    QString lineStr = QString("%1       %2%3              %4")
                      .arg(memStr, -6, QLatin1Char(' '))
                      .arg(symbolStr, -9, QLatin1Char(' '))
                      .arg(dotStr, -8, QLatin1Char(' '))
                      .arg(comment);
    assemblerListingList.append(lineStr);
    listingTraceList.append(lineStr);
    hasCheckBox.append(false);
}

void DotEquate::appendSourceLine(QStringList &assemblerListingList, QStringList &listingTraceList, QList<bool> &hasCheckBox)
{
    QString symbolStr = symbolDef;
    if (symbolStr.length() > 0) {
        symbolStr.append(":");
    }
    QString dotStr = ".EQUATE";
    QString oprndStr = argument->getArgumentString();
    QString lineStr = QString("             %1%2%3%4")
                      .arg(symbolStr, -9, QLatin1Char(' '))
                      .arg(dotStr, -8, QLatin1Char(' '))
                      .arg(oprndStr, -12)
                      .arg(comment);
    assemblerListingList.append(lineStr);
    listingTraceList.append(lineStr);
    hasCheckBox.append(false);
}

void DotWord::appendSourceLine(QStringList &assemblerListingList, QStringList &listingTraceList, QList<bool> &hasCheckBox)
{
    QString memStr = QString("%1").arg(memAddress, 4, 16, QLatin1Char('0')).toUpper();
    QString codeStr = QString("%1").arg(argument->getArgumentValue(), 4, 16, QLatin1Char('0')).toUpper();
    if ((Pep::burnCount == 1) && (memAddress < Pep::romStartAddress)) {
        codeStr = "    ";
    }
    QString symbolStr = symbolDef;
    if (symbolStr.length() > 0) {
        symbolStr.append(":");
    }
    QString dotStr = ".WORD";
    QString oprndStr = argument->getArgumentString();
    QString lineStr = QString("%1%2%3%4%5%6")
                      .arg(memStr, -6, QLatin1Char(' '))
                      .arg(codeStr, -7, QLatin1Char(' '))
                      .arg(symbolStr, -9, QLatin1Char(' '))
                      .arg(dotStr, -8, QLatin1Char(' '))
                      .arg(oprndStr, -12)
                      .arg(comment);
    assemblerListingList.append(lineStr);
    listingTraceList.append(lineStr);
    hasCheckBox.append(false);
}

void CommentOnly::appendSourceLine(QStringList &assemblerListingList, QStringList &listingTraceList, QList<bool> &hasCheckBox)
{
    assemblerListingList.append("             " + comment);
    listingTraceList.append("             " + comment);
    hasCheckBox.append(false);
}

void BlankLine::appendSourceLine(QStringList &assemblerListingList, QStringList &listingTraceList, QList<bool> &hasCheckBox)
{
    assemblerListingList.append("");
    listingTraceList.append("");
    hasCheckBox.append(false);
}

bool DotBlock::processFormatTraceTags(int &sourceLine, QString &errorString) {
    if (symbolDef.isEmpty()) {
        return true;
    }
    int pos = IsaAsm::rxFormatTag.indexIn(comment);
    if (pos > -1) {
        QString formatTag = IsaAsm::rxFormatTag.cap(0);
        Enu::ESymbolFormat tagType = IsaAsm::formatTagType(formatTag);
        int multiplier = IsaAsm::formatMultiplier(formatTag);
        if (argument->getArgumentValue() != IsaAsm::tagNumBytes(tagType) * multiplier) {
            errorString = ";WARNING: Format tag does not match number of bytes allocated by .BLOCK.";
            sourceLine = sourceCodeLine;
            return false;
        }
        Pep::symbolFormat.insert(symbolDef, tagType);
        Pep::symbolFormatMultiplier.insert(symbolDef, multiplier);
        Pep::blockSymbols.append(symbolDef);
    }
    return true;
}

bool DotEquate::processFormatTraceTags(int &, QString &) {
    if (symbolDef.isEmpty()) {
        return true;
    }
    int pos = IsaAsm::rxFormatTag.indexIn(comment);
    if (pos > -1) {
        QString formatTag = IsaAsm::rxFormatTag.cap(0);
        Pep::symbolFormat.insert(symbolDef, IsaAsm::formatTagType(formatTag));
        Pep::symbolFormatMultiplier.insert(symbolDef, IsaAsm::formatMultiplier(formatTag));
        Pep::equateSymbols.append(symbolDef);
    }
    return true;
}

bool DotBlock::processSymbolTraceTags(int &sourceLine, QString &errorString) {
    // For global structs.
    if (symbolDef.isEmpty()) {
        return true;
    }
    if (Pep::blockSymbols.contains(symbolDef)) {
        return true; // Pre-existing format tag takes precedence over symbol tag.
    }

    int numBytesAllocated = argument->getArgumentValue();
    QString symbol;
    QStringList list;
    int numBytesListed = 0;
    int pos = 0;
    while ((pos = IsaAsm::rxSymbolTag.indexIn(comment, pos)) != -1) {
        symbol = IsaAsm::rxSymbolTag.cap(1);
        if (!Pep::equateSymbols.contains(symbol)) {
            errorString = ";WARNING: " + symbol + " not specified in .EQUATE.";
            sourceLine = sourceCodeLine;
            return false;
        }
        numBytesListed += IsaAsm::tagNumBytes(Pep::symbolFormat.value(symbol)) * Pep::symbolFormatMultiplier.value(symbol);
        list.append(symbol);
        pos += IsaAsm::rxSymbolTag.matchedLength();
    }
    if (numBytesAllocated != numBytesListed && numBytesListed > 0) {
        errorString = ";WARNING: Number of bytes allocated (" + QString("%1").arg(numBytesAllocated) +
                      ") not equal to number of bytes listed in trace tag (" + QString("%1").arg(numBytesListed) + ").";
        sourceLine = sourceCodeLine;
        return false;
    }
    Pep::blockSymbols.append(symbolDef);
    Pep::globalStructSymbols.insert(symbolDef, list);
    return true;
}

bool NonUnaryInstruction::processFormatTraceTags(int &, QString &) {
    if (mnemonic == Enu::EMnemonic::CALL && argument->getArgumentString() == "malloc") {
        int pos = IsaAsm::rxFormatTag.indexIn(comment);
        if (pos > -1) {
            QStringList list;
            QString formatTag = IsaAsm::rxFormatTag.cap(0);
            Enu::ESymbolFormat tagType = IsaAsm::formatTagType(formatTag);
            int multiplier = IsaAsm::formatMultiplier(formatTag);
            QString symbolDef = QString("%1").arg(memAddress); // Dummy symbol for format tag in malloc
            if (!Pep::equateSymbols.contains(symbolDef)){
                // Limitation: only one dummy format per program
                Pep::equateSymbols.append(symbolDef);
            }
            Pep::symbolFormat.insert(symbolDef, tagType); // Any duplicates have value replaced
            Pep::symbolFormatMultiplier.insert(symbolDef, multiplier);
            list.append(symbolDef);
            Pep::symbolTraceList.insert(memAddress, list);
        }
    }
    return true;
}

bool NonUnaryInstruction::processSymbolTraceTags(int &sourceLine, QString &errorString) {
    if (mnemonic == Enu::EMnemonic::ADDSP || mnemonic == Enu::EMnemonic::SUBSP) {
        int numBytesAllocated;
        if (addressingMode != Enu::EAddrMode::I) {
            errorString = ";WARNING: Stack trace not possible unless immediate addressing is specified.";
            sourceLine = sourceCodeLine;
            return false;
        }
        numBytesAllocated = argument->getArgumentValue();
        QString symbol;
        QStringList list;
        int numBytesListed = 0;
        int pos = 0;
        while ((pos = IsaAsm::rxSymbolTag.indexIn(comment, pos)) != -1) {
            symbol = IsaAsm::rxSymbolTag.cap(1);
            if (!Pep::equateSymbols.contains(symbol)) {
                errorString = ";WARNING: " + symbol + " not specified in .EQUATE.";
                sourceLine = sourceCodeLine;
                return false;
            }
            numBytesListed += IsaAsm::tagNumBytes(Pep::symbolFormat.value(symbol)) * Pep::symbolFormatMultiplier.value(symbol);
            list.append(symbol);
            pos += IsaAsm::rxSymbolTag.matchedLength();
        }
        if (numBytesAllocated != numBytesListed) {
            QString message = (mnemonic == Enu::EMnemonic::ADDSP) ? "deallocated" : "allocated";
            errorString = ";WARNING: Number of bytes " + message + " (" + QString("%1").arg(numBytesAllocated) +
                          ") not equal to number of bytes listed in trace tag (" + QString("%1").arg(numBytesListed) + ").";
            sourceLine = sourceCodeLine;
            return false;
        }
        Pep::symbolTraceList.insert(memAddress, list);
        return true;
    }
    else if (mnemonic == Enu::EMnemonic::CALL && argument->getArgumentString() == "malloc") {
        int pos = 0;
        QString symbol;
        QStringList list;
        while ((pos = IsaAsm::rxSymbolTag.indexIn(comment, pos)) != -1) {
            symbol = IsaAsm::rxSymbolTag.cap(1);
            if (!Pep::equateSymbols.contains(symbol) && !Pep::blockSymbols.contains(symbol)) {
                errorString = ";WARNING: " + symbol + " not specified in .EQUATE.";
                sourceLine = sourceCodeLine;
                return false;
            }
            list.append(symbol);
            pos += IsaAsm::rxSymbolTag.matchedLength();
        }

        if (!list.isEmpty()) {
            Pep::symbolTraceList.insert(memAddress, list);
        }
        return true;
    }
    else {
        return true;
    }
}
