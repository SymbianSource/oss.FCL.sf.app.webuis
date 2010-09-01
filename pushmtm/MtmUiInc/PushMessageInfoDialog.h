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
* Description:  Class definition of CPushMessageInfoDialog.
*
*/



#ifndef PUSHMESSAGEINFODIALOG_H
#define PUSHMESSAGEINFODIALOG_H

//  INCLUDES

#include <AknPopup.h>
#include <msvstd.h>
#include <ConeResLoader.h>

// FORWARD DECLARATIONS

class CMsvSession;
class CAknSingleHeadingPopupMenuStyleListBox;
class CFont;

// CLASS DECLARATION

/**
* Avkon environment is required.
* It executes a waiting dialog.
* Usage.
* #include "PushMessageInfoDialog.h"
* CPushMessageInfoDialog* dlg = new (ELeave) CPushMessageInfoDialog;
* CleanupStack::PushL( dlg );
* dlg->ExecuteLD( msvSession, id );
* CleanupStack::Pop( dlg );
*
* Note that this class is independent from CMsgInfoMessageInfoDialog defined 
* in muiumsginfo.h. The mentioned class cannot be used, because it does not 
* support Push message information.
*/
class CPushMessageInfoDialog : public CAknPopupList
    {
    public: // Constructors

        /**
        * Constructor.
        */
        CPushMessageInfoDialog();

        /**
        * Destructor.
        */
        virtual ~CPushMessageInfoDialog();

    public: // New functions

        /**
        * Execute the dialog with this member.
        * @param aMsvSession Message Server session.
        * @param aId The id of the entry.
        * @return None.
        */
        void ExecuteLD( CMsvSession& aMsvSession, TMsvId aId );

    private: // Constructors and destructor

        /**
        * Symbian OS constructor.
        * @param aMsvSession Message Server session.
        * @param aId The id of the entry.
        * @return None.
        */
        void ConstructL( CMsvSession& aMsvSession, TMsvId aId );

    private: // New functions

        /**
        * Add a message info item.
        * @param aHeaderResourceId Res id of the header text.
        * @param aText The value text of the item.
        * @return None.
        */
        void AddMessageInfoItemL
            ( TInt aHeaderResourceId, const TDesC& aText ) const;

        /**
        * Convert UTC time to Home Time.
        * @param aUniversalTime Universal (UTC) time to be converted.
        * @return Home time.
        */
        TTime ConvertUniversalToHomeTime( const TTime& aUniversalTime ) const;

        /**
        * Add message info items to the dialog.
        * @param aMsvSession Message Server session.
        * @param aId The id of the entry.
        * @return None.
        */
        void AddMessageInfoItemsL( CMsvSession& aMsvSession, 
                                   TMsvId aId ) const;

    private: // Data

        CAknSingleHeadingPopupMenuStyleListBox* iListBox;       ///< Owns.
        CDesCArrayFlat* iListBoxModel;  ///< Owns.
        RConeResourceLoader iResourceLoader;
        TFileName iResourceFile;
        // The following members help wrapping text.
        const CFont* iSecondColumnFont; ///< Reference to the font. Not owned.
        TInt         iSecondColumnWidth;
        TInt         iPushSLEnabled;
;
    };

#endif // PUSHMESSAGEINFODIALOG_H
            
// End of file.
