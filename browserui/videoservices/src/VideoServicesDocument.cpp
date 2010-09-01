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
* Description:  CVideoServicesDocument implementation
*
*/


// INCLUDE FILES
#include "VideoServicesDocument.h"
#include "VideoServicesAppUi.h"

// ================= MEMBER FUNCTIONS =======================

// constructor
CVideoServicesDocument::CVideoServicesDocument( CEikApplication& aApp )
: CAknDocument( aApp )    
    {
    }

// destructor
CVideoServicesDocument::~CVideoServicesDocument()
    {
    }

// EPOC default constructor can leave.
void CVideoServicesDocument::ConstructL()
    {
    }

// Two-phased constructor.
CVideoServicesDocument* CVideoServicesDocument::NewL( CEikApplication& aApp )
    {
    CVideoServicesDocument* self = new ( ELeave ) CVideoServicesDocument( aApp );
    CleanupStack::PushL( self );
    self->ConstructL();
    CleanupStack::Pop();
    return self;
    }
    
// ----------------------------------------------------
// CVideoServicesDocument::CreateAppUiL()
// constructs CVideoServicesAppUi
// ----------------------------------------------------
//
CEikAppUi* CVideoServicesDocument::CreateAppUiL()
    {
	return new (ELeave) CVideoServicesAppUi;   
    }

// End of File  
