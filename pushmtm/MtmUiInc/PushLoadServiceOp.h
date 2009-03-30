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
* Description:  Declaration of CPushLoadServiceOp.
*
*/



#ifndef PUSHLOADSERVICEOP_H
#define PUSHLOADSERVICEOP_H

// INCLUDE FILES

//#include "PushMtmOperation.h"
#include "PushMtmUiOperation.h"
#include <e32base.h>
#include <e32std.h>
#include <apparc.h>
#include <AknServerApp.h>
// FORWARD DECLARATIONS

class CSchemeHandler;
class CPushMsgEntryBase;
class CPushMtmSettings;

// CLASS DECLARATION

/**
* This asynchronous operation is responsible for downloading service that is 
* indicated by the URL field of the pushed service message. It means that it 
* is applicable only for SI and SL pushed messages.
* The operation uses the Scheme Handler to launch the appropriate handler 
* application for the given URL.
*/
class CPushLoadServiceOp : public CPushMtmUiOperation, 
                           public MAknServerAppExitObserver
    {
    public: // Constructors and destructor

        /**
        * Two-phased constructor. Leaves on failure.
        * @param aSession Message Server Session to be used by this operation.
        * @param aEntryId Id of entry to operate on.
        * @param aObserverStatus Observer's status.
        * @return The constructed object.
        */
        static CPushLoadServiceOp* NewL( CMsvSession& aSession, 
                                         TMsvId aEntryId, 
                                         TRequestStatus& aObserverStatus );

        /**
        * Destructor.
        */
        virtual ~CPushLoadServiceOp();

    public: // Functions from CPushMtmOperation

        /**
        * Start or restart the operation. The expiration state of the entry 
        * is checked (of course only in case of SI) and if it is expired, 
        * then the entry is deleted and the operation is cancelled.
        * @return None.
        */
        void StartL();

    protected: // Constructors

        /**
        * Constructor.
        * @param aSession Message Server Session to be used by this operation.
        * @param aEntryId Id of entry to operate on.
        * @param aObserverStatus Observer's status.
        */
        CPushLoadServiceOp( CMsvSession& aSession, 
                            TMsvId aEntryId, 
                            TRequestStatus& aObserverStatus );

        /**
        * Symbian OS constructor.
        */
        void ConstructL();

    protected: // New functions

        /**
        * Check the expiration state of the context. It is SI specific. 
        * In case of other push messages it returns EFalse.
        * @return ETrue or EFalse.
        */
        TBool IsExpiredL( const TMsvEntry& aTEntry ) const;

        /**
        * Retrieve the context and get the URL from the context.
        * @param aUrl The URL filled from the context.
        * @return The context entry.
        */
	    CPushMsgEntryBase* RetrieveContextAndGetUrlLC( TPtrC& aUrl ) const;

        /**
        * Authenticate the message and display user query if necessary.
        * @param aPushMsg The push message to be authenticated.
        * @param aMtmSettings The push settings.
        * @return ETrue if the message is authenticated.
        */
        TBool AuthenticateL( const CPushMsgEntryBase& aPushMsg, 
                             const CPushMtmSettings& aMtmSettings ) const;

        /**
        * Execute a query which asks the user if (s)he wants to download 
        * the service anyway.
        * @param aResId Resource to be used.
        * @return ETrue, if the user wants to download it.
        */
        TBool ExecuteUserQueryL( TInt aResId ) const;

        /**
        * If no scheme exists in the URL, then "http://" is insetred.
        * @param aUrl The original URL.
        * @return The validated URL.
        */
        HBufC* ValidateLC( TDesC& aUrl ) const;

    protected: // Functions from base classes

        /**
        * If the operation was not cancelled in StartL, then it extracts 
        * the URL from the message and launches the Document Handler to 
        * execute the proper handler for the scheme.
        * @return None.
        */
        void RunL();

        /**
        * Cancel protocol implementation.
        */
        void DoCancel();

        /**
        * It calls CPushMtmOperation::RunError( aError ).
        */
        TInt RunError( TInt aError );

        void HandleServerAppExit(TInt aReason);

    private:

        TMsvEntry iTEntry; ///< Current context.
        TBool iDone; ///< Indicates that the operation is ready.
        CSchemeHandler* iSchemeHandler; ///< Handles the URL. Owned.
    };

#endif // PUSHLOADSERVICEOP_H

// End of file.
