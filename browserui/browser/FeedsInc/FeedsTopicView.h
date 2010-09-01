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
* Description:  A view to browse a feed's topics.
*
*/


#ifndef FEEDS_TOPIC_VIEW_H
#define FEEDS_TOPIC_VIEW_H


// INCLUDES
#include "BrowserAppViewBase.h"

#include "FeedsTopicContainer.h"
#include <AknToolbarObserver.h>

// CONSTANTS

// MACROS

// DATA TYPES

// FUNCTION PROTOTYPES

// FORWARD DECLARATIONS
class MApiProvider;

// CLASS DECLARATION


/**
*  A view to browse a feed's topics.
*  @lib FeedsEngine.lib
*  @since 3.0
*/
class CFeedsTopicView: public CBrowserViewBase,
                       public MAknToolbarObserver
    {
    public:
        /**
        * Two-phased constructor.
        */
        static CFeedsTopicView* NewL( MApiProvider& aApiProvider,	TRect& aRect );
        
        /**
        * Destructor.
        */        
        virtual ~CFeedsTopicView();

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
        * Sets the current feed
        *
        * @since 3.0
        * @param aFeed The new feed.
        * @param aIndex The inital items to select.
        * @return Void
        */
        void SetCurrentFeedL(const CFeedsEntity& aFeed, TInt aIndex);

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
        
    private:  // New Methods
        /**
        * C++ default constructor.
        */
        CFeedsTopicView( MApiProvider& aApiProvider );
        
        /**
        * By default Symbian 2nd phase constructor is private.
        */
        void ConstructL(TRect& aRect);


    public:  // Friends
        friend class CTopicContainer;
        
        
    private:
		CFeedsTopicContainer*  iContainer;
        TBool             iContainerOnStack;
        TInt              iInitialItem;
        TBool             iPenEnabled;
    };

#endif      // FEEDS_TOPIC_VIEW_H
            
// End of File

