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
* Description:  Application Ui declaration.
*
*/



#ifndef PUSHVIEWERMSGEDAPPUI_H
#define PUSHVIEWERMSGEDAPPUI_H

//  INCLUDES

#include <MsgEditorAppUi.h>
#include <bldvariant.hrh>

// FORWARD DECLARATIONS

class CMsgEditorView;
class CPushMsgEntryBase;
class CPushViewerDocument;

// CLASS DECLARATION

/**
* Push Viewer application's UI that creates a MsgEditor-based view.
* This application uses a message server entry as a "model" or "context".
*/
class CPushViewerMsgEdAppUi : public CMsgEditorAppUi
    {
    public: // Constructors and destructor

        /**
        * Constructor.
        */
        CPushViewerMsgEdAppUi();

        /**
        * Destructor.
        */
        virtual ~CPushViewerMsgEdAppUi();

    public: // New functions
        
        /**
        * Return the model of the application that is a push message entry.
        * @return The base class for all push entries.
        */
        CPushMsgEntryBase& Model() const;

    private: // Constructors

        /**
        * Constructor.
        * @return None.
        */
        void ConstructL();

    private: // New functions

        /**
        * Return the document of the application.
        * @return The doc.
        */
        CPushViewerDocument& PushDoc() const;

        /**
        * Call CMsgEditorAppUi::UpdateNaviPaneL() with the right images.
        * @return None.
        */
        void UpdateNaviPaneL();

        /**
        * Construct the view and make it visible.
        * @return None.
        */
        void ConstructViewL();

        /**
        * Chect the expiration state of the context. It is SI specific. 
        * In case of other push messages it returns EFalse.
        * @return ETrue or EFalse.
        */
        TBool IsExpired();

        /**
        * Show information note and terminate the application.
        */
        void NotifyAndExitL( TInt aResId );

        /**
        * Construct the two body text: one holds the message, 
        * the other holds the href. The order depends on the 
        * length of the message text.
        * @param aBodyControl1 The first body control.
        * @param aBodyControl2 The second body control.
        * @return Index of the Href control or KErrNotFound if no href in the msg.
        */
        TInt ConstructBodyTextL( CMsgBodyControl& aBodyControl1, 
                                 CMsgBodyControl& aBodyControl2 ) const;

    private: // Functions from base classes 

        /**
        * Terminate the application with this method.
        * @return None.
        */
        void DoMsgSaveExitL();

        /**
        * Construct the context, update the navi pane and construct the view.
        * @return None.
        */
        void LaunchViewL();

        /**
        * This method is called when the context is changed.
        * @return None.
        */
        void HandleEntryChangeL();

        /**
        * This method is called when the context is deleted.
        * @return None.
        */
        void HandleEntryDeletedL();

        /**
        * We handle the Left/Right arrow here. Other event keys are 
        * handled by the view.
        * @return EKeyWasConsumed.
        */
        TKeyResponse HandleKeyEventL( const TKeyEvent& aKeyEvent, TEventCode aType );

        /**
        * Dynamically initialize the menu takin into consideration 
        * the current context.
        * @return None.
        */
        void DynInitMenuPaneL( TInt aMenuId, CEikMenuPane* aMenuPane );

        /**
        * It's just an empty implementation.
        * @return None.
        */
        void GetHelpContextForControl( TCoeHelpContext& aContext ) const;

#ifdef __SERIES60_HELP
        /**
        * Get help context from the appui.
        * @return Context.
        */
        CArrayFix<TCoeHelpContext>* HelpContextL() const;
#endif // __SERIES60_HELP

        /**
        * Handle commands.
        * @return None.
        */
        void HandleCommandL( TInt aCommand );

    private: // Data members

        CPushMsgEntryBase* iContext; ///< Message wrapper. Owned.
        CMsgEditorView* iView; ///< MsgEditor style view. Owned.
    };

#endif // PUSHVIEWERMSGEDAPPUI_H

// End of File
