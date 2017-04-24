// UtilTime functions
#ifndef __UTIL_TIME_HEADER_
#define __UTIL_TIME_HEADER_
#include "Util.h"

#ifdef __GNUC__
	//#include <sys/timeb.h>
	#include <sys/time.h>
#elif defined(_MSC_VER)
	#define HIGH_PRECISION
#endif

#ifdef DEBUG
	//#define USE_CHRONO 1
#else
	#define USE_CHRONO 1
#endif

BEGIN_Q_NAMESPACE

#ifdef HIGH_PRECISION
	
#endif

	inline qu64 qGetCurrentTime(void){
#ifdef _MSC_VER
		return GetTickCount64();
		// another windows chrono methods
		// GetTickCount()
		// timeGetTime()
		// QueryPerformanceCounter()
#elif defined(__GNUC__)
		//struct _timeb timeBuffer;
		//_ftime(&timeBuffer);
		//return (qu64)(timeBuffer.time*1000000 + timeBuffer.millitm*1000);
		struct timeval tv;
		gettimeofday(&tv, NULL);
		return tv.tv_sec*1000 + tv.tv_usec/1000;
#else
	#error "not supported yet"
#endif
	}

#ifdef USE_CHRONO
	#define FPS_CHRONO(_text) \
		{ \
			static int nbFrame = 0; \
			static q::Chrono chrono; \
			const int NB_FRAME_MAX = 120; \
			if(nbFrame == 0) \
			{ \
				chrono.Start(); \
			} \
			else if(nbFrame == NB_FRAME_MAX) \
			{ \
				qu64 diff = chrono.Stop(); \
				q::Print(_text, (1000*nbFrame)/((double)diff)); \
				nbFrame = 0; \
				chrono.Start(); \
			} \
			nbFrame++; \
		}

	#define START_CHRONO(_name) \
		Chrono _name; \
		_name.Start();

	#define END_CHRONO(_name) \
		_name.Stop()

	#define PRINT_CHRONO(_name) \
		_name.Print(#_name)

	class Chrono
	{
	public:
		inline Chrono(void) : m_Timer(0)
							, m_EndTimer(0){
		}

		inline void Start(void){
			m_Timer = qGetCurrentTime();
		}

		inline qu64 Stop(void){
			m_EndTimer = qGetCurrentTime();
			return m_EndTimer - m_Timer;
		}

		inline void Print(const char *_chronoName){
			qu64 diff = m_EndTimer - m_Timer;
			qf64 seconde = static_cast<qf64>(diff)/1000.;
			q::qPrint("Chrono %s diff %lu ms, %f s\n", _chronoName, diff, seconde);
		}

		inline qu64 GetTime(void){
			return m_EndTimer - m_Timer;
		}
	private:
		qu64 m_Timer;
		qu64 m_EndTimer;
	};
#else
	#define FPS_CHRONO(_text)
	#define START_CHRONO(_name) 
	#define END_CHRONO(_name) 
	#define PRINT_CHRONO(_name) 
#endif


	class Date
	{
	public:
		inline Date() : m_MilliSeconds(0.)
		{
		}

		inline ~Date()
		{
		}

		void SetTimeClock(void);
		void SetTime(double _milliSeconds);
		
		inline double GetTime(void)
		{
			return m_MilliSeconds;
		}

		void SetDateFromString(qString _str);
		qString ToStr(void);

	private:
		double m_MilliSeconds;
	};
END_Q_NAMESPACE

#endif
