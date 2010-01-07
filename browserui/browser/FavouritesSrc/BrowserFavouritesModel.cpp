/*
* Copyright (c) 2002-2007 Nokia Corporation and/or its subsidiary(-ies).
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
*      Implementation of CBrowserFavouritesModel.
*      
*
*/


// INCLUDE FILES
#include <StringLoader.h>
#include <aknviewappui.h>
#include <Avkon.hrh>
#include <AknForm.h>    // This contains CAknDialog
#include <AknQueryDialog.h>
#include <favouritesdb.h>
#include <favouritesitem.h>
#include <favouritesitemlist.h>
#include <favouriteslimits.h>
#include <activefavouritesdbnotifier.h>
#include <BrowserNG.rsg>

#include "BrowserFavouritesModel.h"
#include "BrowserFavouritesModelObserver.h"
#include "BrowserUtil.h"
#include "BrowserDialogs.h"
#include <browserdialogsprovider.h>
#include "BrowserFavouritesView.h"

#include "Browser.hrh"

#include <FeatMgr.h>
#include <collate.h>         // TCollateMethod
#include "CommonConstants.h"


// MACROS

/// Cosmetic purpose macro (resource names are overly too long).
#define _R(str) R_BROWSER_FAVOURITES_ ## str

/// Time to wait before retry database OpenL() or BeginL() (==0.2 secs).
#ifdef KRetryWait
#undef KRetryWait
#endif
#define KRetryWait 200 /*TTimeIntervalMicroSeconds32*/

// CONSTANTS

/// Granularity of observer array.
LOCAL_C const TInt KGranularityObservers = 4;

/// Number of attempts to retry after unsuccesful database OpenL() or BeginL().
LOCAL_C const TInt KLocalRetryCount = 2;

/**
* Estimated number of bytes in FFS for one writing operation. This amount
* is checked to be available before each transaction which places write-lock.
* Exact size is not known (because what happens in a transaction is unknown).
* A typical bookmark is estimated to be less than ~200 bytes; so assuming the
* typical operation of adding one bookmark (with database transaction
* overhead and safety) is estimated to be 400 bytes. This should be a safe
* guess.
*/
LOCAL_C const TInt KEstimatedFfsUsage = 400;

// ==================== LOCAL FUNCTIONS ====================

/**
* Comparison function for two items.
* Comparison rule:
*   - Start Page is smaller than anything else;
*   - otherwise, Homepage is smaller than anything else;
*   - otherwise, Last Visited is smaller than anything else;
*   - otherwise, any item is smaller than any folder;
*   - otherwise, Root Folder is smaller than any other;
*   - otherwise compare by name.
* @param aLeft item to compare to aRight.
* @param aRight Item to compare to aLeft.
* @return
*   - negative value, if aLeft is less than aRight;
*   - 0,              if aLeft equals to aRight;
*   - positive value, if aLeft is greater than aRight.
*/
LOCAL_C TInt CompareItemsL
( const CFavouritesItem& aLeft, const CFavouritesItem& aRight )
    {
    // Start Page is smaller than anything else;
    if ( aLeft.Uid() == KFavouritesStartPageUid )
        {
        return -1;
        }
    if ( aRight.Uid() == KFavouritesStartPageUid )
        {
        return 1;
        }
    // otherwise, Adaptive Bookmarks is smaller than anything else;
    if ( aLeft.Uid() == KFavouritesAdaptiveItemsFolderUid )
        {
        return -1;
        }
    if ( aRight.Uid() == KFavouritesAdaptiveItemsFolderUid )
        {
        return 1;
        }
    // otherwise, Homepage is smaller than anything else;
    if ( aLeft.Uid() == KFavouritesHomepageUid )
        {
        return -1;
        }
    if ( aRight.Uid() == KFavouritesHomepageUid )
        {
        return 1;
        }
    // otherwise, Last Visited is smaller than anything else;
    if ( aLeft.Uid() == KFavouritesLastVisitedUid )
        {
        return -1;
        }
    if ( aRight.Uid() == KFavouritesLastVisitedUid )
        {
        return 1;
        }
    // otherwise, any item is smaller than any folder;
    if ( aLeft.IsItem() && aRight.IsFolder() )
        {
        return -1;
        }
    if ( aRight.IsItem() && aLeft.IsFolder() )
        {
        return 1;
        }
    // otherwise, Root Folder is smaller than any other;
    if ( aLeft.Uid() == KFavouritesRootUid )
        {
        return -1;
        }
	// otherwise, Null ContextId is smaller than any ContextId;
	if ( aLeft.ContextId() == KFavouritesNullContextId && aRight.ContextId() != KFavouritesNullContextId )
		{
		return -1;
		}
	if ( aRight.ContextId() == KFavouritesNullContextId && aLeft.ContextId() != KFavouritesNullContextId )
		{
		return 1;
		}
    // otherwise compare by name.
    return aLeft.Name().CompareC( aRight.Name() );
    }

// ================= MEMBER FUNCTIONS =======================

// ---------------------------------------------------------
// CBrowserFavouritesModel::~CBrowserFavouritesModel
// ---------------------------------------------------------
//
CBrowserFavouritesModel::~CBrowserFavouritesModel()
	{
    delete iNotifier;
    iNotifier = NULL;
    delete iObservers;
    CloseDb();
    iFavouritesSess.Close(); 
	}

// ---------------------------------------------------------
// CBrowserFavouritesModel::SetNameToDefaultL
// ---------------------------------------------------------
//
void CBrowserFavouritesModel::SetNameToDefaultL
( CFavouritesItem& aItem ) const
    {
    HBufC* name = CCoeEnv::Static()->AllocReadResourceLC
        ( StringResourceId( aItem, EDefaultName ) );
    aItem.SetNameL( *name );
    CleanupStack::PopAndDestroy();  // name
    }

// ---------------------------------------------------------
// CBrowserFavouritesModel::OpenDbL
// ---------------------------------------------------------
//
TInt CBrowserFavouritesModel::OpenDbL( TBool aDbErrorNote /*=ETrue*/ )
    {
    TInt err = KErrNone;

    switch ( iState )
        {
        case EOpen:
            {
            // Already open.
            break;
            }

        case ECorrupt:
            {
            // Useless.
            err = KErrNotReady;
            break;
            }

        case EClosed:
            {
            // Try to open now.
            err = TimedOpenDbL();
            if ( err )
                {
                // Failed to open.
                if ( aDbErrorNote )
                    {
                    TBrowserDialogs::ErrorNoteL
                        ( _R(TEXT_CANNOT_ACCESS_DB) );
                    }
                }
            else
                {
                // Success opening.
                iState = EOpen;
                iNotifier->Start();
                }
            break;
            }

        default:
            {
            break;
            }
        }

    return err;
    }

// ---------------------------------------------------------
// CBrowserFavouritesModel::CloseDb
// ---------------------------------------------------------
//
void CBrowserFavouritesModel::CloseDb()
	{
    if ( iNotifier )
        {
        // Need to check existence - ConstructL can leave half the way!
        iNotifier->Cancel();
        }
    if ( iState == EOpen )
        {
        iDb.Close();
        }
    iState = EClosed;
	}

// ---------------------------------------------------------
// CBrowserFavouritesModel::BeginL
// ---------------------------------------------------------
//
TInt CBrowserFavouritesModel::BeginL
( TBool aWrite, TBool aDbErrorNote, TBool aLffsCheck /*=ETrue*/ )
    {
    TInt err;

    // Before attempting db modification, check if we would go below Critical
    // Level in FFS (disk). The value used for checking is an estimate.
    // Reading the db also allocates some bytes (transaction log), but
    // this is not estimated and not checked, because it should be small
    // enough, and would slow down operation. (Typical use is to read a lot
    // and write much less.)
    if (
            aLffsCheck &&
            aWrite &&
            Util::FFSSpaceBelowCriticalLevelL
                ( /*aShowErrorNote=*/aDbErrorNote, KEstimatedFfsUsage )
       )
        {
        err = KErrDiskFull;
        return err;
        }

    // Open database.
    err = OpenDbL( /*aDbErrorNote=*/EFalse ); // Error note (if any) is here.
    // Start a transaction.
    if ( !err )
        {
        err = TimedBeginL( aWrite );
        }

    if ( err && aDbErrorNote )
        {
        TBrowserDialogs::ErrorNoteL( _R(TEXT_CANNOT_ACCESS_DB) );
        }

    return err;
    }

// ---------------------------------------------------------
// CBrowserFavouritesModel::CommitL
// ---------------------------------------------------------
//
void CBrowserFavouritesModel::CommitL()
    {
    iDb.Commit();
    CleanupStack::Pop();  // DO NOT remove this hack.
    }

// ---------------------------------------------------------
// CBrowserFavouritesModel::AddObserverL
// ---------------------------------------------------------
//
void CBrowserFavouritesModel::AddObserverL
( MBrowserFavouritesModelObserver& aObserver )
    {
    iObservers->AppendL( &aObserver );
    }

// ---------------------------------------------------------
// CBrowserFavouritesModel::RemoveObserver
// ---------------------------------------------------------
//
void CBrowserFavouritesModel::RemoveObserver
( MBrowserFavouritesModelObserver& aObserver )
    {
    TInt i;
    TInt count = iObservers->Count();
    for ( i = 0; i < count; i++ )
        {
        if ( iObservers->At( i ) == &aObserver )
            {
            iObservers->Delete( i );
            return;
            }
        }
    }

// ---------------------------------------------------------
// CBrowserFavouritesModel::AddL
// ---------------------------------------------------------
//
TInt CBrowserFavouritesModel::AddL
        (
        CFavouritesItem& aItem,
        TBool aDbErrorNote,
        CBrowserFavouritesModel::TRenameMode aRenameMode /*=EAskIfRename*/
        )
    {
    return AddUpdateL( aItem, KFavouritesNullUid, aDbErrorNote, aRenameMode );
    }

// ---------------------------------------------------------
// CBrowserFavouritesModel::UpdateL
// ---------------------------------------------------------
//
TInt CBrowserFavouritesModel::UpdateL
        (
        CFavouritesItem& aItem,
        TInt aUid,
        TBool aDbErrorNote,
        CBrowserFavouritesModel::TRenameMode aRenameMode /*=EAskIfRename*/
        )
    {
    return AddUpdateL( aItem, aUid, aDbErrorNote, aRenameMode );
    }

// ---------------------------------------------------------
// CBrowserFavouritesModel::SetHomepageL
// ---------------------------------------------------------
//
TInt CBrowserFavouritesModel::SetHomepageL
        (
        CFavouritesItem& aItem,
        TBool aDbErrorNote,
        CBrowserFavouritesModel::TRenameMode aRenameMode /*=EAskIfRename*/
        )
    {
    return AddUpdateL
        ( aItem, KFavouritesHomepageUid, aDbErrorNote, aRenameMode );
    }

// ---------------------------------------------------------
// CBrowserFavouritesModel::SetLastVisitedL
// ---------------------------------------------------------
//
TInt CBrowserFavouritesModel::SetLastVisitedL
        (
        CFavouritesItem& aItem,
        TBool aDbErrorNote,
        CBrowserFavouritesModel::TRenameMode aRenameMode /*=EAskIfRename*/
        )
    {
    return AddUpdateL
        ( aItem, KFavouritesLastVisitedUid, aDbErrorNote, aRenameMode );
    }

// ---------------------------------------------------------
// CBrowserFavouritesModel::AnyFoldersL
// ---------------------------------------------------------
//
CBrowserFavouritesModel::TDbQuery CBrowserFavouritesModel::AnyFoldersL()
    {
    TDbQuery ret = EError;
    if ( BeginL( /*aWrite=*/EFalse, /*aDbErrorNote=*/ETrue ) == KErrNone )
        {
        TInt count = 0;
        if ( iDb.Count( count, KFavouritesRootUid, CFavouritesItem::EFolder )
                                                                == KErrNone )
            {
            ret = count ? EYes : ENo;
            }
        CommitL();
        }
    return ret;
    }

// ---------------------------------------------------------
// CBrowserFavouritesModel::AnyNonEmptyFoldersL
// ---------------------------------------------------------
//
CBrowserFavouritesModel::TDbQuery
CBrowserFavouritesModel::AnyNonEmptyFoldersL( CArrayFix<TInt>& aUids )
    {
    TDbQuery ret = EError;
    if ( BeginL( /*aWrite=*/EFalse, /*aDbErrorNote=*/ETrue ) == KErrNone )
        {
        ret = ENo;
        TInt itemCount;
        TInt i;
        TInt uidCount = aUids.Count();
        for ( i = 0; i < uidCount; i++ )
            {
            if ( iDb.Count( itemCount, aUids.At( i ) ) == KErrNone )
                {
                if ( itemCount )
                    {
                    // We have a non-empty folder.
                    ret = EYes;
                    break;
                    }
                }
            else
                {
                // Error querying; quit.
                ret = EError;
                break;
                }
            }
        CommitL();
        }
    return ret;
    }

// ---------------------------------------------------------
// CBrowserFavouritesModel::FolderInfoL
// ---------------------------------------------------------
//
TInt CBrowserFavouritesModel::FolderInfoL
( TInt aFolder, TInt& aFolderCount, TInt& aFolderIndex )
    {
    TInt err = KErrNotFound;
    
    if(iApiProvider.CalledFromAnotherApp())
        {
        err = KErrNone;
        aFolderIndex = 0;
        aFolderCount = 1;
        return err;
        }
    
    CFavouritesItemList* folders = GetFoldersSortedLC();
    // Iterate until we find the folder.
    TInt i;
    TInt actualCount = 0;
    aFolderCount = actualCount = folders->Count();
    for ( i = 0; i < actualCount; i++ )
        {
        if(folders->At( i )->IsHidden())
            {
                aFolderCount--;
            }
        if ( folders->At( i )->Uid() == aFolder )
            {
            aFolderIndex = i;
            err = KErrNone;
            }
        }
    if(aFolderIndex > aFolderCount)
        {
            aFolderIndex += (aFolderCount - actualCount);
        }
    CleanupStack::PopAndDestroy();  // folders
    return err;
    }

// ---------------------------------------------------------
// CBrowserFavouritesModel::NextFolderL
// ---------------------------------------------------------
//
TInt CBrowserFavouritesModel::NextFolderL( TInt aFolder, TBool aForward )
    {
    if ( AknLayoutUtils::LayoutMirrored() )
        {
        aForward = !aForward;
        }
    TInt folder = KFavouritesNullUid;
    
    if(iApiProvider.CalledFromAnotherApp())
        {
        return folder;
        }
    
    CFavouritesItemList* folders = GetFoldersSortedLC();
    // Iterate until we find the folder.
    TInt i;
    TInt count = folders->Count();
    for ( i = 0; i < count; i++ )
        {
        if ( folders->At( i )->Uid() == aFolder )
            {
            i += aForward ? 1 : -1;
            if ( i >= 0 && i < count )
                {
                // Have the neighbour.
                folder = folders->At( i )->Uid();
                
                while(folders->At( i )->IsHidden())
                    {
                    i += aForward ? 1 : -1;
                    if ( i < 0 || i >= count )
                        {
                        folder = KFavouritesNullUid;
                        break;
                        }
                     else
                        {
                        // Have the neighbour.
                        folder = folders->At( i )->Uid();
                        }
                    }
                }
            break;
            }
        }
    CleanupStack::PopAndDestroy();  // folders
    return folder;
    }

// ---------------------------------------------------------
// CBrowserFavouritesModel::RenameQueryL
// ---------------------------------------------------------
//
TBool CBrowserFavouritesModel::RenameQueryL( CFavouritesItem& aItem, TBool aNewItem )
    {
    TBool ret = EFalse;
    TBuf<KFavouritesMaxName> buf = aItem.Name();
    CAknTextQueryDialog* dialog = CAknTextQueryDialog::NewL( buf );
    dialog->PrepareLC( _R(NAME_QUERY_OK_CANCEL) );
    dialog->SetPredictiveTextInputPermitted(ETrue);
    HBufC* prompt;
    if ( aNewItem )
        {
        prompt =
            StringLoader::LoadLC( StringResourceId( aItem, ENewPrompt ) );
        }
    else
        {
        prompt =
            StringLoader::LoadLC( StringResourceId( aItem, ERenamePrompt ) );
        }
    dialog->SetPromptL( *prompt );
    CleanupStack::PopAndDestroy();  // prompt
    TInt button = dialog->RunLD();
    if ( button )
        {
        aItem.SetNameL( buf.Left(KFavouritesMaxName) );
        ret = ETrue;
        }
    return ret;
    }

// ---------------------------------------------------------
// CBrowserFavouritesModel::SortL
// ---------------------------------------------------------
//
void CBrowserFavouritesModel::SortL( CFavouritesItemList& aList )
    {
    TBuf<KFavouritesMaxName> name;
    TInt i;
    TInt count = aList.Count();

    TInt resId = 0;

    for ( i = 0; i < count; i++ )
        {
        resId = CBrowserFavouritesView::GetSeamlessFolderResourceID( 
        			aList.At(i)->ContextId() );
        
        if(resId) // a seamless link folder. Get the localised name.
            {
            HBufC* seamlessLinkName = CCoeEnv::Static()->AllocReadResourceLC( resId );
            name = seamlessLinkName->Des();
            CleanupStack::PopAndDestroy(); // seamlessLinkName
			aList.At( i )->SetNameL( name );
            }
		}

    aList.SortL( &CompareItemsL );
    }

// ---------------------------------------------------------
// CBrowserFavouritesModel::MakeUniqueNameL
// ---------------------------------------------------------
//
TInt CBrowserFavouritesModel::MakeUniqueNameL
( CFavouritesItem& aItem )
    {
    TInt err = BeginL( /*aWrite=*/EFalse, /*aDbErrorNote=*/ETrue );
    if ( !err )
        {
        err = iDb.MakeUniqueName( aItem );
        CommitL();
        }
    return err;
    }

// ---------------------------------------------------------
// CBrowserFavouritesModel::HandleFavouritesDbEventL
// ---------------------------------------------------------
//
void CBrowserFavouritesModel::HandleFavouritesDbEventL
( RDbNotifier::TEvent aEvent )
    {
    switch ( aEvent )
        {
        case RDbNotifier::ECommit:
            {
            // Database has changed, notify observers.
            NotifyObserversL();
            break;
            }

        case RDbNotifier::ERollback:
            {
            // Database corrupted, close now and wait until everyone closes it.
            iState = ECorrupt;
            iDb.Close();
            // But keep the notifier alive.
            break;
            }

        case RDbNotifier::EClose:
            {
            // Database is closed by all clients.
            if ( iState == ECorrupt )
                {
                // Database was closed by the model (by damage), not the user.
                // Try to reopen it now; that will recover it.
                iState = EClosed;
                iNotifier->Cancel();
                if ( OpenDbL( /*aDbErrorNote=*/EFalse ) == KErrNone )
                    {
                    // Succesfully reopened. Data may have changed.
                    NotifyObserversL();
                    }
                }
            break;
            }

		default:
            {
			break;
            }
        }
    }

// ---------------------------------------------------------
// CBrowserFavouritesModel::CBrowserFavouritesModel
// ---------------------------------------------------------
//
CBrowserFavouritesModel::CBrowserFavouritesModel( MApiProvider& aApiProvider )
	:
	iApiProvider( aApiProvider )
	{
    iState = EClosed;
	}

// ---------------------------------------------------------
// CBrowserFavouritesModel::ConstructL
// ---------------------------------------------------------
//
void CBrowserFavouritesModel::ConstructL()
	{
    iObservers = new (ELeave) CArrayPtrFlat<MBrowserFavouritesModelObserver>
        ( KGranularityObservers );
    iNotifier = new (ELeave) CActiveFavouritesDbNotifier( iDb, *this );
    User::LeaveIfError( iFavouritesSess.Connect() );
	}

// ---------------------------------------------------------
// CBrowserFavouritesModel::TimedOpenDbL
// ---------------------------------------------------------
//
TInt CBrowserFavouritesModel::TimedOpenDbL()
    {
    TInt retriesLeft = KLocalRetryCount;
    TInt err = ( iDb.Open( iFavouritesSess, GetDBName() ));
    while ( err && retriesLeft-- )
        {        
        User::After( KRetryWait );
        err = ( iDb.Open( iFavouritesSess, GetDBName() ));
        }
    return err;
    }
    
// ---------------------------------------------------------
// CBrowserFavouritesModel::GetDBName
// ---------------------------------------------------------
//
const TDesC& CBrowserFavouritesModel::GetDBName()
    {
    //should be overridden by bookmarks model
    return KBrowserSavedPages; //default value
    }

// ---------------------------------------------------------
// CBrowserFavouritesModel::TimedBeginL
// ---------------------------------------------------------
//
TInt CBrowserFavouritesModel::TimedBeginL( TBool /*aWrite*/ )
    {
    TInt retriesLeft = KLocalRetryCount;
    TInt err =  iDb.Begin();
    if ( err == KErrNone )
        {
        iDb.CleanupRollbackPushL();  
        }
    while ( err && retriesLeft-- )
        {
        err = iDb.Begin() ;
        if ( err == KErrNone )
            {
            iDb.CleanupRollbackPushL();  
            }
        }
    return err;
    }

// ---------------------------------------------------------
// CBrowserFavouritesModel::AddUpdateL
// ---------------------------------------------------------
//
TInt CBrowserFavouritesModel::AddUpdateL
        (
        CFavouritesItem& aItem,
        TInt aUid,
        TBool aDbErrorNote,
        TRenameMode aRenameMode
        )
    {
    TBool done = EFalse;
    TInt err = KErrNone;

    while ( !done )
        {
        err = BeginL( /*aWrite=*/ETrue, aDbErrorNote );
        if ( err )
            {
            // Database error.
            done = ETrue;
            }
        else
            {
            // Database OK, try to add/update item...
            switch ( aUid )
                {
                    {
                case KFavouritesHomepageUid:
                    // Homepage update.
                    err = iDb.SetHomepage( aItem );
                    break;
                    }

                case KFavouritesLastVisitedUid:
                    {
                    // Last Visited update.
                    err = iDb.SetLastVisited( aItem );
                    break;
                    }

                case KFavouritesNullUid:
                    {
                    // Check first if the item added has the same name as any of the special folders
                    
                    TBool itemExists = EFalse;
                    if (aItem.Type() == CFavouritesItem::EFolder)
                        {   
                        #ifdef __RSS_FEEDS
                        HBufC* webFeedsBuf = StringLoader::LoadLC( R_BROWSER_APPS_FEEDS_LIST );
                        TPtrC webFeeds = webFeedsBuf->Des();
                        if(CompareIgnoreCase(aItem.Name(),webFeeds))
                            {
                            itemExists = ETrue;
                            }
                        CleanupStack::PopAndDestroy(); 
                        #endif
                        
               		    if ( ( ADAPTIVEBOOKMARKS ) &&
                            ( iApiProvider.Preferences().AdaptiveBookmarks() ==
                                            EWmlSettingsAdaptiveBookmarksOn ))
               		        {
               		        HBufC* recentUrlsBuf = StringLoader::LoadLC( R_BROWSER_ADAPTIVE_BOOKMARKS_FOLDER);
                            TPtrC recentUrls = recentUrlsBuf->Des();
                            if(CompareIgnoreCase(aItem.Name(),recentUrls))
                                {
                                itemExists = ETrue;
                                }
                            CleanupStack::PopAndDestroy(); 
               		        }
                        }
                    
                      if (( itemExists ) && (aItem.Type() == CFavouritesItem::EFolder))
                        {
                        err = KErrAlreadyExists;
                        }
                    else
                        {
                        // New item addition.
                        err = iDb.Add( aItem, aRenameMode == EAutoRename );                        
                        }
                    break;
                    }

                default:
                    {
                    // Existing item update.
                    err = iDb.Update( aItem, aUid, aRenameMode == EAutoRename );
                    break;
                    }
                }
            CommitL();
            
            // ...and see the outcome.
            switch( err )
                {
                case KErrNone:
                    {
                    // All is well.
                    done = ETrue;
                    break;
                    }
                case KErrAlreadyExists:
                    {
                    // The name is conflicting.
                    switch ( aRenameMode )
                        {
                        case EAutoRename:
                            {
                            // We should never get here;
                            // this is handled by the Engine's AutoRename.
                            __ASSERT_DEBUG( EFalse, Util::Panic\
                                ( Util::EFavouritesInternal ) );
                            done = ETrue;
                            break;
                            }
                        case EDontRename:
                            {
                            // No rename is needed.
                            // Return with KErrAlreadyExists.
                            done = ETrue;
                            break;
                            }
                        case EAskIfRename:
                        default:
                            {
                            // Prompt the user for new name.
                            // Retry with new name, or return if Cancelled.
							TBrowserDialogs::InfoNoteL
											(
											R_BROWSER_INFO_NOTE,
											_R(TEXT_FLDR_NAME_ALREADY_USED),
											aItem.Name()
											);
                            done = !RenameQueryL( aItem, EFalse );
                            break;
                            }
                        }
                    break;
                    }
                case KErrPathNotFound:
                    {
                    // The name is conflicting.
                    switch ( aRenameMode )
                        {
                        case EAutoRename:
                            {
                            // We should never get here;
                            // this is handled by the Engine's AutoRename.
                            __ASSERT_DEBUG( EFalse, Util::Panic\
                                ( Util::EFavouritesInternal ) );
                            done = ETrue;
                            break;
                            }
                        case EDontRename:
                            {
                            // No rename is needed.
                            // Return with KErrAlreadyExists.
                            done = ETrue;
                            break;
                            }
                        case EAskIfRename:
                        default:
                            {
                            // Prompt the user for new name.
                            // Retry with new name, or return if Cancelled.
							TBrowserDialogs::InfoNoteL
												(
												R_BROWSER_INFO_NOTE,
												_R( TEXT_PRMPT_ILLEGAL_CHAR ),
												aItem.Name()
												);							
                            done = !RenameQueryL( aItem, EFalse );
                            break;
                            }
                        }
                    break;
					}
                    
                case KErrNotFound:
                    {
                    // Update leaved with KErrNotFound meaning someone deleted entry while it was edited
                    // Add entry as new item instead.
                    if (aUid != KFavouritesNullUid)
                        {
                        aUid = KFavouritesNullUid;
                        }
                    break;
                    }
                    
                default:
                    {
                    // Some error; quit.
                    done = ETrue;
                    break;
                    }
                }
            }
        }
    return err;
    }

// ---------------------------------------------------------
// CBrowserFavouritesModel::GetFoldersSortedLC
// ---------------------------------------------------------
//
CFavouritesItemList* CBrowserFavouritesModel::GetFoldersSortedLC()
    {
    CFavouritesItemList* folders = new (ELeave) CFavouritesItemList();
    CleanupStack::PushL( folders );
    // Get all folders.
    iDb.GetAll( *folders, KFavouritesRootUid, CFavouritesItem::EFolder );
     // Sort folders,
    folders->SortL( &CompareItemsL );
    return folders;
    }

// ---------------------------------------------------------
// CBrowserFavouritesModel::NotifyObserversL
// ---------------------------------------------------------
//
void CBrowserFavouritesModel::NotifyObserversL()
    {
    TInt i;
    TInt count = iObservers->Count();
    for ( i = 0; i < count; i++ )
        {
        iObservers->At( i )->HandleFavouritesModelChangeL();
        }
    }

// ---------------------------------------------------------
// CBrowserBookmarksModel::ManualSortL
// ---------------------------------------------------------
//
void CBrowserFavouritesModel::ManualSortL(TInt aFolder, CBrowserBookmarksOrder* aBMOrder, CArrayFixFlat<TInt>* aOrderArray, CFavouritesItemList* aItems)
    {
    if ( Database().GetData( aFolder ,*aBMOrder ) == KErrNone)
        {
        if( aOrderArray->Count() )
            {
            TInt swapTo = 0;
            CFavouritesItem* swappedItem = CFavouritesItem::NewL();
            CleanupStack::PushL(swappedItem);
            for ( TInt i=0; i<aOrderArray->Count();i++ )    
                {
                TInt swapFrom = aItems->UidToIndex( ( *aOrderArray )[i] );
                if (swapFrom != KErrNotFound)
                    {
                    if ((swapFrom != swapTo) && (swapTo < aItems->Count() ) )
                        {
                        (*swappedItem) = *( *aItems )[swapFrom];
                        *( *aItems )[swapFrom] = *( *aItems )[swapTo];
                        *( *aItems )[swapTo] = (*swappedItem);
                        }
                    swapTo++;
                    }
                }
            CleanupStack::PopAndDestroy(swappedItem);
            }
        else
            {
              SortL(*aItems);
            }
        }
    else
        {
        SortL(*aItems);
        }
    }

// ---------------------------------------------------------
// CBrowserFavouritesModel::AddUidToLastPlaceL
// ---------------------------------------------------------
//
void CBrowserFavouritesModel::AddUidToLastPlaceL(
        TInt aUid,
        CArrayFixFlat<TInt>* aOrderArray,
        CBrowserBookmarksOrder* aCurrentOrder )
    {
    if( aCurrentOrder )
        {
        TInt index(0);
        if( aOrderArray->Count() )
            {
           	index = aOrderArray->Count();
            }
        aOrderArray->InsertL( index , aUid );
        }
    }

// ---------------------------------------------------------
// CBrowserFavouritesModel::OpenFavouritesFile
// ---------------------------------------------------------
//

TInt CBrowserFavouritesModel::OpenFavouritesFile ( RFavouritesFile & aFavFile, TInt aUid )
{
return aFavFile.Open( iDb, aUid );
}

// ---------------------------------------------------------
// CBrowserFavouritesModel::ReplaceFavouritesFile
// ---------------------------------------------------------
//

TInt CBrowserFavouritesModel::ReplaceFavouritesFile ( RFavouritesFile & aFavFile, TInt aUid )
{
return aFavFile.Replace( iDb, aUid );
}

// ---------------------------------------------------------------------------
// CBrowserFavouritesModel::CompareIgnoreCase
// Compare two descriptors
// ---------------------------------------------------------------------------
//
TBool CBrowserFavouritesModel::CompareIgnoreCase( const TDesC& aFirst, 
                                      const TDesC& aSecond )
    {
    // Get the standard method
    TCollationMethod meth = *Mem::CollationMethodByIndex( 0 );
    meth.iFlags |= TCollationMethod::EFoldCase;
    meth.iFlags |= TCollationMethod::EIgnoreNone;

    // Magic: Collation level 3 is used
    return aFirst.CompareC( aSecond, 3, &meth ) == 0;
    }


// End of File
