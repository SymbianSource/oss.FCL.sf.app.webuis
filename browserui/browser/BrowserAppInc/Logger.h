/*
* Copyright (c) 2002-2006 Nokia Corporation and/or its subsidiary(-ies).
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
* Description:  Implemented logger functionality of the module
*
*/


#ifndef BROWSER_LOGGER_H
#define BROWSER_LOGGER_H

#if defined( I__BROWSER_LOG_ENABLED ) || defined ( I__BROWSER_PERF_LOG_ENABLED )

// INCLUDES
#include <bldvariant.hrh>
#include <e32std.h>
#include <e32def.h>
#include <e32cmn.h>
#include <flogger.h>


// log files are stored to KDebugLogDirFull folder
_LIT(KDebugLogDependency,       "C:\\Logs\\");
_LIT(KDebugLogDoubleBackSlash,  "\\");

_LIT( KDebugLogDir,             "Browser");
_LIT( KDebugLogFile,            "Browser.log");
_LIT( KDebugPerfLogFile,        "BrowserPerf.log");
_LIT( KLogMessageAppBanner,     "Browser: module (%d.%d.%d) started" );
_LIT( KPerLogBanner, 			"*************Performance Logging *****************");
// _LIT( KDebugLogTimeFormatString,"%H:%T:%S:%*C3" );
_LIT( KDebugLogExit,            "Browser: module exit" );

#endif

#if defined( I__BROWSER_LOG_ENABLED )

// MACROS
// indentation is not supported by the hardware at this moment
#if defined( __WINSCW__ )
#define I__BROWSER_LOG_INDENT
#endif

/**
* Use this macro in order to initialize logger :
* - create log directory,
* - write version information into the log file
*/
#define LOG_CREATE { TFileName path( KDebugLogDependency ); \
    path.Append( KDebugLogDir ); \
    path.Append( KDebugLogDoubleBackSlash ); \
    RFs& fs = CEikonEnv::Static()->FsSession(); \
    fs.MkDirAll( path ); \
    RFileLogger::WriteFormat( KDebugLogDir, KDebugLogFile, EFileLoggingModeOverwrite, KLogMessageAppBanner ); \
    }

/**
* Use this macro for writing information about exiting.
*/
#define LOG_DELETE { RFileLogger::Write( KDebugLogDir, KDebugLogFile, EFileLoggingModeAppend, KDebugLogExit ); }

/**
* Use this function at any points of a function for writing a hex dump 
* of the specified data to the log.
* @param p The data the is to be converted to a hex string.
* @param l How many of the characters in p are to be converted. 
* Conversion always starts from position 0 within p.
*/
//#define LOG_WRITE_HEXDUMP( p, l ) { RFileLogger::HexDump( KDebugLogDir, KDebugLogFile, EFileLoggingModeAppend, _S("RLog_Ex"), _S("RL_cont"), p, l ); }

// -------------------------------------------------

class MLoggerProvider
    {
    public:
        virtual TInt* LoggerIndent() = 0;
        virtual void LoggerIndentIncrement( TInt aValue ) = 0;
        virtual void LoggerIndentDecrement( TInt aValue ) = 0;
    };

class CBrowserLogger: public CBase
    {
    public:
        CBrowserLogger( const TDesC& aFunc );
        ~CBrowserLogger();
    
    /**
    * Write formatted log.
    * @param aFmt Format string.
    */
    static void Write( TRefByValue<const TDesC16> aFmt, ... );
    
    /**
    * Write formatted log.
    * @param aFmt Format string.
    * @param aList Variable argument list.
    */
    static void Write( TRefByValue<const TDesC16> aFmt, VA_LIST& aList );

    /**
    * Write formatted log.
    * @param aFmt Format string.
    */
    static void Write(TRefByValue<const TDesC8> aFmt, ... );

    /**
    * Write formatted log.
    * @param aFmt Format string.
    * @param aList Variable argument list.
    */
    static void Write( TRefByValue<const TDesC8> aFmt, VA_LIST& aList );

    protected:
        TPtrC   iFunc;
        const MLoggerProvider* iOwner;
    // make the indentation available by 'static' keyword
#if defined ( I__BROWSER_LOG_INDENT )
        static
#endif
        TInt iIndent;
    };

// -------------------------------------------------
// Usage:
// - call LOG_ENTERFN( "function name" ); at entry point of a function
// otherwise BROWSER_LOG call won't work
// - call BROWSER_LOG( ( _L( "log string" ) ) ); OR
// you can add parameters to this call like this:
// - BROWSER_LOG( ( _L( "format string: %d" ), value ) );
// -------------------------------------------------
// future development if _L() will be removed:
// #define _BL( a ) TPtrC( _S( a ) )
// BROWSER_LOG( ( _BL( "format string: %d" ), a ) )
// -------------------------------------------------

#define LOG_ENTERFN( a ) _LIT( _br_temp_log_string18, a ); \
    CBrowserLogger _br_entry_exit( _br_temp_log_string18 )
#define BROWSER_LOG( _params_ ) \
    _br_entry_exit.Write _params_
#define LOG_WRITE( _params_ ) \
    BROWSER_LOG( ( _L( _params_ ) ) )
#define LOG_WRITE_FORMAT( _format_, _params_ ) \
    BROWSER_LOG( ( _L( _format_ ), _params_ ) )

#else // I__BROWSER_LOG_ENABLED

// Empty macros
#define LOG_CREATE
#define LOG_DELETE
#define LOG_ENTERFN( a )
#define LOG_WRITE( a )
#define LOG_WRITE_FORMAT( a, b )
#define LOG_WRITE_TIMESTAMP( a )
#define LOG_WRITE_HEXDUMP( p, l )
#define BROWSER_LOG( a )

#endif // I__BROWSER_LOG_ENABLED


#ifdef I__BROWSER_PERF_LOG_ENABLED

/**
* Use this macro in order to initialize performance logger:
* - create log directory,
* 
*/
#define PERFLOG_CREATE { TFileName path( KDebugLogDependency ); \
    path.Append( KDebugLogDir ); \
    path.Append( KDebugLogDoubleBackSlash ); \
    RFs& fs = CEikonEnv::Static()->FsSession(); \
    fs.MkDirAll( path ); \
    RFileLogger::WriteFormat( KDebugLogDir, KDebugPerfLogFile, EFileLoggingModeOverwrite, KPerLogBanner);\
    }

#define PERFLOG_LOCAL_INIT \
	TTime __time1_brbrbr;\
	TTime __time2_brbrbr;\
	TTimeIntervalMicroSeconds diff_brbrbr;


#define PERFLOG_STOPWATCH_START \
	__time1_brbrbr.HomeTime();
	
#define PERFLOG_STOPWATCH_STOP \
	__time2_brbrbr.HomeTime();\
	diff_brbrbr = __time2_brbrbr.MicroSecondsFrom( __time1_brbrbr );

#define PERFLOG_WRITE( _param_ ){\
	_LIT( __perf_temp_1234_, "%S took %Ld ms" );\
	_LIT(__perf_temp_param, _param_ );\
	RFileLogger::WriteFormat( KDebugLogDir, KDebugPerfLogFile, EFileLoggingModeAppend, __perf_temp_1234_, &__perf_temp_param(), diff_brbrbr .Int64() );\
	}
	
#define PERFLOG_STOP_WRITE( _params_ )\
	PERFLOG_STOPWATCH_STOP\
	PERFLOG_WRITE( _params_ )

//It may look ugly in code
#define PERF( _a, _b ) \
	{\
	PERFLOG_LOCAL_INIT\
	PERFLOG_STOPWATCH_START\
	_b;\
	PERFLOG_STOP_WRITE( _a )\
	}


#else

#define PERFLOG_CREATE
#define PERFLOG_LOCAL_INIT
#define PERFLOG_STOPWATCH_START
#define PERFLOG_STOPWATCH_STOP
#define PERFLOG_WRITE(a)
#define PERFLOG_STOP_WRITE(a)
#define PERF( _a, _b ) _b
#endif
	
	

#endif	// BROWSER_LOGGER_H

// End of file
