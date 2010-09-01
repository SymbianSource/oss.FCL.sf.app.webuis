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
*      Declaration of class CWmlBrowserFavouritesIncrementalOp.
*      
*
*/


#ifndef WML_BROWSER_FAVOURITES_INCREMENTAL_OP
#define WML_BROWSER_FAVOURITES_INCREMENTAL_OP

// INCLUDE FILES

#include <e32base.h>
#include <AknProgressDialog.h>

// FORWARD DECLARATION

class CBrowserFavouritesModel;
class CAknWaitDialog;

// CLASS DECLARATION

/**
* Base incremental operation with a wait dialog; an Active Object.
* Pure virtual.
*/
class CBrowserFavouritesIncrementalOp: 
                     public CAsyncOneShot, public MProgressDialogCallback
    {
    public:     // construct / destruct

        /**
        * Destructor.
        */
        virtual ~CBrowserFavouritesIncrementalOp();

    public:     // new methods

        /**
        * Start the operation and exclude the caller from the Active Scheduler
        * until done (this method returns when the operation is done).
        */
        void ExecuteL();

    protected:  // construct / destruct

        /**
        * Constructor.
        * @param aPriority Active Object priority.
        * @param aModel Data model.
        */
        CBrowserFavouritesIncrementalOp
            (
            CBrowserFavouritesModel& aModel,
            TInt aPriority = CActive::EPriorityStandard
            );

        /**
        * Second phase constructor.
        */
        void ConstructL();

    protected:  // new methods

        /**
        * Perform next step. Derived classes must provide this method.
        * @return Number of steps to come (0 if done).
        */
        virtual TInt StepL() = 0;

        /**
        * Create and prepare the wait note.
        */
        virtual void CreateWaitNoteLC() = 0;

        /**
        * Called when all steps successfully executed; wait note is finished,
        * but waiting has not been ended. Not called if cancelled or in case of
        * any error. After this call ExecuteL returns.
        * Empty implementation provided.
        */
        virtual void DoneL();

    protected:  // from CActive

        /**
        * Perform one step (or quit if finished).
        */
        void RunL();

        /**
        * Cancel protocol implementation.
        */
        void DoCancel();

        /**
        * Error handling: abandon operation.
        */
        TInt RunError( TInt aError );

    protected:  // from MProgressDialogCallback

        /**
        * Callback method; gets called when a dialog is dismissed.
        * (This is where iWait is stopped - we wait for the dialog).
        * @param aButtonId Button id.
        */
        void DialogDismissedL( TInt aButtonId );

    protected:  // data

        CBrowserFavouritesModel* iModel; ///< Data model. Not owned.
        CAknWaitDialog* iWaitNote;          ///< Wait note. Owned.

    private:    // data

        CActiveSchedulerWait* iWait;        ///< Wait object. Owned.

    };

#endif
// End of File
