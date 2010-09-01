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
* Description:  
*      Declaration of class CWmlBrowserBookmarkEditDialog.
*      
*
*/


#ifndef BROWSER_BOOKMARKS_EDIT_DIALOG_H
#define BROWSER_BOOKMARKS_EDIT_DIALOG_H

// INCLUDE FILES

#include <AknForm.h>
// #include <favouriteslimits.h>
#include <bldvariant.hrh>

// FORWARD DECLARATION

class CFavouritesItem;
class CBrowserFavouritesModel;
class CBrowserBookmarksEditFormApModel;
class MApiProvider;
//class MCommsModel;
// class CAknsListBoxBackgroundControlContext;
class CAknNavigationControlContainer;


// CLASS DECLARATION

/**
* Dialog to edit and save bookmark attributes.
*/
class CBrowserBookmarkEditDialog: public CAknForm
    {
    public:     // construct / destruct

        /**
        * Two-phased constructor. Leaves on failure.
        * @param aItem Item to edit. Not owned.
        * @param aModel Model containing data (actual database handling).
        * @param aUid Uid of the edited item. When it is KFavouritesNullUid, the
        * item is new (not yet added to the database). Otherwise, it is the Uid
        * of the existing item.
        * @param aCommsModel Access Point model. Not owned.
        * @return The constructed dialog.
        */
        static CBrowserBookmarkEditDialog* NewL
            (
            CFavouritesItem& aItem,
            CBrowserFavouritesModel& aModel,
			MApiProvider& aApiProvider,
            //MCommsModel& aCommsModel,
            TInt aUid = KFavouritesNullUid
            );

        /**
        * Destructor.
        */
        virtual ~CBrowserBookmarkEditDialog();

    public:     // from CAknForm

        /**
        * Handle menu commands.
        * @param aCommandId Command to handle.
        */
    	void ProcessCommandL( TInt aCommandId );

    public:     // from base classes    	
    	void HandleResourceChange( TInt aType );

        void HandleResourceChangeL( TInt aType );


    protected:  // Construct / destruct

        /**
        * Constructor.
        * @param aItem Item to edit. Not owned.
        * @param aModel Model containing data (actual database handling).
        * handling.
        * @param aUid Uid of the edited item. When it is KFavouritesNullUid, the item
        * is new (not yet added to the database). Otherwise, it is the Uid
        * of the existing item.
        * @return The constructed dialog.
        */
        CBrowserBookmarkEditDialog
            (
            CFavouritesItem& aItem,
            CBrowserFavouritesModel& aModel,
            MApiProvider& aApiProvider,
			TInt aUid
            );

        /**
        * Second phase constructor. Leaves on failure.
        * @param aApModel Access Point model. Not owned.
        */
        void ConstructL( /*MCommsModel& aCommsModel*/ );

    protected:  // from CAknForm (CEikDialog)

        /**
        * Handle button press. Same as base class method, except displays note
        * "Bookmark saved" or "Not saved", as appropriate, if exiting.
        * @param aButtonId Button pressed.
        * @return ETrue if the dialog should exit.
        */
        TBool OkToExitL( TInt aButtonId );

    protected:  // From CAknForm
        /**
        * Get data from the editor back to the item and try to save it
        * to the database.
        * @return ETrue if successfully saved, EFalse otherwise.
        */
	    TBool SaveFormDataL() ;

        /**
        * Initialize context-sensitive menu.
        * @param aResourceId Resource id of the menu pane.
        * @param aMenuPane Menu pane object being initialized.
        */
        void DynInitMenuPaneL( TInt aResourceId, CEikMenuPane* aMenuPane ) ;

    private:    // From CAknForm

        /**
        * Fill the dialog controls with data from the item.
        */
	    void PreLayoutDynInitL();

    private:    // new methods

        /**
        * Fill the dialog controls with data from the item.
        */
        void LoadFormDataL();

        /**
        * Try to do the database update with the item.
        * @return ETrue if successfully saved, EFalse otherwise.
        */
        TBool SaveItemL();
        
        /**
        * Check if host part of url is too long.
        * Cant be over 255 characters.
        * @return ETrue if 255 or under, EFalse otherwise.
        */
        TBool IsValidUrlHost( const TDesC& aUrl );

        /**
        * Get the text from an edwin control. This method is a workaround for
        * an AVKON form bug; the form goes into read-only state BEFORE
        * SaveFormDataL is called. So the control there is no longer edwin.
        * *** Remove this method when forms are fixed. ***
        * @param aText Buffer to receive text.
        * @param aControlId Control id.
        */
        void GetEdwinText( TDes& aText, TInt aControlId );

#ifdef __SERIES60_HELP

        /**
        * Get help context for the control.
        * @param aContext The context that is filled in.
        * @return None.
        */
        void GetHelpContext( TCoeHelpContext& aContext ) const;
#endif // __SERIES60_HELP

		inline MApiProvider& ApiProvider() { return iApiProvider; }

    private:    // data
        
        CFavouritesItem* iItem;             ///< Item to edit. Not owned.
        CBrowserFavouritesModel* iModel; ///< Model containing data.
        TInt iUid;  ///< Uid of the item. KFavouritesNullUid for new items.
        TBool iExitDialog;  ///< ETrue if the dialog must be dismissed.
        CBrowserBookmarksEditFormApModel* iFormApModel;  ///< AP model.
        TInt iExitNoteResourceId;   ///< Resource id to show at exit, or 0.
        CAknNavigationControlContainer* iNaviPane;
        CAknNavigationDecorator* iNaviDecorator; //Owned
		MApiProvider& iApiProvider;
    };
 
#endif

// End of file