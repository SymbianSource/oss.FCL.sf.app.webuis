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
* Description: 
*      Declaration of class CBrowserWindowSelectionView.
*      
*
*/


#ifndef BROWSERWINDOWSELECTIONVIEW_H
#define BROWSERWINDOWSELECTIONVIEW_H

// INCLUDES
#include <aknview.h>
#include "BrowserAppViewBase.h"
#include "WindowInfoProvider.h"
#include "WindowObserver.h"

// FORWARD DECLARATIONS
class CBrowserWindowSelectionContainer;

// CLASS DECLARATION

/**
*  CBrowserWindowSelectionView view class.
* 
*/
class CBrowserWindowSelectionView : public CBrowserViewBase, 
                                           MWindowInfoProvider,
                                           MWindowObserver
    {
    public: // Constructors and destructor

        /**
        * Two-phased constructor. Leaves on failure. Places the instance
        * on the cleanup stack.
        * @param aApiProvider The API provider.
        * @return The constructed view.
        */
        static CBrowserWindowSelectionView* NewLC( MApiProvider& aApiProvider );

		/**
		* Constructor
		*/
		CBrowserWindowSelectionView( MApiProvider& aApiProvider );		

        /**
        * default constructor.
        */
        void ConstructL();

        /**
        * Destructor.
        */
        ~CBrowserWindowSelectionView();

    public: // Functions from base classes
        
        /**
        * From CAknView returns Uid of View
        * @return TUid uid of the view
        */
        TUid Id() const;
        
        /**
        * Return command set id, to be displayed.
		* @since 1.2
        * @return The command set's resource id.
        */
        virtual TInt CommandSetResourceIdL();

		/**
		* Set command set lsk,rsk,msk dynamically via pointers.
		* Derived classes should implement, though it can be empty.
		* If it does nothing, empty softkeys will be assigned
		* @since 5.0
		*/
		void CommandSetResourceDynL(TSKPair& /*lsk*/, TSKPair& /*rsk*/, TSKPair& /*msk*/) {};

        /**
        * From MEikMenuObserver delegate commands from the menu
        * @param aCommand a command emitted by the menu 
        * @return void
        */
        void HandleCommandL(TInt aCommand);

        /**
        * From CAknView reaction if size change
        * @return void
        */
        void HandleClientRectChange();
        
        /**
        * Changes the tabs according to available windows, by
        * recreating all tabs based on WindowManager.
        */                
        void UpdateTabsL();
        
        /**
        *
        */
        void UpdateTumbnailL();
        
        /**
        * Updates the title of the view according to the highlighted tab.
        */        
        void UpdateTitleL();
        
        /**
        * From MWindowObserver
        */
        void WindowEventHandlerL( TWindowEvent aEvent, TInt aWindowId );
        
        /**
        * @return Id of the window to activate.
        */                        
        TInt GetWindowIdFromTabIndex( TInt aActiveTabIndex );
        
    private:

        /**
        * From CAknView activate the view
        * @param aPrevViewId 
        * @param aCustomMessageId 
        * @param aCustomMessage 
        * @return void
        */
        void DoActivateL(const TVwsViewId& aPrevViewId,TUid aCustomMessageId,
            const TDesC8& aCustomMessage);

        /**
        * From CAknView deactivate the view (free resources)
        * @return void
        */
        void DoDeactivate();
        
        /**
        * From MWindowInfoProvider
        */
        HBufC* CreateWindowInfoLC( const CBrowserWindow& aWindow );
        
        /**
        * @return Pointer to the window`s title text.
        */
        HBufC* GetWindowTitleFromTabIndex( TInt aActiveTabIndex );
        
    private: // Data
        CBrowserWindowSelectionContainer* iContainer;
        CArrayFixFlat<CWindowInfo*>* iWindowInfo; // Array of all available windows
        CArrayFixFlat<TInt>* iWindowIds; // Array of window id`s
        CArrayFixFlat<HBufC*>* iTitles; // Array of window titles
        TVwsViewId iLastViewId; 
        TBool iWindowEventsDisabled; // False if the object not handles events.
    };

#endif

// End of File
