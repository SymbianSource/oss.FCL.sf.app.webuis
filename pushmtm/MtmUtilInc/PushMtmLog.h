/*
* Copyright (c) 2002 Nokia Corporation and/or its subsidiary(-ies).
* All rights reserved.
* This component and the accompanying materials are made available
* under the terms of the License "Eclipse Public License v1.0"
* which accompanies this distribution, and is available
* at the URL "http://www.eclipse.org/legal/epl-v10.html".
*
* Initial Contributors:
* Nokia Corporation - initial contribution.
*
* Contributors:
*
* Description:  Logger facility for the Push MTM modules.
*
*/



#ifndef PUSHMTMLOG_H
#define PUSHMTMLOG_H

//
#ifdef __TEST_LOG__
//

#include <e32std.h>
#include <e32def.h>
#include <flogger.h>

_LIT( KPushLogDir,                   "PushMtm" );
_LIT( KPushLogFile,                  "PushLog.txt" );
_LIT( KPushLogEnterFn,               "-> %S" );
_LIT( KPushLogLeaveFn,               "<- %S" );
_LIT( KPushLogTimeFormatString,      "%H:%T:%S:%*C2" );
_LIT( KPushLogTimeStampFormatString, "%S %S" );

#define PUSHLOG_ENTERFN(a)        {_LIT(temp, a); RFileLogger::WriteFormat(KPushLogDir, KPushLogFile, EFileLoggingModeAppend, KPushLogEnterFn, &temp);}
#define PUSHLOG_LEAVEFN(a)        {_LIT(temp, a); RFileLogger::WriteFormat(KPushLogDir, KPushLogFile, EFileLoggingModeAppend, KPushLogLeaveFn, &temp);}
#define PUSHLOG_WRITE(a)          {_LIT(temp, a); RFileLogger::Write(KPushLogDir, KPushLogFile, EFileLoggingModeAppend, temp);}
#define PUSHLOG_WRITE_FORMAT(a,b) {_LIT(temp, a); RFileLogger::WriteFormat(KPushLogDir, KPushLogFile, EFileLoggingModeAppend, temp, b);}
#define PUSHLOG_WRITE_FORMAT2(a,b,c) {_LIT(temp, a); RFileLogger::WriteFormat(KPushLogDir, KPushLogFile, EFileLoggingModeAppend, temp, b, c);}
#define PUSHLOG_WRITE_TIMESTAMP(a) {_LIT(temp, a); TTime time; time.HomeTime(); TBuf<32> timeBuf; TRAPD(err, time.FormatL( timeBuf, KPushLogTimeFormatString )); if (!err) {RFileLogger::WriteFormat(KPushLogDir, KPushLogFile, EFileLoggingModeAppend, KPushLogTimeStampFormatString, &temp, &timeBuf);} }
// Use HEXDUMP with a TDesC8-derived parameter.
#define PUSHLOG_HEXDUMP(aBuf)     {RFileLogger::HexDump(KPushLogDir, KPushLogFile, EFileLoggingModeAppend, _S(" "), _S(" "), aBuf.Ptr(), aBuf.Size());}
// Guard "log-only" argument name with this (expands to argument).
#define LOG_ONLY( argName ) argName

//
#else // #ifndef __TEST_LOG__
//

#define PUSHLOG_ENTERFN(a)
#define PUSHLOG_LEAVEFN(a)
#define PUSHLOG_WRITE(a)
#define PUSHLOG_WRITE_FORMAT(a,b)
#define PUSHLOG_WRITE_FORMAT2(a,b,c)
#define PUSHLOG_WRITE_TIMESTAMP(a)
#define PUSHLOG_HEXDUMP(aBuf)
// Guard "log-only" argument name with this (expands to nothing).
#define LOG_ONLY( argName )

//
#endif // __TEST_LOG__
//
#endif // PUSHMTMLOG_H

// End of File.
