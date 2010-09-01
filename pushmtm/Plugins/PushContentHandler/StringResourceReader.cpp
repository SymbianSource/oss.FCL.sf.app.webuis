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
*     Implementation of CStringResourceReader
*
*
*/


// INCLUDE FILES

#include "StringResourceReader.h"
#include <f32file.h>
#include <barsread.h>
#include <bautils.h>

// ================= MEMBER FUNCTIONS =======================

// ---------------------------------------------------------
// CStringResourceReader::CStringResourceReader
// ---------------------------------------------------------
//
CStringResourceReader::CStringResourceReader
    ( RFs& aFs, const TDesC& aRscFileWithPathAndDrive )
:   CBase(), iFs( aFs ), iInitialized( EFalse )
    {
    iRscFileName.Copy( aRscFileWithPathAndDrive );
    }

// ---------------------------------------------------------
// CStringResourceReader::~CStringResourceReader
// ---------------------------------------------------------
//
CStringResourceReader::~CStringResourceReader()
    {
    iResourceFile.Close();
    }

// ---------------------------------------------------------
// CStringResourceReader::AllocReadResourceL
// ---------------------------------------------------------
//
HBufC* CStringResourceReader::AllocReadResourceL( TInt aResId )
    {
    InitializeL();
    //
    HBufC8* buf8 = iResourceFile.AllocReadLC( aResId );
#ifdef _UNICODE
    const TPtrC buf( (const TUint16*)buf8->Ptr(), buf8->Size()/2 );
#else
    const TPtrC buf( buf8->Ptr(), buf8->Size() );
#endif
    HBufC* retBuf = buf.AllocL();
    CleanupStack::PopAndDestroy( buf8 );

    return retBuf;
    }

// ---------------------------------------------------------
// CStringResourceReader::AllocReadResourceLC
// ---------------------------------------------------------
//
HBufC* CStringResourceReader::AllocReadResourceLC( TInt aResId )
    {
    HBufC* temp = AllocReadResourceL( aResId );
    CleanupStack::PushL( temp );
    return temp;
    }

// ---------------------------------------------------------
// CStringResourceReader::InitializeL
// ---------------------------------------------------------
//
void CStringResourceReader::InitializeL()
    {
    if ( !iInitialized )
        {
        TFileName resourceFileName( iRscFileName );
        BaflUtils::NearestLanguageFile( iFs, resourceFileName );
        iResourceFile.OpenL( iFs, resourceFileName );
        iResourceFile.ConfirmSignatureL( iResourceFile.SignatureL() );
        iInitialized = ETrue;
        }
    }

// End of file.
