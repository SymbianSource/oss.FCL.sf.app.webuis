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
*      Defines whether WIM is part of WmlBrowser or not
*      
*
*/


// INCLUDES
#if !defined(__WMLBROWSERBUILD_H__)
#define __WMLBROWSERBUILD_H__

#include <bldvariant.hrh>

// Use this flag to send bookmark as plaing ascii text
// instead of binary OTA.
#define _BOOKMARK_SENT_ASCII

#define CHANGECONN_ALWAYSASK_FEATURE ( FeatureManager::FeatureSupported( KFeatureIdProtocolWlan ) )

#define HELP ( FeatureManager::FeatureSupported( KFeatureIdHelp ) )

#define ADAPTIVEBOOKMARKS (FeatureManager::FeatureSupported( KFeatureIdBrowserAdaptiveBookm ))

// #define WSP ( FeatureManager::FeatureSupported( KFeatureIdWsp ) )

//browser autocomplete
#define AUTOCOMP ( FeatureManager::FeatureSupported( KFeatureIdBrowserUrlCompletion )  )

#define AVKONELAF ( FeatureManager::FeatureSupported( KFeatureIdAvkonELaf )  )
#define AVKONAPAC ( FeatureManager::FeatureSupported( KFeatureIdAvkonApac )  )
#define CHINESE ( FeatureManager::FeatureSupported( KFeatureIdChinese ) )

#define IMEI_NOTIFICATION ( FeatureManager::FeatureSupported( KFeatureIdBrowserIMEINotification ) )

#define HAS_SIDE_VOLUME_KEYS ( FeatureManager::FeatureSupported( KFeatureIdSideVolumeKeys ) )

#define PROGRESSIVE_DOWNLOAD ( FeatureManager::FeatureSupported( KFeatureIdBrowserProgressiveDownload ) )

#endif // __WMLBROWSERBUILD_H__


// End of File
