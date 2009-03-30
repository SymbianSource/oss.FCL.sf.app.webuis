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
* Description: 
*  CBrowserAsyncExit class can exit the Browser in async mode. It calls the CBrowserAppUi`s
*  ExitBrowserL() method, when the object completes the request.
*  The class helps avoiding to send  event.iCode  EKeyApplication1; event
*  to the embedded browser to exit.
*
*
*/

#ifndef __BROWSERASYNCEXIT_H
#define __BROWSERASYNCEXIT_H

// INCLUDES
#include <e32base.h>

// FORWARD DECLARATIONS
class MApiProvider;

// CLASS DEFINITION
class CBrowserAsyncExit : public CActive
    {
    public:  // constructors

        /**
        * Create a CBrowserAsyncExit object, Leaves on failure.
        * @param aApiProvider Api provider. Not owned.
        * @return A pointer to the created instance of CBrowserAsyncExit.
        */
        static CBrowserAsyncExit* NewL( MApiProvider* aApiProvider );

        /**
        * Create a CBrowserAsyncExit object. Leaves on failure.
        * @param aApiProvider Api provider. Not owned.
        * @return A pointer to the created instance of CBrowserAsyncExit
        */
        static CBrowserAsyncExit* NewLC( MApiProvider* aApiProvider );

    private:  // default c++, and 2nd phase constructor

        /**
        * Constructs this object
        * @param aApiProvider Api provider. Not owned.
        */
        CBrowserAsyncExit( MApiProvider* aApiProvider );

        /**
        * Performs second phase construction of this object
        */
        void ConstructL();

    public:

        /**
        * Destroy the object and release all memory objects
        */
        ~CBrowserAsyncExit();

        /**
        * Complete an asynchronous request.
        */
        void Start();

    protected: // from CActive

        /**
        * Cancel any outstanding requests
        */
        void DoCancel();

        /**
        * Handles object`s request completion event.
        */
        void RunL();

    private:

        /**
        * To access CBrowserAppUi`s BrowserExitL() method.
        */
        MApiProvider* iApiProvider;  // not owned
    };

#endif  // __BROWSERASYNCEXIT_H

// End of file
