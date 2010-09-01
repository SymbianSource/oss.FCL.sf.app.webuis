/*
* Copyright (c) 2003 Nokia Corporation and/or its subsidiary(-ies).
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
* Description:  COperatorMenuDocument implementation
*
*/


// INCLUDE FILES
#include "OperatorMenuDocument.h"
#include "OperatorMenuAppUi.h"

// ================= MEMBER FUNCTIONS =======================

// constructor
COperatorMenuDocument::COperatorMenuDocument( CEikApplication& aApp )
: CAknDocument( aApp )    
    {
    }

// destructor
COperatorMenuDocument::~COperatorMenuDocument()
    {
    }

// EPOC default constructor can leave.
void COperatorMenuDocument::ConstructL()
    {
    }

// Two-phased constructor.
COperatorMenuDocument* COperatorMenuDocument::NewL( CEikApplication& aApp )
    {
    COperatorMenuDocument* self = new ( ELeave ) COperatorMenuDocument( aApp );
    CleanupStack::PushL( self );
    self->ConstructL();
    CleanupStack::Pop();
    return self;
    }
    
// ----------------------------------------------------
// COperatorMenuDocument::CreateAppUiL()
// constructs COperatorMenuAppUi
// ----------------------------------------------------
//
CEikAppUi* COperatorMenuDocument::CreateAppUiL()
    {
	return new (ELeave) COperatorMenuAppUi;   
    }

// End of File  
