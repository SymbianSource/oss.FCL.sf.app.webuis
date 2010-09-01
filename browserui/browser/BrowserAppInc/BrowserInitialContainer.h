/*
* Copyright (c) 2006 Nokia Corporation and/or its subsidiary(-ies).
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
*      Declaration of class CBrowserInitialContainer.
*      
*
*/


#ifndef BROWSERINITIALCONTAINER_H
#define BROWSERINITIALCONTAINER_H

// INCLUDES
#include <coecntrl.h>
#include <eiklbo.h>
   
// FORWARD DECLARATIONS
class CBrowserInitialView;
class CBrowserGotoPane;

// CLASS DECLARATION

/**
*  CBrowserInitialContainer  container control class.
*/
class CBrowserInitialContainer : public CCoeControl, MCoeControlObserver
    {
    public: 
        
        /**
        * default constructor.
        * @param aRect Frame rectangle for container.
        */
        void ConstructL(const TRect& aRect);
        
        /**
        * Constructor
        */
		CBrowserInitialContainer( CBrowserInitialView *aView );        

        /**
        * Destructor.
        */
        ~CBrowserInitialContainer();


		
    public: // Functions from base classes		

    public: // New functions
    
        /**
        * Opens the selected listbox item
        */            
        void OpenSelectedListboxItemL( );
        
        /**
        * Access the created goto pane.
        * @return Handle to web menu`s goto pane.
        */        
		CBrowserGotoPane* GotoPane() { return iGotoPane; }

        /**
        * Shuts down goto pane.
        */        		
		void ShutDownGotoURLEditorL();
		

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
        CCoeControl* ComponentControl( TInt aIndex ) const;

        /**
        * From CCoeControl,Draw.
        */
        void Draw(const TRect& aRect) const;
       	
		/**
		* From MCoeControlObserver
		* Acts upon changes in the hosted control's state. 
		*
		* @param aControl The control changing its state
		* @param aEventType	The type of control event 
		*/
        void HandleControlEventL( CCoeControl* aControl,TCoeEvent aEventType );

        
    private: // New functions        
    		
        /**
        * From CCoeControl
        */        		
		TKeyResponse OfferKeyEventL(const TKeyEvent& aKeyEvent,TEventCode aType);
		
        /**
        * Gets the resource id of web menu`s title.
        */        		
		TInt TitleResourceId();
		
		CBrowserInitialView* iView;  // not owned
		
    private: //data
		CBrowserGotoPane* iGotoPane; // the gotopane       
    };

#endif

// End of File
