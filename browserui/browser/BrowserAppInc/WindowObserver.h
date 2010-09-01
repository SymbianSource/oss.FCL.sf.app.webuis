/*
* Copyright (c) 2005 Nokia Corporation and/or its subsidiary(-ies).
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
*      Declaration of class CBrowserWebMenuView.
*      Observes browser window events.
*      
*
*/


#ifndef __WINDOWOBSERVER_H__
#define __WINDOWOBSERVER_H__

// Events sent to observer
enum TWindowEvent
    {
    EWindowClose,
    EWindowOpen,
    EWindowActivate,
    EWindowLoadStart,
    EWindowLoadStop,
    EWindowCntDisplayed
    };


class MWindowObserver
    {
    public:            
        // Called on window events.   
        virtual void WindowEventHandlerL( TWindowEvent aEvent, TInt aWindowId ) = 0;
    };

#endif  // __WINDOWOBSERVER_H__

// End of file
