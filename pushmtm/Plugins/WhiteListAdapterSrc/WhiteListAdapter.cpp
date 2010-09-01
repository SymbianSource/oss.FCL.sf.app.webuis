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
*     Handles white list settings in provisioning.
*
*/


// INCLUDE FILES

	// User includes
#include "PushInitiator.h"
#include "PushInitiatorList.h"
#include "PushMtmSettings.h"
#include "WhiteListAdapter.h"
#include "WhiteListAdapterDef.h"
#include "WhiteListAdapterItem.h"
#include "WhiteListAdapterLogger.h"
#include "WhiteListAdapterPanic.h"
#include "WhiteListAdapterUids.h"
	// System includes
#include <CWPCharacteristic.h>
#include <CWPParameter.h>

// CONSTANTS

// 'Push enabled' flag settings
const TInt KPushEnabledFlagLength = 1;
const TUint16 KPushEnabledFlagTrue = '1';
const TUint16 KPushEnabledFlagFalse = '0';

// Granularity value for dynamic arrays
const TInt KDynamicArrayGranularity = 3;

// ================= MEMBER FUNCTIONS ======================

// ---------------------------------------------------------
// CWhiteListAdapter::CWhiteListAdapter
// ---------------------------------------------------------
//
CWhiteListAdapter::CWhiteListAdapter() : CWPAdapter(),
iItems( KDynamicArrayGranularity ),
iItemsToBeAdded( KDynamicArrayGranularity ),
iLogicalProxyPushSupport( EPushDontCare ), iPhysicalProxyIsVisited( EFalse )
	{
    CLOG( ( ELogBasic, WHITELISTADAPTER_LOG_LEVEL,
        _L( "WhiteListAdapter c'tor" ) ) );
	}

// ---------------------------------------------------------
// CWhiteListAdapter::ConstructL
// ---------------------------------------------------------
//
void CWhiteListAdapter::ConstructL()
	{
    CLOG( ( ELogBasic, WHITELISTADAPTER_LOG_LEVEL,
        _L( "-> WhiteListAdapter::ConstructL" ) ) );

    iMsvSession = CMsvSession::OpenSyncL( *this );
	iPushMtmSettings = CPushMtmSettings::NewL();

    CLOG( ( ELogBasic, WHITELISTADAPTER_LOG_LEVEL,
        _L( "<- WhiteListAdapter::ConstructL" ) ) );
	}

// ---------------------------------------------------------
// CWhiteListAdapter::NewL
// ---------------------------------------------------------
//
CWhiteListAdapter* CWhiteListAdapter::NewL()
	{
	CWhiteListAdapter* self = new (ELeave) CWhiteListAdapter;

	CleanupStack::PushL( self );

	self->ConstructL();

	CleanupStack::Pop();	// self

	return self;
	}

// ---------------------------------------------------------
// CWhiteListAdapter::~CWhiteListAdapter
// ---------------------------------------------------------
//
CWhiteListAdapter::~CWhiteListAdapter()
	{
    CLOG( ( ELogBasic, WHITELISTADAPTER_LOG_LEVEL,
        _L( "-> WhiteListAdapter d'tor" ) ) );

	delete iCurrentItem;

	delete iPushMtmSettings; // It has to be deleted before iMsvSession, 
    // because it uses iMsvSession->FileSession().
	delete iMsvSession;

    CLOG( ( ELogDetailed, WHITELISTADAPTER_LOG_LEVEL,
        _L( "WhiteListAdapter d'tor, iItemsToBeAdded.Count:%d" ),
        iItemsToBeAdded.Count() ) );
    iItemsToBeAdded.ResetAndDestroy();
    iItemsToBeAdded.Close();

    CLOG( ( ELogDetailed, WHITELISTADAPTER_LOG_LEVEL,
        _L( "WhiteListAdapter d'tor, iItems.Count:%d" ), iItems.Count() ) );
	iItems.ResetAndDestroy();
	iItems.Close();

    CLOG( ( ELogBasic, WHITELISTADAPTER_LOG_LEVEL,
        _L( "<- WhiteListAdapter d'tor" ) ) );
	}

// ---------------------------------------------------------
// CWhiteListAdapter::ItemCount
// ---------------------------------------------------------
//
TInt CWhiteListAdapter::ItemCount() const
	{
    CLOG( ( ELogBasic, WHITELISTADAPTER_LOG_LEVEL,
        _L( "WhiteListAdapter::ItemCount %d" ), iItems.Count() ) );

	return iItems.Count();
	}

// ---------------------------------------------------------
// CWhiteListAdapter::SummaryTitle
// ---------------------------------------------------------
//
const TDesC16& CWhiteListAdapter::SummaryTitle( TInt /*aIndex*/ ) const
	{
    // As WL text will never be shown, we simply return an empty string here.
	return KNullDesC16();
	}

// ---------------------------------------------------------
// CWhiteListAdapter::SummaryText
// ---------------------------------------------------------
//
const TDesC16& CWhiteListAdapter::SummaryText( TInt /*aIndex*/ ) const
	{
    // As WL text will never be shown, we simply return an empty string here.
	return KNullDesC16();
	}

// ---------------------------------------------------------
// CWhiteListAdapter::SaveL
// ---------------------------------------------------------
//
void CWhiteListAdapter::SaveL( TInt aItem )
	{
	__ASSERT_DEBUG( 0 <= aItem && aItem < iItems.Count(),
				Panic( EArrayIndexOutOfBounds ) );

    CLOG( ( ELogBasic, WHITELISTADAPTER_LOG_LEVEL,
        _L( "-> WhiteListAdapter::SaveL %d" ), aItem ) );

	CPushInitiator* pi = new (ELeave) CPushInitiator;
	CleanupStack::PushL( pi );
	pi->SetAddressL( iItems[aItem]->Address(),
					 iItems[aItem]->AddressType() );

	TUint32 entryId = iPushMtmSettings->PushInitiatorList().AddL( pi );
	CleanupStack::Pop( pi );	// pi, ownership taken over
    // Save the ID of the added entry:
    iItems[aItem]->SetId( entryId );

	iPushMtmSettings->SaveL();

    CLOG( ( ELogBasic, WHITELISTADAPTER_LOG_LEVEL,
        _L( "<- WhiteListAdapter::SaveL" ) ) );
    }

// ---------------------------------------------------------
// CWhiteListAdapter::CanSetAsDefault
// ---------------------------------------------------------
//
TBool CWhiteListAdapter::CanSetAsDefault( TInt /*aItem*/ ) const
	{
	return EFalse;
	}

// ---------------------------------------------------------
// CWhiteListAdapter::SetAsDefaultL
// ---------------------------------------------------------
//
void CWhiteListAdapter::SetAsDefaultL( TInt /*aItem*/ )
	{
	}

// ---------------------------------------------------------
// CWhiteListAdapter::DetailsL
// ---------------------------------------------------------
//
TInt CWhiteListAdapter::DetailsL( TInt /*aItem*/,
                                  MWPPairVisitor& /*aVisitor*/ )
	{
	return KErrNotSupported;
	}

// ---------------------------------------------------------
// CWhiteListAdapter::ContextExtension
// ---------------------------------------------------------
//
TInt CWhiteListAdapter::ContextExtension( MWPContextExtension*& aExtension )
    {
    aExtension = this;
    return KErrNone;
    }

// ---------------------------------------------------------
// CWhiteListAdapter::VisitL
// ---------------------------------------------------------
//
void CWhiteListAdapter::VisitL( CWPCharacteristic& aCharacteristic )
	{
	if ( aCharacteristic.Type() == KWPPxLogical )
		{
        CLOG( ( ELogBasic, WHITELISTADAPTER_LOG_LEVEL,
            _L( "-> WhiteListAdapter::VisitL PXLOGICAL" ) ) );

		aCharacteristic.AcceptL( *this );

        CLOG( ( ELogBasic, WHITELISTADAPTER_LOG_LEVEL,
            _L( "WhiteListAdapter::VisitL PXLOGICAL visited, push support is %d" ),
            iLogicalProxyPushSupport ) );

        // iItemsToBeAdded contains physical proxies whose push support
        // is either PushEnabled or PushDontCare (i.e. PushDisabled proxies
        // are not present in this array).
        if ( iLogicalProxyPushSupport == EPushEnabled )
            {
            // Knowing that the logical proxy's push support is PushEnabled,
            // we have to add each element of the above-mentioned array.
            for ( TInt i = 0; i < iItemsToBeAdded.Count(); i++ )
                {
                // If addition fails, then we delete the item. Otherwise,
                // the ownership has been taken over by iItems array.
                if ( iItems.Append( iItemsToBeAdded[i] ) != KErrNone )
                    {
                    delete iItemsToBeAdded[i];
                    }
                }
            }
        else
            {
            // We're here : logical proxy's push support is either PushDontCare
            // or PushDisabled. In this case we have to add only those elements
            // in the array whose push support is PushEnabled. This is because
            // physical proxy's push support definition is stronger than the
            // logical proxy's definition.
            for ( TInt i = 0; i < iItemsToBeAdded.Count(); i++ )
                {
                TWhiteListAdapterItem* item = iItemsToBeAdded[i];
                if ( item->PushSupport() == EPushEnabled )
                    {
                    // If addition fails, then we delete the item. Otherwise,
                    // the ownership has been taken over by iItems array.
                    if ( iItems.Append( item ) != KErrNone )
                        {
                        delete item;
                        }
                    }
                else
                    {
                    delete item;
                    }
                }
            }

        // There is only one thing we need to do here : reset the array.
        // At this point, each element in the array is invalid : if the
        // element happened to be valid, then the other array (iItems)
        // took over the ownership. Otherwise (i.e. the element was
        // invalid), the element got deleted manually.
        // Thus, we can safely reset the array.
        iItemsToBeAdded.Reset();

		iLogicalProxyPushSupport = EPushDontCare;

        CLOG( ( ELogBasic, WHITELISTADAPTER_LOG_LEVEL,
            _L( "<- WhiteListAdapter::VisitL PXLOGICAL" ) ) );
		}
	else if ( aCharacteristic.Type() == KWPPxPhysical )
		{
		// This deletion is for safety's sake : if one of the leaving methods
		// in this section (below) has left, then it may happen that this
		// method is called again and iCurrentItem is not NULL. If it is the
		// case, we can avoid having an orphaned memory area by deleting the
		// current item before allocating a new one.
		// However, in most cases iCurrentItem will be NULL.
        CLOG( ( ELogBasic, WHITELISTADAPTER_LOG_LEVEL,
            _L( "-> WhiteListAdapter::VisitL PXPHYSICAL" ) ) );

        delete iCurrentItem;
		iCurrentItem = NULL;
		iCurrentItem = new (ELeave) TWhiteListAdapterItem();

		iPhysicalProxyIsVisited = ETrue;

		aCharacteristic.AcceptL( *this );

		iPhysicalProxyIsVisited = EFalse;

		if ( iCurrentItem->Valid() )
			{
			User::LeaveIfError( iItemsToBeAdded.Append( iCurrentItem ) );
            iCurrentItem = NULL; // Ownership transferred.
			
            CLOG( ( ELogBasic, WHITELISTADAPTER_LOG_LEVEL,
              _L( "WhiteListAdapter::VisitL PXPHYSICAL valid & added" ) ) );
			}
		else
			{
            CLOG( ( ELogBasic, WHITELISTADAPTER_LOG_LEVEL,
                _L( "WhiteListAdapter::VisitL PXPHYSICAL invalid" ) ) );
			// invalid physical proxy (at least from our point of view)
			delete iCurrentItem;
            iCurrentItem = NULL;
			}

        CLOG( ( ELogBasic, WHITELISTADAPTER_LOG_LEVEL,
            _L( "<- WhiteListAdapter::VisitL PXPHYSICAL" ) ) );
		}
	}

// ---------------------------------------------------------
// CWhiteListAdapter::VisitL
// ---------------------------------------------------------
//
void CWhiteListAdapter::VisitL( CWPParameter& aParameter )
	{
	switch ( aParameter.ID() )
		{
		case EWPParameterPhysicalProxyID :
			{
			__ASSERT_DEBUG( iCurrentItem, Panic( ENullProxy ) );

            CLOG( ( ELogBasic, WHITELISTADAPTER_LOG_LEVEL,
                _L( "WhiteListAdapter::VisitL PhysicalProxyID" ) ) );

			if ( iCurrentItem )
				{
				iCurrentItem->SetProxyId( aParameter.Value() );
				}

			break;
			}
		case EWPParameterPxAddr :
			{
			__ASSERT_DEBUG( iCurrentItem, Panic( ENullProxy ) );

            CLOG( ( ELogBasic, WHITELISTADAPTER_LOG_LEVEL,
                _L( "WhiteListAdapter::VisitL PxAddr" ) ) );

            if ( iCurrentItem )
				{
				iCurrentItem->SetAddress( aParameter.Value() );
				}

			break;
			}
		case EWPParameterPxAddrType :
			{
			__ASSERT_DEBUG( iCurrentItem, Panic( ENullProxy ) );

            CLOG( ( ELogBasic, WHITELISTADAPTER_LOG_LEVEL,
                _L( "WhiteListAdapter::VisitL PxAddrType" ) ) );

			if ( iCurrentItem )
				{
				iCurrentItem->SetAddressType( aParameter.Value() );
				}

			break;
			}
		case EWPParameterPushEnabled :
			{
			if ( iCurrentItem )
				{
                CLOG( ( ELogBasic, WHITELISTADAPTER_LOG_LEVEL,
                    _L( "WhiteListAdapter::VisitL Physical PushEnabled" ) ) );

                // it is a physical proxy
				iCurrentItem->SetPushSupport
								( ResolvePushSupport( aParameter.Value() ) );
				}
			else
				{
                CLOG( ( ELogBasic, WHITELISTADAPTER_LOG_LEVEL,
                    _L( "WhiteListAdapter::VisitL Logical PushEnabled" ) ) );

				// it is a logical proxy
				iLogicalProxyPushSupport =
									ResolvePushSupport( aParameter.Value() );
				}

			break;
			}
		default :
			break;
		}
	}

// ---------------------------------------------------------
// CWhiteListAdapter::VisitLinkL
// ---------------------------------------------------------
//
void CWhiteListAdapter::VisitLinkL( CWPCharacteristic& /*aCharacteristic*/ )
	{
	}

// ---------------------------------------------------------
// CWhiteListAdapter::HandleSessionEventL
// ---------------------------------------------------------
//
void CWhiteListAdapter::HandleSessionEventL( TMsvSessionEvent /*aEvent*/,
											TAny* /*aArg1*/,
											TAny* /*aArg2*/,
											TAny* /*aArg3*/ )
	{
	}

// ---------------------------------------------------------
// CWhiteListAdapter::ResolvePushSupport
// ---------------------------------------------------------
//
TPushSupport CWhiteListAdapter::ResolvePushSupport( const TDesC& aPushSupport )
	{
	TPushSupport support = EPushDontCare;

	if ( aPushSupport.Length() == KPushEnabledFlagLength )
		{
		if ( aPushSupport[0] == KPushEnabledFlagTrue )
			{
			support = EPushEnabled;
			}
		else if ( aPushSupport[0] == KPushEnabledFlagFalse )
			{
			support = EPushDisabled;
			}
		}

	return support;
	}

// ---------------------------------------------------------
// CWhiteListAdapter::SaveDataL
// ---------------------------------------------------------
//
const TDesC8& CWhiteListAdapter::SaveDataL( TInt aIndex ) const
	{
    CLOG( ( ELogBasic, WHITELISTADAPTER_LOG_LEVEL,
        _L( "WhiteListAdapter::SaveDataL %d %d %d" ), 
        iItems.Count(), aIndex, iItems[aIndex]->Id() ) );
    return ( iItems[aIndex]->IdBuf() );
	}

// ---------------------------------------------------------
// CWhiteListAdapter::DeleteL
// ---------------------------------------------------------
//
void CWhiteListAdapter::DeleteL( const TDesC8& aSaveData )
	{
    // Copy source into a package buffer:
    TPckgBuf<TUint32> entryIdBuf;
    entryIdBuf.Copy( aSaveData );
    // Extract the entry ID:
    TUint32 entryId( entryIdBuf() );
    CLOG( ( ELogBasic, WHITELISTADAPTER_LOG_LEVEL,
        _L( "WhiteListAdapter::DeleteL ID %d" ), entryId ) );

    // Now entryId holds the ID - we can remove it from WL:
    CPushInitiatorList& whiteList = iPushMtmSettings->PushInitiatorList();
    // Find the index of the Initiator with the given ID.
    TInt index( KErrNotFound );
    // As the entries are unique in WL by the ID, it's enough 
    // to continue the loop until first hit.
    const TInt count( whiteList.Count() );
    for ( TInt i = 0; i < count; ++i )
        {
        const CPushInitiator& initiator = whiteList.At(i);
        if ( initiator.EntryID() == entryId )
            {
            index = i;
            // Found:
            break;
            }
        }

    CLOG( ( ELogBasic, WHITELISTADAPTER_LOG_LEVEL,
        _L( "WhiteListAdapter::DeleteL index %d" ), index ) );
    if ( index != KErrNotFound )
        {
        whiteList.Delete( index );
        // Commit changes.
        iPushMtmSettings->SaveL();
        CLOG( ( ELogBasic, WHITELISTADAPTER_LOG_LEVEL,
            _L( "WhiteListAdapter::DeleteL Saved..." ) ) );
        }
	}

// ---------------------------------------------------------
// CWhiteListAdapter::Uid
// ---------------------------------------------------------
//
TUint32 CWhiteListAdapter::Uid() const
	{
    return (TUint32)KWhiteListAdapterImplementationUid;
	}

// End of file
