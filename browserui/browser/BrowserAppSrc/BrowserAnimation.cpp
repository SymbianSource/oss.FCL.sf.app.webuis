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
*      Helper class to put animation to contextPane.
*      
*
*/


// INCLUDE FILES

#include "BrowserAnimation.h"
#include "logger.h"

#include <aknappui.h>
#include <eikspane.h>
#include <akncontext.h>
#include <AknBitmapAnimation.h>
#include <barsread.h>  // for TResourceReader
#include <AknsUtils.h>  // for TAknsItemID

// ================= MEMBER FUNCTIONS =======================

// ---------------------------------------------------------
// CBrowserAnimation::NewL()
// ---------------------------------------------------------
//
CBrowserAnimation* CBrowserAnimation::NewL( TInt aResourceId )
    {
LOG_ENTERFN("Anim::NewL");
    CBrowserAnimation* self = new( ELeave )CBrowserAnimation;
    CleanupStack::PushL( self );
    self->ConstructL( aResourceId );
    CleanupStack::Pop();  // self
    return self;
    }

// ---------------------------------------------------------
// CBrowserAnimation::~CBrowserAnimation()
// ---------------------------------------------------------
//
CBrowserAnimation::~CBrowserAnimation()
    {
    delete iAnimation;
    }

// ---------------------------------------------------------
// CBrowserAnimation::StartL()
// ---------------------------------------------------------
//
void CBrowserAnimation::StartL()
    {
LOG_ENTERFN("Anim::StartL");
    if ( iAnimation && !iIsRunning )
        {
        // Getting status pane control
        CEikStatusPane* sp = STATIC_CAST( CAknAppUi*,
              CEikonEnv::Static()->EikAppUi() )
             ->StatusPane();

        // Fetch pointer to contextpane
        CAknContextPane* contextPane = STATIC_CAST( CAknContextPane*,
             sp->ControlL( TUid::Uid( EEikStatusPaneUidContext ) ) );

        //this sets an empty bitmap to the contextpanes background to
        //avoid to have the original one in the back of the animation
        //the ownership is taken by the contextpane
        CFbsBitmap *emptyFbsBitmap = new (ELeave) CFbsBitmap(); 
          
        CleanupStack::PushL( emptyFbsBitmap );
        emptyFbsBitmap->Create( TSize(1, 1), ENone);
        CEikImage * emptyEikImage = new (ELeave) CEikImage( );   
        emptyEikImage->SetBitmap( emptyFbsBitmap );
        contextPane->SetPicture( emptyEikImage );
        CleanupStack::Pop();//emptyFbsBitmap ownership is taken

        iAnimation->StartAnimationL(); // displays the animation
        iIsRunning = ETrue;
        }
    }

// ---------------------------------------------------------
// CBrowserAnimation::StopL()
// ---------------------------------------------------------
//
void CBrowserAnimation::StopL()
    {
LOG_ENTERFN("Anim::StopL");
    if ( iAnimation && iIsRunning )     // Is the animation running?
        {
        iAnimation->CancelAnimation(); // ends the animation
        iIsRunning = EFalse;

        // Getting status pane control
        CEikStatusPane* sp = STATIC_CAST( CAknAppUi*,
            CEikonEnv::Static()->EikAppUi() )->StatusPane();

        // Fetch pointer to contextpane
        CAknContextPane* contextPane = NULL;
        TRAP_IGNORE( contextPane = STATIC_CAST( CAknContextPane*, sp->ControlL
            ( TUid::Uid( EEikStatusPaneUidContext ) ) ) );

        if ( contextPane )
            {
            contextPane->SetPictureToDefaultL();
            contextPane->DrawNow();	// draw original icon
            }
        }
    }

// ---------------------------------------------------------
// CBrowserAnimation::ConstructL()
// ---------------------------------------------------------
//
void CBrowserAnimation::ConstructL( TInt aResourceId )
    {
LOG_ENTERFN("Anim::ConstructL");
PERFLOG_LOCAL_INIT
PERFLOG_STOPWATCH_START
    // Getting status pane control	
    CEikStatusPane* sp = STATIC_CAST( CAknAppUi*,
        CEikonEnv::Static()->EikAppUi() ) ->StatusPane();

    if ((sp && sp->PaneCapabilities(
        TUid::Uid(EEikStatusPaneUidContext)).IsInCurrentLayout()))
        {
        // Fetch pointer to contextpane
        CAknContextPane* contextPane =
            STATIC_CAST( CAknContextPane*,
            sp->ControlL( TUid::Uid( EEikStatusPaneUidContext ) ) );

        iAnimation = CAknBitmapAnimation::NewL();
        iAnimation->SetContainerWindowL( *contextPane );

        TResourceReader rr;
        TAknsItemID animationSkinID;
        animationSkinID.Set( EAknsMajorAvkon, aResourceId );

        if( !iAnimation->ConstructFromSkinL( animationSkinID ) )
            {
            iAnimation->ControlEnv()->CreateResourceReaderLC( rr, aResourceId );
            iAnimation->ConstructFromResourceL( rr ); // read from resource
            CleanupStack::PopAndDestroy(); // rr
            }

        iAnimation->SetRect(contextPane->Rect());
PERFLOG_STOP_WRITE("Animation ConstructL phase 1")
#if 0
        // Start and stop animation out of the screen.
        // This way it saves context pane's content as background frame.
        const TInt KWmlAnimationDummyPos = 1000;
        iAnimation->SetPosition( TPoint( KWmlAnimationDummyPos,
                                         KWmlAnimationDummyPos ) );
        iAnimation->SetRect(contextPane->Rect());
PERFLOG_STOPWATCH_START
        StartL();
        StopL();
        iAnimation->SetPosition( TPoint( 0, 0 ) );
PERFLOG_STOP_WRITE("Animation ConstructL phase 2")
#endif
        }
    }

//  End of File  
