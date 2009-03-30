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
* Description:  Encapsulates a edit feed dialog.
*
*/


#ifndef FEEDS_EDIT_FEED_DIALOG_H
#define FEEDS_EDIT_FEED_DIALOG_H


// INCLUDES
#include <aknform.h>

// CONSTANTS

// MACROS

// DATA TYPES

// FUNCTION PROTOTYPES

// FORWARD DECLARATIONS
class CAknViewAppUi;

// CLASS DECLARATION


/**
*  Encapsulates a edit feed dialog.
*  @lib FeedsEngine.lib
*  @since 3.0
*/
class MFeedsEditFeedDialogObserver
    {
    public:
        /**
        * Called when a feed's name and/or url is changed -- this is called 
        * after both IsValidFeedName and IsValidFeedUrl are called.
        *
        * @since 3.0
        * @param aName The feed's new name.
        * @param aUrl The feed's new url.
        * @param aFreq The frequency for auto updating.
        * @return void.
        */
        virtual void UpdateFeedL(const TDesC& aName, const TDesC& aUrl, TInt aFreq) = 0;

        /**
        * Called when a new feed is created -- this is called after both
        * IsValidFeedName and IsValidFeedUrl are called.
        *
        * @since 3.0
        * @param aName The feed's new name.
        * @param aUrl The feed's new url.
        * @param aFreq The frequency for auto updating.
        * @return void.
        */
        virtual void NewFeedL(const TDesC& aName, const TDesC& aUrl, TInt aFreq) = 0;

        /**
        * Validates the Feed's updated name.
        *
        * @since 3.0
        * @param aName The feed's new name.
        * @param aIsEditing True if the edit dialog was opened to edit a feed.        
        * @return ETrue if the value is valid.
        */
        virtual TBool IsFeedNameValidL(const TDesC* aName, TBool aIsEditing) = 0;

        /**
        * Validates the Feed's updated url.
        *
        * @since 3.0
        * @param aUrl The feed's new url.
        * @return ETrue if the value is valid.
        */
        virtual TBool IsFeedUrlValidL(const TDesC* aUrl) = 0;
    };


class CFeedsEditFeedDialog: public CAknForm
	{
	public:
        /**
        * Two-phased constructor.
        */
		static CFeedsEditFeedDialog* NewL(MFeedsEditFeedDialogObserver& aObserver,
        		CAknViewAppUi* aAppUi,
                const TDesC& aName, const TDesC& aUrl, TInt aFreq);

        /**
        * Two-phased constructor.
        */
		static CFeedsEditFeedDialog* NewL(MFeedsEditFeedDialogObserver& aObserver,
            CAknViewAppUi* aAppUi);

        /**
        * Destructor.
        */        
        virtual ~CFeedsEditFeedDialog();

    public:

        /**
        * Takes any action required when the current line is changed
        * to aControlId
        * 
        * @since 7.1
        * @param aControlId The control ID
        * @return void
        */
        void LineChangedL( TInt aControlId );

        /**
        * Handles the auto update freuency modification commands
        *
        * @since 7.1
        * @return void
        */
        void HandleFreqCmdL();

        /**
        * Pops up a custom dialog to allow user to set setting items
        *
        * @since 7.1
        * @param aTitle The title of the field.
        * @param aValues The values of the radio buttons.
        * @param aCurentItem The currently selected item
        * @return TInt The newly selected value from the radio buttons.
        */
        TInt ShowRadioButtonSettingPageL(TDesC& aTitle,
                                            CDesCArrayFlat* aValues,
                                            TInt aCurrentItem );

        /**
        * Shows an information dialog, with an ok soft key.
        * 
        * @since 7.1
        * @param aNoteText Text to display.
        * @return void
        */
        void ShowInfoDialogwithOkSoftKeyL( const TDesC& aNoteText );


	public:  // From CAknForm
        /**
        * Called to setup the options menu with the dialog is active.
        *
        * @since ?
        * @param aResourceId  The resource id of the menu.
        * @param aMenuPane The menu.
        * @return void.
        */
		virtual void DynInitMenuPaneL(TInt aResourceId, CEikMenuPane* aMenuPane);

        /**
        * Called by the framework whenever the 'Save' menu item is selected.
        *
        * @since ?
		* @return ETrue if the form data has been saved, otherwise EFalse.
        */
		virtual TBool SaveFormDataL(); 

        /**
        * Called by the framework before the form is initialised.
        *
        * @since ?
		* @return void.
        */
		virtual void PreLayoutDynInitL();

        /**
        * Post-layout dialog initialisation.
        * 
        * @since 7.1
        * @return void.
        */
        void PostLayoutDynInitL();

        /**
        * From CCoeControl.     
        * Handles pointer events
        *
        * @since 7.1
        * @param Details of pointer event.
        * @return void.
        */
        void HandlePointerEventL(const TPointerEvent& aPointerEvent);		

        /**
        * Handle button press. Same as base class method, except calling
        * HandleFreqCmdL in the case of auto Update modifications.
        *
        * @since 7.1
        * @param aButtonId Button pressed.
        * @return ETrue if the dialog should exit.
        */
        TBool OkToExitL( TInt aButtonId  );


        /**
        * Called by the framework to process the options menu.
        *
        * @since ?
        * @param aCommandId  The command.
        * @return void.
        */
        virtual void ProcessCommandL(TInt aCommandId);
        
        /**
        * Handle key events.
        *
        * @since 7.1
        * @param aKeyEvent Details of key event.
        * @param aType Type of key event.
        * @return EKeyWasConsumed if the key was handled else EKeyWasNotConsumed.
        */
        TKeyResponse OfferKeyEventL(const TKeyEvent& aKeyEvent,
                                                TEventCode aType);

#ifdef __SERIES60_HELP
        /**
        * Get help context for the control.
        *
        * @since ?
        * @param aContext The context that is filled in.
        * @return None.
        */
        virtual void GetHelpContext(TCoeHelpContext& aContext) const;
#endif // __SERIES60_HELP


	private:  // New methods
        /**
        * C++ default constructor.
        */
		CFeedsEditFeedDialog (MFeedsEditFeedDialogObserver& aObserver,
                CAknViewAppUi* aAppUi,
                const TDesC& aName, const TDesC& aUrl, TInt aFreq);

		/**
		* Loads the form data before the form is displayed.
        *
        * @since ?
		* @return ETrue if the form data has been saved, otherwise EFalse.
        */
		void LoadFormValuesFromDataL();


	private:
        MFeedsEditFeedDialogObserver&  iObserver;
        CAknViewAppUi*            iAppUi;   // not owned
        TBool                     iIsNewFeed;
        TBool                     iExitDialog;

        const TDesC&              iName;
        const TDesC&              iUrl;
        TInt                      iFreq;
        TInt                      iPreviousFreq;
        TInt                      iSelectedDlgLine;
        TBool                     iActionCancelled;
	};

#endif      // FEEDS_EDIT_FEED_DIALOG_H
            
// End of File