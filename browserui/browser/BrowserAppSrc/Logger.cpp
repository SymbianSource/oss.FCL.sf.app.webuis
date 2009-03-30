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
* Description:  Implemented logger functionality of the module
*
*/


#if defined( I__BROWSER_LOG_ENABLED ) 

// INCLUDES

#include "Logger.h"

// CONSTANTS

// the indentation value used in every function entry and exit points
// use value 0 to switch indentation off
const TInt KBrowserIndentValue = 0;

// the left x character of this string is used to indent
// function entry and exit points. DO NOT cut is smaller!
// it has to be synched with the call stack level and KBrowserIndentValue:
// .Length() / KBrowserIndentValue >= max size of callstack
_LIT( KBrowserLoggerIndentString,
"                                                                      " );
_LIT( KDebugLogEnterFn,         "Browser: %S-> %S" );
_LIT( KDebugLogLeaveFn,         "Browser: %S<- %S" );

// ---------------------------------------------------------------------------

// initialize static member variable
#if defined ( I__BROWSER_LOG_INDENT )
TInt CBrowserLogger::iIndent( 0 );
#endif

// ---------------------------------------------------------------------------

CBrowserLogger::CBrowserLogger( const TDesC& aFunc )
        : iFunc( aFunc )
#if ! defined ( I__BROWSER_LOG_INDENT )
        , iIndent( 0 )
#endif
    {
    TPtrC a = KBrowserLoggerIndentString().Left( iIndent );
    iIndent += KBrowserIndentValue;
    RFileLogger::WriteFormat( KDebugLogDir, KDebugLogFile,
        EFileLoggingModeAppend, KDebugLogEnterFn, &a, &iFunc );
    }

// ---------------------------------------------------------------------------

CBrowserLogger::~CBrowserLogger()
    {
    iIndent -= KBrowserIndentValue;
    TPtrC a = KBrowserLoggerIndentString().Left( iIndent );
    RFileLogger::WriteFormat( KDebugLogDir, KDebugLogFile,
        EFileLoggingModeAppend, KDebugLogLeaveFn, &a, &iFunc );
    }

// ---------------------------------------------------------------------------

void CBrowserLogger::Write( TRefByValue<const TDesC16> aFmt, ... )
    {
    VA_LIST list;
    VA_START( list, aFmt );
    RFileLogger::WriteFormat( KDebugLogDir, KDebugLogFile, 
        EFileLoggingModeAppend, aFmt, list );
    VA_END( list );
    }

// ---------------------------------------------------------------------------

void CBrowserLogger::Write( TRefByValue<const TDesC16> aFmt, VA_LIST& aList )
    {
    RFileLogger::WriteFormat( KDebugLogDir, KDebugLogFile,
        EFileLoggingModeAppend, aFmt, aList );
    }

// ---------------------------------------------------------------------------

void CBrowserLogger::Write(TRefByValue<const TDesC8> aFmt, ... )
    {
    VA_LIST list;
    VA_START( list, aFmt );
    RFileLogger::WriteFormat( KDebugLogDir, KDebugLogFile,
        EFileLoggingModeAppend, aFmt, list );
    VA_END( list );
    }

// ---------------------------------------------------------------------------

void CBrowserLogger::Write( TRefByValue<const TDesC8> aFmt, VA_LIST& aList )
    {
    RFileLogger::WriteFormat( KDebugLogDir, KDebugLogFile,
        EFileLoggingModeAppend, aFmt, aList );
    }
    
    
// Code segment for displaying an info note dialog box.  Can be used later as a debug aid.

    /*
    TInt    width, height;
    HBufC* header;
    TBuf<100> message; 

    message.Format(_L("width: %d ; height: %d"), width, height );

    header = StringLoader::LoadLC( R_WML_SECURITY_VIEW_TITLE );    

    CAknMessageQueryDialog* dlg = CAknMessageQueryDialog::NewL( message );

    dlg->PrepareLC( R_BROWSER_SECURITY_INFO );
    CAknPopupHeadingPane* hPane = dlg->QueryHeading();
    if ( hPane )
    {
    hPane->SetTextL( *header );
    }
    dlg->RunLD();
    CleanupStack::PopAndDestroy( 1 ); // header
    */


#endif // I__BROWSER_LOG_ENABLED

// End of file
