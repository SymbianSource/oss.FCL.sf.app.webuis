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
* Description:  Implementation of CPushMtmUiOperation.
*
*/



//  INCLUDES

#include "PushMtmUiOperation.h"
#include "PushMtmUtil.h"
#include <coemain.h>
#include <ErrorUI.h>

// ================= MEMBER FUNCTIONS =======================

// ---------------------------------------------------------
// CPushMtmUiOperation::CPushMtmUiOperation
// ---------------------------------------------------------
//
CPushMtmUiOperation::CPushMtmUiOperation( CMsvSession& aSession, 
                                          TMsvId aId, 
                                          TRequestStatus& aObserverStatus )
:   CPushMtmOperation( aSession, aId, aObserverStatus ), 
    iCoeEnv( *CCoeEnv::Static() ), 
    iResourceLoader( iCoeEnv ), 
    iResourceOpened( EFalse )
    {
    }

// ---------------------------------------------------------
// CPushMtmUiOperation::ConstructL
// ---------------------------------------------------------
//
void CPushMtmUiOperation::ConstructL()
    {
    iMtmUtil = CPushMtmUtil::NewL( iMsvSession );
    }

// ---------------------------------------------------------
// CPushMtmUiOperation::~CPushMtmUiOperation
// ---------------------------------------------------------
//
CPushMtmUiOperation::~CPushMtmUiOperation()
    {
    Cancel();
    delete iMtmUtil;
    iResourceLoader.Close();
    }

// ---------------------------------------------------------
// CPushMtmUiOperation::AssureResourceL
// ---------------------------------------------------------
//
void CPushMtmUiOperation::AssureResourceL( const TDesC& aResourceFile )
    {
    if ( !iResourceOpened )
        {
        // Add resource file.
        TFileName resourceFileName( aResourceFile );
        iResourceLoader.OpenL( resourceFileName );
        iResourceOpened = ETrue;
        }
    }

// ---------------------------------------------------------
// CPushMtmUiOperation::ShowGlobalErrorNoteL
// ---------------------------------------------------------
//
void CPushMtmUiOperation::ShowGlobalErrorNoteL( TInt aError ) const
    {
    CErrorUI* errorUi = CErrorUI::NewLC( iCoeEnv );
    if ( errorUi->ShowGlobalErrorNoteL( aError ) == EFalse )
        {
        User::Leave( KErrGeneral );
        }
    CleanupStack::PopAndDestroy( errorUi );
    }

// End of file.

