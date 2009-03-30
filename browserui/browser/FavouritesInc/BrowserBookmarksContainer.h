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
*      Declaration of class CBrowserBookmarksContainer.
*      
*
*/


#ifndef BROWSER_BOOKMARKS_CONTAINER_H
#define BROWSER_BOOKMARKS_CONTAINER_H

// INCLUDE FILES

#include "BrowserFavouritesContainer.h"
#include <bldvariant.hrh>

// FORWARD DECLARATION

class CBrowserBookmarksGotoPane;

// CLASS DECLARATION

/**
* Top-level control, which contains all component controls in the bookmarks view.
*/
class CBrowserBookmarksContainer : public CBrowserFavouritesContainer
	{
	public :	// construction, destruction

        /**
        * Two-phased constructor. Leaves on failure.
        * @param aRect Rectangle of the control (available client rect).
        * @param aView The view to which this container belongs.
        * @return The constructed container.
        */
		static CBrowserBookmarksContainer* NewL(
            const TRect& aRect,
            CBrowserFavouritesView& aView);

        /**
        * Destructor.
        */
        virtual ~CBrowserBookmarksContainer();

    public:     // new methods

        /**
        * Activate Goto Mode
        */
        
        void SetGotoActiveL();
        
        /**
        * Deactivate Goto Mode
        */

		void SetGotoInactiveL();

    public:     // access to components

         /**
        * Get the Goto Pane.
        * @return The Goto Pane.
        */
        inline CBrowserBookmarksGotoPane* GotoPane() const;


        /**
        * Check if we are in Goto Mode
        * @return ETrue if we are in Goto Mode; EFalse otherwise.
        */
        inline TBool GotoPaneActive() const;
        
    public:	    // from CBrowserFavouritesContainer

        /**
        * Count component controls.
        * @return Number of component controls.
        */
		virtual TInt CountComponentControls() const;

        /**
        * Get a component control by index.
        * @param aIndex Index of component control to be returned.
        * @return Component control or NULL.
        */
		virtual CCoeControl* ComponentControl( TInt aIndex ) const;

        /**
        * Handle size change, lay out and draw components.
        */
		virtual void SizeChanged();

        /**
        * Handle key event.
        * @param aKeyEvent The key event.
        * @param aType Key event type.
        * @return Response (was the key event consumed?).
        */
		virtual TKeyResponse OfferKeyEventL( const TKeyEvent& aKeyEvent, TEventCode aType );

        /**
        * Handle cursor change.
        * @param aListBox Listbox in which the cursor has changed.
        */
        void HandleCursorChangedL( CEikListBox* aListBox );

        /**
        * Check if some editing is in progress. 
        */
        TBool IsEditing();

        /**
        * Get tab index belonging to this view.
        * @return The tab index.
        */
        TInt TabIndex();

        /**
        * Get resource id for text to be displayed in the empty listbox.
        * @return Listbox empty text resource id.
        */
        TInt ListboxEmptyTextResourceId();

         /**
        * Get resource id for text to be displayed in the title.
        * @return Listbox empty text resource id.
        */
        TInt TitleResourceId();
        
        /**
        * Handles focus changing
        * @param aDrawNow 
        */
        void FocusChanged( TDrawNow aDrawNow );


        /**
        * HandlePointerEventL
        * From CCoeControl
        *
        */
        void HandlePointerEventL(const TPointerEvent& /*aPointerEvent*/);

    protected:  // Construct / destruct

        /**
        * Constructor.
        */
        CBrowserBookmarksContainer();

        /**
        * Called by ConstructL. Leaves on failure.
        * Derived classes can override this to add more controls.
        * @param aRect Rectangle of the control (available client rect).
        * @param aView The view to which this container belongs.
        * @return The constructed container.
        */
		virtual void ConstructComponentControlsL(
            const TRect& aRect,
            CBrowserFavouritesView& aView );
        
	protected:  // from CBrowserFavouritesContainer

        /**
        * Create icon handler for the listbox.
        * @return Icon handler for the listbox.
        */
        MBrowserFavouritesListboxIconHandler* CreateListboxIconHandlerL();
#ifdef __SERIES60_HELP
	private:
        /**
        * Get help context for the control.
        * @param aContext The context that is filled in.
        * @return None.
        */
        void GetHelpContext( TCoeHelpContext& aContext ) const;

#endif //__SERIES60_HELP

    private:    // data

        /**
        * Goto Pane.
        */
        CBrowserBookmarksGotoPane* iGotoPane;
        
        
        /**
        * ETrue if Goto pane is active, EFalse by default
        */
        TBool iGotoPaneActive;
        
        TBool iSelectionKeyDownPressed;
        TBool iSelectionKeyLongPress;
	};

#include "BrowserBookmarksContainer.inl"

#endif

// End of file