/*
* Copyright (c) 2007 Nokia Corporation and/or its subsidiary(-ies).
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
*      Helper class that handles the toolbar
*      
*
*/


#ifndef BROWSERCONTENTVIEWTOOLBAR_H
#define BROWSERCONTENTVIEWTOOLBAR_H

//  INCLUDES
#include <e32std.h>
#include "browser.hrh"

// FORWARD DECLARATIONS
class CBrowserContentView;
class CAknToolbar;



//save and load last state


// CLASS DECLARATION

/**
*  Helper class that handles the toolbar
*  
*  @lib Browser.app
*  @since Series 60 1.2
*/ 
class CBrowserContentViewToolbar : public CBase
    {
	public :	// constructor, destructor

        /**
        * Two-phased constructor.
        */
 		static CBrowserContentViewToolbar* NewL(CBrowserContentView* aBrowserContentView);

        /**
        * Destructor.
        */
 		virtual ~CBrowserContentViewToolbar();

	private :	// construction

		/**
		* Default C++ constructor.
		* @param aBrowserContentView
		*/
		CBrowserContentViewToolbar(CBrowserContentView* aBrowserContentView);

        /**
        * Symbian OS constructor that can leave.
        */
 		void ConstructL();

	public:  // New functions


		/**
		* Enable/disable the toolbar's buttons
		*/
        void UpdateButtonsStateL();
        
    private: // data

        CBrowserContentView* iBrowserContentView;
    };

#endif // BROWSERCONTENTVIEWTOOLBAR_H
            
// End of File
