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
*      This file contains the class definition of CPushAsyncOpRunner.
*      
*
*/



#ifndef PUSHASYNCOPRUNNER_H
#define PUSHASYNCOPRUNNER_H

//  INCLUDES

#include "PushMtmOperation.h"
#include <e32std.h>
#include <msvstd.h>

// FORWARD DECLARATIONS

class CMsvSession;
class CMsvOperation;

// CLASS DECLARATION

/**
* This class is intended as a base class of asynchronous ui operations. 
* It provides the syntax, and the semantics should be implemented by derived classes: 
* default implementations are provided, and derived classes may override this.
* The main goal was to make those Push Mtm Ui's functions asynchronous, that are 
* really asynchronous, and avoid making them synchronous with using 
* CActiveScheduler::Start() and CActiveScheduler::Stop(). This makes the 
* design clearer and may prevent some problems raised by the usage of Start()'s and 
* Stop()'s.
*/
class CPushAsyncOpRunner : public CPushMtmOperation
    {
    public: // Functions from base classes

        /**
        * Start or restart the operation. OnStartupL is called. See OnStartupL.
        * @return None.
        */
        IMPORT_C void StartL();

        /**
        * Get progress information.
        * @return A package buffer (of type TPushMtmProgressBuf), containing 
        *         progress information.
        */
        IMPORT_C const TDesC8& ProgressL();

    protected: // Constructors and destructor

        /**
        * Constructor. CActiveScheduler::Add( this ) is called. Derived 
        * classes shouldn't call CActiveScheduler::Add( this ) in their constructor.
        * @param aSession Msv session to use.
        * @param aEntryId Id of the context to operate on.
        * @param aObserverStatus Observer status.
        * @return None.
        */
        IMPORT_C CPushAsyncOpRunner( CMsvSession& aSession, 
                                     const TMsvId aEntryId, 
                                     TRequestStatus& aObserverStatus );

        /**
        * Destructor.
        */
        IMPORT_C virtual ~CPushAsyncOpRunner();

    protected: // New functions

        /**
        * This method is called from StartL. Default implementation returns ETrue 
        * and sets aIsObserving to EFalse.
        * @param aIsObserving Used only if the function returns EFalse. 
        *                     In this case 
        *                     indicate in this variable if this operation observes 
        *                     an other one. If so, then it is set to active, but 
        *                     not completed - this will be made by the observed 
        *                     operation.
        * Derived classes may override this.
        * @return Return ETrue if the operation has completed. Return EFalse 
        *         if the operation has not completed yet. In the latter case 
        *         parameter aIsObserving is also used.
        */
        IMPORT_C virtual TBool OnStartupL( TBool& aIsObserving );

        /**
        * This method is called from RunL. Default implementation returns ETrue 
        * and sets aIsObserving to EFalse.
        * @param aIsObserving Used only if the function returns EFalse. 
        *                     In this case 
        *                     indicate in this variable if this operation observes 
        *                     an other one. If so, then it is set to active, but 
        *                     not completed - this will be made by the observed 
        *                     operation.
        * Derived classes may override this.
        * @return Return ETrue if the operation has completed. Return EFalse 
        *         if the operation has not completed yet. In the latter case 
        *         parameter aIsObserving is also used.
        */
        IMPORT_C virtual TBool OnRunLActionL( TBool& aIsObserving );

        /**
        * Set observed operation and take ownership.
        * @param aOp The operation.
        * @return None.
        */
        IMPORT_C void SetOperation( CMsvOperation* aOp );

        /**
        * Delete the observed operation.
        * @return None.
        */
        IMPORT_C void DeleteOperation();

        /**
        * Return the observed operation.
        * @return The operation.
        */
        IMPORT_C CMsvOperation& Operation();

    protected: // Functions from base classes

        /**
        * RunL protocol implementation. See OnRunLActionL.
        * @return None.
        */
        IMPORT_C void RunL();

        /**
        * Default implementation cancels the observed operation, and 
        * completes the observer with KErrCancel.
        * Derived classes may override this.
        * @return None.
        */
	    IMPORT_C void DoCancel();

        /**
        * Default implementation cancels the observed operation, 
        * completes the observer with aError and forwards the 
        * error to the scheduler.
        * Derived classes may override this.
        * @param aError Leave code.
        * @return aError.
        */
        IMPORT_C TInt RunError( TInt aError );

    private: // Data members

        TBool       iReady;             ///< ETrue, if the operation is ready.
        CMsvOperation* iOperation;         ///< The observed operation. Has.
        TBufC8<1>   iDummyProgressBuf;  ///< Dummy progress buffer. Ignored.
    };

#endif // PUSHASYNCOPRUNNER_H
            
// End of file.
