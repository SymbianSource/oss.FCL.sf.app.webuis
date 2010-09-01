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
*      Implementation of CWmlBrowserBookmarksEditFormApModel.
*      
*
*/


// INCLUDE FILES

#include <coemain.h>
#include <ApListItem.h>
#include <ApListItemList.h>
#include <AknQueryValueText.h>
#include "BrowserBookmarksEditFormApModel.h"
#include "CommsModel.h"
#include "Browser.hrh"
#include <BrowserNG.rsg>

// CONSTANTS

/// Granularity of iDesCArrayText.
LOCAL_D const TInt KGranularity = 4;

// ================= MEMBER FUNCTIONS =======================

// ---------------------------------------------------------
// CBrowserBookmarksEditFormApModel::NewL
// ---------------------------------------------------------
//
CBrowserBookmarksEditFormApModel*
CBrowserBookmarksEditFormApModel::NewL( MCommsModel& aCommsModel )
    {
    CBrowserBookmarksEditFormApModel* model =
        new (ELeave) CBrowserBookmarksEditFormApModel( aCommsModel );
    CleanupStack::PushL( model );
    model->ConstructL();
    CleanupStack::Pop();    // model
    return model;
    }

// ---------------------------------------------------------
// CBrowserBookmarksEditFormApModel::~CBrowserBookmarksEditFormApModel
// ---------------------------------------------------------
//
CBrowserBookmarksEditFormApModel::~CBrowserBookmarksEditFormApModel()
    {
    delete iTextValue;
    delete iTextArray;
    delete iDesCArrayText;
    delete iApList;
    }

// ---------------------------------------------------------
// CBrowserBookmarksEditFormApModel::ReadApDataL
// ---------------------------------------------------------
//
void CBrowserBookmarksEditFormApModel::ReadApDataL()
    {
    // Work on new lists, until they are fully created.
    CApListItemList* apList = iCommsModel->CopyAccessPointsL();
    //CApListItemList* apList = new (ELeave) CApListItemList(); // Test empty.
    CleanupStack::PushL( apList );
    CDesCArrayFlat* desCArrayText =
        new (ELeave) CDesCArrayFlat( KGranularity );
    CleanupStack::PushL( desCArrayText );

    TInt count = apList->Count();
    if ( count )
        {
        // We have some access points; fill iDesCArray with names (otherwise
        // it stays empty).

        // Add "Default" first.
        HBufC* defaultApName = CCoeEnv::Static()->AllocReadResourceLC
            ( R_BROWSER_BOKMARKS_TEXT_DEFAULT_WAPAP );
        desCArrayText->AppendL( *defaultApName );
        CleanupStack::PopAndDestroy();  // defaultApName

        // Then continue with real AP names.
        TInt i;
        TPtrC apName;
        for ( i = 0; i < count; i++ )
            {
            apName.Set( apList->At( i )->Name() );
            if ( apName.Length() > KBrowserMaxWapApName )
                {
                // Since AP names are of unlimited length, we might have to
                // trim.
                apName.Set( apName.Left( KBrowserMaxWapApName ) );
                }
            desCArrayText->AppendL( apName );
            }
        }

    // Replace inst vars with new lists only after success.
    CleanupStack::Pop( 2 ); // desCArrayText, apList: become members
    delete iDesCArrayText;          // (does not leave)
    iDesCArrayText = desCArrayText; // (does not leave)
    delete iApList;                 // (does not leave)
    iApList = apList;               // (does not leave)
    iTextArray->SetArray( *iDesCArrayText );
    }

// ---------------------------------------------------------
// CBrowserBookmarksEditFormApModel::SetCurrentWapApL
// ---------------------------------------------------------
//
void CBrowserBookmarksEditFormApModel::SetCurrentWapApL
( const TFavouritesWapAp& aAp )
    {
    TInt count = iApList->Count();

    if ( count )
        {
        // Default value index is KErrNotFound ("Invalid" is displayed).
        // If AP is Null, or specified but does not exist, this value is used.
        TInt index = KErrNotFound;
        if ( aAp.IsDefault() )
            {
            // For "Default" Ap-s, highlight 0th element "Default".
            index = 0;
            }
        else if ( !aAp.IsNull() )
            {
            // Get index of element having this ApId. If not found, index stays
            // KErrNotFound (That will display the "Invalid" text).
            //
            // "Null" and "Default" checks already done: call to ApId is safe.
            TUint32 apId = aAp.ApId();
            TInt i;
            for ( i = 0; i < count; i++ )
                {
                if ( iApList->At( i )->Uid() == apId )
                    {
                    // Got it; i is the index of the AP in the array. Add 1
                    // to it, since 0th element is "Default".
                    index = i + 1;
                    break;
                    }
                }
            }
        iTextValue->SetCurrentValueIndex( index );
        }
    }

// ---------------------------------------------------------
// CBrowserBookmarksEditFormApModel::CurrentWapAp
// ---------------------------------------------------------
//
TFavouritesWapAp CBrowserBookmarksEditFormApModel::CurrentWapAp() const
    {
    TFavouritesWapAp ap;

    ap.SetNull();   // "Null" value used if no AP-s exist or AP is "Invalid".

    TInt count = iApList->Count();
    if ( count )
        {
        TInt index = iTextValue->CurrentValueIndex();
        if ( index == 0 )
            {
            // Oth element ("Default").
            ap.SetDefault();
            }
        else if ( index > 0 && index <= count )
            {
            // "Index-1" is a valid index into array (real AP).
            //
            // (CurrentValueIndex may return an out-of-bounds value, if AP is
            // "Invalid". This value is -1; but is not well published; so
            // instead of testing for -1, we do proper range checking. That
            // is the safest we can do, and adds minuscule overhead.)
            ap.SetApId( iApList->At( index - 1 )->Uid() );
            }
        }

    return ap;
    }

// ---------------------------------------------------------
// CBrowserBookmarksEditFormApModel::ApCount
// ---------------------------------------------------------
//
TInt CBrowserBookmarksEditFormApModel::ApCount()
    {
    return iApList->Count();
    }

// ---------------------------------------------------------
// CBrowserBookmarksEditFormApModel::CBrowserBookmarksEditFormApModel
// ---------------------------------------------------------
//
CBrowserBookmarksEditFormApModel::CBrowserBookmarksEditFormApModel
( MCommsModel& aCommsModel )
: iCommsModel( &aCommsModel )
    {
    }

// ---------------------------------------------------------
// CBrowserBookmarksEditFormApModel::ConstructL
// ---------------------------------------------------------
//
void CBrowserBookmarksEditFormApModel::ConstructL()
    {
    iApList = new (ELeave) CApListItemList();
    iDesCArrayText = new (ELeave) CDesCArrayFlat( KGranularity );
    iTextArray = CAknQueryValueTextArray::NewL();
    iTextArray->SetArray( *iDesCArrayText );
    iTextValue = CAknQueryValueText::NewL();
    iTextValue->SetArrayL( iTextArray );

    ReadApDataL();
    }

// End of File
