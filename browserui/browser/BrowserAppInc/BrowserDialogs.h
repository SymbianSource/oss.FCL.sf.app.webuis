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
*      Helper class that pops up several dialogs.
*      
*
*/


#ifndef BROWSERDIALOGS_H
#define BROWSERDIALOGS_H

//  INCLUDES
#include <e32std.h>

// FORWARD DECLARATIONS
class CAknWaitDialog;

// CLASS DECLARATION

/**
*  TBrowserDialogs, Implements several dialogs used in Browser.
*  
*  @lib Browser.app
*  @since Series 60 1.2
*/ 
class TBrowserDialogs
    {
	public:  // New functions
        /**
        * Functions from NWSS.
        */

        /**
        * Input prompt dialog used by Search and Home page features library
        * @since Series 60 2.8
        * @param aMsg prompt message
        * @param aResp entered text
        * @param aMaxLength maximum length allowed in editor
        * @param aIsUrlAddress flag for allowing only latin input for url addresses 
        * @return AVKON error code
        */
		static TInt DialogPromptReqL( const TDesC& aMsg, 
                                        TDes* aResp, 
                                        TBool aIsUrlAddress, 
                                        TInt aMaxLength = 0 );

        /**
        * Confirmation query dialog used by WTAI library
        * @since Series 60 1.2
        * @param aMsg confirmation message
        * @param aYesMessage left softkey text
        * @param aNoMessage right softkey text
        * @return AVKON error code
        */
        static TInt DialogQueryReqL( const TDesC& aMsg, 
                                     const TDesC& aYesMessage, 
                                     const TDesC& aNoMessage);

	
    public:     // Confirmation queries.


        /**
        * Display an confirmation query with given dialog resource
        * @since Series 60 1.2
        * @param aPrompt resource if of prompt to display.
        * @param aResId resource id of command set (aResId = 0 means R_AVKON_SOFTKEYS_YES_NO as per default )
        * @param aAnimation animation overrides default one
        * @return selected softkey id.
        * If you give your own aResId you have to check the return value!
        * In that case return value are commands 
        * as your specified in resource of that CBA.
        */
		static TInt ConfirmQueryDialogL( const TInt aPromptResourceId, 
                                         TInt aResId = 0,
                                         TInt aAnimation = 0 );

        /**
        * Display an confirmation query with given dialog resource
        * @since Series 60 1.2
        * @param aPrompt Prompt to display.
        * @param aResId resource id of dialog
        * @param aAnimation animation overrides default one
        * @return selected softkey id.
        */
		static TInt ConfirmQueryDialogL( const TDesC& aPrompt, 
                                         TInt aResId = 0,
                                         TInt aAnimation = 0 );


        /**
        * Display an confirmation query with buttons Yes and No.
        * @since Series 60 1.2
        * @param aPrompt Prompt to display.
        * @return ETrue if accepted, EFalse if not.
        */
        static TBool ConfirmQueryYesNoL( TDesC& aPrompt );


        /**
        * Display an confirmation query with buttons Yes and No.
        * @since Series 60 1.2
        * @param aPromptResourceId Resource id of the prompt (not the
        * dialog!).
        * @return ETrue if accepted, EFalse if not.
        */
        static TBool ConfirmQueryYesNoL( const TInt aPromptResourceId );


        /**
        * Display an confirmation query with buttons Yes - No.
        * @since Series 60 1.2
        * @param aPromptResourceId Resource id of the prompt (not the
        * dialog!).
        * @param aString This string will be substituted in place of the first
        * %U in the format string.
        * @return ETrue if accepted, EFalse if not.
        */
        static TBool ConfirmQueryYesNoL
						( const TInt aPromptResourceId, const TDesC& aString );


        /**
        * Display an confirmation query with buttons Yes and No.
        * @since Series 60 1.2
        * @param aPromptResourceId Resource id of the prompt (not the
        * dialog!).
        * @param aNum This number will be substituted in place of the first
        * %N in the format string.
        * @return ETrue if accepted, EFalse if not.
        */
        static TBool ConfirmQueryYesNoL
							( const TInt aPromptResourceId, const TInt aNum );


    public:     // Information notes.

        /**
        * Display an information note with no buttons.
        * @since Series 60 1.2
        * @param aDialogResourceId Resource id of note to display.
        * Available notes: R_WMLBROWSER_OK_NOTE (check mark animation);
        * R_WMLBROWSER_INFO_NOTE (info mark animation).
        * @param aPrompt Prompt to display.
        */
        static void InfoNoteL( TInt aDialogResourceId, const TDesC& aPrompt );


        /**
        * Display an information note with no buttons.
        * @since Series 60 1.2
        * @param aDialogResourceId Resource id of note to display.
        * Available notes: R_WMLBROWSER_OK_NOTE (check mark animation);
        * R_WMLBROWSER_INFO_NOTE (info mark animation).
        * @param aPromptResourceId Resource id of the prompt.
        */
        static void InfoNoteL
					( TInt aDialogResourceId, const TInt aPromptResourceId );
		

        /**
        * Display an information note with no buttons.
        * @since Series 60 1.2
        * @param aDialogResourceId Resource id of note to display.
        * Available notes: R_WMLBROWSER_OK_NOTE (check mark animation);
        * R_WMLBROWSER_INFO_NOTE (info mark animation).
        * @param aPromptResourceId Resource id of the prompt.
        * @param aString This string will be substituted in place of the first
        * %U in the format string.
        */
		static void InfoNoteL
			(
            TInt aDialogResourceId,
            const TInt aPromptResourceId,
            const TDesC& aString
            );


        /**
        * Display an information note with no buttons.
        * @since Series 60 1.2
        * @param aDialogResourceId Resource id of note to display.
        * Available notes: R_WMLBROWSER_OK_NOTE (check mark animation);
        * R_WMLBROWSER_INFO_NOTE (info mark animation).
        * @param aPromptResourceId Resource id of the prompt.
        * @param aNum This number will be substituted in place of the first
        * %N in the format string.
        */
        static void InfoNoteL
			(
            TInt aDialogResourceId,
            const TInt aPromptResourceId,
            const TInt aNum
            );


    public:     // error note

        /**
        * Display an error note with buttons Ok - <empty>.
        * @since Series 60 1.2
        * @param aPromptResourceId Resource id of the prompt (not the
        * dialog!).
        */
        static void ErrorNoteL( const TInt aPromptResourceId );

        /**
        * Display an error note with buttons Ok - <empty>.
        * @since Series 60 1.2
        * @param aPrompt Prompt to display.
        */
        static void ErrorNoteL( const TDesC& aPrompt );

    };

#endif
            
// End of File