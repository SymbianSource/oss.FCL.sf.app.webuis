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
*      Declaration of class CBrowserBookmarksGotoPane.
*      
*
*/


#ifndef BROWSER_BOOKMARKS_GOTO_PANE_H
#define BROWSER_BOOKMARKS_GOTO_PANE_H

// INCLUDE FILES

#include <coecntrl.h>
#include "BrowserAdaptiveListPopup.h"
// FORWARD DECLARATION

class CEikEdwin;
class CAknInputFrame;
class CBrowserBookmarksGotoPane;
class CAknsListBoxBackgroundControlContext;
class CAknsFrameBackgroundControlContext;
class CBrowserFavouritesView;

// class CBrowserAdaptiveListPopup;
// CLASS DECLARATION

/**
* Mixin class for handling Bookmark Goto Pane events.
*/
class MBookmarksGotoPaneObserver
    {

    public:     // types

        enum TEvent                 /// Goto Pane events.
            {
            EEventEditingModeChanged,   ///< Editing mode changed.
            EEventEnterKeyPressed       ///< Enter key pressed during editing.
            };

    public:     // new methods

        /**
        * Handle Goto Pane event. Derived classes must implement.
        * @param aGotoPane The Goto Pane in which the event occurred.
        * @param aEvent Goto Pane event.
        */
        virtual void HandleBookmarksGotoPaneEventL
            ( CBrowserBookmarksGotoPane* aGotoPane, TEvent aEvent ) = 0;
    };

/**
* Goto Pane for the WML Browser. It looks just like a search field.
* It has two modes, Editing and Non-editing. In editing mode the user can edit
* text and move the cursor. In non-editing mode any key/other event which
* normally means editing enters editing mode automatically. When editing mode
* is entered, existing text is replaced with a default text.
* (Editing in fact corresponds to being focused or not; the two things go
* together.)
*/
class CBrowserBookmarksGotoPane: public CCoeControl
    {
    public:     // Construct / destruct

        /**
        * Two-phased constructor. Leaves on failure.
        * @param aParent Parent control.
        * @param aIcon Icon to display. If NULL, a default is used.
        * @param aTextLimit Maximum length of text.
        * @return The constructed Goto Pane control.
        */
        static CBrowserBookmarksGotoPane* NewL
            ( const CCoeControl& aParent, CBrowserFavouritesView* aView );

        /**
        * Destructor.
        */
        virtual ~CBrowserBookmarksGotoPane();
        
        /**
         * SetupSkinContext Sets up the skin
         * return None
         */
        void SetupSkinContextL();

    public:     // new methods (editing)
    
        /**
        * Check if we are editing.
		* @since 1.2
        * @return ETrue if editing, EFalse otherwise.
        */
        inline TBool IsEditing() const;

		/**
		* Begin editing the pane. Sets up the visibility, etc for editing
		*/
		void BeginEditingL();


        /**
        * Cancel editing, but leave the text in place.
		* @since 1.2
        */
        void CancelEditingL();

        /**
        * Set text. If editing, it is now cancelled.
		* @since 1.2
        * @param aText Text to set.
        * @param aClipToFit If ETrue, text is clipped "..."  if doesn't fit.
		* @param aCancelEditing If EFalse, editing is not cancelled
        */
        void SetTextL( const TDesC& aText, TBool aClipToFit = ETrue, TBool aCancelEditing = ETrue );

        /**
        * Get text; ownership passed. The returned buffer is zero-terminated.
		* @since 1.2
        * @return Text in a buffer or NULL. Owner is the caller.
        */
        HBufC* GetTextL();

        /**
        * Freeze / unfreeze. If frozen, setting text by calling SetTextL or
        * Cancelling is disallowed. Use with caution, don't let frozen state
        * "stay in" due to some leave!
        * (This is used when during dowload we want to protect Goto Pane from
        * being updated if some notification kicks in).
		* @since 1.2
        * @param aFreeze ETrue for freeze, EFalse to unfreeze.
        */
        inline void Freeze( TBool aFreeze );

    public:     // observer support

        /**
        * Set observer. Pass NULL pointer to unset. Panics if already set.
		* @since 1.2
        * @param aObserver The observer to be notified about Goto Pane events.
        */
        void SetGPObserver( MBookmarksGotoPaneObserver* aObserver );

    public:     // Showing / hiding extra line

        /**
        * Set line state (show or hide line). Ugly stuff.
		* @since 1.2
        * @param aLineVisible ETrue for showing line, EFalse for hiding.
        */
        void SetLineState( TBool aLineVisible );

		void HandleResourceChange( TInt aType );
    public: // from CCoeControl

        /**
        * Handle key event.
        * @param aKeyEvent The key event.
        * @param aType Key event type.
        * @return Response (was the key event consumed?).
        */
        virtual TKeyResponse OfferKeyEventL
            ( const TKeyEvent& aKeyEvent, TEventCode aType );

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
        * Handle focus change.
        * @param aDrawNow Draw now?
        */
        virtual void FocusChanged( TDrawNow aDrawNow );

		/**
		* Gives back a pointer to adaptivepopuplist
		*/
		CBrowserAdaptiveListPopup* PopupList();		

       /**
        * Mop supply
        * @param TTypeUid aId
        */
        TTypeUid::Ptr MopSupplyObject( TTypeUid aId );

        /**
        * Returns editor control of goto pane.
        * @since Series60 1.2
        * @return editor control of goto pane
        */
        CEikEdwin* Editor() const;

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
        CBrowserBookmarksGotoPane(CBrowserFavouritesView* aView);

        /**
        * Second-phase constructor. Leaves on failure.
        * @param aParent Parent control.
        */
        void ConstructL
            ( const CCoeControl& aParent );

    protected:  // from CCoeControl

        /**
        * Handle size change, lay out components.
        */
        virtual void SizeChanged();

    private:    // new methods

        /**
        * Change editing mode.
        * @param aEditing ETrue for editing mode.
        */
        inline void SetEditingL( TBool aEditing );

        void ActivateVKB();

    private:    // data
		CBrowserFavouritesView* iView; //Not owned.
        /// Editor. Owned.
        CEikEdwin* iEditor;
        /// Input frame. Owned.
        CAknInputFrame* iInputFrame;
        /// Editing mode.
        TBool iEditing;
        /// Observer or NULL. Not owned.
        MBookmarksGotoPaneObserver* iGPObserver;
        /// ETrue if frozen.
        TBool iFrozen;
		/// ETrue if initial focusing was done 
		///(needed for permanent selection to be working)
		TBool iEverFocused;
        /// adaptive popuplist
        CBrowserAdaptiveListPopup* iBAdaptiveListPopup;
        CAknsListBoxBackgroundControlContext* iSkinContext;
        CAknsFrameBackgroundControlContext* iInputContext;
    };

#include "BrowserBookmarksGotoPane.inl"

#endif
// End of File
