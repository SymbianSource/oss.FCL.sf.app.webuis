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
* Description:  Encapsulates a edit folder dialog.
*
*/


#ifndef FEEDS_EDIT_FOLDER_DIALOG_H
#define FEEDS_EDIT_FOLDER_DIALOG_H


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
*  The observer interface for the edit folder dialog.
*  @lib FeedsEngine.lib
*  @since 3.0
*/
class MFeedsEditFolderDialogObserver
    {
    public:
        /**
        * Called when a folder's name is changed -- this is called 
        * after IsValidFolderName is called.
        *
        * @since 3.0
        * @param aName The folder's new name.
        * @return void.
        */
        virtual void UpdateFolderL(const TDesC& aName) = 0;

        /**
        * Called when a new folder is created -- this is called 
        * after IsValidFolderName is called.
        *
        * @since 3.0
        * @param aName The folder's new name.
        * @return void.
        */
        virtual void NewFolderL(const TDesC& aName) = 0;

        /**
        * Validates the folder's updated name.
        *
        * @since 3.0
        * @param aName The folder's new name.
        * @param aIsEditing True if the edit dialog was opened to edit a folder.        
        * @return ETrue if the value is valid.
        */
        virtual TBool IsFolderNameValidL(const TDesC* aName, TBool aIsEditing) = 0;
    };


/**
*  Encapsulates a edit folder dialog.
*  @lib FeedsEngine.lib
*  @since 3.0
*/
class CFeedsEditFolderDialog: public CAknForm
	{
	public:
        /**
        * Two-phased constructor.
        */
		static CFeedsEditFolderDialog* NewL(MFeedsEditFolderDialogObserver& aObserver,
        		CAknViewAppUi* aAppUi,
                const TDesC& aName);

        /**
        * Two-phased constructor.
        */
		static CFeedsEditFolderDialog* NewL(MFeedsEditFolderDialogObserver& aObserver,
            CAknViewAppUi* aAppUi);

        /**
        * Destructor.
        */        
        virtual ~CFeedsEditFolderDialog();


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
        * Called by the framework whenver the 'Save' menu item is selected.
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
        * Called by the framework to process the options menu.
        *
        * @since ?
        * @param aCommandId  The command.
		* @return void.
        */
        virtual void ProcessCommandL(TInt aCommandId);


	private:  // New methods
        /**
        * C++ default constructor.
        */
		CFeedsEditFolderDialog(MFeedsEditFolderDialogObserver& aObserver,
                CAknViewAppUi* aAppUi,
                const TDesC& aName);

		/**
		* Loads the form data before the form is displayed.
        *
        * @since ?
		* @return ETrue if the form data has been saved, otherwise EFalse.
        */
		void LoadFormValuesFromDataL();


	private:
        MFeedsEditFolderDialogObserver&  iObserver;
        CAknViewAppUi*              iAppUi;   // not owned
        TBool                       iIsNewFolder;
        TBool                       iExitDialog;

        const TDesC&                iName;
	};

#endif      // FEEDS_EDIT_FOLDER_DIALOG_H
            
// End of File