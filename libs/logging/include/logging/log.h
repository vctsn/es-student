#pragma once

#define LOG_LEVEL_NONE 0
#define LOG_LEVEL_ERR  1
#define LOG_LEVEL_INF  2
#define LOG_LEVEL_DBG  3

#ifndef LOG_LEVEL
#define LOG_LEVEL LOG_LEVEL_INF
#endif

void log_print(const char *level, const char *func, int line, const char *fmt, ...);

#if LOG_LEVEL >= LOG_LEVEL_ERR
#define LOG_ERR(...) log_print("err", __func__, __LINE__, __VA_ARGS__)
#else
#define LOG_ERR(...) ((void)0)
#endif

#if LOG_LEVEL >= LOG_LEVEL_INF
#define LOG_INF(...) log_print("inf", __func__, __LINE__, __VA_ARGS__)
#else
#define LOG_INF(...) ((void)0)
#endif

#if LOG_LEVEL >= LOG_LEVEL_DBG
#define LOG_DBG(...) log_print("dbg", __func__, __LINE__, __VA_ARGS__)
#else
#define LOG_DBG(...) ((void)0)
#endif
