#include "DelegateOpt.h"
#include "WorkerThreadStd.h"
#include "ThreadMsg.h"
#include "Timer.h"

#ifdef WIN32
#include <Windows.h>
#endif

using namespace std;
using namespace DelegateLib;

#define MSG_DISPATCH_DELEGATE	1
#define MSG_EXIT_THREAD			2
#define MSG_TIMER				3

//----------------------------------------------------------------------------
// WorkerThread
//----------------------------------------------------------------------------
WorkerThread::WorkerThread(const std::string& threadName) : m_thread(nullptr), m_timerExit(false), THREAD_NAME(threadName)
{
}

//----------------------------------------------------------------------------
// ~WorkerThread
//----------------------------------------------------------------------------
WorkerThread::~WorkerThread()
{
	ExitThread();
}

//----------------------------------------------------------------------------
// CreateThread
//----------------------------------------------------------------------------
bool WorkerThread::CreateThread()
{
	if (!m_thread)
	{
		m_thread = std::unique_ptr<std::thread>(new thread(&WorkerThread::Process, this));

#ifdef WIN32
		// Get the thread's native Windows handle
		auto handle = m_thread->native_handle();

		// Set the thread name so it shows in the Visual Studio Debug Location toolbar
		std::wstring wstr(THREAD_NAME.begin(), THREAD_NAME.end());
		HRESULT hr = SetThreadDescription(handle, wstr.c_str());
		if (FAILED(hr))
		{
			// Handle error if needed
		}
#endif
	}
	return true;
}

//----------------------------------------------------------------------------
// GetThreadId
//----------------------------------------------------------------------------
std::thread::id WorkerThread::GetThreadId()
{
	if (m_thread == nullptr)
		throw std::invalid_argument("Thread pointer is null");

	return m_thread->get_id();
}

//----------------------------------------------------------------------------
// GetCurrentThreadId
//----------------------------------------------------------------------------
std::thread::id WorkerThread::GetCurrentThreadId()
{
	return this_thread::get_id();
}

//----------------------------------------------------------------------------
// GetQueueSize
//----------------------------------------------------------------------------
size_t WorkerThread::GetQueueSize()
{
	lock_guard<mutex> lock(m_mutex);
	return m_queue.size();
}

//----------------------------------------------------------------------------
// ExitThread
//----------------------------------------------------------------------------
void WorkerThread::ExitThread()
{
	if (!m_thread)
		return;

	// Create a new ThreadMsg
	std::shared_ptr<ThreadMsg> threadMsg(new ThreadMsg(MSG_EXIT_THREAD, 0));

	// Put exit thread message into the queue
	{
		lock_guard<mutex> lock(m_mutex);
		m_queue.push(threadMsg);
		m_cv.notify_one();
	}

    m_thread->join();
    m_thread = nullptr;
}

//----------------------------------------------------------------------------
// DispatchDelegate
//----------------------------------------------------------------------------
void WorkerThread::DispatchDelegate(std::shared_ptr<DelegateLib::DelegateMsg> msg)
{
	if (m_thread == nullptr)
		throw std::invalid_argument("Thread pointer is null");

	// Create a new ThreadMsg
    std::shared_ptr<ThreadMsg> threadMsg(new ThreadMsg(MSG_DISPATCH_DELEGATE, msg));

	// Add dispatch delegate msg to queue and notify worker thread
	std::unique_lock<std::mutex> lk(m_mutex);
	m_queue.push(threadMsg);
	m_cv.notify_one();
}

//----------------------------------------------------------------------------
// TimerThread
//----------------------------------------------------------------------------
void WorkerThread::TimerThread()
{
    while (!m_timerExit)
    {
        std::this_thread::sleep_for(100ms);

        std::shared_ptr<ThreadMsg> threadMsg (new ThreadMsg(MSG_TIMER, 0));

        // Add timer msg to queue and notify worker thread
        std::unique_lock<std::mutex> lk(m_mutex);
        m_queue.push(threadMsg);
        m_cv.notify_one();
    }
}

//----------------------------------------------------------------------------
// Process
//----------------------------------------------------------------------------
void WorkerThread::Process()
{
    m_timerExit = false;
    std::thread timerThread(&WorkerThread::TimerThread, this);

	while (1)
	{
		std::shared_ptr<ThreadMsg> msg;
		{
			// Wait for a message to be added to the queue
			std::unique_lock<std::mutex> lk(m_mutex);
			while (m_queue.empty())
				m_cv.wait(lk);

			if (m_queue.empty())
				continue;

			msg = m_queue.front();
			m_queue.pop();
		}

		switch (msg->GetId())
		{
			case MSG_DISPATCH_DELEGATE:
			{
				// Get pointer to DelegateMsg data from queue msg data
				auto delegateMsg = msg->GetData();
				ASSERT_TRUE(delegateMsg);

				auto invoker = delegateMsg->GetDelegateInvoker();
				ASSERT_TRUE(invoker);

				// Invoke the delegate destination target function
				bool success = invoker->Invoke(delegateMsg);
				ASSERT_TRUE(success);
				break;
			}

            case MSG_TIMER:
				Timer::ProcessTimers();
				break;

			case MSG_EXIT_THREAD:
			{
                m_timerExit = true;
                timerThread.join();
                return;
			}

			default:
				throw std::invalid_argument("Invalid message ID");
		}
	}
}

