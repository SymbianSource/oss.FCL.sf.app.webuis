/*
* Copyright (c) 2005-2008 Nokia Corporation and/or its subsidiary(-ies).
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
* Description:  A view to browse a user's list of feeds.
*
*/


#ifndef FEEDS_FOLDER_VIEW_H
#define FEEDS_FOLDER_VIEW_H


// INCLUDES
#include "BrowserAppViewBase.h"
#include "FeedsFolderContainer.h"
#include <AknToolbarObserver.h>

// CONSTANTS

// MACROS

// DATA TYPES

// FUNCTION PROTOTYPES

// FORWARD DECLARATIONS
class MApiProvider;

// CLASS DECLARATION

/**
*  A view to browse a user's list of feeds.
*  @lib FeedsEngine.lib
*  @since 3.0
*/
class CFeedsFolderView: public CBrowserViewBase,
                        public MAknToolbarObserver
    {
    public:
        /**
        * Two-phased constructor.
        */
        static CFeedsFolderView* NewL( MApiProvider& aApiProvider, TRect& aRect );
        
        /**
        * Destructor.
        */        
        virtual ~CFeedsFolderView();

	public:     // From CBrowserViewBase
        
        /**
        * Return command set id, to be displayed.
        * @return The command set's resource id.
        */
        TInt CommandSetResourceIdL();

		/**
		* Set command set lsk,rsk,msk dynamically via pointers.
		* Derived classes should implement, though it can be empty.
		* If it does nothing, empty softkeys will be assigned
		* @since 5.0
		*/
		void CommandSetResourceDynL(TSKPair& /*lsk*/, TSKPair& /*rsk*/, TSKPair& /*msk*/) {};
			        
        /**
        * @see CAknView
        */
		void HandleClientRectChange();

	public: // From CAknView
        /**
        * Returns the id of the view.
        *
        * @since ?
        * @return the id.
        */
		virtual TUid Id() const;

        /**
        * Processes commands.
        *
        * @since ?
        * @param aCommand the command to process.
        * @return void.
        */
		virtual void HandleCommandL(TInt aCommand);


    protected: // From CAknView
        /**
        * Called when the view is activated.
        *
        * @since ?
        * @param aPrevViewId the id of the prev view.
        * @param aCustomMessageId The activation message id.
        * @param aCustomMessage Activation params.
        * @return void.
        */
		virtual void DoActivateL(const TVwsViewId& aPrevViewId, TUid aCustomMessageId, 
                const TDesC8& aCustomMessage);
    
        /**
        * Called when the view is deactivated.
        *
        * @since ?
        * @return void.
        */
		virtual void DoDeactivate();

        /**
        * Disables unrelated menu options.
        *
        * @since ?
        * @param aResourceId  The resource id of the menu.
        * @param aMenuPane The menu.
        * @return void.
        */
		virtual void DynInitMenuPaneL(TInt aResourceId, CEikMenuPane* aMenuPane);

    public: // MAknToolbarObserver
        /**
        * Called when toolbar events occur
        *
        * @since 5.0
        * @return void
        */
        void OfferToolbarEventL( TInt aCommand ) { HandleCommandL(aCommand); }


    public: // New Methods
        /**
        * Updates the options softkey to reflect the command set.
        *
        * @since 3.0
        * @param aCommandSet The new "command set".
        * @return void.
        */
        void UpdateCbaL(TInt aCommandSet);

        
        /**
        * Called to notify the view the that root folder has changed.
        *
        * @since 3.1
        * @param aRootFolder The new root folder.
        * @return Void.
        */
        void RootFolderChangedL(const CFeedsEntity& aRootFolder);

        /**
        * Changes the current folder.
        *
        * @since 3.1
        * @param aFolder The new current folder.
        * @param aResetSelection If ETrue the current selection is set to the first item.
        * @return void.
        */
        void SetCurrentFolder(const CFeedsEntity& aFolder, TBool aResetSelection = EFalse);

        /**
        * Calculate the unread count based on delta.
        *
        * @since 3.1
        * @return the calculated value.
        */
        TInt UnreadCountChangedL( TInt aUnreadCountDelta );

        /**
        * Set the unread count.
        *
        * @since 3.1
        * @return void.
        */
        void SetUnreadCountL( TInt aUnreadCount );


    public:  // Friends
        friend class CFeedsFolderContainer;
        
        
    private:
        /**
        * C++ default constructor.
        */
        CFeedsFolderView( MApiProvider& aApiProvider );
        
        /**
        * By default Symbian 2nd phase constructor is private.
        */
        void ConstructL(TRect& aRect);

        /**
        * Check for any Marked Feeds Folders
        * @since 3.2
        * @return ETrue if any feeds folders are currently marked
        *  otherwise, return EFalse
        */        
        TBool AnyFoldersMarked();
        
        /**
        * Check for any Marked items in Feeds view
        * @since 3.2
        * @return ETrue anything is marked otherwise returns EFalse
        */        
        TBool AnyMarkedItems();
        
        /**
        * Check if current item is marked
        * @since 5.0
        * @return ETrue if current item is marked, otherwise returns EFalse
        */
        TBool IsCurrentItemMarked();



       
    private:
        const CFeedsEntity*  iRootFolder;
		CFeedsFolderContainer*   iContainer;
        TBool               iContainerOnStack;
        TInt                iInitialItem;
        TBool               iPenEnabled;
    };

#endif      // FEEDS_FOLDER_VIEW_H
            
// End of File


