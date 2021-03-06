#include "mbthreadedconnmanager.h"

#include "mbconnection.h"

#define QT_MODBUS_DEBUG
#ifdef QT_MODBUS_DEBUG
#include <QDebug>
#define MBDebug() qDebug() << __PRETTY_FUNCTION__  << QThread::currentThread()
#else
#define MBDebug() ;
#endif //QT_MODBUS_DEBUG

class MBCommand {
public:
    enum CommandType {
        Connect,
        Close,

        ReadBits,
        ReadInputBits,
        ReadRegisters,
        ReadInputRegisters,

        WriteBit,
        WriteBits,
        WriteRegister,
        WriteRegisters,

        ReportSlaveId,

        SendRawRequest,
        WaitForConfirmation
    };

    MBCommand(MBConnection *conn, CommandType type, int invokeId):
        _conn(conn),
        _type(type),
        _invokeId(invokeId)
    {}

    virtual void exec() = 0;


    MBConnection *_conn;
    CommandType _type;
    int _invokeId;
};

class MBConnectCommand:
        public MBCommand {
public:
    MBConnectCommand(MBConnection *conn, int invokeId):
        MBCommand(conn, MBCommand::Connect, invokeId)
    {}

    virtual void exec() {
        MBDebug();
        int ret = _conn->mbConnect();
        if (ret < 0) {
            _conn->emitErrorOccured(_invokeId, errno, MBConnection::Connect);
            _conn->emitConnectionChanged(_invokeId, false);
        }
        else
            _conn->emitConnectionChanged(_invokeId, 0 == ret);
    }
};

class MBCloseCommand:
        public MBCommand {
public:
    MBCloseCommand(MBConnection *conn, int invokeId):
        MBCommand(conn, MBCommand::Close, invokeId)
    {}

    virtual void exec() {
        MBDebug();
        _conn->close();
        _conn->emitConnectionChanged(_invokeId, false);
    }
};


class MBReadBitsCommand:
        public MBCommand {
public:
    MBReadBitsCommand(MBConnection *conn, int slaveId, int addr, QVector<quint8> *result, int invokeId):
        MBCommand(conn, MBCommand::ReadBits, invokeId),
        _slaveId(slaveId),
        _addr(addr),
        _result(result) {}

    virtual void exec() {
        MBDebug();
        _conn->setSlave(_slaveId);
        int ret = _conn->readBits(_addr, _result->size(), _result);
        if (ret < 0)
            _conn->emitErrorOccured(_invokeId, errno, MBConnection::ReadBits);
        else
            _conn->emitRequestFinished(_invokeId, ret, MBConnection::ReadBits);
    }

    int _slaveId;
    int _addr;
    QVector<quint8> *_result;
};

class MBReadInputBitsCommand:
        public MBCommand {
public:
    MBReadInputBitsCommand(MBConnection *conn, int slaveId, int addr, QVector<quint8> *result, int invokeId):
        MBCommand(conn, MBCommand::ReadInputBits, invokeId),
        _slaveId(slaveId),
        _addr(addr),
        _result(result) {}

    virtual void exec() {
        MBDebug();
        _conn->setSlave(_slaveId);
        int res = _conn->readInputBits(_addr, _result->size(), _result);
        if (res < 0)
            _conn->emitErrorOccured(_invokeId, errno, MBConnection::ReadInputBits);
        else
            _conn->emitRequestFinished(_invokeId, res, MBConnection::ReadInputBits);
    }

    int _slaveId;
    int _addr;
    QVector<quint8> *_result;
};

class MBReadRegistersCommand:
        public MBCommand {
public:
    MBReadRegistersCommand(MBConnection *conn, int slaveId, int addr, QVector<quint16> *result, int invokeId):
        MBCommand(conn, MBCommand::ReadRegisters, invokeId),
        _slaveId(slaveId),
        _addr(addr),
        _result(result) {}

    virtual void exec() {
        MBDebug();
        _conn->setSlave(_slaveId);
        int res = _conn->readRegisters(_addr, _result->size(), _result);
        if (res < 0)
            _conn->emitErrorOccured(_invokeId, errno, MBConnection::ReadRegisters);
        else
            _conn->emitRequestFinished(_invokeId, res, MBConnection::ReadRegisters);
    }

    int _slaveId;
    int _addr;
    QVector<quint16> *_result;
};

class MBReadInputRegistersCommand:
        public MBCommand {
public:
    MBReadInputRegistersCommand(MBConnection *conn, int slaveId, int addr, QVector<quint16> *result, int invokeId):
        MBCommand(conn, MBCommand::ReadInputRegisters, invokeId),
        _slaveId(slaveId),
        _addr(addr),
        _result(result) {}

    virtual void exec() {
        MBDebug();
        _conn->setSlave(_slaveId);
        int res = _conn->readInputRegisters(_addr, _result->size(), _result);
        if (res < 0)
            _conn->emitErrorOccured(_invokeId, errno, MBConnection::ReadInputRegisters);
        else
            _conn->emitRequestFinished(_invokeId, res, MBConnection::ReadInputRegisters);
    }

    int _slaveId;
    int _addr;
    QVector<quint16> *_result;
};


class MBWriteBitCommand:
        public MBCommand {
public:
    MBWriteBitCommand(MBConnection *conn, int slaveId, int coilAddr, int status, int invokeId):
        MBCommand(conn, MBCommand::WriteBit, invokeId),
        _slaveId(slaveId),
        _coilAddr(coilAddr),
        _status(status) {}

    virtual void exec() {
        MBDebug();
        _conn->setSlave(_slaveId);
        int ret = _conn->writeBit(_coilAddr, _status);
        if (ret < 0)
            _conn->emitErrorOccured(_invokeId, errno, MBConnection::WriteBit);
        else
            _conn->emitRequestFinished(_invokeId, ret, MBConnection::WriteBit);
    }

    int _slaveId;
    int _coilAddr;
    int _status;
};

class MBWriteRegisterCommand:
        public MBCommand {
public:
    MBWriteRegisterCommand(MBConnection *conn, int slaveId, int regAddr, int value, int invokeId):
        MBCommand(conn, MBCommand::WriteRegister, invokeId),
        _slaveId(slaveId),
        _regAddr(regAddr),
        _value(value) {}

    virtual void exec() {
        MBDebug();
        _conn->setSlave(_slaveId);
        int res = _conn->writeRegister(_regAddr, _value);
        if (res < 0)
            _conn->emitErrorOccured(_invokeId, errno, MBConnection::WriteRegister);
        else
            _conn->emitRequestFinished(_invokeId, res, MBConnection::WriteRegister);
    }

    int _slaveId;
    int _regAddr;
    int _value;
};

class MBWriteBitsCommand:
        public MBCommand {
public:
    MBWriteBitsCommand(MBConnection *conn, int slaveId, int addr, QVector<quint8> *data, int invokeId):
        MBCommand(conn, MBCommand::WriteBits, invokeId),
        _slaveId(slaveId),
        _addr(addr),
        _data(data) {}

    virtual void exec() {
        MBDebug();
        _conn->setSlave(_slaveId);
        int res = _conn->writeBits(_addr, _data->size(), _data);
        if (res < 0)
            _conn->emitErrorOccured(_invokeId, errno, MBConnection::WriteBits);
        else
            _conn->emitRequestFinished(_invokeId, res, MBConnection::WriteBits);
    }

    int _slaveId;
    int _addr;
    QVector<quint8> *_data;
};

class MBWriteRegistersCommand:
        public MBCommand {
public:
    MBWriteRegistersCommand(MBConnection *conn, int slaveId, int addr, QVector<quint16> *data, int invokeId):
        MBCommand(conn, MBCommand::WriteRegisters, invokeId),
        _slaveId(slaveId),
        _addr(addr),
        _data(data) {}

    virtual void exec() {
        MBDebug();
        _conn->setSlave(_slaveId);
        int res = _conn->writeRegisters(_addr, _data->size(), _data);
        if (res < 0)
            _conn->emitErrorOccured(_invokeId, errno, MBConnection::WriteRegisters);
        else
            _conn->emitRequestFinished(_invokeId, res, MBConnection::WriteRegisters);
    }

    int _slaveId;
    int _addr;
    QVector<quint16> *_data;
};

class MBReportSlaveIdCommand:
        public MBCommand {
public:
    MBReportSlaveIdCommand(MBConnection *conn, int slaveId, QVector<quint8> *dest, int invokeId):
        MBCommand(conn, MBCommand::ReportSlaveId, invokeId),
        _slaveId(slaveId),
        _dest(dest) {}

    virtual void exec() {
        MBDebug();
        _conn->setSlave(_slaveId);
        int res = _conn->reportSlaveId(_dest);
        if (res < 0)
            _conn->emitErrorOccured(_invokeId, errno, MBConnection::ReportSlaveId);
        else
            _conn->emitRequestFinished(_invokeId, res, MBConnection::ReportSlaveId);
    }

    int _slaveId;
    QVector<quint8> *_dest;
};

class MBRawRequestCommand:
        public MBCommand {
public:
    MBRawRequestCommand(MBConnection *conn, int slaveId, QVector<quint8> *req, int invokeId):
        MBCommand(conn, MBCommand::SendRawRequest, invokeId),
        _slaveId(slaveId),
        _req(req) {}

    virtual void exec() {
        MBDebug();
        _conn->setSlave(_slaveId);
        int res = _conn->sendRawRequest(_req);
        if (res < 0)
            _conn->emitErrorOccured(_invokeId, errno, MBConnection::RawRequest);
        else
            _conn->emitRawRequestSent(_invokeId, res);
    }

    int _slaveId;
    QVector<quint8> *_req;
};

class MBWaitForConfirmationCommand:
        public MBCommand {
public:
    MBWaitForConfirmationCommand(MBConnection *conn, int slaveId, QVector<quint8> *resp, int invokeId):
        MBCommand(conn, MBCommand::SendRawRequest, invokeId),
        _slaveId(slaveId),
        _resp(resp) {}

    virtual void exec() {
        MBDebug();
        _conn->setSlave(_slaveId);
        int res = _conn->waitForConfirmation(_resp);
        if (res < 0)
            _conn->emitErrorOccured(_invokeId, errno, MBConnection::RawConfirmation);
        else
            _conn->emitConfirmationReceived(_invokeId, res);
    }

    int _slaveId;
    QVector<quint8> *_resp;
};

MBThreadedConnManager::MBThreadedConnManager(QObject *parent) :
    QObject(parent),
    _currentCommand(0),
    _managedConnectionsCnt(0)
{
    moveToThread(&_managedThread);
    /* this connection is to be of queued type, since *Async commandAdded is to be emitted
     * from the other thread than the _managedThread, where MBThreadedConnManager works in.
     */
    connect(this, SIGNAL(commandAdded()), this, SLOT(_nextCommandAdded()), Qt::QueuedConnection);
    connect(&_managedThread, SIGNAL(finished()), this, SLOT(_threadFinished()));
}

MBThreadedConnManager::~MBThreadedConnManager()
{
    if (!_managedThread.isFinished()) {
        _managedThread.quit();
        if (!_managedThread.wait(10 * 1000)) {//give it time to finish, hopefully it succeeds...
            _managedThread.terminate();//... or kill it, which is dangerous
        }
    }
}

const QThread *MBThreadedConnManager::managedThread()
{
    return &_managedThread;
}

void MBThreadedConnManager::registerConnection(MBConnection *conn)
{
    ++_managedConnectionsCnt;
    conn->moveToThread(&_managedThread);
    conn->setParent(this);
    if (_managedConnectionsCnt > 0) {
        if (!_managedThread.isRunning()) {
            qDebug() << "Starting the thread";
            _managedThread.start();
        }
    }
}

void MBThreadedConnManager::unregisterConnection(MBConnection *conn)
{
    --_managedConnectionsCnt;
    if (0 == _managedConnectionsCnt) {
        qDebug() << "Stopping the thread";
        _managedThread.exit();
    }
    Q_UNUSED(conn);
}

void MBThreadedConnManager::mbConnectAsync(MBConnection *conn, int invokeId)
{
    _appendCommand(new MBConnectCommand(conn, invokeId));
}

bool MBThreadedConnManager::cancelRequest(MBConnection *conn, int invokeId)
{
    MBDebug();
    bool isCancelled(false);

    _mutex.lock();

    QList<MBCommand*>::Iterator it = _commands.begin();
    while (_commands.end() != it) {
        MBCommand *cmd = *it;
        if ((invokeId == cmd->_invokeId) &&
                (conn == cmd->_conn)) {
            it = _commands.erase(it);
            qDebug("%s: cancelled %d type (reqIdL %d)", __FUNCTION__, cmd->_type, invokeId);
            delete cmd;
            isCancelled = true;
            //we are done - there should be no more duplications
            break;
        }
        else
            ++it;
    }

    _mutex.unlock();

    if (isCancelled) {
        conn->emitRequestCancelled(invokeId);
    }

    return isCancelled;
}

void MBThreadedConnManager::closeAsync(MBConnection *conn, bool prioritize, int invokeId)
{
    MBDebug();
    MBCommand *closeCmd = new MBCloseCommand(conn, invokeId);
    if (prioritize) {
        QList<int> cancelledIds;

        _mutex.lock();
        // if we are to give high priority to the close command, first
        // we have to cancell all scheduled commands for this connection
        QList<MBCommand*>::Iterator it = _commands.begin();
        while (_commands.end() != it) {
            MBCommand* cmd = *it;
            if (conn == cmd->_conn) {
                it = _commands.erase(it);
                cancelledIds.append(cmd->_invokeId);
                qDebug("%s: removing %d type (reqId: %d)", __FUNCTION__, cmd->_type, cmd->_invokeId);
                delete cmd;
            }
            else
                ++it;
        }
        _mutex.unlock();

        foreach (int iId, cancelledIds) {
            conn->emitRequestCancelled(iId);
        }

        _prependCommand(closeCmd);
    }
    else {
        _appendCommand(closeCmd);
    }
}


void MBThreadedConnManager::readBitsAsync(MBConnection *conn, int slaveId, int addr, QVector<quint8> *result, int invokeId)
{
    MBDebug();
    _appendCommand(new MBReadBitsCommand(conn, slaveId, addr, result, invokeId));
}

void MBThreadedConnManager::readInputBitsAsync(MBConnection *conn, int slaveId, int addr, QVector<quint8> *result, int invokeId)
{
    MBDebug();
    _appendCommand(new MBReadInputBitsCommand(conn, slaveId, addr, result, invokeId));
}

void MBThreadedConnManager::readRegistersAsync(MBConnection *conn, int slaveId, int addr, QVector<quint16> *result, int invokeId)
{
    MBDebug();
    _appendCommand(new MBReadRegistersCommand(conn, slaveId, addr, result, invokeId));
}

void MBThreadedConnManager::_appendCommand(MBCommand *cmd)
{
    MBDebug();
    _mutex.lock();
    _commands.append(cmd);
    _mutex.unlock();

    emit commandAdded();
}

void MBThreadedConnManager::_prependCommand(MBCommand *cmd)
{
    MBDebug();
    _mutex.lock();
    _commands.prepend(cmd);
    _mutex.unlock();

    emit commandAdded();
}


void MBThreadedConnManager::writeBitAsync(MBConnection *conn, int slaveId, int coilAddr, int status, int invokeId)
{
    MBDebug();
    _appendCommand(new MBWriteBitCommand(conn, slaveId, coilAddr, status, invokeId));
}

void MBThreadedConnManager::writeRegisterAsync(MBConnection *conn, int slaveId, int regAddr, int value, int invokeId)
{
    MBDebug();
    _appendCommand(new MBWriteRegisterCommand(conn, slaveId, regAddr, value, invokeId));
}

void MBThreadedConnManager::writeBitsAsync(MBConnection *conn, int slaveId, int addr, QVector<quint8> *data, int invokeId)
{
    MBDebug();
    _appendCommand(new MBWriteBitsCommand(conn, slaveId, addr, data, invokeId));
}

void MBThreadedConnManager::writeRegistersAsync(MBConnection *conn, int slaveId, int addr, QVector<quint16> *data, int invokeId)
{
    MBDebug();
    _appendCommand(new MBWriteRegistersCommand(conn, slaveId, addr, data, invokeId));
}

void MBThreadedConnManager::reportSlaveIdAsync(MBConnection *conn, int slaveId, QVector<quint8> *dest, int invokeId)
{
    MBDebug();
    _appendCommand(new MBReportSlaveIdCommand(conn, slaveId, dest, invokeId));
}

void MBThreadedConnManager::sendRawRequestAsync(MBConnection *conn, int slaveId, QVector<quint8> *req, int invokeId)
{
    MBDebug();
    _appendCommand(new MBRawRequestCommand(conn, slaveId, req, invokeId));
}

void MBThreadedConnManager::waitForConfirmationAsync(MBConnection *conn, int slaveId, QVector<quint8> *resp, int invokeId)
{
    MBDebug();
    _appendCommand(new MBWaitForConfirmationCommand(conn, slaveId, resp, invokeId));
}

void MBThreadedConnManager::_threadFinished()
{
    MBDebug();
    qDebug("%s: managed thread is finished", __FUNCTION__);
}

void MBThreadedConnManager::_nextCommandAdded()
{
    MBDebug();

    Q_ASSERT(0 == _currentCommand);

    //if we have any commands then lock, take one and unlock.
    //The one command taken is then executed (with signal emission (finished or error)) and deleted.
    if (!_commands.isEmpty()) {
        _mutex.lock();
        _currentCommand = _commands.takeFirst();
        _mutex.unlock();

        _currentCommand->exec();
        delete _currentCommand;
        _currentCommand = 0;
    }
    else {
        qDebug() << __PRETTY_FUNCTION__ << "commands are empty, skipping...";
    }
}
