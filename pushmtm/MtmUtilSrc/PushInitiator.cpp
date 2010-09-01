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
* Description:  Implementation of CPushInitiator.
*
*/



// INCLUDE FILES

#include "PushInitiator.h"

// ================= MEMBER FUNCTIONS =======================

// ---------------------------------------------------------
// CPushInitiator::CPushInitiator
// ---------------------------------------------------------
//
EXPORT_C CPushInitiator::CPushInitiator() 
:   CBase(),
    iType( ETypeIpv4 )
    {}

// ---------------------------------------------------------
// CPushInitiator::~CPushInitiator
// ---------------------------------------------------------
//
EXPORT_C CPushInitiator::~CPushInitiator() 
    {
    delete iAddress;
    }

// ---------------------------------------------------------
// CPushInitiator::SetAddressL
// ---------------------------------------------------------
//
EXPORT_C void CPushInitiator::SetAddressL( const TDesC& aAddr, 
                                           TAddrType aType )
    {
    HBufC* temp = aAddr.AllocL();
    delete iAddress;
    iAddress = temp;
    iType = aType;
    }

//  End of File.
