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
* Description:  Definition of CPushViewerApplication.
*
*/



#ifndef PUSHVIEWERAPP_H
#define PUSHVIEWERAPP_H

//  INCLUDES

#include <aknapp.h>

// FORWARD DECLARATION

class CApaAppServer;

// CLASS DECLARATION

/**
* Push Viewer application
*/
class CPushViewerApplication : public CAknApplication
    {
    private: // Functions from base classes

        /**
        * From CApaApplication.
        */
        TUid AppDllUid() const;

        /**
        * From CEikApplication.
        */
        CApaDocument* CreateDocumentL();
        
        /*
        * From CEikApplication.
        */
        void NewAppServerL( CApaAppServer*& aAppServer );
    };

#endif // PUSHVIEWERAPP_H

// End of File
