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
* Description:  Header file for CVideoServicesApp
*
*/


#ifndef VIDEOSERVICESAPP_H
#define VIDEOSERVICESAPP_H

// INCLUDES
#include <aknapp.h>

// CONSTANTS
// UID of the application
const TUid KUidVideoServices = { 0x10281893 };

// CLASS DECLARATION

/**
* CVideoServicesApp application class.
* Provides factory to create concrete document object.
* 
*/
class CVideoServicesApp : public CAknApplication
    {
 
    private: //From base class

        /**
        * From CApaApplication, creates CVideoServicesDocument document object.
        * @return A pointer to the created document object.
        */
        CApaDocument* CreateDocumentL();
        
        /**
        * From CApaApplication, returns application's UID (KUidVideoServices).
        * @return The value of KUidVideoServices.
        */
        TUid AppDllUid() const;
    };
    
#endif

// End of File
