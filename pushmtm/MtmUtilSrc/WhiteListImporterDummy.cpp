/*
* Copyright (c) 2003, 2004 Nokia Corporation and/or its subsidiary(-ies).
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
* Description:  Dummy implementation of CWhiteListConverter.
*
*/


// INCLUDE FILES

#include "WhiteListImporter.h"
#include "PushMtmLog.h"


// ================= MEMBER FUNCTIONS =======================

// ---------------------------------------------------------
// CWhiteListConverter::NewL
// ---------------------------------------------------------
//
CWhiteListConverter* CWhiteListConverter::NewL
    ( CPushInitiatorList& aPushInitiatorList )
    {
	PUSHLOG_ENTERFN("CWhiteListConverter::NewL Dummy");
    CWhiteListConverter* converter = 
        new (ELeave) CWhiteListConverter( aPushInitiatorList );
    return converter;
	PUSHLOG_LEAVEFN("CWhiteListConverter::NewL Dummy");
    }

// ---------------------------------------------------------
// CWhiteListConverter::~CWhiteListConverter
// ---------------------------------------------------------
//
CWhiteListConverter::~CWhiteListConverter()
    {
	PUSHLOG_ENTERFN("CWhiteListConverter::~CWhiteListConverter Dummy");
    delete iBuf;
    iNextCh = NULL; // Only used.
    iMaxCh = NULL; // Only used.
	PUSHLOG_LEAVEFN("CWhiteListConverter::~CWhiteListConverter Dummy");
    }

// ---------------------------------------------------------
// CWhiteListConverter::Buffer2ListL
// ---------------------------------------------------------
//
void CWhiteListConverter::Buffer2ListL( const TDesC& /*aSource*/ )
    {
	PUSHLOG_ENTERFN("CWhiteListConverter::Buffer2ListL Dummy");
	PUSHLOG_LEAVEFN("CWhiteListConverter::Buffer2ListL Dummy");
    }

// ---------------------------------------------------------
// CWhiteListConverter::List2BufferL
// ---------------------------------------------------------
//
HBufC* CWhiteListConverter::List2BufferL()
    {
	PUSHLOG_ENTERFN("CWhiteListConverter::Buffer2ListL Dummy");
	PUSHLOG_WRITE("Returning KNullDesC");
    return KNullDesC().AllocL();
	PUSHLOG_LEAVEFN("CWhiteListConverter::Buffer2ListL Dummy");
    }
    
// ---------------------------------------------------------
// CWhiteListConverter::CWhiteListConverter
// ---------------------------------------------------------
//
CWhiteListConverter::CWhiteListConverter( CPushInitiatorList& aPushInitiatorList ) 
:   iPushInitiatorList( aPushInitiatorList )
    {
	PUSHLOG_ENTERFN("CWhiteListConverter::CWhiteListConverter Dummy");
	PUSHLOG_LEAVEFN("CWhiteListConverter::CWhiteListConverter Dummy");
    }

// ---------------------------------------------------------
// CWhiteListConverter::ConstructL
// ---------------------------------------------------------
//
void CWhiteListConverter::ConstructL()
    {
	PUSHLOG_ENTERFN("CWhiteListConverter::ConstructL Dummy");
	PUSHLOG_LEAVEFN("CWhiteListConverter::ConstructL Dummy");
    }

// ---------------------------------------------------------
// CWhiteListConverter::GetChar
// ---------------------------------------------------------
//
inline void CWhiteListConverter::GetChar()
    {
	PUSHLOG_ENTERFN("CWhiteListConverter::GetChar Dummy");
	PUSHLOG_LEAVEFN("CWhiteListConverter::GetChar Dummy");
    }

// ---------------------------------------------------------
// CWhiteListConverter::NextLineL
// ---------------------------------------------------------
//
TBool CWhiteListConverter::NextLineL()
    {
	PUSHLOG_ENTERFN("CWhiteListConverter::NextLineL Dummy");
	PUSHLOG_WRITE("Returning ETrue");
    return ETrue;
	PUSHLOG_LEAVEFN("CWhiteListConverter::NextLineL Dummy");
    }

// ---------------------------------------------------------
// CWhiteListConverter::NextTokenL
// ---------------------------------------------------------
//
TPtrC CWhiteListConverter::NextTokenL( TUint /*aStopChar*/ )
    {
	PUSHLOG_ENTERFN("CWhiteListConverter::NextTokenL Dummy");
	PUSHLOG_WRITE("Returning TPtrC");
    return TPtrC();
	PUSHLOG_LEAVEFN("CWhiteListConverter::NextTokenL Dummy");
    }

// ---------------------------------------------------------
// CWhiteListConverter::AttrsL
// ---------------------------------------------------------
//
void CWhiteListConverter::AttrsL()
    {
	PUSHLOG_ENTERFN("CWhiteListConverter::AttrsL Dummy");
	PUSHLOG_LEAVEFN("CWhiteListConverter::AttrsL Dummy");
    }

