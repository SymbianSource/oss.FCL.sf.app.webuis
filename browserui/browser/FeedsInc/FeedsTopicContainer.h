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
* Description:  A container to browse a feed's topics.
*
*/


#ifndef FEEDS_TOPIC_CONTAINER_H
#define FEEDS_TOPIC_CONTAINER_H


// INCLUDES
#include <coecntrl.h>
#include <coemop.h>
#include <eiklbo.h>

// CONSTANTS

// MACROS

// DATA TYPES

// FUNCTION PROTOTYPES

// FORWARD DECLARATIONS
class CAknSingleGraphicStyleListBox;
class CAknNavigationDecorator;
class CItem;
class CFeedsEntity;
class CFeedsTopicView;
class MApiProvider;

// CLASS DECLARATION


/**
*  A container to browse a feed's topics.
*  @lib FeedsEngine.lib
*  @since 3.0
*/
class CFeedsTopicContainer: public CCoeControl, public MEikListBoxObserver
    {
    public:
        /**
        * Two-phased constructor.
        */
        static CFeedsTopicContainer* NewL(
        	CFeedsTopicView* aView,
            MApiProvider& aApiProvider,
            const TRect& aRect);

        /**
        * Destructor.
        */        
        virtual ~CFeedsTopicContainer();


    public: // From CoeControl
	    /**
	    * Handles key event.
	    *
        * @param aKeyEvent The key event.
        * @param aType The type of the event.
        * @return Indicates whether the key event was used by this control or not
        */
        virtual TKeyResponse OfferKeyEventL(const TKeyEvent& aKeyEvent, TEventCode aType);

#ifdef __SERIES60_HELP
        /**
        * Get help context for the control.
        *
        * @since ?
        * @param aContext The context that is filled in.
        * @return None.
        */
        void virtual GetHelpContext(TCoeHelpContext& aContext) const;
#endif // __SERIES60_HELP


    protected: // From CoeControl
        /**
        * Called by framework when the view size is changed.
        *
        * @since ?
        * @return void.
        */
        virtual void SizeChanged();

        /**
        * Called by the framework when a display resource changes (i.e. skin or layout).
        *
        * @since ?
        * @return void.
        */
        virtual void HandleResourceChange(TInt aType);

        /**
        * Returns number of components.
	    *
        * @since ?
        * @return Number of component controls.
        */
        virtual TInt CountComponentControls() const;

        /**
        * Returns pointer to particular component.
	    *
        * @since ?
        * @param aIndex Index whose control's pointer has to returned.
        * @return Pointer to component control
        */
        virtual CCoeControl* ComponentControl(TInt aIndex) const;


    public: // MEikListBoxObserver
        /**
        * Processes key events from the listbox.
        *
        * @since ?
        * @param aListBox Listbox being observed.
        * @param aEventType Event observed.
        * @return void
        */
        virtual void HandleListBoxEventL(CEikListBox* aListBox, TListBoxEvent aEventType);


    public:  // New methods.
        /**
        * Sets the current feed
        *
        * @since 3.0
        * @param aFeed The new feed.
        * @param aIndex The inital items to select.
        * @return Void
        */
        void SetCurrentFeedL(const CFeedsEntity& aFeed, TInt aIndex);

        /**
        * Returns the index of the current item.
        *
        * @since 3.0
        * @return The index of the current item.
        */
        TInt CurrentIndex() const;

        /**
        * Returns the number of items.
        *
        * @since 3.0
        * @return The the current item.
        */
        TInt ItemCount() const;

        /**
        * Update the view's title.
        *
        * @since 3.0
        * @return void.
        */
        void UpdateTitleL();

        /**
        * Handles selections and open commands
        *
        * @since 3.0
        * @return void
        */
        void HandleOpenL(void);

        /**
        * Handles updating the current Feed.
        *
        * @since 3.1
        * @return void
        */
        void HandleUpdateFeedL(void);

        /**
        * Clears the navigation pane.
        *
        * @since 3.1
        * @return void
        */
        void ClearNavigationPane();


    private:
        /**
        * C++ default constructor.
        */
        CFeedsTopicContainer
                    ( CFeedsTopicView* aView,
            		MApiProvider& aApiProvider );

        
        /**
        * By default Symbian 2nd phase constructor is private.
        */
        void ConstructL(const TRect& aRect);

        /**
        * Inits the container.
        *
        * @since 3.0
        * @param aRect The drawing rect.
        * @return void
        */
        void InitContainerL(const TRect& aRect);

        /**
        * Inits the array of needed icons.
        *
        * @since 3.0
        * @return void.
        */
        void InitIconArrayL();

        /**
        * Loads and appends an icon to the icon array..
        *
        * @since 3.0
        * @param aIcons The icon array.
        * @param aID Item ID of the masked bitmap to be created.
        * @param aFilename Filename to be used to construct the item, 
        *        if no matching item was found in the currently active skin.
        * @param aFileBitmapId ID of the bitmap in the file. 
        *        Used only if no matching item was found in the currently 
        *        active skin.
        * @param aFileMaskId ID of the mask in the file.
        *        Used only if no matching item was found in the currently
        *        active skin.
        * @return void.
        */
        void AppendIconL(CArrayPtr<CGulIcon>* aIcons, const TAknsItemID& aID, 
                const TDesC& aFilename, const TInt aFileBitmapId, const TInt aFileMaskId);
        
        /**
        * Sets the list-box's values.
        *
        * @since 3.0
        * @return void
        */
        void UpdateListBoxL();

        /**
        * Handles the changes needed to the Navigation Pane.
        *
        * @since 3.1
        * @return void
        */
        void UpdateNavigationPaneL();
        
    public:  // Friends
        friend class CFeedsTopicView;

    protected:
    	CFeedsTopicView*				iView;				// not owned
    	MApiProvider& 					iApiProvider;       // not owned
        CAknSingleGraphicStyleListBox*  iListBox;
        CDesCArraySeg*                  iListBoxRows;
      	CAknNavigationDecorator*        iNaviDecorator;

        const CFeedsEntity*             iFeed;
    };

#endif      // FEEDS_TOPIC_CONTAINER_H
            
// End of File