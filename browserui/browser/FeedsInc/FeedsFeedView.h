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
* Description:  A view to browse a given feed.
*
*/


#ifndef FEED_VIEW_H
#define FEED_VIEW_H


// INCLUDES
#include "BrowserAppViewBase.h"
#include "FeedsFeedContainer.h"
#include <AknToolbarObserver.h>

// CONSTANTS

// MACROS

// DATA TYPES

// FUNCTION PROTOTYPES

// FORWARD DECLARATIONS
class MApiProvider;

// CLASS DECLARATION


/**
*  A view to browse a given feed.
*  @lib FeedsEngine.lib
*  @since 3.0
*/
class CFeedsFeedView: public CBrowserViewBase,
                      public MAknToolbarObserver
    {
    public:
        /**
        * Two-phased constructor.
        */
        static CFeedsFeedView* NewL( MApiProvider& aApiProvider,	TRect& aRect );
        
        /**
        * Destructor.
        */        
        virtual ~CFeedsFeedView();

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

    public:  // New Methods

        /**
        * Sets the initial item once the view is activated.
        *
        * @since 3.1
        * @param aItemIndex The index of the inital item.
        * @return void
        */
        void SetInitialItem(TInt aItemIndex);

        /**
        * Update the toolbar buttons' state
        *
        * @since 5.0
        * @return void
        */
        void UpdateToolbarButtonsState();
        
        /**
        * Disconnect Browser Control
        *
        * @since 3.1
        * @return void
        */
        void DisconnectL();
        
    private:  // New Methods
        /**
        * C++ default constructor.
        */
        CFeedsFeedView( MApiProvider& aApiProvider );
        
        /**
        * By default Symbian 2nd phase constructor is private.
        */
        void ConstructL(TRect& aRect);
        
        
    public:  // Friends
        friend class CFeedsFeedContainer;


    private:
		CFeedsFeedContainer*  iContainer;
        TBool            iContainerOnStack;
        TInt             iInitialItem;
        TBool            iPageScalerEnabled;
        TBool            iPenEnabled;
    };

#endif      // FEED_VIEW_H
            
// End of File

