#ifndef __THREAD_HEADER_
#define __THREAD_HEADER_
#include "common/util/Util.h"

//#undef USE_MULTITHREADING
#ifdef USE_MULTITHREADING
	#ifdef _MSC_VER
		#include <windows.h>
		#define USE_WINDOWS_THREAD
		//#define USE_QT_THREAD
		#define Q_USE_THREAD
	#elif defined(__GNUC__)
		// no thread for now
		//#define USE_PTHREAD
		//#define Q_USE_THREAD
		#undef USE_MULTITHREADING
	#else
		#error "not supported yet"
	#endif

	//#define __DEBUG_THREAD 1
	#ifdef __DEBUG_THREAD
		#define Q_PRINT_THREAD(...) q::qPrint(__VA_ARGS__)
	#else
		#define Q_PRINT_THREAD(...)
	#endif

	#define NB_PROCS (NB_THREADS)

	#define WAIT_DEFAULT_TIME (3000) // in ms
#endif

BEGIN_Q_NAMESPACE

#ifdef USE_WINDOWS_THREAD
	//#define USE_CRITICAL_SECTION
	#define USE_MUTEX
	#ifdef USE_CRITICAL_SECTION
		class qMutex{
		public:
			inline qMutex(void){
				BOOL bret = InitializeCriticalSectionAndSpinCount(&m_section, 0x00000400);
				qAssert(bret);
				if(!bret){
					qPrint("qMutex::qMutex() error\n");
				}
			}

			inline ~qMutex(void){
				DeleteCriticalSection(&m_section);
			}

			inline void lock(void){
				EnterCriticalSection(&m_section);
			}

			inline void unlock(void){
				LeaveCriticalSection(&m_section);
			}
		private:
			CRITICAL_SECTION m_section;
		};
	#else
		class qMutex{
		public:
			inline qMutex(void){
				m_Mutex = CreateMutex(NULL // default security attributes
					, FALSE // initially not owned
					, NULL); // unnamed mutex
				qAssert(m_Mutex != NULL);
				if(m_Mutex == NULL){
					qPrint("qMutex::qMutex() error\n");
				}
			}

			inline ~qMutex(void){
				CloseHandle(m_Mutex);
			}

			inline void lock(void){
				DWORD dwWaitResult = WaitForSingleObject(m_Mutex // handle to mutex
					, INFINITE); // no time-out interval
				qAssert(dwWaitResult == WAIT_OBJECT_0);
				if(dwWaitResult != WAIT_OBJECT_0){
					qPrint("qMutex::lock() error\n");
				}
			}

			inline void unlock(void){
				BOOL bret = ReleaseMutex(m_Mutex);
				qAssert(bret);
				if(!bret){
					qPrint("qMutex::unlock() error\n");
				}
			}
		private:
			HANDLE m_Mutex;
		};
	#endif

	class qMutexLocker{
	public:
		inline qMutexLocker(qMutex *_mutex): m_Mutex(_mutex){
			m_Mutex->lock();
		}

		inline ~qMutexLocker(void){
			m_Mutex->unlock();
		}

	private:
		qMutex *m_Mutex;
	};

	class qThread{
	public:
		inline qThread(void){
			Q_PRINT_THREAD("qThread::qThread()\n");
			m_IsRunning = Q_FALSE;
		}

		inline ~qThread(void){
			Q_PRINT_THREAD("qThread::~qThread()\n");
			terminate();
		}

		inline void start(void){
			Q_PRINT_THREAD("qThread::start()\n");
			{
				qMutexLocker mutexLocker(&m_IsRunningMutex);
				qAssert(m_IsRunning == Q_FALSE);
				if(m_IsRunning == Q_FALSE){
					m_HThread = CreateThread(NULL, NULL, qThread::staticStart, (LPVOID)this, NULL, &m_ThreadId);
#ifdef __DEBUG_THREAD
					if(m_HThread == NULL){
						Q_PRINT_THREAD("CreateThread failed\n");
					}
#endif
					qAssert(m_HThread != NULL);
					//SetThreadPriority(m_HThread, THREAD_PRIORITY_TIME_CRITICAL);
					m_IsRunning = Q_TRUE;
				}
				else{
					qPrint("qThread::start() error thread already started\n");
				}
			}
		}

		// warning don't call this function in the concerned thread
		qbool wait(unsigned long time = ULONG_MAX){
			Q_PRINT_THREAD("%d qThread::wait()\n", m_ThreadId);
			DWORD ret = WaitForSingleObject(m_HThread, ULONG_MAX);
			if(ret == WAIT_OBJECT_0){
				return Q_TRUE;
			}
			return Q_FALSE;
		}

	protected:
		// warning don't call this function in the concerned thread
		inline void terminate(void){
			Q_PRINT_THREAD("%d qThread::terminate()\n", m_ThreadId);
			{
				qMutexLocker mutexLocker(&m_IsRunningMutex);
				if(m_IsRunning == Q_TRUE){
					TerminateThread(m_HThread, 0);
					CloseHandle(m_HThread);
					m_IsRunning = Q_FALSE;
				}
			}
		}

	private:
		virtual void run(void) = 0;

		inline static DWORD WINAPI staticStart(LPVOID lpParam){
			qThread *thread = static_cast<qThread*>(lpParam);
			Q_PRINT_THREAD("%d qThread::staticStart()\n", thread->m_ThreadId);
			thread->run();
			return 0;
		}

		qbool m_IsRunning;
		qMutex m_IsRunningMutex;
		DWORD m_ThreadId;
		HANDLE m_HThread;
	};

	#define qSleep(...) Sleep(__VA_ARGS__)
	#define qMainThreadSleep(...) Sleep(__VA_ARGS__)

#elif defined(USE_QT_THREAD)
	typedef QMutex qMutex;
	typedef QMutexLocker qMutexLocker;
	typedef QThread qThread;
	
	#define qSleep(...) msleep(__VA_ARGS__)
	// TODO find better solution but now it's the only solution I found to sleep main thread in mevislab
	#define qMainThreadSleep(...) { \
			QMutex mutex; \
			mutex.lock(); \
			QWaitCondition waitCondition; \
			waitCondition.wait(&mutex, __VA_ARGS__); \
			mutex.unlock(); \
		}
		
#elif defined(USE_PTHREAD)

#endif

#ifdef USE_MULTITHREADING
	#define BEGIN_MUTEX(_mutex) \
		{ \
			static qMutex _mutex; \
			qMutexLocker _mutex##Locker(&_mutex);

	#define END_MUTEX(_mutex) \
		}
#endif

END_Q_NAMESPACE

#endif
