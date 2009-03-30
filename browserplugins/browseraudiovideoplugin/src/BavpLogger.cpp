/*
* Copyright (c) 2006 Nokia Corporation and/or its subsidiary(-ies).
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
* Description:  Interface to Browser for handling embedded video requests.
*
*/


// INCLUDE FILES
#include <flogger.h>
#include "BavpLogger.h"

// Logging methods for debugging. Only used in the UDEB version not UREL.
//
// aWrite: The aWrite boolean is used to allow the Log() methods to be left in the 
// code, but not write to a log file. To write to the log file, set aWrite to
// ETrue.  You should always set aWrite to EFalse before checking in,
// this will ensure the log file will not be filled with excessive entries.  This
// is important, because the Bavp requires on-hardware debugging.
//
// AudioVideo: The name of the directory (folder) where the log file is written.
// The path "C:\Logs\AudioVideo" must be specified to enable the logging. If no
// path is specified, no log file is created and populated.
//
// log: This is the file name that is created and logging entries added.
//
#ifdef _DEBUG
void Log( TBool aWrite, TPtrC aData )
    {
    if ( aWrite )
        {
         RFileLogger::WriteFormat( _L( "AudioVideo" ), _L( "bavp.log" ),
                                   EFileLoggingModeAppend, _L( "%S" ),
                                   &aData );
        }
    }

void Log( TBool aWrite, TPtrC aData, TInt aInt )
    {
    if ( aWrite )
        {
        RFileLogger::WriteFormat( _L( "AudioVideo" ), _L( "bavp.log" ),
                                  EFileLoggingModeAppend, _L( "%S, %d" ),
                                  &aData, aInt );
        }
    }

void Log( TBool aWrite, const TPtrC s, const TPtrC p)
    {
    if ( aWrite )
        {
        RFileLogger::WriteFormat( _L( "AudioVideo" ), _L( "bavp.log" ),
                                  EFileLoggingModeAppend, _L("%S, %S"),
                                  &s, &p);
        }
    }
    
 void Log( TBool aWrite, const TPtrC8 s, const TPtrC8 p)
    {
    if ( aWrite )
        {
        RFileLogger::WriteFormat( _L( "AudioVideo" ), _L( "bavp.log" ),
                                  EFileLoggingModeAppend, _L8("%S %S"),
                                  &s, &p);
        }
    }    
#else
void Log( TBool /*aWrite*/, TPtrC /*aData*/ )
    {
    }

void Log( TBool /*aWrite*/, TPtrC /*aData*/, TInt /*aInt*/ )
    {
    }
void Log( TBool /*aWrite*/, const TPtrC /*s*/, const TPtrC /*p*/)
    {
    }
void Log( TBool /*aWrite*/, const TPtrC8 /*s*/, const TPtrC8 /*p*/)
    {
    }    
#endif
