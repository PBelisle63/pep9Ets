#include "cpucontrolsection.h"
#include "cpudatasection.h"
#include "code.h"
#include "microcodeprogram.h"
#include "symbolentry.h"
#include "pep.h"
#include <QDebug>
#include "symbolentry.h"
#include "symboltable.h"
#include "symboltable.h"
#include "memorysection.h"
#include <QElapsedTimer>
QElapsedTimer timer;
CPUControlSection *CPUControlSection::_instance = nullptr;
CPUTester *CPUTester::_instance = nullptr;
CPUControlSection::CPUControlSection(CPUDataSection * data, MemorySection* memory): QObject(nullptr), data(data), memory(memory),
    microprogramCounter(0), microCycleCounter(0), macroCycleCounter(0),
    inSimulation(false), hadControlError(false), isPrefetchValid(false)
{

}

CPUControlSection *CPUControlSection::getInstance()
{
    if(_instance == nullptr)
    {
        _instance = new CPUControlSection(CPUDataSection::getInstance(),MemorySection::getInstance());
    }
    return _instance;
}

CPUControlSection::~CPUControlSection()
{
    //This object should last the lifetime of the  program, it does not need to be cleaned up
}

void CPUControlSection::setMicrocodeProgram(MicrocodeProgram *program)
{
    this->program = program;
    microprogramCounter = 0;
}

int CPUControlSection::getLineNumber() const
{
    return microprogramCounter;
}

const MicrocodeProgram *CPUControlSection::getProgram() const
{
    return program;
}

const MicroCode *CPUControlSection::getCurrentMicrocodeLine() const
{
    return program->getCodeLine(microprogramCounter);
}

QString CPUControlSection::getErrorMessage() const
{
    if(memory->hadError()) return memory->getErrorMessage();
    else if(data->hadErrorOnStep()) return data->getErrorMessage();
    else if(hadErrorOnStep()) return errorMessage;
    else return "";
}

bool CPUControlSection::hadErrorOnStep() const
{
    return hadControlError || data->hadErrorOnStep() || memory->hadError();
}

bool CPUControlSection::getExecutionFinished() const
{
    return executionFinished;
}

void CPUControlSection::onSimulationStarted()
{
    inSimulation = true;
    executionFinished = false;
}

void CPUControlSection::onSimulationFinished()
{
    data->clearClockSignals();
    data->clearControlSignals();
    executionFinished = true;
}

void CPUControlSection::onDebuggingStarted()
{
    onSimulationStarted();
}

void CPUControlSection::onDebuggingFinished()
{
    onSimulationFinished();
}

void CPUControlSection::onStep() noexcept
{
    //Do step logic
    int PC;
    if(microprogramCounter == 0) PC = data->getRegisterBankWord(6);
    const MicroCode* prog = program->getCodeLine(microprogramCounter);
    this->setSignalsFromMicrocode(prog);
    data->setSignalsFromMicrocode(prog);
    data->onStep();
    branchHandler();
    microCycleCounter++;
    if(microprogramCounter==0)
    {
        macroCycleCounter++;
        //qDebug()<<"Insturction #"<<macroCycleCounter<<" ; Cyle # "<<microCycleCounter;
        auto temp = data->getRegisterBankByte(8);
        const QString format("0x%1: ");
        if(Pep::isUnaryMap[Pep::decodeMnemonic[temp]])
        {
            qDebug().noquote().nospace()<<format.arg(QString::number(PC,16),4,'0')
                                        << QString("%1").arg(Pep::enumToMnemonMap[Pep::decodeMnemonic[temp]].toLower(),6,' ');
        }
        else
        {
            qDebug().noquote().nospace()<< format.arg(QString::number(PC,16),4,'0')
                                        << QString("%1").arg(Pep::enumToMnemonMap[Pep::decodeMnemonic[temp]].toLower(),6,' ')
                                        << "  "<< QString("%1").arg(QString::number(data->getRegisterBankWord(9),16),4,'0')
                                        << ", " <<Pep::intToAddrMode(Pep::decodeAddrMode[temp]).toLower();
        }
    }

}

void CPUControlSection::onClock()noexcept
{
    //Do clock logic
    if(!inSimulation)
    {
        data->onClock();
    }
    else
    {
        //One should not get here, otherwise that would mean that we clocked in a simulation
    }
}

void CPUControlSection::onRun()noexcept
{
    timer.start();
#pragma message ("This needs to be ammended for errors in execution")
    while(!executionFinished)
    {
        onStep();
        //If there was an error on the control flow
        if(this->hadErrorOnStep())
        {
            qDebug() << "The control section died";
            break;
        }
    }
    qDebug().noquote().nospace() << QString("%1").arg(Pep::enumToMnemonMap[Pep::decodeMnemonic[data->getRegisterBankByte(8)]].toLower(),6,' ');
    qDebug() <<"Execution time (ms): "<<timer.elapsed();
    qDebug() <<"Hz rating: "<< microCycleCounter / (((float)timer.elapsed())/1000);
}

void CPUControlSection::onClearCPU()noexcept
{
    data->onClearCPU();
    inSimulation = false;
    microprogramCounter = 0;
    microCycleCounter = 0;
    macroCycleCounter = 0;
    hadControlError = false;
    executionFinished = false;
    isPrefetchValid = false;
    errorMessage = "";
}

void CPUControlSection::onClearMemory() noexcept
{
    memory->onClearMemory();
}

void CPUControlSection::branchHandler()
{
    const MicroCode* prog = program->getCodeLine(microprogramCounter);
    int temp = microprogramCounter;
    quint8 byte = 0;
    QString tempString;
    const SymbolTable* symTable = this->program->getSymTable();;
    std::shared_ptr<SymbolEntry> val;
    switch(prog->getBranchFunction())
    {
    case Enu::Unconditional:
        temp = prog->getTrueTarget()->getValue();
        break;
    case Enu::uBRGT:
        if((!data->getStatusBit(Enu::STATUS_N)))
        {
            temp = prog->getTrueTarget()->getValue();
        }
        else
        {
            temp = prog->getFalseTarget()->getValue();
        }
        break;
    case Enu::uBRGE:
        if((!data->getStatusBit(Enu::STATUS_N)) || data->getStatusBit(Enu::STATUS_Z))
        {
            temp = prog->getTrueTarget()->getValue();
        }
        else
        {
            temp = prog->getFalseTarget()->getValue();
        }
        break;
    case Enu::uBREQ:
        if(data->getStatusBit(Enu::STATUS_Z))
        {
            temp = prog->getTrueTarget()->getValue();
        }
        else
        {
            temp = prog->getFalseTarget()->getValue();
        }
        break;
    case Enu::uBRLE:
        if(data->getStatusBit(Enu::STATUS_N) || data->getStatusBit(Enu::STATUS_Z))
        {
            temp = prog->getTrueTarget()->getValue();
        }
        else
        {
            temp = prog->getFalseTarget()->getValue();
        }
        break;
    case Enu::uBRLT:
        if(data->getStatusBit(Enu::STATUS_N))
        {
            temp = prog->getTrueTarget()->getValue();
        }
        else
        {
            temp = prog->getFalseTarget()->getValue();
        }
        break;
    case Enu::uBRNE:
        if((!data->getStatusBit(Enu::STATUS_Z)))
        {
            temp = prog->getTrueTarget()->getValue();
        }
        else
        {
            temp = prog->getFalseTarget()->getValue();
        }
        break;
    case Enu::uBRV:
        if(data->getStatusBit(Enu::STATUS_V))
        {
            temp = prog->getTrueTarget()->getValue();
        }
        else
        {
            temp = prog->getFalseTarget()->getValue();
        }
        break;
    case Enu::uBRC:
        if(data->getStatusBit(Enu::STATUS_C))
        {
            temp = prog->getTrueTarget()->getValue();
        }
        else
        {
            temp = prog->getFalseTarget()->getValue();
        }
        break;
    case Enu::uBRS:
        if(data->getStatusBit(Enu::STATUS_S))
        {
            temp = prog->getTrueTarget()->getValue();
        }
        else
        {
            temp = prog->getFalseTarget()->getValue();
        }
        break;
    case Enu::IsPrefetchValid:
        if(isPrefetchValid)
        {
            temp = prog->getTrueTarget()->getValue();
        }
        else
        {
            temp = prog->getFalseTarget()->getValue();
        }
        break;
    case Enu::IsUnary:
        byte = data->getRegisterBankByte(8);
        if(Pep::isUnaryMap[Pep::decodeMnemonic[byte]])
        {
            temp = prog->getTrueTarget()->getValue();
        }
        else
        {
            temp = prog->getFalseTarget()->getValue();
        }
        break;
    case Enu::IsPCEven:
        if(data->getRegisterBankByte(7)%2 == 0)
        {
            temp = prog->getTrueTarget()->getValue();
        }
        else
        {
            temp = prog->getFalseTarget()->getValue();
        }
        break;
    case Enu::AddressingModeDecoder:
        temp = data->getRegisterBankByte(8);
        tempString = Pep::intToAddrMode(Pep::decodeAddrMode[temp]).toLower()+"Addr";
        if(symTable->exists(tempString))
        {
            val = symTable->getValue(tempString);
            if(val->isDefined())
            {
                temp = val->getValue();
            }
            else
            {
                executionFinished = true;
                hadControlError = true;
                errorMessage = "ERROR: AMD jumped to multiply defined instr - " + tempString;
            }

        }
        else
        {
            executionFinished = true;
            hadControlError = true;
            errorMessage = "ERROR: AMD looked for undefined inst - " + tempString;
        }
        break;
    case Enu::InstructionSpecifierDecoder:
        temp = data->getRegisterBankByte(8);
        tempString = Pep::enumToMnemonMap[Pep::decodeMnemonic[temp]].toLower();
        if(symTable->exists(tempString))
        {
            val = symTable->getValue(tempString);
            if(val->isDefined())
            {
                temp = val->getValue();
            }
            else
            {
                executionFinished = true;
                hadControlError = true;
                errorMessage = "ERROR: ISD jumped to multiply defined instr - " + tempString;
            }

        }
        else
        {
            executionFinished = true;
            hadControlError = true;
            errorMessage = "ERROR: ISD looked for undefined inst - " + tempString;
        }
        break;
    case Enu::Stop:
        executionFinished = true;
        break;
    default:
        //This should never occur
        break;

    }
    if(hadControlError) //If there was an error in the control section, make sure the CPU stops
    {
        executionFinished = true;
    }
    else if(temp == microprogramCounter&&prog->getBranchFunction()!=Enu::Stop)
    {
        hadControlError = true;
        errorMessage = "Don't branch to yourself";
        executionFinished  = true;
    }
    else
    {
        microprogramCounter = temp;
    }
}

void CPUControlSection::setSignalsFromMicrocode(const MicroCode *line)
{
    int val;
    if(line->getClockSignal(Enu::EClockSignals::PValidCk))
    {
        val = line->getControlSignal(Enu::EControlSignals::PValid);
        if(val == Enu::signalDisabled)
        {
            errorMessage = "Error: Asserted PValidCk, but PValid was disabled.";
            hadControlError = true;
        }
        else
        {
            isPrefetchValid = val;
        }
    }
}

void CPUControlSection::initCPUStateFromPreconditions()
{
    onClearCPU();
    QList<UnitPreCode*> preCode;
    microprogramCounter=0;
    if(program == nullptr)
    {
        qDebug()<<"Can't init from null program";
    }
    for(Code* x : program->getObjectCode())
    {
        if(x->hasUnitPre())preCode.append((UnitPreCode*)x);
    }
    //Handle data section logic
    for(auto x : preCode)
    {
        x->setUnitPre(data);
    }

}

bool CPUControlSection::testPost()
{
    QList<UnitPreCode*> preCode;
    if(program == nullptr)
    {
        qDebug()<<"Can't init from null program";
    }
    for(Code* x : program->getObjectCode())
    {
        if(x->hasUnitPost())preCode.append((UnitPreCode*)x);
    }
    QString err;
    bool t=false;
    for(auto x : preCode)
    {
       ((UnitPostCode*) x)->testPostcondition(data,err);
        if(err!="")t=true;
    }
    qDebug()<<"The postcondtions were:"<<!t;
    return !t;
}

CPUTester *CPUTester::getInstance()
{
    if(_instance == nullptr)
    {
        _instance = new CPUTester(CPUControlSection::getInstance(),CPUDataSection::getInstance());
    }
    return _instance;
}

CPUTester::CPUTester(CPUControlSection *control, CPUDataSection *data): QObject(nullptr),control(control),data(data)
{

}

CPUTester::~CPUTester()
{

}

