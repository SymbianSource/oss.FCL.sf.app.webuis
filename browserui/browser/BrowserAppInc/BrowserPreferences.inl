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
*      Inline methods for CBrowserPreferences.
*
*
*/


#ifndef WML_BROWSER_PREFERENCES_INL
#define WML_BROWSER_PREFERENCES_INL

#include <bldvariant.hrh>

// ================= MEMBER FUNCTIONS =======================


// ---------------------------------------------------------
// CBrowserPreferences::DefaultSnapId()
// ---------------------------------------------------------
//

TUint CBrowserPreferences::DefaultSnapId() const
    { 
           
    return iAllPreferences.iDefaultSnapId ;
        
    }


// ---------------------------------------------------------
// CBrowserPreferences::DefaultAccessPoint
// ---------------------------------------------------------
//
TUint CBrowserPreferences::DefaultAccessPoint() const
    {
    if ( ( iOverridenSettings ) && iCustomAp )
        {
        return iCustomAp;
        }
    else
        {
        return iAllPreferences.iDefaultAccessPoint ;
        }
    }

// ---------------------------------------------------------
// CBrowserPreferences::AssociatedVpn
// ---------------------------------------------------------
//
TUint CBrowserPreferences::AssociatedVpn() const
    {
    return iAllPreferences.iAssocVpn;
    }

// ---------------------------------------------------------
// CBrowserPreferences::DefaultAPDetails
// ---------------------------------------------------------
//
CApAccessPointItem* CBrowserPreferences::DefaultAPDetails()
    {
    return iAllPreferences.iDefaultAPDetails;
    }

// ---------------------------------------------------------
// CBrowserPreferences::AutoLoadContent
// ---------------------------------------------------------
//
TInt CBrowserPreferences::AutoLoadContent() const
	{
	return iAllPreferences.iAutoLoadContent;
	}

// ---------------------------------------------------------
// CBrowserPreferences::PageOverview
// ---------------------------------------------------------
//
TBool CBrowserPreferences::PageOverview() const
	{
	return iAllPreferences.iPageOverview;
	}
	
// ---------------------------------------------------------
// CBrowserPreferences::BackList
// ---------------------------------------------------------
//
TBool CBrowserPreferences::BackList() const
	{
	return iAllPreferences.iBackList;
	}
	
// ---------------------------------------------------------
// CBrowserPreferences::AutoRefresh
// ---------------------------------------------------------
//
TBool CBrowserPreferences::AutoRefresh() const
	{
	return iAllPreferences.iAutoRefresh;
	}

// ---------------------------------------------------------
// CBrowserPreferences::TextWrap
// ---------------------------------------------------------
//
TBool CBrowserPreferences::TextWrap() const
	{
	return iAllPreferences.iTextWrap;
	}

// ---------------------------------------------------------
// CBrowserPreferences::FontSize
// ---------------------------------------------------------
//
TInt CBrowserPreferences::FontSize() const
	{
	return iAllPreferences.iFontSize;
	}

// ---------------------------------------------------------
// CBrowserPreferences::Encoding
// ---------------------------------------------------------
//
TUint32 CBrowserPreferences::Encoding() const
    {
    return iAllPreferences.iEncoding;
    }

// ---------------------------------------------------------
// CBrowserPreferences::AdaptiveBookmarks
// ---------------------------------------------------------
//
TWmlSettingsAdaptiveBookmarks CBrowserPreferences::AdaptiveBookmarks() const
	{
	return iAllPreferences.iAdaptiveBookmarks;
	}

// ---------------------------------------------------------
// CBrowserPreferences::FullScreen()
// ---------------------------------------------------------
//
TWmlSettingsFullScreen CBrowserPreferences::FullScreen() const
 	{
   	return iAllPreferences.iFullScreen;
   	}
 
// ---------------------------------------------------------
// CBrowserPreferences::Cookies
// ---------------------------------------------------------
//
TWmlSettingsCookies CBrowserPreferences::Cookies() const
	{
	return iAllPreferences.iCookies;
	}

// ---------------------------------------------------------
// CBrowserPreferences::Ecma
// ---------------------------------------------------------
//
TWmlSettingsECMA CBrowserPreferences::Ecma() const
	{
	return iAllPreferences.iEcma;
	}

// ---------------------------------------------------------
// CBrowserPreferences::ScriptLog
// ---------------------------------------------------------
//
TUint32 CBrowserPreferences::ScriptLog() const
	{
	return iAllPreferences.iScriptLog;
	}

// ---------------------------------------------------------
// CBrowserPreferences::HttpSecurityWarnings
// ---------------------------------------------------------
//
TBool CBrowserPreferences::HttpSecurityWarnings() const
	{
	return iAllPreferences.iHttpSecurityWarnings;
	}

// ---------------------------------------------------------
// CBrowserPreferences::IMEINotification
// ---------------------------------------------------------
//
TWmlSettingsIMEI CBrowserPreferences::IMEINotification() const
	{
	return iAllPreferences.iIMEINotification;
	}

// ---------------------------------------------------------
// CBrowserPreferences:::HttpSecurityWarningsStatSupressed
// ---------------------------------------------------------
//
TBool CBrowserPreferences::HttpSecurityWarningsStatSupressed() const
	{
	return iAllPreferences.iHTTPSecuritySupressed;
	}

// ---------------------------------------------------------
// CBrowserPreferences:::DownloadsOpen
// ---------------------------------------------------------
//
TBool CBrowserPreferences::DownloadsOpen() const
	{
	return iAllPreferences.iDownloadsOpen;
	}


// ---------------------------------------------------------
// CBrowserPreferences::QueryOnExit
// ---------------------------------------------------------
//
TBool CBrowserPreferences::QueryOnExit() const
    {
    return iAllPreferences.iQueryOnExit;
    }

// ---------------------------------------------------------
// CBrowserPreferences::SendReferrer
// ---------------------------------------------------------
//
TBool CBrowserPreferences::SendReferrer() const
    {
    return iAllPreferences.iSendReferrer;
    }

// ---------------------------------------------------------
// CBrowserPreferences::ShowConnectionDialogs()
// ---------------------------------------------------------
//
TBool CBrowserPreferences::ShowConnectionDialogs() const
	{	
	return iAllPreferences.iConnDialogs;
	}

// ---------------------------------------------------------
// CBrowserPreferences::MediaVolume()
// ---------------------------------------------------------
//
TInt CBrowserPreferences::MediaVolume() const
	{	
	return iAllPreferences.iMediaVolume;
	}

// ---------------------------------------------------------
// CBrowserPreferences::HomePageType()
// ---------------------------------------------------------
//
TWmlSettingsHomePage CBrowserPreferences::HomePageType() const
    {
    return iAllPreferences.iHomePgType;
    }

// ---------------------------------------------------------
// CBrowserPreferences::PopupBlocking()
// ---------------------------------------------------------
//
TBool CBrowserPreferences::PopupBlocking() const
    {
    return iAllPreferences.iPopupBlocking;
    }

// ---------------------------------------------------------
// CBrowserPreferences::FormDataSaving()
// ---------------------------------------------------------
//
TWmlSettingsFormData CBrowserPreferences::FormDataSaving() const
    {
    return iAllPreferences.iFormDataSaving;
    }
    
// ---------------------------------------------------------
// CBrowserPreferences::AccessKeys()
// ---------------------------------------------------------
//
TBool CBrowserPreferences::AccessKeys() const
    {
    return iAllPreferences.iAccessKeys;
    }
   
    
// ----------------------------------------------------------------------------
// CBrowserPreferences::AutomaticUpdatingAP
// ----------------------------------------------------------------------------
//
TUint CBrowserPreferences::AutomaticUpdatingAP() const
    {
    return iAllPreferences.iAutomaticUpdatingAP;        
    }
    
// ----------------------------------------------------------------------------
// CBrowserPreferences::AutomaticUpdatingWhileRoaming
// ----------------------------------------------------------------------------
//
TBool CBrowserPreferences::AutomaticUpdatingWhileRoaming() const
    {
    return iAllPreferences.iAutomaticUpdatingWhileRoaming;        
    }    
// ----------------------------------------------------------------------------
// CBrowserPreferences::ShortCutFuncHomePg
// ----------------------------------------------------------------------------
//
HBufC* CBrowserPreferences::ShortCutFuncHomePg() const
    {
    return iAllPreferences.iShortCutFuncHomePg;    
    }    

// ----------------------------------------------------------------------------
// CBrowserPreferences::ShortCutFuncBkMark
// ----------------------------------------------------------------------------
//
HBufC* CBrowserPreferences::ShortCutFuncBkMark() const
    {
    return iAllPreferences.iShortCutFuncBkMark;    
    }    

// ----------------------------------------------------------------------------
// CBrowserPreferences::ShortCutFuncFindKeyWord
// ----------------------------------------------------------------------------
//
HBufC* CBrowserPreferences::ShortCutFuncFindKeyWord() const
    {
    return iAllPreferences.iShortCutFuncFindKeyWord;    
    }    

// ----------------------------------------------------------------------------
// CBrowserPreferences::ShortCutFuncPrePage
// ----------------------------------------------------------------------------
//
HBufC* CBrowserPreferences::ShortCutFuncPrePage() const
    {
    return iAllPreferences.iShortCutFuncPrePage;    
    }    

// ----------------------------------------------------------------------------
// CBrowserPreferences::ShortCutFuncSwitchWin
// ----------------------------------------------------------------------------
//
HBufC* CBrowserPreferences::ShortCutFuncSwitchWin() const
    {
    return iAllPreferences.iShortCutFuncSwitchWin;    
    }    

// ----------------------------------------------------------------------------
// CBrowserPreferences::ShortCutFuncMiniImage
// ----------------------------------------------------------------------------
//
HBufC* CBrowserPreferences::ShortCutFuncMiniImage() const
    {
    return iAllPreferences.iShortCutFuncMiniImage;    
    }    

// ----------------------------------------------------------------------------
// CBrowserPreferences::ShortCutFuncFullScreen
// ----------------------------------------------------------------------------
//
HBufC* CBrowserPreferences::ShortCutFuncFullScreen() const
    {
    return iAllPreferences.iShortCutFuncFullScreen;    
    }   

// ----------------------------------------------------------------------------
// CBrowserPreferences::ShortCutFuncGoAddr
// ----------------------------------------------------------------------------
//
HBufC* CBrowserPreferences::ShortCutFuncGoAddr() const
    {
    return iAllPreferences.iShortCutFuncGoAddr;    
    }    

// ----------------------------------------------------------------------------
// CBrowserPreferences::ShortCutFuncZoomIn
// ----------------------------------------------------------------------------
//
HBufC* CBrowserPreferences::ShortCutFuncZoomIn() const
    {
    return iAllPreferences.iShortCutFuncZoomIn;    
    }    

// ----------------------------------------------------------------------------
// CBrowserPreferences::ShortCutFuncZoomOut
// ----------------------------------------------------------------------------
//
HBufC* CBrowserPreferences::ShortCutFuncZoomOut() const
    {
    return iAllPreferences.iShortCutFuncZoomOut;    
    }    

// ----------------------------------------------------------------------------
// CBrowserPreferences::ShortCutFuncZoomMode
// ----------------------------------------------------------------------------
//
HBufC* CBrowserPreferences::ShortCutFuncZoomMode() const
    {
    return iAllPreferences.iShortCutFuncZoomMode;    
    }    

// ---------------------------------------------------------
// CBrowserPreferences::RotateDisplay()
// ---------------------------------------------------------
//
TBool CBrowserPreferences::RotateDisplay() const
    {
    return iAllPreferences.iRotateDisplay;
    }


// ---------------------------------------------------------
// CBrowserPreferences::ShowToolbarOnOff()
// ---------------------------------------------------------
//
TInt CBrowserPreferences::ShowToolbarOnOff() const
    {
    return iAllPreferences.iToolbarOnOff;
    }    

   
// ---------------------------------------------------------
// CBrowserPreferences::ShowToolbarButton1Cmd()
// ---------------------------------------------------------
//
TInt CBrowserPreferences::ShowToolbarButton1Cmd() const
    {
    return iAllPreferences.iToolbarButton1Cmd;
    }    

   
// ---------------------------------------------------------
// CBrowserPreferences::ShowToolbarButton2Cmd()
// ---------------------------------------------------------
//
TInt CBrowserPreferences::ShowToolbarButton2Cmd() const
    {
    return iAllPreferences.iToolbarButton2Cmd;
    }    
   
// ---------------------------------------------------------
// CBrowserPreferences::ShowToolbarButton3Cmd()
// ---------------------------------------------------------
//
TInt CBrowserPreferences::ShowToolbarButton3Cmd() const
    {
    return iAllPreferences.iToolbarButton3Cmd;
    }    
   
// ---------------------------------------------------------
// CBrowserPreferences::ShowToolbarButton4Cmd()
// ---------------------------------------------------------
//
TInt CBrowserPreferences::ShowToolbarButton4Cmd() const
    {
    return iAllPreferences.iToolbarButton4Cmd;
    }    
   
// ---------------------------------------------------------
// CBrowserPreferences::ShowToolbarButton5Cmd()
// ---------------------------------------------------------
//
TInt CBrowserPreferences::ShowToolbarButton5Cmd() const
    {
    return iAllPreferences.iToolbarButton5Cmd;
    }    
   
// ---------------------------------------------------------
// CBrowserPreferences::ShowToolbarButton6Cmd()
// ---------------------------------------------------------
//
TInt CBrowserPreferences::ShowToolbarButton6Cmd() const
    {
    return iAllPreferences.iToolbarButton6Cmd;
    }    
   
// ---------------------------------------------------------
// CBrowserPreferences::ShowToolbarButton7Cmd()
// ---------------------------------------------------------
//
TInt CBrowserPreferences::ShowToolbarButton7Cmd() const
    {
    return iAllPreferences.iToolbarButton7Cmd;
    }    

// ---------------------------------------------------------
// CBrowserPreferences::ShortcutKey1Cmd()
// ---------------------------------------------------------
//
TInt CBrowserPreferences::ShortcutKey1Cmd() const
    {
    return iAllPreferences.iShortcutKey1Cmd;
    }    

// ---------------------------------------------------------
// CBrowserPreferences::ShortcutKey2Cmd()
// ---------------------------------------------------------
//
TInt CBrowserPreferences::ShortcutKey2Cmd() const
    {
    return iAllPreferences.iShortcutKey2Cmd;
    }    

// ---------------------------------------------------------
// CBrowserPreferences::ShortcutKey3Cmd()
// ---------------------------------------------------------
//
TInt CBrowserPreferences::ShortcutKey3Cmd() const
    {
    return iAllPreferences.iShortcutKey3Cmd;
    }    

// ---------------------------------------------------------
// CBrowserPreferences::ShortcutKey4Cmd()
// ---------------------------------------------------------
//
TInt CBrowserPreferences::ShortcutKey4Cmd() const
    {
    return iAllPreferences.iShortcutKey4Cmd;
    }    

// ---------------------------------------------------------
// CBrowserPreferences::ShortcutKey5Cmd()
// ---------------------------------------------------------
//
TInt CBrowserPreferences::ShortcutKey5Cmd() const
    {
    return iAllPreferences.iShortcutKey5Cmd;
    }    

// ---------------------------------------------------------
// CBrowserPreferences::ShortcutKey6Cmd()
// ---------------------------------------------------------
//
TInt CBrowserPreferences::ShortcutKey6Cmd() const
    {
    return iAllPreferences.iShortcutKey6Cmd;
    }    

// ---------------------------------------------------------
// CBrowserPreferences::ShortcutKey7Cmd()
// ---------------------------------------------------------
//
TInt CBrowserPreferences::ShortcutKey7Cmd() const
    {
    return iAllPreferences.iShortcutKey7Cmd;
    }    

// ---------------------------------------------------------
// CBrowserPreferences::ShortcutKey8Cmd()
// ---------------------------------------------------------
//
TInt CBrowserPreferences::ShortcutKey8Cmd() const
    {
    return iAllPreferences.iShortcutKey8Cmd;
    }    

// ---------------------------------------------------------
// CBrowserPreferences::ShortcutKey9Cmd()
// ---------------------------------------------------------
//
TInt CBrowserPreferences::ShortcutKey9Cmd() const
    {
    return iAllPreferences.iShortcutKey9Cmd;
    }    

// ---------------------------------------------------------
// CBrowserPreferences::ShortcutKey0Cmd()
// ---------------------------------------------------------
//
TInt CBrowserPreferences::ShortcutKey0Cmd() const
    {
    return iAllPreferences.iShortcutKey0Cmd;
    }    

// ---------------------------------------------------------
// CBrowserPreferences::ShortcutKeyStarCmd()
// ---------------------------------------------------------
//
TInt CBrowserPreferences::ShortcutKeyStarCmd() const
    {
    return iAllPreferences.iShortcutKeyStarCmd;
    }    

// ---------------------------------------------------------
// CBrowserPreferences::ShortcutKeyHashCmd()
// ---------------------------------------------------------
//
TInt CBrowserPreferences::ShortcutKeyHashCmd() const
    {
    return iAllPreferences.iShortcutKeyHashCmd;
    }    
 
// ---------------------------------------------------------
// CBrowserPreferences::ShortcutKeysForQwerty()
// ---------------------------------------------------------
//
TInt CBrowserPreferences::ShortcutKeysForQwerty() const
    {
    return iAllPreferences.iShortcutKeysForQwerty;
    }    
  
// ---------------------------------------------------------
// CBrowserPreferences::ZoomLevelMinimum()
// ---------------------------------------------------------
//
TUint32 CBrowserPreferences::ZoomLevelMinimum() const
    {
    return iAllPreferences.iZoomLevelMin;
    }    

// ---------------------------------------------------------
// CBrowserPreferences::ZoomLevelMaximum()
// ---------------------------------------------------------
//
TUint32 CBrowserPreferences::ZoomLevelMaximum() const
    {
    return iAllPreferences.iZoomLevelMax;
    }    

// ---------------------------------------------------------
// CBrowserPreferences::ZoomLevelDefault()
// ---------------------------------------------------------
//
TUint32 CBrowserPreferences::ZoomLevelDefault() const
    {
    return iAllPreferences.iZoomLevelDefault;
    }    
    

// ---------------------------------------------------------
// CBrowserPreferences::CursorShowMode()
// ---------------------------------------------------------
//
TBrCtlDefs::TCursorSettings CBrowserPreferences::CursorShowMode() const
    {
    return iAllPreferences.iCursorShowMode;
    }   
 
// ----------------------------------------------------------------------------
// CBrowserPreferences::URLSuffixList
// ----------------------------------------------------------------------------
//
HBufC* CBrowserPreferences::URLSuffixList() const
    {
    return iAllPreferences.iUrlSuffixList;    
    }    

// ----------------------------------------------------------------------------
// CBrowserPreferences::MaxRecentUrls
// ----------------------------------------------------------------------------
//
TInt CBrowserPreferences::MaxRecentUrls() const
    {
    return iAllPreferences.iMaxRecentUrlsToShow;    	
    }

// ----------------------------------------------------------------------------
// CBrowserPreferences::SearchFeature
// ----------------------------------------------------------------------------
//
TInt CBrowserPreferences::SearchFeature() const
    {
    return iAllPreferences.iSearch;        
    }

// ----------------------------------------------------------------------------
// CBrowserPreferences::ServiceFeature
// ----------------------------------------------------------------------------
//
TInt CBrowserPreferences::ServiceFeature() const
    {
    return iAllPreferences.iService;        
    }

// ---------------------------------------------------------
// CBrowserPreferences::EnterKeyMode()
// ---------------------------------------------------------
//
TBrCtlDefs::TEnterKeySettings CBrowserPreferences::EnterKeyMode() const
    {
    return iAllPreferences.iEnterKeyMode;
    }    

#endif

// End of File
