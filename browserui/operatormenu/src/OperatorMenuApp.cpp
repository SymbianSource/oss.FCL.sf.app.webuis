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
* Description:  COperatorMenuApp Implementation
*
*/


// INCLUDE FILES
#include "OperatorMenuApp.h"
#include "OperatorMenuDocument.h"
#include <eikstart.h>

// ================= MEMBER FUNCTIONS =======================

LOCAL_C CApaApplication* NewApplication()
	{
	return new COperatorMenuApp;
	}
	
GLDEF_C TInt E32Main()
	{
	return EikStart::RunApplication( NewApplication );
	}

// ---------------------------------------------------------
// COperatorMenuApp::AppDllUid()
// Returns application UID
// ---------------------------------------------------------
//
TUid COperatorMenuApp::AppDllUid() const
    {
    return KUidOperatorMenu;
    }

   
// ---------------------------------------------------------
// COperatorMenuApp::CreateDocumentL()
// Creates COperatorMenuDocument object
// ---------------------------------------------------------
//
CApaDocument* COperatorMenuApp::CreateDocumentL()
    {
    return COperatorMenuDocument::NewL( *this );
    }
// End of File  

