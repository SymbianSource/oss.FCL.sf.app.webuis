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
*      Logging macros for White List Adapter.
*      
*
*/


#ifndef WHITELISTADAPTER_LOGGER_H
#define WHITELISTADAPTER_LOGGER_H

// INCLUDES

#include <e32std.h>
#include <e32def.h>
#include <flogger.h>

// TYPES

enum TWhiteListAdapterLogMask       ///< Log mask bits.
    {
    ELogOff         = 0x00000000,   ///< Don't log.
    ELogBasic       = 0x00000001,   ///< Log only basic activities.
    ELogDetailed    = 0x00000002,   ///< Detailed activities use this mask.
    ELogAll         = 0xFFFFFFFF    ///< Log all.
    };

// MACROS

// Determines what to log. Construct this from TWhiteListAdapterLogMask values.
#define WHITELISTADAPTER_LOG_MASK ELogAll
/// Determines log detail (0==basic level).
#define WHITELISTADAPTER_LOG_LEVEL 5

// CLASS DECLARATION

/**
* Logger class.
*/
class WhiteListAdapterLogger
    {
    public:     // new methods

    /**
    * Write formatted log.
    * @param aMask Log mask.
    * @param aLevel Log level.
    * @param aFmt Format string.
    */
    static void Write
        ( TInt32 aMask, TInt aLevel, TRefByValue<const TDesC16> aFmt, ... );

    /**
    * Write formatted log.
    * @param aMask Log mask.
    * @param aLevel Log level.
    * @param aFmt Format string.
    * @param aList Variable argument list.
    */
    static void Write
        (
        TInt32 aMask,
        TInt aLevel,
        TRefByValue<const TDesC16> aFmt,
        VA_LIST& aList
        );

    /**
    * Write formatted log.
    * @param aMask Log mask.
    * @param aLevel Log level.
    * @param aFmt Format string.
    */
    static void Write
        ( TInt32 aMask, TInt aLevel, TRefByValue<const TDesC8> aFmt, ... );

    /**
    * Write formatted log.
    * @param aMask Log mask.
    * @param aLevel Log level.
    * @param aFmt Format string.
    * @param aList Variable argument list.
    */
    static void Write
        (
        TInt32 aMask,
        TInt aLevel,
        TRefByValue<const TDesC8> aFmt,
        VA_LIST& aList
        );

    /**
    * Write hex dump.
    * @param aMask Log mask.
    * @param aLevel Log level.
    * @param aHeader Header string.
    * @param aMargin Margin.
    * @param aPtr Data.
    * @param aLen Data length.
    */
    static void HexDump
        (
        TInt32 aMask,
        TInt aLevel, 
        const TText* aHeader,
        const TText* aMargin,
        const TUint8* aPtr,
        TInt aLen
        );
    };

#ifdef __TEST_WHITELISTADAPTER_LOG__

    /// Write formatted to log.
    #define CLOG( body ) WhiteListAdapterLogger::Write body
    /// Write hex dump.
    #define CDUMP( body ) WhiteListAdapterLogger::HexDump body
    /// Guard "log-only" argument name with this (expands to argument).
    #define LOG_ONLY( argName ) argName

#else /* not defined __TEST_WHITELISTADAPTER_LOG__ */

    /// Do nothing (log disabled).
    #define CLOG( body )
    /// Do nothing (log disabled).
    #define CDUMP( body )
    /// Guard "log-only" argument name with this (expands to nothing).
    #define LOG_ONLY( argName )


#endif /* def __TEST_WHITELISTADAPTER_LOG__ */

#endif /* def WHITELISTADAPTER_LOGGER_H */
