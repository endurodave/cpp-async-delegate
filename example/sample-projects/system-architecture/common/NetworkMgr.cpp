/// @file
/// @see https://github.com/endurodave/cpp-async-delegate
/// David Lafreniere, 2025.

#include "NetworkMgr.h"

using namespace DelegateMQ;
using namespace std;

MulticastDelegateSafe<void(DataPackage&)> NetworkMgr::DataPackageRecv;

NetworkMgr::NetworkMgr() :
    m_thread("NetworkMgr"),
    m_argStream(ios::in | ios::out | ios::binary)
{
    // Set the delegate interfaces
    m_dataPackageDel.SetStream(&m_argStream);
    m_dataPackageDel.SetSerializer(&m_dataPackageSer);
    m_dataPackageDel.SetDispatcher(&m_dispatcher);
    m_dataPackageDel.SetErrorHandler(MakeDelegate(this, &NetworkMgr::ErrorHandler));
    m_dataPackageDel = MakeDelegate(this, &NetworkMgr::RecvSensorData, DATA_PACKAGE_ID);

    // Set the transport
#ifdef SERVER_APP
    m_transportSend.Create(Transport::Type::PUB, "tcp://*:5555");
    m_dispatcher.SetTransport(&m_transportSend);

    m_transportRecv.Create(Transport::Type::SUB, "tcp://localhost:5556");
#else
    m_transportRecv.Create(Transport::Type::SUB, "tcp://localhost:5555");

    m_transportSend.Create(Transport::Type::PUB, "tcp://*:5556");
    m_dispatcher.SetTransport(&m_transportSend);

    // Client receive async delegates
    m_receiveIdMap[DATA_PACKAGE_ID] = &m_dataPackageDel;
#endif

    // Create the receiver thread
    m_thread.CreateThread();
}

NetworkMgr::~NetworkMgr()
{
    m_thread.ExitThread();
}

void NetworkMgr::Start()
{
    // Start a timer to poll data
    m_recvTimer.Expired = MakeDelegate(this, &NetworkMgr::Poll, m_thread);
    m_recvTimer.Start(std::chrono::milliseconds(50));
}

void NetworkMgr::Stop()
{
    m_recvTimer.Stop();
    m_thread.ExitThread();
}

void NetworkMgr::SendStart()
{
    //TODO
}

void NetworkMgr::SendStop()
{
    //TODO
}

void NetworkMgr::SendDataPackage(DataPackage& data)
{
    // Reinvoke SendDataPackage call onto NetworkMgr thread
    if (Thread::GetCurrentThreadId() != m_thread.GetThreadId())
        return MakeDelegate(this, &NetworkMgr::SendDataPackage, m_thread)(data);

    // Send data to remote. 
    m_dataPackageDel(data);
}

// Poll called periodically on m_thread context
void NetworkMgr::Poll()
{
    // Get incoming data
    MsgHeader header;
    auto arg_data = m_transportRecv.Receive(header);
     
    // Incoming remote delegate data arrived?
    if (!arg_data.str().empty())
    {
        auto receiveDelegate = m_receiveIdMap[header.GetId()];
        if (receiveDelegate)
        {
            // Invoke the receiver target function with the sender's argument data
            receiveDelegate->Invoke(arg_data);
        }
        else
        {
            cout << "Received delegate not found!" << endl;
        }
    }
}

void NetworkMgr::ErrorHandler(DelegateError, DelegateErrorAux)
{
    // Handle communication error as necessary
    ASSERT_TRUE(0);
}

void NetworkMgr::RecvSensorData(DataPackage& data)
{
    DataPackageRecv(data);
}

