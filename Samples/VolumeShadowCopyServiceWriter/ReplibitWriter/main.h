/*
**++
**
** Copyright (c) 2018 eFolder Inc
**
**
** Module Name:
**
**	main.h
**
*/

#ifndef _MAIN_H_
#define _MAIN_H_

extern "C" int __cdecl wmain(__in int argc, __in_ecount(argc) wchar_t** argv);
BOOL WINAPI ConsoleHandler(DWORD dwSignal);

#endif
