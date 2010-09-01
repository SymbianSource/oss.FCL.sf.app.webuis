/*
* Copyright (c) 2005 Nokia Corporation and/or its subsidiary(-ies).
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
* Description:  Manages the feed related views and implements the FeedsViewBridge.
*
*/

#ifndef FEEDS_CLIENT_UTILITIES_H
#define FEEDS_CLIENT_UTILITIES_H

// INCLUDES
#include <brctlspecialloadobserver.h>

#include "Preferences.h"
#include "FeedsWaitDialog.h"
#include "FeedsFileSearchAgent.h"
#include "CommonConstants.h"
#include <feedsinterface.h>
#include <feedsmap.h>
#include <transaction.h>

// CONSTANTS

// MACROS

// DATA TYPES

// FUNCTION PROTOTYPES

// FORWARD DECLARATIONS
class CAknViewAppUi;
class CEikMenuPane;
class MApiProvider;
class CFeedsFolderView;
class CFeedsTopicView;
class CFeedsFeedView;
class CBrowserGotoPane;

// CLASS DECLARATION


/**
*  Manages the feed related views and implements the FeedsViewBridge.
*  @lib FeedsViewBridge.lib
*  @since 3.1
*/
class CFeedsClientUtilities: public CBase, 
        public MPreferencesObserver,
        public MFeedsWaitDialogListener,
     	public MFeedsFileSearchAgentCallback,
     	public MFeedsApiObserver
    {
    public:  // Constructors and destructor

        static CFeedsClientUtilities* NewL( CAknViewAppUi& aAppUI, MApiProvider& aApiProvider );

        /**
        * Destructor.
        */        
        ~CFeedsClientUtilities();

    public:  // from MPreferencesObserver

        /**
        * Derived classes should implement this method, and act accordingly.
        */
        void HandlePreferencesChangeL( const TPreferencesEvent aEvent,
                                       TPreferencesValues& aValues,
                                       TBrCtlDefs::TBrCtlSettings aSettingType );
   
    public: //From MFeedsApiObserver
        /**
        * This function is called by CFeedsInterface class whenever
        * request is completed
        * 
        * @param aTrans Pointer to current transaction class
        * @param aEvent event related to this call.
        */
        virtual void RequestCompleted(CTransaction* aTrans,TInt aEvent);

	public:
        /**
        * Request to create a network connection.
        *
        * @since 7.1
        * @param aConnectionPtr A pointer to the new connection. If NULL, the 
        *                       proxy filter will automatically create a network connection.
        * @param aSockSvrHandle A handle to the socket server.
        * @param aNewConn A flag if a new connection was created. If the 
        *                 connection is not new, proxy filter optimization will not 
        *                 read the proxy again from CommsBd.
        * @param aBearerType The bearer type of the new connection
        * @return void
        */
    	void NetworkConnectionNeededL( TInt* aConnectionPtr, TInt& aSockSvrHandle,
                                       TBool& aNewConn, TApBearerType& aBearerType );

    public:
    
        /**
        * Set Browser prefs to Feed engine
        * @since 3.1
        * @param aValues Browser preferences       
        * @return void
        */    
        void SetPreferencesToFeedL( const TPreferencesValues& aValues );
        
        /**
        * Load Url in new window
        *
        * @since 3.1
        * @return void.
        */
    	void LoadUrlL( const TDesC& aUrl );
    	
        /**
        * Shows the folder view, loading the folder list from the FeedsServer if need be.
        *
        * @since 3.1
        * @param aCalledFromView to recored from which view this was invoked. Default value
        *   is BrowserBookmarksViewId if this argument is not specified.
        * @return void.
        */
        void ShowFolderViewL(TUid aCalledFromView = KUidBrowserBookmarksViewId);

        /**
        * Shows the topic view, loading the given feed associated with the given 
        * folder item.
        *
        * @since 3.1
        * @return void.
        */
        void ShowTopicViewL(const CFeedsEntity& aFolderItem);

        /**
        * Shows the folder view and subscribes to the given feed.
        *
        * @since 3.1
        * @param aTitle The feed's title.
        * @param aUrl The feed's url.
        * @return void.
        */
        void SubscribeToL(const TDesC& aTitle, const TDesC& aUrl);

        /**
        * Returns the feed related mime-types.  This makes 
        * for a clean way to pass downloaded feeds content from the client to the FeedsServer 
        * (via CFeedsClientUtilities::HandleFeedL).
        *
        * @since 3.1
        * @return An array of supported mime-types.
        */
        TPtrC SupportedMimeTypesL();
                
        /**
        * Returns Feeds Server related settings.
        *
        * @since 3.1
        * @return Feeds Server related settings.
        */
        TInt FeedsServerSettingsL(TFeedsServerSetting& aSetting);
        
        /**
        * Sets Feeds Server related settings.
        *
        * @since 3.1
        * @param aNewSettings The new settings.
        * @return void.
        */
        void SetFeedsServerSettingsL(const TFeedsServerSetting& aNewSetting);

        /**
        * Sets a Browser Control related settings.  These settings are directly passed to all 
        * Browser Control instances used by the CFeedsViewBridge.  As such see the Browser
        * Control documentation for infomation about the settings.
        *
        * @since 3.1
        * @param aSetting The setting to update
        * @param aValue The new value of the setting
        * @return void.
        */
        void SetBrowserControlSettingL(TUint aSetting, TUint aValue);
        
        /**
        * Called by RequestHandlerCompleted when the root FolderItem is either ready or an error occured.
        *
        * @since 3.1
        * @param aStatus The result code of the FetchRootFolderItemL method.
        * @param aTrans The type of transaction that was made
        * @return void.
        */
        void FolderItemRequestCompleted(TInt aStatus, CTransaction::TTransactionType aTrans=CTransaction::ENone);

        /**
        * Leaving method which is called by FolderItemRequestCompleted
        *
        * @since 3.2
        * @param aStatus The result code of the FetchRootFolderItemL method.
        * @param aTrans The type of transaction that was made
        * @return void.
        */
        void FolderItemRequestCompletedL(TInt aStatus, CTransaction::TTransactionType aTransType=CTransaction::ENone);

        /**
        * Called by RequestHandlerCompleted when the FolderItem has changed.  The client
        * should request an updated copy from the server.
        *
        * @since 3.1
        * @return void.
        */
        void FolderItemChanged() {};

        /**
        * Called by RequestHandlerCompleted when the asynchronous request is complete.
        *
        * @since 3.1
        * @pram aStatus The completion status code.
        * @return void.
        */
        void FeedRequestCompleted(TInt aStatus);
        
        /**
        * Leaving method which is called from FeedRequestCompleted
        *
        * @since 3.2
        * @pram aStatus The completion status code.
        * @return void.
        */
        void FeedRequestCompletedL(TInt aStatus);

        /**
        * Notifies the observer that the feed has changed.  This method is called
        * when the feed is updated by the feeds server.
        *
        * @since 3.1
        * @return void.
        */
        void FeedChanged() {};

        /**
        * Notifies the observer that the feed's item status has changed.  
        * This method is called when __another__ client updates the feed's 
        * item status (via UpdateFeedItemStatusL).
        *
        * @since 3.1
        * @return void.
        */
        void FeedItemStatusChanged() {};


    public:  // From MWaitDialogListener
        /**
        * Called when the user presses the cancel button.
        *
        * @since 3.0
        * @return void.
        */
        void DialogDismissedL();
        

    public:  // New Methods.
        /**
        * Searches for a FolderItem with the given name.  If "this"
        * isn't a folder it only checks whether or not it matches
        * the given name.  If "this" is a folder it also checks 
        * all embedded items
        *
        * @since 7.1
        * @param aName The title of the feed
        * @param aFolder The folder to be searched
        * @return void.
        */

        const CFeedsEntity* Search(const TDesC& aName,const CFeedsEntity& aFolder) const;
        /**
        * Searches for a FolderItem with the given id.  If "this"
        * isn't a folder it only checks whether or not it matches
        * the given id.  If "this" is a folder it also checks 
        * all embedded items
        *
        * @since 7.1
        * @param aFolderItemId The id of the folder item.
        * @param aFolder The folder to be searched
        * @return void.
        */

    	const CFeedsEntity* Search(TInt aFolderItemId,const CFeedsEntity& aFolder) const;

        /**
        * Get the root folder from the Feeds server.
        *
        * @since 3.1
        * @return void.
        */
        void FetchRootFolderL();
        
        /**
        * Add a new folder item.
        *
        * @since 3.1
        * @param aTitle The title of the item.
        * @param aUrl The url of the item.
        * @param aIsFolder True if this is a folder.
        * @param aParent The parent folder.
        * @param aFreq The frequency for auto updating.
        * @return void.
        */
        void AddFolderItemL(const TDesC& aTitle, const TDesC& aUrl,
                TBool aIsFolder, const CFeedsEntity& aParent, TInt aFreq);
        
        /**
        * Change the folder item.  If this is a folder then KNullDesC 
        * should be passed to the aUrl argument.
        *
        * @since 3.1
        * @param aFolderItem The item to update.
        * @param aTitle The item's title..
        * @param aUrl The item's url or KNullDesC.
        * @param aFreq The frequency for auto updating.
        * @return void
        */
        void ChangeFolderItemL(CFeedsEntity& aFolderItem, 
                const TDesC& aTitle, const TDesC& aUrl, TInt aFreq);

        /**
        * Delete the folder items. 
        *
        * @since 3.1
        * @param aFolderItem The folder item that's being deleted.
        * @return void
        */
        void DeleteFolderItemsL(RPointerArray<const CFeedsEntity>& aFolderItems);

        /**
        * Move the folder items to a different parent. 
        *
        * @since 3.1
        * @param aFolderItem The folder item that's being moved.
        * @param aParent The folder item's new parent.
        * @return void
        */
        void MoveFolderItemsToL(RPointerArray<const CFeedsEntity>& aFolderItems,
                const CFeedsEntity& aParent);

        /**
        * Move the folder item to a different index. 
        *
        * @since 3.1
        * @param aFolderItem The folder item that's being moved.
        * @param aIndex The new index of the folder item (within its parent).
        * @return void
        */
        void MoveFolderItemsL(RPointerArray<const CFeedsEntity>& aFolderItems,
                TInt aIndex);

        /**
        * Get the given feed from the Feeds server.
        *
        * @since 3.1
        * @param aFeedUrl The id of the feed to fetch.
        * @param aForceUpdate When true the server will ignore the cache.
        * @param aNoCache When true the server will not cache the feed.
        * @return void.
        */
        void FetchFeedL(const CFeedsEntity& aFeedEntity, TBool aForceUpdate = EFalse, 
                TBool aNoCache = EFalse);

        /**
        * Updates the feed with the given id.
        *
        * @since 3.1
        * @param aFeedId The id of the feed to update.
        * @return void.
        */
        void UpdateFeedL(const CFeedsEntity& aFeedEntity);
                                        
        /**
        * Updates the given feeds in the background.
        *
        * @since 3.1
        * @param aFolderItems The feeds to update.
        * @return void.
        */
        void UpdateFolderItemsL(const RPointerArray<const CFeedsEntity>& 
                aFolderItems);
                                        
        /**
        * Updates all of feeds in the background.
        *
        * @since 3.1
        * @return void.
        */
        void UpdateFolderItemsL();
                                        
		/**
		* Handle the menu "Import Feeds" command. This will kick off the search
		* for OMPL feeds files stored on the phone. Control will return via the 
		* FeedsFileSearchComplete callback.
		*
		* @since 3.2
		* @return void.
		*/
		void ImportFeedsL();
		
		/**
		* Given a filepath on the filesystem, begin import feeds from the file path
		* This will set up the path and flags and allow the feeds folder to be 
		* activated before the import actually takes place.
		*
		* @since 3.2
		* @return void
		* @param a filesystem path to import from
		*/
		void BeginImportOPMLFeedsFileL( TFileName& aFilepath );
	
		/**
		* Handle the menu "Export Feeds" command
		* once items have been selected and a name chosen.
		*
		* @since 3.2
		* @return void
		* @param folder items to export and a name to export to
		*/
		void ExportFeedsL(RPointerArray<const CFeedsEntity>& aFolderItems, const TDesC &aExportFileName);

        /**
        * Shows the folder view.
        *
        * @since 3.1
        * @return void.
        */
        void ShowFolderViewLocalL();

        /**
        * Shows the topic view.
        *
        * @since 3.1
        * @param aInitialItem The initial item to show.
        * @return void.
        */
        void ShowTopicViewLocalL(TInt aInitialItem);

        /**
        * Shows the feed view.
        *
        * @since 3.1
        * @param aInitialItem The initial item to show.
        * @return void.
        */
        void ShowFeedViewLocalL(TInt aInitialItem);


        /**
        * Returns the current root folder.
        *
        * @since 3.1
        * @return The current root folder..
        */
        const CFeedsEntity* CurrentRootFolder() ;        

        /**
        * Returns the current feed.
        *
        * @since 3.1
        * @return The current feed.
        */
        CFeedsEntity* CurrentFeed() ; 
                                    
        /**
        * Add a menu item to the given menu.
        *
        * @since 3.1
        * @param aMenuPane The menu to add the item to.
        * @param aCommand The item's command.
        * @param aTitleId The item's title resouce-id.
        * @return void.
        */
        void AddItemL(CEikMenuPane& aMenuPane, TInt aCommand, TInt aTitleId);

        /**
        * Add a sub-menu to the given menu.
        *
        * @since 3.1
        * @param aMenuPane The menu to add the item to.
        * @param aCommand The cascade's command.
        * @param aTitleId The item's title resouce-id.
        * @param aCascade The cascade's resource id.
        * @return void.
        */
        void AddCascadeL(CEikMenuPane& aMenuPane, TInt aCommand, 
                TInt aTitleId, TInt aCascade);
                
        /**
        * Sets the item's status (read/unread/new).
        *
        * @since 3.1
        * @param aItemIndex The index of the item to change.
        * @param aStatus The new status.
        * @return void.
        */
        void SetItemStatusL(CFeedsEntity* aItem, TFeedItemStatus aStatus);

        /**
        * Returns the item's status (read/unread/new).
        *
        * @since 3.1
        * @param aItemIndex The index of the item to change.
        * @return The status.
        */
        TFeedItemStatus ItemStatus(TInt aItemIndex);

        /**
        * Writes the item status out to the server.
        *
        * @since 3.1
        * @return Void.
        */
        void ItemStatusWriteToServerL();

        /**
        * Returns the Favicon to the given url or NULL if one isn't found.
        *
        * @since 3.1
        * @param aUrl The url of the Favicon.
        * @return The associated Favicon or NULL.
        */
        CGulIcon* FaviconL(const TDesC& aUrl);
        
        /**
        * Connect to the server.
        *
        * @since 3.1
        * @param aGetRootFolder If true the root folder is also fetched.
        * @return Void
        */
        void ConnectToServerL(TBool aGetRootFolder = ETrue);

        /**
        * Disconnect from the server.
        *
        * @since 3.1
        * @return Void
        */
        void DisconnectFromServer();

        /**
        * Since the folder list isn't updated after it is fetched the FeedsClientUtilities 
        * tracks the last update times for feeds the user visits in the current session.
        * This method returns the "last updated" timestamp of the given item.
        *
        * @since 3.1
        * @param aItem The folder item to get the last updated time.
        * @return The timestamp.
        */
        TTime FeedUpdatedTime(const CFeedsEntity& aItem);

        /**
        * Resets the "last updated" cache.
        *
        * @since 3.1
        * @return Void.
        */
        void ResetFeedUpdateTime();

        /**
        * Update the feed's "last updated" value in the cache.
        *
        * @since 3.1
        * @param aFeed The feed to udpate the last updated time.
        * @return Void.
        */
        void UpdateFeedUpdatedTimeL(const CFeedsEntity& aFeed);

        /**
        * Initialize context-sensitive menu.
        * @param aResourceId Resource id of the menu pane.
        * @param aMenuPane Menu pane object being initialized.
        */
		void DynInitMenuPaneL( TInt aResourceId, CEikMenuPane* aMenuPane );
		
		/**
        * Processes commands.
        *
        * @since 3.1
        * @param aViewId the view that calls this.
        * @param aCommand the command to process.
        * @return void.
        */
		void HandleCommandL(TUid aViewId, TInt aCommand);

        /**
        * Sets Feeds Server related settings.
        *
        * @since 3.1
        * @return void.
        */
        void DisconnectManualUpdateConnectionL();

        /**
        * Disconnect connection used by FeedsView
        *
        * @since 3.1
        * @return void.
        */
        void DisconnectFeedsViewL();
        
        /**
        * Set the View Id that called Feeds
        *
        * @since 3.1
        * @param View id of the view Feeds was called from.
        */
        void SetCalledFromView(TUid aViewId);

        /**
        * Get the View Id that called Feeds
        *
        * @since 3.1
        * @return the view id Feeds was called from.
        */
        TUid CalledFromView();
       
	public: // from MFeedsFileSearchAgentCallback
        /**
        * Callback to this class when the search agent CActive had 
        * completed its searching for feeds files
        *
        * @since 3.2
        * @return void
        * @param the number of files found
        */
		
		void FeedsFileSearchCompleteL(TInt aCount);

    private:  // New Methods.
        /**
        * C++ default constructor.  aName and 
        * aSourceUrl are adopted by this method.
        */
        CFeedsClientUtilities(CAknViewAppUi& aAppUI, MApiProvider& iApiProvider);
        
        /**
        * By default Symbian 2nd phase constructor is private.
        */
        void ConstructL();
        
        /**
        * Ensures that the views and the connection to the FeedsServer are ready.
        *
        * @since 3.1
        * @param aGetRootFolder If true the root folder is also fetched.
        * @return void.
        */
        void LazyInitL(TBool aGetRootFolder);
        
        /**
        * Show a server error.
        *
        * @since 3.1
        * @param aStatus The status code.
        * @param aRequestType The type of request that was made
        * @return void.
        */
        void ShowServerError(TInt aStatus, CTransaction::TTransactionType aRequestType = CTransaction::ENone);
        
        /**
        * Init a menu item
        * @since 3.1
        * @param aMenuPane Menu pane to be initialized        
        * @param aCommandId The command ID
        * @param aResourceId The resource ID
        * @param aCascadeId The cascade ID
        * @param aFlags Flags
        * @return void
        */    
        void InitMenuItemL( CEikMenuPane* aMenuPane,
                            TInt aCommandId,
                            TInt aResourceId, 
                            TInt aCascadeId = 0,
                            TInt aFlags = 0 );

        /**
        * Send the OPML using the Send UI
        *
        * @since 3.2
        * @param void.
        * @return void.
        */
		void SendOPMLFileL(void);
		
       /**
       * Returns the INITIAL status of each of the items in the feed.
       * The caller can then modify the values and call RFeed::UpdateFeedItemStatusL
       * to request the feeds server to update the feed's item status. 
       *
       * @since 7.1
       * @param aItemIds The ids of the items.
       * @param aItemStatus The coresponding status of each of the items in the feed.
       * @param aFeed The feed whose items' status has to fetched.
       * @return void.
       */
       void ItemStatusL(RArray<TInt>& aItemIds, 
                RArray<TFeedItemStatus>& aItemStatus, const CFeedsEntity& aFeed) const;

		
		
    private:  // Data.
    	CAknViewAppUi&				iAppUI;
    	MApiProvider&				iApiProvider;
        TUid						iNextViewId;
        TBool						iIsUpdatingFeed;      
        CFeedsWaitDialog*			iWaitDialog;
        HBufC*              		iEnteredURL;
        TInt                		iCbaResource;

        CFeedsFolderView* 			iFolderView;  // not owned
		CFeedsTopicView* 			iTopicView;  // not owned
		CFeedsFeedView* 			iFeedView;  // not owned
        
        TBool						iIsConnected;
        RArray<TInt>				iItemIds;
        RArray<TFeedItemStatus>		iItemStatus;
        RArray<TFeedItemStatus>		iItemStatusOrig;

        HBufC*						iMimeTypes;   
        
        RArray<TInt>				iFeedUpdateTimeIds;
        RArray<TTime>				iFeedUpdateTimeTimestamps;
        
        TDesC*						iPendingFolderItemTitle;
        TDesC*						iPendingFolderItemUrl;
        TDesC*						iExportFileName;        
        
		CFeedsFileSearchAgent*					iSearchAgent;
		CArrayFixFlat<TBrCtlSelectOptionData>* 	iSearchOptList;
		
		TFileName 					iTempOPMLImportFilePath;
		TBool						iFeedImportRequested;
		TBool						iRequestCanceled;
        TUid                        iCalledFromView;           
        TBool						iIsWaitDialogDisplayed;
        RFeedsInterface				iFeedsInterface;
        CTransaction::TTransactionType iCurrentRequest;
        CTransaction::TTransactionType iCanceledRequest;
    };

#endif      // FEEDS_CLIENT_UTILITIES_H
            
// End of File
