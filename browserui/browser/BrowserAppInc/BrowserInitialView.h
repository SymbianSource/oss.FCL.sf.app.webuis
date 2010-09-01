/*
* Copyright (c) 2006 Nokia Corporation and/or its subsidiary(-ies).
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
*      Declaration of class CBrowserInitialView.
*      
*
*/


#ifndef BROWSERINITIALVIEW_H
#define BROWSERINITIALVIEW_H

// INCLUDES
#include <aknview.h>
#include "BrowserAppViewBase.h"
#include "BrowserGotoPane.h"

// FORWARD DECLARATIONS
class CBrowserInitialContainer;
class CBrowserBookmarksView;

// CLASS DECLARATION

/**
*  CBrowserInitialView view class.
*/
class CBrowserInitialView : public CBrowserViewBase,
                            public MGotoPaneObserver
    {
    public: // Constructors and destructor

        /**
        * Two-phased constructor. Leaves on failure. Places the instance
        * on the cleanup stack.
        * @param aApiProvider The API provider.
        * @return The constructed view.
        */
        static CBrowserInitialView* NewLC( MApiProvider& aApiProvider );

        /**
        * default constructor.
        */
        void ConstructL();

		/**
		* Constructor
		*/
		CBrowserInitialView( MApiProvider& aApiProvider);
		
        /**
        * Destructor.
        */
        ~CBrowserInitialView();

    public: // Functions from base classes
        
        /**
        * From CAknView returns Uid of View
        * @return TUid uid of the view
        */
        TUid Id() const;

        /**
        * From MEikMenuObserver delegate commands from the menu
        * @param aCommand a command emitted by the menu 
        * @return void
        */
        void HandleCommandL(TInt aCommand);

        /**
        * From CBrowserViewBase reaction if size change
        * @return void
        */
        void HandleClientRectChange();
        
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
        
    public: // Functions from base classes
    
    private:

        /**
        * From CAknView activate the view
        * @param aPrevViewId 
        * @param aCustomMessageId 
        * @param aCustomMessage 
        * @return void
        */
        void DoActivateL( const TVwsViewId& aPrevViewId,
                          TUid aCustomMessageId,
                          const TDesC8& aCustomMessage );

        /**
        * Launches the goto address editor.
        */
        void LaunchGotoAddressEditorL();

        /**
        * Handles GotoPane events
        */        
        void HandleGotoPaneEventL( CBrowserGotoPane* aGotoPane, TEvent aEvent );

        /**
        * Updates GotoPane
        */        
        void UpdateGotoPaneL();

        /**
        * Loads the typed url into content view.
        */        
        void GotoUrlInGotoPaneL();
        
        /**
        * Dynamically initialize menu panes.
        * @param aResourceId ID for the resource to be loaded
        * @param aMenuPane Menu pane
        */
        void DynInitMenuPaneL( TInt aResourceId, CEikMenuPane* aMenuPane );
        
        /**
        * From CAknView deactivate the view (free resources)
        * @return void
        */
        void DoDeactivate();

    public:
        
    private: // Data
        CBrowserInitialContainer* iContainer;
                
        // GotoPane`s text
        HBufC* iEnteredURL;
    };

#endif

// End of File
