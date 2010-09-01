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
* Description:  Declaration of CPushViewerDocument 
*
*/



#ifndef PUSHVIEWERDOC_H
#define PUSHVIEWERDOC_H

//  INCLUDES

#include <msgeditordocument.h>

// CLASS DECLARATION

/**
* Push Viewer application's document.
*/
class CPushViewerDocument : public CMsgEditorDocument
    {
    public: // Constructors and destructor

        static CPushViewerDocument* NewL( CEikApplication& aApp );  

    public: // New functions

        /**
        * Issue EPushMtmCmdLoadService to MTM UI.
        * @return None.
        */
        void IssueLoadServiceL();

        /**
        * Issue KMtmUiFunctionMessageInfo to MTM UI.
        * @return None.
        */
        void IssueMessageInfoL();

    private:  // Constructors and destructor

        CPushViewerDocument( CEikApplication& aApp );

    private: // Functions from base classes

        /**
        * From CEikDocument.
        */
        CEikAppUi* CreateAppUiL();

        /**
        * From CMsgEditorDocument
        */
        TMsvId DefaultMsgFolder() const;

        /**
        * From CMsgEditorDocument
        */
        TMsvId DefaultMsgService() const;

        /**
        * From CMsgEditorDocument
        */
        TMsvId CreateNewL( TMsvId aService, TMsvId aFolder );

        /**
        * From CMsgEditorDocument
        */
        void EntryChangedL();
    };

#endif // PUSHVIEWERDOC_H
            
// End of File
