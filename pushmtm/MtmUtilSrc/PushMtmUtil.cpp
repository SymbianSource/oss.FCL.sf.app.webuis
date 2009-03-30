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
* Description:  CPushMtmUtil class definition.
*
*/



// INCLUDE FILES

#include "PushMtmUtil.h"
#include "PushMtmUtilPanic.h"
#include "PushMtmLog.h"
#include <CSIPushMsgEntry.h>
#include <CSLPushMsgEntry.h>
#include <msvids.h>
#include <msvuids.h>
#include <msvapi.h>
#include <Uri16.h>
#include <UriUtils.h>

// ================= MEMBER FUNCTIONS =======================

// ---------------------------------------------------------
// CPushMtmUtil::NewL
// ---------------------------------------------------------
//
EXPORT_C CPushMtmUtil* CPushMtmUtil::NewL( CMsvSession& aMsvSession )
    {
    CPushMtmUtil* self = new (ELeave) CPushMtmUtil( aMsvSession );
    CleanupStack::PushL( self );
    self->ConstructL();
    CleanupStack::Pop(); // self
    return self;
    }

// ---------------------------------------------------------
// CPushMtmUtil::~CPushMtmUtil
// ---------------------------------------------------------
//
EXPORT_C CPushMtmUtil::~CPushMtmUtil()
    {
    }

// ---------------------------------------------------------
// CPushMtmUtil::MarkServiceUnreadL
// ---------------------------------------------------------
//
EXPORT_C void CPushMtmUtil::MarkServiceUnreadL( TMsvId aEntryId, 
                                                TBool aUnread )
    {
    PUSHLOG_ENTERFN("CPushMtmUtil::MarkServiceUnreadL")

    CMsvEntry* cEntry = iMsvSession.GetEntryL( aEntryId );
    CleanupStack::PushL( cEntry );

    TMsvEntry tEntry( cEntry->Entry() );
    // Change the context only if necessary.
    if ( tEntry.New() != aUnread || tEntry.Unread() != aUnread )
        {
        tEntry.SetNew( aUnread );
        tEntry.SetUnread( aUnread );
        cEntry->ChangeL( tEntry );
        }

    CleanupStack::PopAndDestroy( cEntry ); // cEntry

    PUSHLOG_LEAVEFN("CPushMtmUtil::MarkServiceUnreadL")
    }

// ---------------------------------------------------------
// CPushMtmUtil::FindMessagesL
// ---------------------------------------------------------
//
EXPORT_C CMsvEntrySelection* CPushMtmUtil::FindMessagesL
                                               ( TMsvId aFolderId, 
                                                 TUid aMsgType, 
                                                 TBool aRecursive )
    {
    CMsvEntrySelection* sel = new (ELeave) CMsvEntrySelection;
    CleanupStack::PushL( sel );

    FindMessagesL( aFolderId, aMsgType, aRecursive, *sel );

    CleanupStack::Pop(); // sel
    return sel;
    }

// ---------------------------------------------------------
// CPushMtmUtil::FindMessagesL
// ---------------------------------------------------------
//
EXPORT_C CMsvEntrySelection* CPushMtmUtil::FindMessagesL
( const CMsvEntrySelection& aFolders, TUid aMsgType )
    {
    CMsvEntrySelection* sel = new (ELeave) CMsvEntrySelection;
    CleanupStack::PushL( sel );

    const TInt count( aFolders.Count() );
    for ( TInt i = 0; i < count; ++i )
        {
        FindMessagesL( aFolders[i], aMsgType, EFalse, *sel );
        }

    CleanupStack::Pop(); // sel
    return sel;
    }

// ---------------------------------------------------------
// CPushMtmUtil::FindMessagesL
// ---------------------------------------------------------
//
EXPORT_C CMsvEntrySelection* CPushMtmUtil::FindMessagesL( TUid aMsgType )
    {
    CMsvEntrySelection* pushFolders = FindPushFoldersL();
    CleanupStack::PushL( pushFolders );

    CMsvEntrySelection* msgs = FindMessagesL( *pushFolders, aMsgType );

    CleanupStack::PopAndDestroy( pushFolders ); // pushFolders
    return msgs;
    }

// ---------------------------------------------------------
// CPushMtmUtil::FindPushFoldersL
// ---------------------------------------------------------
//
EXPORT_C CMsvEntrySelection* CPushMtmUtil::FindPushFoldersL()
    {
    // Only the Inbox, the Documents folder and the user 
    // defined folders can contain push messages.
    // The user defined folders can only be in the Documents folder.
    // (See the definition of CMsgFolderSelectionListArray::ConstructL 
    // in msgavkon for more details.)
    CMsvEntrySelection* pushFolders = new (ELeave) CMsvEntrySelection;
    CleanupStack::PushL( pushFolders );

    // Check if Inbox folder exists.
    TMsvEntry dummyFolder;
    TMsvId dummyService;
    TInt err( iMsvSession.GetEntry
        ( KMsvGlobalInBoxIndexEntryId, dummyService, dummyFolder) );

    if ( !err )
        {
        pushFolders->AppendL( KMsvGlobalInBoxIndexEntryId );
        }

    // Check if Documents folder exists.
    const TMsvId KDocumentsIndexEntryId( 0x1008 );
    err = iMsvSession.GetEntry
        ( KDocumentsIndexEntryId, dummyService, dummyFolder);

    if ( !err )
        {
        pushFolders->AppendL( KDocumentsIndexEntryId );
        // Get the user defined folders in Documents.
        const TMsvSelectionOrdering selOrd
            ( KMsvGroupByType, EMsvSortByDescription, ETrue );
        CMsvEntry* documentsFolder = CMsvEntry::NewL
            ( iMsvSession, KDocumentsIndexEntryId, selOrd );
        CleanupStack::PushL( documentsFolder );

        CMsvEntrySelection* userDefFolders = 
            documentsFolder->ChildrenWithTypeL( KUidMsvFolderEntry );
        CleanupStack::PopAndDestroy(); // documentsFolder
        CleanupStack::PushL( userDefFolders );

        const TMsvId KTemplatesIndexEntryId( 0x1009 );
        const TInt count( userDefFolders->Count() );
        for ( TInt i = 0; i < count; ++i )
            {
            // However Templates folder is in Documents, no messages 
            // can be moved there - it cannot contain push messages.
            if ( userDefFolders->At( i ) != KTemplatesIndexEntryId )
                {
                pushFolders->AppendL( userDefFolders->At( i ) );
                }
            }

        CleanupStack::PopAndDestroy(); // userDefFolders
        }

    CleanupStack::Pop(); // pushFolders
    return pushFolders;
    }

// ---------------------------------------------------------
// CPushMtmUtil::FindSiIdLC
// ---------------------------------------------------------
//
EXPORT_C CMsvEntrySelection* CPushMtmUtil::FindSiIdLC( const TDesC& aSiId )
    {
	CMsvEntrySelection* matching = new (ELeave) CMsvEntrySelection;
	CleanupStack::PushL( matching );
	
	CSIPushMsgEntry* siEntry = CSIPushMsgEntry::NewL();
	CleanupStack::PushL( siEntry );

    CMsvEntrySelection* allSi = FindMessagesL( KUidWapPushMsgSI );
    CleanupStack::PushL( allSi );

	// loop until we find a match or reach the end of the SI entries
    const TInt numEntries( allSi->Count() );
	for (TInt count = 0; count < numEntries; ++count )
		{
        siEntry->RetrieveL( iMsvSession, allSi->At( count ) );
		if ( aSiId.CompareF( siEntry->Id() ) == 0 ) // Found a match
            {
			matching->AppendL( allSi->At( count ) );
            }
		}

	CleanupStack::PopAndDestroy( 2 ); // allSi, siEntry

	return matching; //Still on CleanupStack
    }

// ---------------------------------------------------------
// CPushMtmUtil::FindUrlLC
// ---------------------------------------------------------
//
EXPORT_C CMsvEntrySelection* CPushMtmUtil::FindUrlLC( const TDesC& aUrl, 
                                                      TUid aPushType )
	{
    __ASSERT_ALWAYS( aPushType == KUidWapPushMsgSI || 
                     aPushType == KUidWapPushMsgSL, 
                     UtilPanic( EPushMtmUtilPanBadBioType ) );

	CMsvEntrySelection* matching = new (ELeave) CMsvEntrySelection;
	CleanupStack::PushL( matching );
	
    CMsvEntrySelection* all = FindMessagesL( aPushType );
    CleanupStack::PushL( all );

	// loop until we find a match or reach the end of the SI entries
    const TInt numEntries( all->Count() );

    if ( aPushType == KUidWapPushMsgSI )
        {
	    CSIPushMsgEntry* siEntry = CSIPushMsgEntry::NewL();
	    CleanupStack::PushL( siEntry );

	    for (TInt count = 0; count < numEntries; ++count )
		    {
            siEntry->RetrieveL( iMsvSession, all->At( count ) );
		    if ( aUrl.CompareF( siEntry->Url() ) == 0 ) // Found a match
                {
			    matching->AppendL( all->At( count ) );
                }
		    }

        CleanupStack::PopAndDestroy( siEntry ); // siEntry
        }
    else // aPushType == KUidWapPushMsgSL
        {
	    CSLPushMsgEntry* slEntry = CSLPushMsgEntry::NewL();
	    CleanupStack::PushL( slEntry );

	    for (TInt count = 0; count < numEntries; ++count )
		    {
            slEntry->RetrieveL( iMsvSession, all->At( count ) );
		    if ( aUrl.CompareF( slEntry->Url() ) == 0 ) // Found a match
                {
			    matching->AppendL( all->At( count ) );
                }
		    }

        CleanupStack::PopAndDestroy( slEntry ); // slEntry
        }

	CleanupStack::PopAndDestroy( all ); // all

	return matching; //Still on CleanupStack
	}

// ---------------------------------------------------------
// CPushMtmUtil::DeleteEntryL
// ---------------------------------------------------------
//
EXPORT_C void CPushMtmUtil::DeleteEntryL( TMsvId aId )
	{
    CMsvEntry* cEntry = iMsvSession.GetEntryL( aId );
    CleanupStack::PushL( cEntry );
    // Set as parent.
    cEntry->SetEntryL( cEntry->Entry().Parent() );
    // Delete the child.
    cEntry->DeleteL( aId );
    CleanupStack::PopAndDestroy( cEntry ); // cEntry
	}

// ---------------------------------------------------------
// CPushMtmUtil::ConvertDetailsL
// ---------------------------------------------------------
//
EXPORT_C HBufC* CPushMtmUtil::ConvertDetailsL( const TDesC8& aFrom ) const
    {
    HBufC* from = HBufC::NewMaxLC( aFrom.Length() );
    from->Des().Copy( aFrom );

    TUriParser pars;
    User::LeaveIfError( pars.Parse( *from ) );

    HBufC* res = NULL;
    if ( pars.IsPresent( EUriHost ) )
        {
        res = pars.Extract( EUriHost ).AllocL();
        }
    else
        {
        res = from->AllocL();
        }

    CleanupStack::PopAndDestroy( from ); // from
    return res;
    }

// ---------------------------------------------------------
// CPushMtmUtil::SetAttrs
// ---------------------------------------------------------
//
EXPORT_C void CPushMtmUtil::SetAttrs( TMsvEntry& aContext, TUint32 aAttrs )
    {
    aContext.iMtmData2 = aContext.iMtmData2 | 
                         (aAttrs<<KPushMtmShiftOnlyAttrs);
    }

// ---------------------------------------------------------
// CPushMtmUtil::ResetAttrs
// ---------------------------------------------------------
//
EXPORT_C void CPushMtmUtil::ResetAttrs( TMsvEntry& aContext, TUint32 aAttrs )
    {
    aContext.iMtmData2 = aContext.iMtmData2 & 
                         ~(aAttrs<<KPushMtmShiftOnlyAttrs);
    }

// ---------------------------------------------------------
// CPushMtmUtil::Attrs
// ---------------------------------------------------------
//
EXPORT_C TUint32 CPushMtmUtil::Attrs( const TMsvEntry& aContext )
    {
    return (aContext.iMtmData2 >> KPushMtmShiftOnlyAttrs) & 
           KPushMtmMaskOnlyAttrs;
    }

// ---------------------------------------------------------
// CPushMtmUtil::ConvertUriToDisplayFormL
// ---------------------------------------------------------
//
EXPORT_C HBufC* CPushMtmUtil::ConvertUriToDisplayFormL( const TDesC& aUri )
    {
    HBufC8* uri8 = HBufC8::NewMaxLC( aUri.Length() );
    uri8->Des().Copy( aUri );
    TUriParser8 uriParser8;
    User::LeaveIfError( uriParser8.Parse( *uri8 ) );
    CUri* convertedCUri = UriUtils::ConvertToDisplayFormL( uriParser8 );
    CleanupStack::PopAndDestroy( uri8 );
    CleanupStack::PushL( convertedCUri );
    HBufC* convertedUri = convertedCUri->Uri().UriDes().AllocL();
    CleanupStack::PopAndDestroy( convertedCUri );
    return convertedUri;
    }

// ---------------------------------------------------------
// CPushMtmUtil::CPushMtmUtil
// ---------------------------------------------------------
//
CPushMtmUtil::CPushMtmUtil( CMsvSession& aMsvSession ) 
: iMsvSession( aMsvSession )
    {
    }

// ---------------------------------------------------------
// CPushMtmUtil::ConstructL
// ---------------------------------------------------------
//
void CPushMtmUtil::ConstructL()
    {
    }

// ---------------------------------------------------------
// CPushMtmUtil::FindMessagesL
// ---------------------------------------------------------
//
void CPushMtmUtil::FindMessagesL( TMsvId aFolderId, 
                                  TUid aMsgType, 
                                  TBool aRecursive, 
                                  CMsvEntrySelection& aResult )
    {
    // Find the push entries of the specified type in the specified folder.
    CMsvEntry* thisFolder = iMsvSession.GetEntryL( aFolderId );
    CleanupStack::PushL( thisFolder );

	TMsvSelectionOrdering selOrd( thisFolder->SortType() );
	selOrd.SetShowInvisibleEntries( EFalse );
	thisFolder->SetSortTypeL( selOrd );
    CMsvEntrySelection* msgSel = thisFolder->ChildrenWithMtmL( KUidMtmWapPush );
    CleanupStack::PushL( msgSel );

    // The selection may contain any kind of push message.
    // Select only those that have the specified type.
    TMsvEntry tEntry;
    TMsvId service;
    TInt err;
    const TInt msgCnt( msgSel->Count() );
    for ( TInt i = 0; i < msgCnt; ++i )
        {
        err = iMsvSession.GetEntry( msgSel->At(i), service, tEntry );
        if ( err == KErrNotFound )
            {
            // Do nothing, just continue.
            }
        else if ( !err )
            {
            if ( tEntry.iBioType == aMsgType.iUid )
                {
                // We have found an entry of the requested type.
                aResult.AppendL( msgSel->At(i) );
                }
            }
        else
            {
            User::Leave( err );
            }
        }

    CleanupStack::PopAndDestroy( msgSel ); // msgSel

    // Find messages in the subfolders if required.
    if ( aRecursive )
        {
        CMsvEntrySelection* subFolders = 
            thisFolder->ChildrenWithTypeL( KUidMsvFolderEntry );
        CleanupStack::PushL( subFolders );

        const TInt count( subFolders->Count() );
        for ( TInt i = 0; i < count; ++i )
            {
            FindMessagesL( subFolders->At(i), aMsgType, aRecursive, aResult );
            }

        CleanupStack::PopAndDestroy( subFolders ); // subFolders
        }

    CleanupStack::PopAndDestroy( thisFolder ); // thisFolder
    }

// End of file.
