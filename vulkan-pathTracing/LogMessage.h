#pragma once

#ifdef _DEBUG

#include <stdio.h>

#define LOG_MSG(a, ...) print_msg(a, __VA_ARGS__)
#define LOG_WARN(a, ...) print_warn(a, __VA_ARGS__)
#define LOG_CRIT(a, ...) print_crit(a, __VA_ARGS__)

template <typename ...Args>
void print_msg(const char *msg, Args... args)
{
	printf("%s : MESSAGE : ", __TIMESTAMP__);
	printf(msg, args...);
	printf("\n");
}

template <typename ...Args>
void print_warn(const char *msg, Args... args)
{
	printf("%s : WARNING : ", __TIMESTAMP__);
	printf(msg, args...);
	printf("\n");
}

template <typename ...Args>
void print_crit(const char *msg, Args... args)
{
	printf("%s : CRITICAL : ", __TIMESTAMP__);
	printf(msg, args...);
	printf("\n");
}

#else

#define LOG_MSG(a, ...)
#define LOG_WARN(...)
#define LOG_CRIT(...)

#endif