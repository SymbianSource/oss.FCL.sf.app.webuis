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
* Description:  Data supplier for responses loaded from cache
*
*/



// INCLUDE FILES
#include "PushMtmCacheDataSupplier.h"

// EXTERNAL DATA STRUCTURES

// EXTERNAL FUNCTION PROTOTYPES

// CONSTANTS

// MACROS

// LOCAL CONSTANTS AND MACROS

// MODULE DATA STRUCTURES

// LOCAL FUNCTION PROTOTYPES

// FORWARD DECLARATIONS

// ============================= LOCAL FUNCTIONS ===============================


// ============================ MEMBER FUNCTIONS ===============================

// -----------------------------------------------------------------------------
// CPushMtmCacheDataSupplier::CPushMtmCacheDataSupplier
// C++ default constructor can NOT contain any code, that
// might leave.
// -----------------------------------------------------------------------------
//
CPushMtmCacheDataSupplier::CPushMtmCacheDataSupplier(
    HBufC8* aBody ) 
    : iBody( aBody )
    {
    }

// -----------------------------------------------------------------------------
// CPushMtmCacheDataSupplier::NewL
// Two-phased constructor.
// -----------------------------------------------------------------------------
//
CPushMtmCacheDataSupplier* CPushMtmCacheDataSupplier::NewL(
    HBufC8* aBody )
    {
    CPushMtmCacheDataSupplier* self = new(ELeave) CPushMtmCacheDataSupplier( aBody );
    return self;
    }

// Destructor
CPushMtmCacheDataSupplier::~CPushMtmCacheDataSupplier()
    {
    ReleaseData();
    }

// -----------------------------------------------------------------------------
// CPushMtmCacheDataSupplier::GetNextDataPart
// Return the next chunk of response body from the Cache
// -----------------------------------------------------------------------------
//
TBool CPushMtmCacheDataSupplier::GetNextDataPart(
    TPtrC8 &aDataChunk )
    {
    aDataChunk.Set( *iBody );
    return ETrue;
    }

//  End of File
