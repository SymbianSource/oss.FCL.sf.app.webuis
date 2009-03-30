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
* Description:  Push Ui Data Mtm class definition
*
*/



// INCLUDE FILES

#include "PushMtmUiData.h"
#include "PushMtmCommands.hrh"
#include "PushMtmUiDef.h"
#include "PushMtmUiPanic.h"
#include "PushMtmUtil.h"
#include <PushMtmUi.rsg>
#include <PushEntry.h>
#include <msvuids.h>
#include <MTMExtendedCapabilities.hrh>
#include <PushMtm.mbg>
#include <data_caging_path_literals.hrh>
#include <f32file.h>
#include <AknsUtils.h>

// CONSTANTS

// Number of zoom states.
LOCAL_C const TInt KPushIconArrayGranularity = 2;

// ================= MEMBER FUNCTIONS =======================

// ---------------------------------------------------------
// CPushMtmUiData::NewL
// ---------------------------------------------------------
//
EXPORT_C CPushMtmUiData* CPushMtmUiData::NewL
( CRegisteredMtmDll& aRegisteredDll )
    {
    CPushMtmUiData* uiData = new (ELeave) CPushMtmUiData( aRegisteredDll );
    CleanupStack::PushL( uiData );
    uiData->ConstructL();
    CleanupStack::Pop();    // uiData
    return uiData;
    }

// ---------------------------------------------------------
// CPushMtmUiData::~CPushMtmUiData
// ---------------------------------------------------------
//
CPushMtmUiData::~CPushMtmUiData()
    {
    }

// ---------------------------------------------------------
// CPushMtmUiData::CPushMtmUiData
// ---------------------------------------------------------
//
CPushMtmUiData::CPushMtmUiData( CRegisteredMtmDll& aRegisteredDll )
: CBaseMtmUiData( aRegisteredDll )
    {
    }

// ---------------------------------------------------------
// CPushMtmUiData::ContextIcon
// ---------------------------------------------------------
//
const CBaseMtmUiData::CBitmapArray& CPushMtmUiData::ContextIcon
( const TMsvEntry& aContext, TInt /*aStateFlags*/ ) const
    {
	__ASSERT_DEBUG( aContext.iMtm == Type(), UiPanic( EPushMtmUiPanBadType ) );
    __ASSERT_DEBUG( aContext.iType.iUid != KUidMsvFolderEntryValue, 
        UiPanic( EPushMtmUiPanFoldersNotSupported ) );	

	return *iIconArrays->At( aContext.Unread() ? 0 : 1 );
    }

// ---------------------------------------------------------
// CPushMtmUiData::CanCreateEntryL
// ---------------------------------------------------------
//
#ifdef _DEBUG
TBool CPushMtmUiData::CanCreateEntryL( const TMsvEntry& /*aParent*/,
        TMsvEntry& aNewEntry, TInt& aReasonResourceId ) const
#else // _DEBUG
TBool CPushMtmUiData::CanCreateEntryL( const TMsvEntry& /*aParent*/,
        TMsvEntry& /*aNewEntry*/, TInt& aReasonResourceId ) const
#endif // _DEBUG
    {
    __ASSERT_DEBUG( aNewEntry.iMtm == Type(), UiPanic( EPushMtmUiPanBadType ) );
    aReasonResourceId = KErrNotSupported;
    return EFalse;
    }

// ---------------------------------------------------------
// CPushMtmUiData::CanDeleteFromEntryL
// ---------------------------------------------------------
//
#ifdef _DEBUG
TBool CPushMtmUiData::CanDeleteFromEntryL
( const TMsvEntry& aContext, TInt& aReasonResourceId ) const
#else // _DEBUG
TBool CPushMtmUiData::CanDeleteFromEntryL
( const TMsvEntry& /*aContext*/, TInt& aReasonResourceId ) const
#endif // _DEBUG
    {
    __ASSERT_DEBUG( aContext.iMtm == Type(), UiPanic( EPushMtmUiPanBadType ) );
    aReasonResourceId = KErrNotSupported;
    return ETrue;
    }

// ---------------------------------------------------------
// CPushMtmUiData::CanDeleteServiceL
// ---------------------------------------------------------
//
#ifdef _DEBUG
TBool CPushMtmUiData::CanDeleteServiceL
( const TMsvEntry& aService, TInt& aReasonResourceId ) const
#else // _DEBUG
TBool CPushMtmUiData::CanDeleteServiceL
( const TMsvEntry& /*aService*/, TInt& aReasonResourceId ) const
#endif // _DEBUG
    {
    __ASSERT_DEBUG( aService.iMtm == Type(), UiPanic( EPushMtmUiPanBadType ) );
    aReasonResourceId = KErrNotSupported;
    return EFalse;
    }

// ---------------------------------------------------------
// CPushMtmUiData::CanReplyToEntryL
// ---------------------------------------------------------
//
#ifdef _DEBUG
TBool CPushMtmUiData::CanReplyToEntryL
( const TMsvEntry& aContext, TInt& aReasonResourceId ) const
#else // _DEBUG
TBool CPushMtmUiData::CanReplyToEntryL
( const TMsvEntry& /*aContext*/, TInt& aReasonResourceId ) const
#endif // _DEBUG
    {
    __ASSERT_DEBUG( aContext.iMtm == Type(), UiPanic( EPushMtmUiPanBadType ) );
    aReasonResourceId = KErrNotSupported;
    return EFalse;
    }

// ---------------------------------------------------------
// CPushMtmUiData::CanForwardEntryL
// ---------------------------------------------------------
//
#ifdef _DEBUG
TBool CPushMtmUiData::CanForwardEntryL
( const TMsvEntry& aContext, TInt& aReasonResourceId ) const
#else // _DEBUG
TBool CPushMtmUiData::CanForwardEntryL
( const TMsvEntry& /*aContext*/, TInt& aReasonResourceId ) const
#endif // _DEBUG
    {
    __ASSERT_DEBUG( aContext.iMtm == Type(), UiPanic( EPushMtmUiPanBadType ) );
    aReasonResourceId = KErrNotSupported;
    return EFalse;
    }

// ---------------------------------------------------------
// CPushMtmUiData::CanEditEntryL
// ---------------------------------------------------------
//
TBool CPushMtmUiData::CanEditEntryL
( const TMsvEntry& aContext, TInt& aReasonResourceId ) const
    {
    __ASSERT_DEBUG( aContext.iMtm == Type(), UiPanic( EPushMtmUiPanBadType ) );

    aReasonResourceId = KErrNotSupported;

	if ( aContext.iType == KUidMsvServiceEntry )
		{
		// Only the service entry can be changed.
		return ETrue;
		}
    return EFalse;
    }

// ---------------------------------------------------------
// CPushMtmUiData::CanViewEntryL
// ---------------------------------------------------------
//
TBool CPushMtmUiData::CanViewEntryL
( const TMsvEntry& aContext, TInt& aReasonResourceId ) const
    {
    __ASSERT_DEBUG( aContext.iMtm == Type(), UiPanic( EPushMtmUiPanBadType ) );
    
	aReasonResourceId = KErrNotSupported;

    if ( aContext.iType != KUidMsvMessageEntry )
        {
        // Only the messages can be viewed(this function
        // should not be available on a service entry)
        return EFalse;
        }
    return ETrue;
    }

// ---------------------------------------------------------
// CPushMtmUiData::CanOpenEntryL
// ---------------------------------------------------------
//
TBool CPushMtmUiData::CanOpenEntryL
( const TMsvEntry& aContext, TInt& aReasonResourceId ) const
    {
    return CanViewEntryL( aContext, aReasonResourceId );
    }

// ---------------------------------------------------------
// CPushMtmUiData::CanCloseEntryL
// ---------------------------------------------------------
//
TBool CPushMtmUiData::CanCloseEntryL
( const TMsvEntry& aContext, TInt& aReasonResourceId ) const
    {
    return CanOpenEntryL( aContext, aReasonResourceId );
    }

// ---------------------------------------------------------
// CPushMtmUiData::CanCopyMoveToEntryL
// ---------------------------------------------------------
//
TBool CPushMtmUiData::CanCopyMoveToEntryL
( const TMsvEntry& aContext, TInt& aReasonResourceId ) const
    {
    __ASSERT_DEBUG( aContext.iMtm == Type(), UiPanic( EPushMtmUiPanBadType ) );

	aReasonResourceId = KErrNotSupported;

    if ( aContext.iType != KUidMsvMessageEntry )
        {
        // Only the messages can be moved(this function
        // should not be available on a service entry)
        return EFalse;
        }
    return ETrue;
    }

// ---------------------------------------------------------
// CPushMtmUiData::CanCopyMoveFromEntryL
// ---------------------------------------------------------
//
#ifdef _DEBUG
TBool CPushMtmUiData::CanCopyMoveFromEntryL
( const TMsvEntry& aContext, TInt& aReasonResourceId ) const
#else // _DEBUG
TBool CPushMtmUiData::CanCopyMoveFromEntryL
( const TMsvEntry& /*aContext*/, TInt& aReasonResourceId ) const
#endif // _DEBUG
    {
    __ASSERT_DEBUG( aContext.iMtm == Type(), UiPanic( EPushMtmUiPanBadType ) );

	aReasonResourceId = KErrNotSupported;

    return EFalse;
    }

// ---------------------------------------------------------
// CPushMtmUiData::CanCancelL
// ---------------------------------------------------------
//
#ifdef _DEBUG
TBool CPushMtmUiData::CanCancelL
( const TMsvEntry& aContext, TInt& aReasonResourceId ) const
#else // _DEBUG
TBool CPushMtmUiData::CanCancelL
( const TMsvEntry& /*aContext*/, TInt& aReasonResourceId ) const
#endif // _DEBUG
    {
    __ASSERT_DEBUG( aContext.iMtm == Type(), UiPanic( EPushMtmUiPanBadType ) );

	aReasonResourceId = KErrNotSupported;

    return EFalse;
    }

// ---------------------------------------------------------
// CPushMtmUiData::OperationSupportedL
// ---------------------------------------------------------
//
TInt CPushMtmUiData::OperationSupportedL
( TInt aOperationId, const TMsvEntry& aContext ) const
    {
	__ASSERT_DEBUG( aContext.iMtm == Type(), UiPanic( EPushMtmUiPanBadType ) );
    __ASSERT_DEBUG( aContext.iType.iUid != KUidMsvFolderEntryValue, 
        UiPanic( EPushMtmUiPanFoldersNotSupported ) );	

    TInt ret( KErrCancel );

    switch ( aOperationId )
        {

	    case KMtmUiFunctionMessageInfo:
			{
			// The message info operation is always supported.
			ret = ( aContext.iType.iUid == KUidMsvMessageEntryValue ) ? 
                    KErrNone : KErrCancel;
            break;
			}

        case EPushMtmCmdLoadService:
            {
            // This function is available only if the message has URL in it.
            if ( aContext.iType.iUid != KUidMsvMessageEntryValue )
                {
                // LoadService is not supported.
                ret = KErrCancel;
                }
            else
                {
                if ( aContext.iBioType == KUidWapPushMsgSI.iUid && 
                     CPushMtmUtil::Attrs( aContext ) & EPushMtmAttrHasHref )
                    {
                    // SI with not empty URL: LoadService is supported.
                    ret = KErrNone;
                    }
                else if ( aContext.iBioType == KUidWapPushMsgSL.iUid && 
                     CPushMtmUtil::Attrs( aContext ) & EPushMtmAttrHasHref )
                    {
                    // SL with not empty URL: LoadService is supported.
                    ret = KErrNone;
                    }
                else
                    {
                    // LoadService is not supported.
                    ret = KErrCancel;
                    }
                }
            break;
            }

        default:
            {
            // No other operations are supported.
            ret = KErrCancel;
            break;
            }
        }

    return ret;
    }

// ---------------------------------------------------------
// CPushMtmUiData::QueryCapability
// ---------------------------------------------------------
//
TInt CPushMtmUiData::QueryCapability( TUid aCapability, TInt& aResponse ) const
    {
    TInt ret( KErrNotSupported );

    if ( aCapability.iUid == KUidMsvMtmQueryMessageInfo )
		{
		aResponse = ETrue;
        ret = KErrNone;
		}

    return ret;
    }

// ---------------------------------------------------------
// CPushMtmUiData::StatusTextL
// ---------------------------------------------------------
//
HBufC* CPushMtmUiData::StatusTextL( const TMsvEntry& /*aContext*/ ) const
    {
    return NULL;
    }

// ---------------------------------------------------------
// CPushMtmUiData::PopulateArraysL
// ---------------------------------------------------------
//
void CPushMtmUiData::PopulateArraysL()
    {
    ReadFunctionsFromResourceFileL( R_PUSH_MTM_UI_DATA_FUNCTION_ARRAY );
    CreateSkinnedBitmapsL();
    }

// ---------------------------------------------------------
// CPushMtmUiData::GetResourceFileName
// ---------------------------------------------------------
//
void CPushMtmUiData::GetResourceFileName( TFileName& aFileName ) const
    {
    // Extract the file name and extension from the constant
    TParsePtrC fileParser( KPushMtmUiDataResourceFileAndDrive );
    aFileName = fileParser.NameAndExt();
    }

// ---------------------------------------------------------
// CPushMtmUiData::CreateSkinnedBitmapsL
// ---------------------------------------------------------
//
void CPushMtmUiData::CreateSkinnedBitmapsL()
    {
    TParse fileParser;
    fileParser.Set( KPushMtmUiDataBitmapFileAndDrive, &KDC_APP_BITMAP_DIR, NULL );

    MAknsSkinInstance* skins = AknsUtils::SkinInstance();
    TAknsItemID id;
    CFbsBitmap* bitmap;
    CFbsBitmap* bitmapMask;
    TBool found = ETrue;

    for( TInt i = EMbmPushmtmQgn_prop_mce_push_unread; 
              i < EMbmPushmtmQgn_prop_mce_push_read_mask + 1; 
              i++ )
        {
        found = ETrue;
        switch( i )
            {
            // Add to iIconArrays in this order
            case EMbmPushmtmQgn_prop_mce_push_unread:
                id.Set( KAknsIIDQgnPropMcePushUnread );
                break;
            case EMbmPushmtmQgn_prop_mce_push_read:
                id.Set( KAknsIIDQgnPropMcePushRead );
                break;
            default:
                found = EFalse;
                break;
            }
        if( found )
            {
            CArrayPtrFlat<CFbsBitmap>* array = 
                new (ELeave) CArrayPtrFlat<CFbsBitmap>( KPushIconArrayGranularity );
            CleanupStack::PushL( array );

            array->SetReserveL( KPushIconArrayGranularity ); // AppendLs will not LEAVE
            AknsUtils::CreateIconL( skins, id, bitmap, bitmapMask, 
                                    fileParser.FullName(), i, i + 1 );
            // bitmap & bitmapMask is not on the Cleanup Stack, but it is not 
            // necessary, because AppendLs will not LEAVE.
            array->AppendL( bitmap );
            bitmap = 0;
            array->AppendL( bitmapMask );
            bitmapMask = 0;
            iIconArrays->AppendL( array );
            CleanupStack::Pop( array ); // array
            }
        }
    }

// End of file.
