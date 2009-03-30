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
* Description:  Header file for CVideoServicesDocument
*
*/

#ifndef VIDEOSERVICESDOCUMENT_H
#define VIDEOSERVICESDOCUMENT_H

// INCLUDES
#include <akndoc.h>
   
// CONSTANTS

// FORWARD DECLARATIONS
class  CEikAppUi;

// CLASS DECLARATION

/**
*  CVideoServicesDocument application class.
*/
class CVideoServicesDocument : public CAknDocument
    {
    public: // Constructor and destructor
        /**
        * Two-phased constructor.
		* @param aApp Application instance
		* @since Series 60 3.1
        */
        static CVideoServicesDocument* NewL( CEikApplication& aApp );

        /**
        * Destructor.
        */
        ~CVideoServicesDocument();

    private:

        /**
        * Default constructor.
		* @param aApp Application instance
        */
        CVideoServicesDocument( CEikApplication& aApp );

        /**
		* Epoc Constructor
		*/ 
		void ConstructL();

    private:

        /**
        * From CEikDocument, create CVideoServicesAppUi "App UI" object.
        */
        CEikAppUi* CreateAppUiL();
    };

#endif

// End of File
