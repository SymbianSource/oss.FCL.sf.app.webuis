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
* Description:  Server Mtm class declaration
*
*/



#ifndef PUSHMTMSERVER_H
#define PUSHMTMSERVER_H

//  INCLUDES

#include "PushMtmProgress.h"
#include <mtsr.h>

// CLASS DECLARATION

/**
* Server MTM for pushed messages. Currently, all operations are executed
* in the Client MTM, so this class is a collection of dummy implementations.
*
*  @lib ?library
*  @since ?Series60_version
*/
class CPushMtmServer : public CBaseServerMtm
    {
    public:     // Constructors and destructor

        /**
        * Two-phased constructor (exported factory function).
        * Leaves on failure.
        * @param aRegisteredMtmDll Registration data for the MTM DLL.
        * @param aInitialEntry Initial entry.
        * @return The constructed Server MTM.
        */
        IMPORT_C static CPushMtmServer* NewL(
            CRegisteredMtmDll& aRegisteredMtmDll,
            CMsvServerEntry* aInitialEntry );

        /**
        * Destructor.
        */
        virtual ~CPushMtmServer();

    private:  // Constructors

        /**
        * Constructor.
        * @param aRegisteredMtmDll Registration data for the MTM DLL.
        * @param aInitialEntry Initial entry.
        */
        CPushMtmServer(
            CRegisteredMtmDll& aRegisteredMtmDll,
            CMsvServerEntry* aInitialEntry );

        /**
        * Second phase constructor. Leaves on failure. 
        * Add this active object to the scheduler.
        */
        void ConstructL();

    private:  // Functions from base classes

        /**
        * Do nothing, but leave with KErrNotSupported.
        */
        void CopyToLocalL(
            const CMsvEntrySelection& aSelection,
            TMsvId aDestination,
            TRequestStatus& aStatus );

        /**
        * Do nothing, but leave with KErrNotSupported.
        */
        void CopyFromLocalL(
            const CMsvEntrySelection& aSelection,
            TMsvId aDestination,
            TRequestStatus& aStatus );

        /**
        * Do nothing, but leave with KErrNotSupported.
        */
        void CopyWithinServiceL(
            const CMsvEntrySelection& aSelection,
            TMsvId aDestination,
            TRequestStatus& aStatus );

        /**
        * Do nothing, but leave with KErrNotSupported.
        */
        void MoveToLocalL(
            const CMsvEntrySelection& aSelection,
            TMsvId aDestination,
            TRequestStatus& aStatus );

        /**
        * Do nothing, but leave with KErrNotSupported.
        */
        void MoveFromLocalL(
            const CMsvEntrySelection& aSelection,
            TMsvId aDestination,
            TRequestStatus& aStatus );

        /**
        * Do nothing, but leave with KErrNotSupported.
        */
        void MoveWithinServiceL(
            const CMsvEntrySelection& aSelection,
            TMsvId aDestination,
            TRequestStatus& aStatus );

        /**
        * Do nothing, but leave with KErrNotSupported.
        */
        void DeleteAllL(
            const CMsvEntrySelection& aSelection,
            TRequestStatus& aStatus );

        /**
        * Do nothing, but leave with KErrNotSupported.
        */
        void CreateL( TMsvEntry aNewEntry, TRequestStatus& aStatus );

        /**
        * Do nothing, but leave with KErrNotSupported.
        */
        void ChangeL( TMsvEntry aNewEntry, TRequestStatus& aStatus );

        /**
        * Do nothing, but leave with KErrNotSupported.
        */
        void StartCommandL(
            CMsvEntrySelection& aSelection,
            TInt aCommand,
            const TDesC8& aParameter,
            TRequestStatus& aStatus );

        /**
        * Do nothing, but return EFalse.
        */
        TBool CommandExpected();

        /**
        * Return iProgressPckg.
        */
        const TDesC8& Progress();

        /**
        * Do nothing.
        */
        void DoRunL();

        /**
        * Do nothing.
        */
        void DoComplete( TInt aError );

        /**
        * Do nothing.
        */
        void DoCancel();

    protected: // Data

        TPushMtmProgress        iProgress;      ///< Progress information.
        /// Reference to progress information as a buffer pointer.
        TPushMtmProgressPckg    iProgressPckg;
    };

#endif // PUSHMTMSERVER_H
