/*
* Copyright (c) 2002-2006 Nokia Corporation and/or its subsidiary(-ies).
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
*      Implementation of CBrowserCommsModel.
*
*
*/


// INCLUDE FILES

#include <ActiveApDb.h>
#include <ApUtils.h>
#include <ApSelect.h>
#include <ApListItemList.h>
#include <ApListItem.h>
#include <BrowserNG.rsg>
#include "BrowserCommsModel.h"
#include "BrowserDialogs.h"
#include "Browser.hrh"

// CONSTANTS

/// Granularity of observer list.
LOCAL_D const TInt KObserverListGranularity = 4;

// ================= MEMBER FUNCTIONS =======================

// ---------------------------------------------------------
// CBrowserCommsModel::NewL
// ---------------------------------------------------------
//
CBrowserCommsModel* CBrowserCommsModel::NewL()
    {
    CBrowserCommsModel* model = new (ELeave) CBrowserCommsModel();
    CleanupStack::PushL( model );
    model->ConstructL();
    CleanupStack::Pop();    // model
    return model;
    }

// ---------------------------------------------------------
// CBrowserCommsModel::~CBrowserCommsModel
// ---------------------------------------------------------
//
CBrowserCommsModel::~CBrowserCommsModel()
    {
    delete iObservers;
    if ( iDb )
        {
        iDb->RemoveObserver( this );
        }
    delete iCachedApList;
    delete iDb;
    }

// ---------------------------------------------------------
// CBrowserCommsModel::CBrowserCommsModel
// ---------------------------------------------------------
//
CBrowserCommsModel::CBrowserCommsModel(): iUpdatePending( EFalse )
    {
    }

// ---------------------------------------------------------
// CBrowserCommsModel::ConstructL
// ---------------------------------------------------------
//
void CBrowserCommsModel::ConstructL()
    {
    iDb = CActiveApDb::NewL( EDatabaseTypeIAP );
    iDb->AddObserverL( this );
    iCachedApList = new (ELeave) CApListItemList();
    iObservers = new (ELeave) CArrayPtrFlat<MCommsModelObserver>
                                                ( KObserverListGranularity );
    iDelayedRefresh = ETrue;
    //GetAccessPointsL();
    }

// ---------------------------------------------------------
// CBrowserCommsModel::HandleApDbEventL
// ---------------------------------------------------------
//
void CBrowserCommsModel::HandleApDbEventL
( MActiveApDbObserver::TEvent aEvent )
    {
    // We try to refresh AP list in two cases:
    // 1. Database changed;
    // 2. Database unlocked and we have a pending update (previous update was
    //    unsuccessful).
    if ( aEvent == MActiveApDbObserver::EDbChanged ||
         ( aEvent == MActiveApDbObserver::EDbAvailable && iUpdatePending ) )
            {
            // Try to refresh cached AP list.
            iDelayedRefresh = ETrue;
            //GetAccessPointsL();
            NotifyObserversL();
            }
    // MActiveApDbObserver::EDbClosing:
    // MActiveApDbObserver::EDbAvailable:
       // These events will never come. Commdb cannot be closed
        // with a notifier still on it. (It must be deleted to close.)
    }

// ---------------------------------------------------------
// CBrowserCommsModel::ApDb
// ---------------------------------------------------------
//
CActiveApDb& CBrowserCommsModel::ApDb() const
    {
    return *iDb;
    }

// ---------------------------------------------------------
// CBrowserCommsModel::CommsDb
// ---------------------------------------------------------
//
CCommsDatabase& CBrowserCommsModel::CommsDb() const
    {
    return *(iDb->Database());
    }

// ---------------------------------------------------------
// CBrowserCommsModel::CopyAccessPointsL
// ---------------------------------------------------------
//
CApListItemList* CBrowserCommsModel::CopyAccessPointsL()
    {
    if ( iDelayedRefresh )
    	{
    	GetAccessPointsL();
    	iDelayedRefresh = EFalse;
    	}
    CApListItemList* accessPoints = new (ELeave) CApListItemList();
    CleanupStack::PushL( accessPoints );
    CApListItem* ap;
    TInt i;
    TInt count = iCachedApList->Count();
    for ( i = 0; i < count; i++ )
        {
        ap = CApListItem::NewLC( iCachedApList->At( i ) );
        accessPoints->AppendL( ap );
        CleanupStack::Pop();    // ap; owner is now the list.
        }
    CleanupStack::Pop();    // accessPoints
    return accessPoints;
    }

// ---------------------------------------------------------
// CBrowserCommsModel::AccessPoints
// ---------------------------------------------------------
//
const CApListItemList* CBrowserCommsModel::AccessPointsL()
    {
    if ( iDelayedRefresh )
    	{
    	GetAccessPointsL();
    	iDelayedRefresh = EFalse;
    	}
    return iCachedApList;
    }

// ---------------------------------------------------------
// CBrowserCommsModel::RefreshAccessPointsL
// ---------------------------------------------------------
//
void CBrowserCommsModel::RefreshAccessPointsL()
    {
    iDelayedRefresh = ETrue;
    //GetAccessPointsL();
    }

// ---------------------------------------------------------
// CBrowserCommsModel::AddObserverL
// ---------------------------------------------------------
//
void CBrowserCommsModel::AddObserverL( MCommsModelObserver& aObserver )
    {
    iObservers->AppendL( &aObserver );
    }

// ---------------------------------------------------------
// CBrowserCommsModel::RemoveObserver
// ---------------------------------------------------------
//
void CBrowserCommsModel::RemoveObserver( MCommsModelObserver& aObserver )
    {
    TInt i;
    TInt count = iObservers->Count();
    for ( i = 0; i < count; i++ )
        {
        if ( iObservers->At( i ) == &aObserver )
            {
            iObservers->Delete( i );
            break;
            }
        }
    }


// ---------------------------------------------------------
// CBrowserCommsModel::GetAccessPointsL
// ---------------------------------------------------------
//
void CBrowserCommsModel::GetAccessPointsL()
    {
    CApListItemList* apList = new (ELeave) CApListItemList();
    CleanupStack::PushL( apList );
    TRAPD( err, DoGetAccessPointsL( *apList ) );
    switch ( err )
        {
        case KErrNone:
            {
            // Got new data.
            delete iCachedApList;
            CleanupStack::Pop();    // apList: now member.
            iCachedApList = apList;
            iUpdatePending = EFalse;
            //NotifyObserversL();
            break;
            }

        case KErrLocked:
        case KErrAccessDenied:
        case KErrPermissionDenied:
            {
            // Could not access database. Update now pending, old data remains.
            CleanupStack::PopAndDestroy();  // apList
            iUpdatePending = ETrue;
            break;
            }

        default:
            {
            // Other error. Propagate.
            User::Leave( err );
            break;
            }
        }
    }

// ---------------------------------------------------------
// CBrowserCommsModel::DoGetAccessPointsL
// ---------------------------------------------------------
//
void CBrowserCommsModel::DoGetAccessPointsL( CApListItemList& aList ) const
    {
    CApSelect* apSelect = CApSelect::NewLC
        (
        *(iDb->Database()),
        KEApIspTypeAll,
//        KEApIspTypeAll | KEApIspTypeWAPMandatory,
        EApBearerTypeAll,
        KEApSortNameAscending,
		EIPv4 | EIPv6
        );
    (void)apSelect->AllListItemDataL( aList );
    CleanupStack::PopAndDestroy();  // apSelect
    }

// ---------------------------------------------------------
// CBrowserCommsModel::NotifyObserversL
// ---------------------------------------------------------
//
void CBrowserCommsModel::NotifyObserversL()
    {
    if ( iObservers )
        {
        TInt i;
        TInt count = iObservers->Count();
        for ( i = 0; i < count; i++ )
            {
            iObservers->At( i )->HandleCommsModelChangeL();
            }
        }
    }
// End of File
