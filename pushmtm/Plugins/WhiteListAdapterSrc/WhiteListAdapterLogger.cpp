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
* Description: 
*      Implementation of class WhiteListAdapterLogger.   
*      
*
*/


#ifdef __TEST_WHITELISTADAPTER_LOG__

// INCLUDE FILES
    // System includes
#include <flogger.h>

    // User includes
#include "WhiteListAdapterLogger.h"

// ================= CONSTANTS =======================

/// White List Adapter logging directory.
_LIT( KWhiteListAdapterLogDir, "WLA" );
/// White List Adapter log file name.
_LIT( KWhiteListAdapterLogFile, "Log.txt" );

// ================= MEMBER FUNCTIONS =======================

// ---------------------------------------------------------
// WhiteListAdapterLogger::Write()
// ---------------------------------------------------------
//
void WhiteListAdapterLogger::Write
( TInt32 aMask, TInt aLevel, TRefByValue<const TDesC16> aFmt, ... )
    {
    if( ( aMask & WHITELISTADAPTER_LOG_MASK ) &&
        ( aLevel <= WHITELISTADAPTER_LOG_LEVEL ) )
        {
        VA_LIST list;
        VA_START( list, aFmt );
        RFileLogger::WriteFormat
            (
            KWhiteListAdapterLogDir,
            KWhiteListAdapterLogFile,
            EFileLoggingModeAppend,
            aFmt,
            list
            );
        VA_END( list );
        }
    }

// ---------------------------------------------------------
// WhiteListAdapterLogger::Write()
// ---------------------------------------------------------
//
void WhiteListAdapterLogger::Write
( TInt32 aMask, TInt aLevel, TRefByValue<const TDesC16> aFmt, VA_LIST& aList )
    {
    if( ( aMask & WHITELISTADAPTER_LOG_MASK ) &&
        ( aLevel <= WHITELISTADAPTER_LOG_LEVEL ) )
        {
        RFileLogger::WriteFormat
            (
            KWhiteListAdapterLogDir,
            KWhiteListAdapterLogFile,
            EFileLoggingModeAppend,
            aFmt,
            aList
            );
        }
    }

// ---------------------------------------------------------
// WhiteListAdapterLogger::Write()
// ---------------------------------------------------------
//
void WhiteListAdapterLogger::Write
( TInt32 aMask, TInt aLevel, TRefByValue<const TDesC8> aFmt, ... )
    {
    if( ( aMask & WHITELISTADAPTER_LOG_MASK ) &&
        ( aLevel <= WHITELISTADAPTER_LOG_LEVEL ) )
        {
        VA_LIST list;
        VA_START( list, aFmt );
        RFileLogger::WriteFormat
            (
            KWhiteListAdapterLogDir,
            KWhiteListAdapterLogFile,
            EFileLoggingModeAppend,
            aFmt,
            list
            );
        VA_END( list );
        }
    }

// ---------------------------------------------------------
// WhiteListAdapterLogger::Write()
// ---------------------------------------------------------
//
void WhiteListAdapterLogger::Write
( TInt32 aMask, TInt aLevel, TRefByValue<const TDesC8> aFmt, VA_LIST& aList )
    {
    if( ( aMask & WHITELISTADAPTER_LOG_MASK ) &&
        ( aLevel <= WHITELISTADAPTER_LOG_LEVEL ) )
        {
        RFileLogger::WriteFormat
            (
            KWhiteListAdapterLogDir,
            KWhiteListAdapterLogFile,
            EFileLoggingModeAppend,
            aFmt,
            aList
            );
        }
    }

// ---------------------------------------------------------
// WhiteListAdapterLogger::HexDump()
// ---------------------------------------------------------
//
void WhiteListAdapterLogger::HexDump
        (
        TInt32 aMask,
        TInt aLevel, 
        const TText* aHeader,
        const TText* aMargin,
        const TUint8* aPtr,
        TInt aLen
        )
    {
    if( ( aMask & WHITELISTADAPTER_LOG_MASK ) &&
        ( aLevel <= WHITELISTADAPTER_LOG_LEVEL ) )
        {
        RFileLogger::HexDump
            (
            KWhiteListAdapterLogDir,
            KWhiteListAdapterLogFile,
            EFileLoggingModeAppend,
            aHeader,
            aMargin,
            aPtr,
            aLen
            );
        }
    }

#endif // __TEST_WHITELISTADAPTER_LOG__
