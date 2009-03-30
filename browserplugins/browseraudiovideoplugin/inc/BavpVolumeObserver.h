/*
* Copyright (c) 2006 Nokia Corporation and/or its subsidiary(-ies).
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
* Description:  Definition of the interface BavpVolumeObserver used to
*                handle volume level change indications from the Browser
*
*/


#ifndef BAVPVOLUMEOBSERVER_H
#define BAVPVOLUMEOBSERVER_H

/**
*  BavpVolumeObserver class for handling the volume change requests
*  @lib npBrowserVideoPlugin.lib
*  @since 3.2
*/

class MBavpVolumeObserver
    {
    public:
    
        /**
        * UpdateVolume - Method called as part of the Central
        * repository server to set the volume value
		* @since 3.2
        * @param TInt aVolume: Volume value to be set to
		* @return void
        */
		virtual void UpdateVolume( TInt aVolume ) = 0;
    };

#endif // BAVPVOLUMEOBSERVER_H
