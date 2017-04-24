#include "common/util/UtilTime.h"

#ifdef _MSC_VER
	#define WIN32_LEAN_AND_MEAN
	#include <windows.h>
#endif

BEGIN_Q_NAMESPACE

void Date::SetTimeClock(void)
{
	//m_MilliSeconds = static_cast<double>(igstk::RealTimeClock::GetTimeStamp());
	//m_MilliSeconds = GetTickCount();
	m_MilliSeconds = static_cast<double>(qGetCurrentTime());
	//Print("Date::SetTimeClock %f\n", m_MilliSeconds);
}

void Date::SetTime(double _milliSeconds)
{
	m_MilliSeconds = _milliSeconds;
}

void Date::SetDateFromString(qString _str)
{
	m_MilliSeconds = atof(_str.c_str());
	qPrint("Date::SetDateFromString %f vs %s\n", m_MilliSeconds, _str.c_str());
}

qString Date::ToStr(void)
{
	char str[MAX_STR_BUFFER];
	qSprintf(str, MAX_STR_BUFFER, "%f", m_MilliSeconds);
	return qString(str);
}

END_Q_NAMESPACE
