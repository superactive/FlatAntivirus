#ifndef GUIDEBUG_H_
#define GUIDEBUG_H_

#ifdef _DEBUG

void dbg_msgbox (const TCHAR* msg, ...);
void dbg_writelog (const TCHAR* fn, const TCHAR* fmt, ...);

#define DBG_MSGBOX(msg, ...)			dbg_msgbox (msg, __VA_ARGS__)
#define DBG_WRITELOG(fn, fmt, ...)		dbg_writelog (fn, fmt, __VA_ARGS__)

#else

#define DBG_MSGBOX
#define DBG_WRITELOG

#endif

#endif