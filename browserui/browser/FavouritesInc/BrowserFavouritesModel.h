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
*      Declaration of class CBrowserFavouritesModel.
*      
*
*/


#ifndef BROWSER_FAVOURITES_MODEL_H
#define BROWSER_FAVOURITES_MODEL_H

//  INCLUDES

#include <e32base.h>
#include <FavouritesDbObserver.h>
#include "BrowserBookmarksOrder.h"
#include "FavouritesDb.h"
#include "FavouritesFile.h"
#include "ApiProvider.h"

// FORWARD DECLARATION

class CFavouritesItemList;
class CActiveFavouritesDbNotifier;
class MBrowserFavouritesModelObserver;
class CFavouritesItem;

// CLASS DECLARATION

/**
* Model for the Favourites Views of the WML Browser.
* This is a layer above the actual engines that do the data manipulation;
* and provides UI support for that (i.e. dialogs, notes etc.)
* Pure virtual.
*/
class CBrowserFavouritesModel: public CBase, public MFavouritesDbObserver
	{

    public:     // types

        /**
        * Answer for database queries.
        */
        enum TDbQuery
            {
            EYes,   ///< Answer to query is yes.
            ENo,    ///< Answer to query is no.
            EError  ///< Could not get answer (i.e. database is busy etc.).
            };

        /**
        * Rename mode for adding / updating items (what to do if an item
        * to be added / updated in the database has a conflicting name).
        */
        enum TRenameMode
            {
            EAutoRename,    ///< Rename silently.
            EDontRename,    ///< Do not rename (quit operation).
            EAskIfRename    ///< Ask user.
            };

	public:     // construction

        /**
        * Destructor.
        */
        virtual ~CBrowserFavouritesModel();

	public:     // string resources

        /**
        * Resource selector type for strings.
        */
        enum TTextResourceType
            {
            EDefaultName,   ///< Default name (e.g. "New Bookmark").
            ERenamePrompt,  ///< Rename prompt (e.g. "New Folder:").
            ENewPrompt,  ///< New prompt (e.g. "Folder name:").
            ESaved,         ///< Saved note (e.g. "Bookmark saved").
            ENotSaved       ///< Not saved note (e.g. "Bookmark not saved").
            };

        /**
        * Get resource id of some text for an item.
        * @param aItem The item to get resource id for.
        * @param aType Type of resource text.
        * @return Resource id of the text.
        */
        virtual TInt StringResourceId
            (
            const CFavouritesItem& aItem,
            TTextResourceType aType
            ) const = 0;

        /**
        * Set the name of this item to default (e.g. "New Bookmark",
        * "New Folder" etc.). The name that is set is read from resource,
        * the resource id is StringResourceId( EDefaultName ).
        * @param aItem The item to set default name for.
        */
        void SetNameToDefaultL( CFavouritesItem& aItem ) const;

    public:    // database opening / closing / access

        /**
        * Open the database.
        * After succesful open, notifications to observer may come.
        * Safe to call if already open.
        * @param aDbErrorNote If ETrue, unsuccesful open will bring up an
        * error note.
        * @return Error code.
        */
        TInt OpenDbL( TBool aDbErrorNote = ETrue );

        /**
        * Close the database. No further notification may come to the
        * observer.
        */
        void CloseDb();

        /**
        * Start a transaction.
        * @param aWrite If ETrue, the table is opened with write-lock on.
        * @param aDbErrorNote If ETrue, unsuccesful open will bring up an
        * error note.
        * @param aLffsCheck If ETrue (the default), free disk space is
        * checked to be above critical level (only if aWrite==ETrue).
        * Always use the default (except for deleting)!
        * @return Error code.
        */
        TInt BeginL
            ( TBool aWrite, TBool aDbErrorNote, TBool aLffsCheck = ETrue);

        /**
        * Commit the transaction.
        * Safe to call if not open.
        */
        void CommitL();

        /**
        * Add an observer.
        * @param aObserver Observer to be notified about database-related
        * events.
        */
        void AddObserverL( MBrowserFavouritesModelObserver& aObserver );

        /**
        * Remove an observer. Panic if not added.
        * @param aObserver The observer.
        */
        void RemoveObserver( MBrowserFavouritesModelObserver& aObserver );

        /**
        * Get the database.
        * @return The actual database.
        */
        inline RFavouritesDb Database() const;

        /**
        * Opens an item associated with aUid in aFavFile.
        * @return Error code.
        */
                
        TInt OpenFavouritesFile ( RFavouritesFile & aFavFile, TInt aUid );

        /**
        * Opens an item associated with aUid in aFavFile for replacement.
        * @return Error code.
        */
        
        TInt ReplaceFavouritesFile ( RFavouritesFile & aFavFile, TInt aUid );        

        
    public:     // database query / update

        /**
        * Add an item (folder or bookmark) to the database.
        * @param aItem Item to add.
        * @param aDbErrorNote If ETrue, unsuccesful database open will bring up an
        * error note.
        * @param aRenameMode What to do in case of the name is conflicting.
        * @return Error code.
        */
        TInt AddL
            (
            CFavouritesItem& aItem,
            TBool aDbErrorNote,
            TRenameMode aRenameMode
            );

        /**
        * Update an item (folder or bookmark) in the database.
        * @param aItem Item data.
        * @param aUid Uid of item to be updated.
        * @param aDbErrorNote If ETrue, unsuccesful database open will bring up
        * an error note.
        * @param aRenameMode What to do in case of the name is conflicting.
        * @return Error code.
        */
        TInt UpdateL
            (
            CFavouritesItem& aItem,
            TInt aUid,
            TBool aDbErrorNote,
            TRenameMode aRenameMode
            );

        /**
        * Set Homepage.
        * @param aItem Item containing Homepage data.
        * @param aDbErrorNote If ETrue, unsuccesful database open will bring up an
        * error note.
        * @param aRenameMode What to do in case of the name is conflicting.
        * @return Error code.
        */
        TInt SetHomepageL
            (
            CFavouritesItem& aItem,
            TBool aDbErrorNote,
            TRenameMode aRenameMode
            );

        /**
        * Set Last Visited Page.
        * @param aItem Item containing Homepage data.
        * @param aDbErrorNote If ETrue, unsuccesful database open will bring up
        * an error note.
        * @param aRenameMode What to do in case of the name is conflicting.
        * @return Error code.
        */
        TInt SetLastVisitedL
            (
            CFavouritesItem& aItem,
            TBool aDbErrorNote,
            TRenameMode aRenameMode
            );

        /**
        * Check if there are any folders (except the root).
        * @return EYes, ENo or EError (if could not access the database).
        */
        TDbQuery AnyFoldersL();

        /**
        * Check if the given list of uids contains
        * any folders that are not empty.
        * @param aUids List of Uids to check.
        * @return EYes if there are non-empty folder(s), ENo if not,
        * EError if could not access the database.
        */
        TDbQuery AnyNonEmptyFoldersL( CArrayFix<TInt>& aUids );

        /**
        * Get the ordinal number of this folder in the alphabetically sorted
        * list of folders. Expects a transaction.
        * @param aFolder The folder to find.
        * @param aFolderCount On successful return, this will be set to the
        * number of folders.
        * @param On successful return, this will be set to the ordinal number
        * of the given folder.
        * @return Error code.
        */
        TInt FolderInfoL
            ( TInt aFolder, TInt& aFolderCount, TInt& aFolderIndex );

        /**
        * In the alphabetically sorted list of folders, get Uid of folder next
        * to aFolder. Expects a transaction.
        * @param aFolder Folder to start from.
        * @param aForward If ETrue, get next; if EFalse, get previous.
        * @return Uid of the next folder or KFavouritesNullUid in case of any
        * error.
        */
        TInt NextFolderL( TInt aFolder, TBool aForward );

        /**
        * Display a query for renaming this item. If the query is accepted,
        * the item is renamed.
        * @param aItem The item to rename.
        * @param aItem New item, we do not rename.
        * @return ETrue if the query is accepted and the item is renamed.
        */
        TBool RenameQueryL( CFavouritesItem& aItem, TBool aNewItem );

        /**
        * Sort items in a list.
        * @param aList List to sort.
        */
        void SortL( CFavouritesItemList& aList );

    public:     // convenience methods

        /**
        * Check if aItem has a unique name (in its parent folder), and if
        * not, make its name unique.
        * @param aItem Item to set a unique name for.
        * @return Error code.
        */
        TInt MakeUniqueNameL( CFavouritesItem& aItem );

	protected:	// new functions

		//MApiProvider& ApiProvider() const;

    public:     // from MFavouritesDbObserver

        /**
        * Handle database event.
        * @param aEvent Database event.
        */
        void HandleFavouritesDbEventL( RDbNotifier::TEvent aEvent );

        void ManualSortL(TInt aFolder, CBrowserBookmarksOrder* aBMOrder, CArrayFixFlat<TInt>* aOrderArray, CFavouritesItemList* aItems);

        /**
        * Adds the given uid to the last place in bookmarks list
        * @param aUid The uid to add
        * @param aOrderArray the array that stores uids in manual order
        * @param aCurrentOrder the object that stores manual sorting data
        */

        void AddUidToLastPlaceL( TInt aUid , CArrayFixFlat<TInt>* aOrderArray, CBrowserBookmarksOrder* aCurrentOrder);

	protected :	// construction

        /**
        * Constructor.
        */
		CBrowserFavouritesModel( MApiProvider& aApiProvider );

        /**
        * Second phase constructor. Leaves on failure.
        */
		void ConstructL();

        /**
        * Returns with the underlying database's name.
        * Derived classes must provide this method.
        * @return The Database Engine's name.
        */
        virtual const TDesC& GetDBName();
     
    private:    // implementation details

        /**
        * Open the database. If unsuccesful, retry two more times in
        * 0.2 secs interval.
        * @return Error code.
        */
        TInt TimedOpenDbL();

        /**
        * Start a transaction. If unsuccesful, retry two more times in
        * 0.2 secs interval.
        * @param aWrite If ETrue, the table is opened with write-lock on.
        * @return Error code.
        */
        TInt TimedBeginL( TBool aWrite );

        /**
        * Add/update an item (folder or item) in the database; factoring out
        * the common code.
        * @param aItem Item data.
        * @param aUid
        *   - When KFavouritesNullUid, the item is added.
        *   - Otherwise, Uid of the item to be updated.
        * @param aDbErrorNote If ETrue, unsuccesful database open will bring up
        * an error note.
        * @param aRenameMode What to do in case of the name is conflicting.
        * @return Error code.
        */
        TInt AddUpdateL
            (
            CFavouritesItem& aItem,
            TInt aUid,
            TBool aDbErrorNote,
            TRenameMode aRenameMode
            );
            
        /**
        * Compare two descriptors (case insensitive).
        * @since 3.2
        * @param aFirst   first descriptor
        * @param aSecond  second descriptor
        * @return ETrue if strings match, EFalse otherwise
        */
        TBool CompareIgnoreCase( const TDesC& aFirst, const TDesC& aSecond );


	protected:
        /**
        * Get all folders in the root, sorted, and push it on the cleanup
        * stack. No sanity check. Expects a transaction.
        * @return List of folders. Owner is the caller.
        */
	virtual CFavouritesItemList* GetFoldersSortedLC();

	private:	// observer support

        /**
        * Call each observer's HandleFavouritesModelChangeL().
        */
        void NotifyObserversL();

	private:	// types

        enum TState     ///< Database state.
            {
            EClosed,    ///< Database is closed.
            EOpen,      ///< Database is open.
            ECorrupt    ///< Database is corrupted (and is closed)
            };

	private:	// data

        /// Used database object. Owned.
        
        RFavouritesDb iDb;
        RFavouritesSession iFavouritesSess;        
        /// Notifier object. Owned.
        CActiveFavouritesDbNotifier* iNotifier;
        /// Observers. Array owned, contents not.
        CArrayPtrFlat<MBrowserFavouritesModelObserver>* iObservers;
        /// Database state.
        TState iState;

		MApiProvider& iApiProvider;
	};

#include "BrowserFavouritesModel.inl"

#endif

// End of file