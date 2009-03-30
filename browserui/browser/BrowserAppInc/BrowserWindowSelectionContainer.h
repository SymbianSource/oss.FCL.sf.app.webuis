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
* Description: 
*      Declaration of class CBrowserWindowSelectionContainer.
*      
*
*/


#ifndef BROWSERWINDOWSELECTIONCONTAINER_H
#define BROWSERWINDOWSELECTIONCONTAINER_H

// INCLUDES
#include <coecntrl.h>
#include <AknTabObserver.h>

// FORWARD DECLARATIONS
class CBrowserWindowSelectionView;
class CAknNavigationControlContainer;
class CAknNavigationDecorator;
class CAknTabGroup;
class CEikImage;
class CGulIcon;

// CLASS DECLARATION

/**
*  CBrowserWindowSelectionContainer  container control class.
*  
*/
class CBrowserWindowSelectionContainer : public CCoeControl, 
                                                MCoeControlObserver, MAknTabObserver
    {
    public: // Constructors and destructor
        
        /**
        * default constructor.
        * @param aRect Frame rectangle for container.
        */
        void ConstructL(const TRect& aRect);

        /**
        * Constructor
        */
        CBrowserWindowSelectionContainer( CBrowserWindowSelectionView *aView );
        
        /**
        * Destructor.
        */
        ~CBrowserWindowSelectionContainer();

    public: // New functions
        
        /**
        * Gets a pointer to the tab group on navi pane.
        * @return Pointer to the tab group.
        */
        inline CAknTabGroup* GetTabGroup() { return iTabGroup; }
        
        /**
        * Gets a pointer to the decorator on navi pane.        
        * @return Pointer to the decorator.
        */
        inline CAknNavigationDecorator* GetNaviDecorator() { return iNaviDecorator; }
        
        /**
        * Creates tabs on navipane, from the array of strings.
        * @param An array containing the name of the windows.
        * @param The tab index, that will be activated after creation.
        */
        void ChangeTabL( CArrayFixFlat<HBufC*>* aTabTexts, TInt aActivateTab );
        
		/**
		* The id of the window, which thumbnail to get.
		* @param aWindowId Window id.
		*/        
        void GetWindowThumbnailL( TInt aWindowId );        

    public: // Functions from base classes
        /**
        * HandlePointerEventL
        * From CCoeControl
        *
        */
        void HandlePointerEventL(const TPointerEvent& aPointerEvent);

        /**
        * TabChangedL
        * From MAknTabObserver
        *
        */
		void TabChangedL(TInt aIndex);

    private: // Functions from base classes

        /**
        * From CoeControl,SizeChanged.
        */
        void SizeChanged();

        /**
        * From CoeControl,CountComponentControls.
        */
        TInt CountComponentControls() const;

        /**
        * From CCoeControl,ComponentControl.
        */
        CCoeControl* ComponentControl(TInt aIndex) const;

        /**
        * From CCoeControl,Draw.
        */
        void Draw(const TRect& aRect) const;
        
        /**
        * From CCoeControl Handles key events
        */
        TKeyResponse OfferKeyEventL( const TKeyEvent& aKeyEvent, TEventCode aType );
       	
		/**
		* From MCoeControlObserver
		* Acts upon changes in the hosted control's state. 
		*
		* @param	aControl	The control changing its state
		* @param	aEventType	The type of control event 
		*/
        void HandleControlEventL(CCoeControl* aControl,TCoeEvent aEventType);
        
    private: //data        
        CBrowserWindowSelectionView* iView; // not owned
        CAknNavigationDecorator* iNaviDecorator; // not owned
        CAknTabGroup* iTabGroup; // not owned
        CGulIcon* iThumbnail;
        //test
        //CFbsBitmap* iThumbnail;
        //CEikImage* iTabIcon;
    };

#endif

// End of File
