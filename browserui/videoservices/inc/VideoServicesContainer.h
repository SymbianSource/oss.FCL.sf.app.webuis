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
* Description:  Header file for CVideoServicesContainer
*
*/


#ifndef VIDEOSERVICESCONTAINER_H
#define VIDEOSERVICESCONTAINER_H

// INCLUDES
#include <coecntrl.h>
   
// FORWARD DECLARATIONS
class CEikLabel;        // for example labels

// CLASS DECLARATION

/**
*  CVideoServicesContainer  container control class.
*  
*/
class CVideoServicesContainer : public CCoeControl, MCoeControlObserver
    {
    public: // Constructors and destructor
        
		/**
		* Two phased constructor
		* @param aRect Frame rectangle for container
		* @since Series 60 3.0
		*/
		static CVideoServicesContainer* NewL( const TRect& aRect );

        /**
        * Destructor.
        */
        ~CVideoServicesContainer();

    private: // Functions from base classes

        /**
        * EPOC default constructor.
        * @param aRect Frame rectangle for container.
        * @return void
		* @since Series 60 3.1
        */
        void ConstructL( const TRect& aRect );

       /**
        * From CCoeControl,Draw.
		* @param aRect Frame rectangle to draw
        * @return void
		* @since Series 60 3.1
        */
        void Draw( const TRect& aRect ) const;

       /**
        * From CCoeControl, HandleControlEventL
		* @param aControl Active Control
		* @param aEventType Type of control event
        * @return void
		* @since Series 60 3.1
        */
        void HandleControlEventL( CCoeControl* aControl, TCoeEvent aEventType );
        
    private: // data

    };

#endif

// End of File
