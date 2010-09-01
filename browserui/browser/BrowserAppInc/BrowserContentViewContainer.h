/*
* Copyright (c) 2002-2006 Nokia Corporation and/or its subsidiary(-ies).
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
*     Browser content view container
*
*
*/


#ifndef __BROWSERCONTENTVIEWCONTAINER_H
#define __BROWSERCONTENTVIEWCONTAINER_H

//  INCLUDES

#include <coecntrl.h>
#include "Browser.hrh"
#include <bldvariant.hrh>

#include <e32hashtab.h>


// CONSTANTS

// DATA TYPES

// FORWARD DECLARATIONS

class CBrowserGotoPane;
class CBrowserGotoPane;
class MApiProvider;
class CBrowserContentView;

/**
*  Container for Browser content view.
*
*  @lib Browser.app
*  @since Series 60 1.2
*/
class CBrowserContentViewContainer : public CCoeControl
    {
    public: // Constuctors and destructor

        static CBrowserContentViewContainer* NewL( CBrowserContentView* aView,
                                                   MApiProvider& aApiProvider );

        ~CBrowserContentViewContainer();

    public:     // New functions

        /**
        * Get pointer to Goto pane.
        * @since Series 60 1.2
        * @return Pointer to CBrowserGotoPane
        */
		CBrowserGotoPane* GotoPane() { return iGotoPane; } ;

        /**
        * Get pointer to Findkeyword pane.
        * @since Series 60 3.0
        * @return Pointer to CBrowserGotoPane
        */
		CBrowserGotoPane* FindKeywordPane() { return iFindKeywordPane; } ;

        /*
         * set the iGotoPane 
         */
        void SetGotoPane(CBrowserGotoPane* aGotoPane) { iGotoPane = aGotoPane; };

        /*
         * set the iFindKeywordPane 
         */
        void SetFindKeywordPane(CBrowserGotoPane* aFindKeywordPane) { iFindKeywordPane = aFindKeywordPane; };

        /**
        * Shut down Goto URL editor.
        * @since Series 60 1.2
        */
	    void  ShutDownGotoURLEditorL();
	    
        /**
        * Shut down Findkeyword pane.
        * @since Series 60 3.0
        */
    	void  ShutDownFindKeywordEditorL();


    private:

        /**
        * C++ default constructor.
        */
        CBrowserContentViewContainer( CBrowserContentView* aView,
                                      MApiProvider& aApiProvider );

        /**
        * Symbian 2nd phase constructor.
        */
        void ConstructL();


    private: // From CCoeControl

        /*
        * Handle pointer events
        */
        void HandlePointerEventL(const TPointerEvent& aPointerEvent);
        
        /**
        * Handle key events.
        */
        TKeyResponse OfferKeyEventL(const TKeyEvent& aKeyEvent,TEventCode aType);

        /**
        * Responds to focus changed.
        */
        void FocusChanged(TDrawNow aDrawNow);

        /**
        * Responds to size changes.
        */
        void SizeChanged();

        /**
        * Gets the number of controls contained in a compound control.
        */
        TInt CountComponentControls() const;

        /**
        * Gets the specified component of a compound control.
        */
        CCoeControl* ComponentControl(TInt aIndex) const;

    public:     // for dynamic layout switch updating from contentview

        void HandleResourceChange( TInt aType );

    private:    // New functions

#ifdef __SERIES60_HELP

        /**
        * Get help context for the control.
        * @param aContext The context that is filled in.
        * @return None.
        */
        void GetHelpContext( TCoeHelpContext& aContext ) const;

#endif // __SERIES60_HELP


        /**
        * For Configurable short cut keys: create HashTable to map key to function
        */
		void CreateShortCutFuncsHashTable();

        /**
        * For Configurable short cut keys: insert key/value pair
        */
		void InsertFuncToHashTable(HBufC* aKeyStr, TInt aFunc);

        /**
        * For Configurable short cut keys: convert string to TUint
        */
		TUint MyAtoi(TPtrC aData);

        /**
        * For Configurable short cut keys: invoke short cut function for key pressed 
        */
		TKeyResponse InvokeFunction(TUint aCode);


    private:
		CBrowserGotoPane* iFindKeywordPane;
   		CBrowserContentView* iView;
        MApiProvider& iApiProvider;
		CBrowserGotoPane* iGotoPane;

        TBool iSelectionKeyPressed;

		// True if the key was pressed long, otherwise key was pressed short.
    	TBool iIsKeyLongPressed;
		// True if the key was pressed, and the key was not consumed by the engine.
    	TBool iIsShortPressAllowed;


		// For shout cut functions
		// True if the hash table for short cut functions has been created; False otherwise.
    	TBool iShortCutFuncsReady;

		// maps iCode to shortcut function
		typedef RHashMap<TUint, TInt> RShortCutFuncMap;    
		RShortCutFuncMap iShortCutFuncMap;


    };

#endif

// End of File
