/*
* Copyright (c) 2003 Nokia Corporation and/or its subsidiary(-ies).
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
* Description:  Header file for COperatorMenuDocument
*
*/

#ifndef OPERATORMENUDOCUMENT_H
#define OPERATORMENUDOCUMENT_H

// INCLUDES
#include <akndoc.h>
   
// CONSTANTS

// FORWARD DECLARATIONS
class  CEikAppUi;

// CLASS DECLARATION

/**
*  COperatorMenuDocument application class.
*/
class COperatorMenuDocument : public CAknDocument
    {
    public: // Constructor and destructor
        /**
        * Two-phased constructor.
		* @param aApp Application instance
		* @since Series 60 2.0
        */
        static COperatorMenuDocument* NewL( CEikApplication& aApp );

        /**
        * Destructor.
        */
        ~COperatorMenuDocument();

    private:

        /**
        * Default constructor.
		* @param aApp Application instance
        */
        COperatorMenuDocument( CEikApplication& aApp );

        /**
		* Epoc Constructor
		*/ 
		void ConstructL();

    private:

        /**
        * From CEikDocument, create COperatorMenuAppUi "App UI" object.
        */
        CEikAppUi* CreateAppUiL();
    };

#endif

// End of File

