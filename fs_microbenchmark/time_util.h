#ifndef __TIME_UTIL_H__
#define __TIME_UTIL_H__

#include <Windows.h>

FILETIME ft_start;
FILETIME ft_end;

#define TIMER_START() GetSystemTimeAsFileTime(&ft_start)
#define TIMER_END() GetSystemTimeAsFileTime(&ft_end)
#define TIMER_ELAPSE_SEC() ((double)((((ULONGLONG)ft_end.dwHighDateTime << 32) | (ULONGLONG)ft_end.dwLowDateTime ) - (((ULONGLONG)ft_start.dwHighDateTime << 32) | (ULONGLONG)ft_start.dwLowDateTime )) / 10000000.0)

#endif /* __TIME_UTIL_H__ */
