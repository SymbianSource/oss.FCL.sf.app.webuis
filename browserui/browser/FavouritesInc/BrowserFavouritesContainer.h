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
*      Declaration of class CBrowserFavouritesContainer.
*      
*
*/


#ifndef BROWSER_FAVOURITES_CONTAINER_H
#define BROWSER_FAVOURITES_CONTAINER_H

// INCLUDE FILES
#include <coecntrl.h>
#include <akntabobserver.h>
#include "BrowserFavouritesListboxCursorObserver.h"
#include <aknnavidecoratorobserver.h>

// FORWARD DECLARATION
class CBrowserFavouritesView;
class CBrowserFavouritesListbox;
class CAknNavigationDecorator;
class CAknSearchField;
class MBrowserFavouritesListboxIconHandler;
class CAknsListBoxBackgroundControlContext;

// CLASS DECLARATION

/**
* Top-level control, which contains all component controls in the favourites
* view.
* Manages (destroys/creates) component controls.
* Handles key events and forwards them to the appropriate component control.
* Pure virtual.
*/
class CBrowserFavouritesContainer:
			public CCoeControl,
			public MBrowserFavouritesListboxCursorObserver,
            public MAknNaviDecoratorObserver
{
public :	// construction, destruction

    /**
    * Default destructor.
    */
	virtual ~CBrowserFavouritesContainer();

public:     // new methods


    /**
    * Show the tabs in the navi pane.
	* @since 1.2
    */
    void ShowRootNaviPane();

    /**
    * Show folder info in navi pane.
	* @since 1.2
    * @param aFolderIndex Index of the current folder.
    * @param aFolderCount Total number of folders.
    */
    void ShowFolderNaviPaneL( TInt aFolderIndex, TInt aFolderCount );

    /**
    * Get tab index belonging to this view.
    * Derived classes must provide this method.
	* @since 1.2
    * @return The tab index.
    */
    virtual TInt TabIndex() = 0;

    /**
    * Get resource id for text to be displayed in the empty listbox.
    * Derived classes must provide this method.
	* @since 1.2
    * @return Listbox empty text resource id.
    */
    virtual TInt ListboxEmptyTextResourceId() = 0;

    /**
    * Get resource id for text to be displayed in the title.
    * Derived classes must provide this method.
	* @since 1.2
    * @return Listbox empty text resource id.
    */
    virtual TInt TitleResourceId() = 0;


public:     // access to components

    /**
    * Get the creating view.
	* @since 1.2
    * @return The creating view.
    */
    inline CBrowserFavouritesView& View() const;

    /**
    * Get the browsing listbox.
	* @since 1.2
    * @return The browsing listbox.
    */
	inline CBrowserFavouritesListbox* Listbox() const;


public:	// from CCoeControl

	/**
    * Handle resource change.
    * @param aType Event type.
    */
    void HandleResourceChange( TInt aType );

    /**
    * Count component controls.
    * @return Number of component controls.
    */
	TInt CountComponentControls() const;

    /**
    * Get a component control by index.
    * @param aIndex Index of component control to be returned.
    * @return Component control or NULL.
    */
	CCoeControl* ComponentControl( TInt aIndex ) const;

    /**
    * Handle size change, lay out and draw components.
    */
	void SizeChanged();

    /**
    * Handle key event. Forwards all key events to the listbox, if present.
    * @param aKeyEvent The key event.
    * @param aType Key event type.
    * @return Response (was the key event consumed?).
    */
	TKeyResponse OfferKeyEventL
        ( const TKeyEvent& aKeyEvent, TEventCode aType );
    
    /**
    * Handles focus changing
    * @param aDrawNow
    */
    void FocusChanged( TDrawNow aDrawNow );

public:     // from MWmlBrowserFavouritesListboxCursorObserver

    /**
    * Handle cursor change.
    * @param aListBox Listbox in which the cursor has changed.
    */
    virtual void HandleCursorChangedL( CEikListBox* aListBox );       

public:  // from MAknNaviDecoratorObserver
    virtual void HandleNaviDecoratorEventL( TInt aEventID ); 

protected:  // construction

    /**
    * Called by ConstructL. Leaves on failure.
    * Derived classes can override this to add more controls.
    * @param aRect Rectangle of the control (available client rect).
    * @param aView The view to which this container belongs.
    */
	virtual void ConstructComponentControlsL
        (
        const TRect& aRect,
        CBrowserFavouritesView& aView
        );

    /**
    * Second phase constructor. Leaves on failure.
    * Do not override this method in derived classes. Use this one,
    * and override ConstructComponentControlsL.
    * @param aRect Rectangle of the control (available client rect).
    * @param aView The view to which this container belongs.
    */
	void ConstructL
        (
        const TRect& aRect,
        CBrowserFavouritesView& aView
        );

    /**
    * Called by ConstructComponentControlsL. Leaves on failure.
    * Derived classes must provide this to create own icon handler.
    * @return Icon handler for the listbox.
    */
    virtual MBrowserFavouritesListboxIconHandler*
        CreateListboxIconHandlerL() = 0;

private: // from MObjectProvider
TTypeUid::Ptr MopSupplyObject(TTypeUid aId);


private:    // data

    /// The creating view. Not owned.
    CBrowserFavouritesView* iView;
    /// Browsing listbox. Owned.
	CBrowserFavouritesListbox* iListbox;

    /// Navi Pane tabgroup. Owned.
    CAknNavigationDecorator* iNaviPaneTabsRoot;

    /// Navi Pane text. Owned.
	CAknNavigationDecorator* iNaviPaneTabsFolder;
    /// Icon handler. Owned.
    MBrowserFavouritesListboxIconHandler* iIconHandler;
	CAknsListBoxBackgroundControlContext* iSkinContext;

};

#include "BrowserFavouritesContainer.inl"

#endif

// End of file
