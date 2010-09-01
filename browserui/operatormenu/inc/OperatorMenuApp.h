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
* Description:  Header file for COperatorMenuApp
*
*/


#ifndef OPERATORMENUAPP_H
#define OPERATORMENUAPP_H

// INCLUDES
#include <aknapp.h>

// CONSTANTS
// UID of the application
const TUid KUidOperatorMenu = { 0x10008D5E };

// CLASS DECLARATION

/**
* COperatorMenuApp application class.
* Provides factory to create concrete document object.
* 
*/
class COperatorMenuApp : public CAknApplication
    {
 
    private: //From base class

        /**
        * From CApaApplication, creates COperatorMenuDocument document object.
        * @return A pointer to the created document object.
        */
        CApaDocument* CreateDocumentL();
        
        /**
        * From CApaApplication, returns application's UID (KUidOperatorMenu).
        * @return The value of KUidOperatorMenu.
        */
        TUid AppDllUid() const;
    };
    
#endif

// End of File

