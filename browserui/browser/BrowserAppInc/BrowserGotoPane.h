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
* Description:  Browser Goto pane.
*
*/


#ifndef WML_GOTO_PANE_BASE_H
#define WML_GOTO_PANE_BASE_H

// DEFINES
#ifndef GOTOPANE_POPUPLIST_DISABLE
#define GOTOPANE_POPUPLIST_DISABLE EFalse
#endif

#ifndef GOTOPANE_POPUPLIST_ENABLE
#define GOTOPANE_POPUPLIST_ENABLE ETrue
#endif

// INCLUDE FILES
#include <coecntrl.h>
#include <eikedwin.h>
#include <Avkon.mbg>

// FORWARD DECLARATION
class CAknInputFrame;
class CBrowserGotoPane;
class CBrowserAdaptiveListPopup;
class CBrowserContentView;

// CLASS DECLARATION
/**
* Mixin class for handling Goto Pane events.
*  
* @lib Browser.app
* @since Series60 1.2
*/
class MGotoPaneObserver
    {

    public:     // types

        enum TEvent                 /// Goto Pane events.
            {
            EEventEnterKeyPressed   ///< Enter (OK) key pressed.
            };

    public:     // new methods

        /**
        * Handle Goto Pane event. Derived classes must implement.
        * @since Series60 1.2
        * @param aGotoPane The Goto Pane in which the event occurred
        * @param aEvent Goto Pane event
        */
        virtual void HandleGotoPaneEventL( CBrowserGotoPane* aGotoPane, TEvent aEvent ) = 0;
    };


/**
* Key event handled. Has higher priority than FEP,
* to be able to catch up/down key event and forward
* them to goto pane's editor.
*
* @lib Browser.app
* @since Series 60 1.2        
*/
class CBrowserKeyEventHandled : public CCoeControl, public MEikEdwinObserver
    {
    public:
        
        /**
        * public constructor
        * @param aGotoPane pointer to goto pane that key events are controled
        */
        CBrowserKeyEventHandled( CBrowserGotoPane& aGotoPane );

        /**
        * Catches key events before FEP could do it.
        * @since Series60 1.2
        * @param aKeyEvent
        * @param aType
        * @return
        */
        virtual TKeyResponse OfferKeyEventL(const TKeyEvent& aKeyEvent,TEventCode aType);

        /**
        * Catches EEventTextUpdate event.
        * @since Series60 1.2
        * @param aKeyEvent
        * @param aType
        * @return
        */
        virtual void HandleEdwinEventL(CEikEdwin* aEdwin,TEdwinEvent aEventType);

        /**
        * Put control to control stack or remove it from there.
        * @since Series60 1.2
        * @param aEnable ETrue puts control to the control stack with higher 
        * priority than FEP has. EFalse removes control from the stack.
        */
        void EnableL( TBool aEnable );

        /**
        * Set iFirstKeyEvent to TRUE.
        * @since Series60 1.2
        */
        void Reset();

        /**
        * Get LeftRightEvent.
        * @since Series60 1.2
        */

        TBool IsLeftRightEvent() { return iLeftRightEvent; }

        /**
        * SetFirstKey.
        * @since Series60 3.2
        */
        void SetFirstKeyEvent(TBool aFirstKeyEvent) {iFirstKeyEvent = aFirstKeyEvent;}


    protected:
        
        // key event catching enable flag
        TBool   iEnabled;   
        
        // only first key event is catched
        TBool   iFirstKeyEvent; 
        
        //
        TBool iLeftRightEvent;
        
        // goto pane control of which key events are catched
        CBrowserGotoPane& iGotoPane;                                        
    };

/**
* Goto Pane Base for the WML Browser & Bookmarks. 
* It looks just like a search field.
* 
* @lib Browser.app
* @since Series 60 1.2
*/
class CBrowserGotoPane:
        public CCoeControl, public MCoeControlObserver, public MCoeFepObserver
    {
    public:
        /**
        * Two-phased constructor. Leaves on failure.
        * @since Series60 1.2
        * @param aParent Parent control
        * @param aIconBitmapId Id of the icon to display
        * @param aIconMaskId Id of the icon`s mask
        * @param aPopupListStatus True if the pane uses popuplist,
        *                         otherwise false.
        * @param aContentView Pointer to content view
        * @param aFindKeywordMode Set true if the pane is a find keyword pane.
        * @return The constructed Goto Pane control
        */
        static CBrowserGotoPane* NewL
              ( 
              const CCoeControl* aParent, 
              TInt aIconBitmapId = EMbmAvkonQgn_indi_find_goto, 
              TInt aIconMaskId  = EMbmAvkonQgn_indi_find_goto_mask, 
              TBool aPopupListStatus = GOTOPANE_POPUPLIST_ENABLE,
              CBrowserContentView* aContentView = NULL,
              TBool aFindKeywordMode = EFalse
              );

        /**
        * Destructor.
        */
        virtual ~CBrowserGotoPane();

        /**
        * Makes goto pane (un)visible.
        * @since Series60 1.2
        * @param ETrue to set visible
        */
        virtual void MakeVisibleL( TBool aVisible );

        /**
        * Non-leaving version of MakeVisibleL.
        * @since Series60 1.2
        * @param ETrue to set visible
        */
		virtual void MakeVisible( TBool aVisible );

        /**
        * See description in calslbs.h.
        */
        void HandleFindSizeChanged();

        /**
        * Enable key event handler.
        * @since Series60 1.2
        * @param aEnable TRUE to enable
        */
        void EnableKeyEventHandlerL( TBool aEnable );

        /**
        * Activate GoTo Pane.
        */
        void SetGotoPaneActiveL();

        /**
        * Activate Search Pane.
        */
        void SetSearchPaneActiveL();
        
        /**
        * Check if we are in Goto Mode
        * @return ETrue if we are in Goto Mode; EFalse otherwise.
        */
        inline TBool GotoPaneActive() const { return iGotoPaneActive; }
        
        /**
        * Check if we are in Search Mode
        * @return ETrue if we are in Search Mode; EFalse otherwise.
        */
        inline TBool SearchPaneActive() const { return iSearchPaneActive; }
        
    public:	// from MCoeControlObserver

        /**
        * Handle control event.
        * @since Series60 1.2
        * @param aControl The control
        * @param aEventType Event type
        */
	    void HandleControlEventL
            ( CCoeControl* aControl, TCoeEvent aEventType );

    public: // from CCoeControl

        /**
        * Handle key event.
        * @since Series60 1.2
        * @param aKeyEvent The key event
        * @param aType Key event type
        * @return Response (was the key event consumed?)
        */
        virtual TKeyResponse OfferKeyEventL
            ( const TKeyEvent& aKeyEvent, TEventCode aType );

        /**
        * HandlePointerEventL
        * From CCoeControl
        *
        */
        void HandlePointerEventL(const TPointerEvent& /*aPointerEvent*/);
        
        /**
        * HandleResourceChange
        * From CCoeControl
        */
        void HandleResourceChange(TInt aType);
        

    public:     // from CCoeControl

        /**
        * Count component controls.
        * @since Series60 1.2
        * @return Number of component controls
        */
        virtual TInt CountComponentControls() const;

        /**
        * Get a component control by index.
        * @since Series60 1.2
        * @param aIndex Index of component control to be returned
        * @return Component control or NULL
        */
        virtual CCoeControl* ComponentControl( TInt aIndex ) const;

        /**
        * Sets the OrdinalPosition of GotoPane
        * @param aPos The OrdinalPosition
        */
        void SetOrdinalPosition( TInt aPos );

    public:     // Text manipulation

        /**
        * Length of the Search text in the editor.
        * @return The text length
        */
        TInt SearchTextLength() const;

        /**
        * Length of the text in the editor.
        * @since Series60 1.2
        * @return The text length
        */
        TInt TextLength() const;

        /**
        * Get text from the editor.
        * @since Series60 1.2
        * @return text from the editor
        */
        HBufC* GetTextL() const;

        /**
        * Set text.
        * @since Series60 1.2
        * @param aTxt Text to set
        * appended if needed.
        */
        virtual void SetTextL( const TDesC& aTxt );

        /**
        * Set Editor Font Posture to Italic.
        */
        void SetTextModeItalicL( );
        
        /**
        * Set Search text.
        * @param aTxt Text to set
        * appended if needed.
        */
        void SetSearchTextL( const TDesC& aTxt );
        
        /**
        * Selects all text in the editor.
        * See CEikEdwin's SelectAllL();
        * @since Series60 1.2
        */
        void SelectAllL();

        /**
        * Set info text.
        * @since Series60 1.2
        * @param aText Text to be displayed as info
        */
        void SetInfoTextL( const TDesC& aText );

        /**
        * Perform clipboard functionality.
        * @since Series60 1.2
        * @param aClipboardFunc Function to perform
        */
        void ClipboardL( CEikEdwin::TClipboardFunc aClipboardFunc );

        /**
        * Returns editor control of goto pane.
        * @since Series60 1.2
        * @return editor control of goto pane
        */
        CEikEdwin* Editor() const;

    public:     // observer support

        /**
        * Set observer. Pass NULL pointer to unset. Panics if already set.
        * @since Series60 1.2
        */
        void SetGPObserver( MGotoPaneObserver* aObserver );

         /**
        * Gives back a pointer to adaptivepopuplist
        */
        CBrowserAdaptiveListPopup* PopupList();
		
         /**
        * Handles the completion of a FEP transaction
        */		
		void HandleCompletionOfTransactionL();

         /**
        * Handles the start of a FEP transaction
        */		
		void HandleStartOfTransactionL();		

    protected:  // Construct / destruct
        /**
        * Constructor.
        * @param 
        */
        CBrowserGotoPane( CBrowserContentView* aContentView, TBool aFindKeywordMode );

        /**
        * Second-phase constructor. Leaves on failure.
        * @param aParent Parent control
        */
        void ConstructL( const CCoeControl* aParent, TInt aIconBitmapId, TInt aIconMaskId, TBool aPopupListStatus );

        /**
        * Constructs Search Pane.
        */
        void ConstructSearchPaneL();
        
    protected:  // from CCoeControl

        /**
        * Handle size change, lay out and draw components.
        */
        virtual void SizeChanged();

        /**
        * Handle focus change.
        * @since Series60 1.2
        * @param aDrawNow Draw now?
        */
        virtual void FocusChanged( TDrawNow aDrawNow );

    private:

        /**
        * Set the text and redraw.
        * @since Series60 1.2
        */
        void DoSetTextL( const TDesC& aTxt );
        
        void ActivateVKB();


    protected:    // data
    
		CBrowserContentView* iContentView; //Not owned.
		
		// If the control is in find keyword pane mode.
		TBool iFindKeywordMode;
		
		// HandleFEPFind
		TBool iHandleFEPFind;
		
		// The previously typed text into iEditor
		HBufC* iPrevKeyword;
					
        // Editor. Owned.
        CEikEdwin* iEditor;  
        
        // Input frame. Owned.
        CAknInputFrame* iInputFrame;
        
        // Search Editor. Owned.
        CEikEdwin* iSearchEditor;  
        
        // Search Input frame. Owned.
        CAknInputFrame* iSearchInputFrame;
                
        // Observer (may be NULL). Not owned.
        MGotoPaneObserver* iGPObserver;
		
        
        CBrowserKeyEventHandled *iGotoKeyHandled;

        /// adaptive popuplist
        CBrowserAdaptiveListPopup* iBAdaptiveListPopup;
        
        // Represents Active Editors 
        TBool iGotoPaneActive;
        TBool iSearchPaneActive;
        
        // Default text in Search Editor.Owned.
        HBufC* iDefaultSearchText;
        
        // Search Input Frame Icon Id, used for tracking the Search Icon Changes.
        TInt iSearchIconId;
        
        // Stores the Search Icon File Path.
        TFileName iSearchIconFilePath;
    };

#endif
