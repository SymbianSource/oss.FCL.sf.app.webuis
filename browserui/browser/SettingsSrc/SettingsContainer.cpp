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
*     Container of the information about the active settings
*
*/


// INCLUDE FILES
#include <bldvariant.hrh>
#include <charconv.h>
#include <aknkeys.h>
#include <AknTabGrp.h>
#include <AknViewAppUi.h>
#include <AknTitle.h>
#include <aknutils.h>
#include <BrowserNG.rsg>
#include <AKNLISTS.H>
#include <AknRadioButtonSettingPage.h>
#include <akntextsettingpage.h>
#include <barsread.h> // for TResourceReader
#include <gulicon.h>
#include <aknnavi.h>
#include <aknnavide.h>
#include <FeatMgr.h>
#include <ApSettingsHandlerUi.h>
#include <ApEngineConsts.h>
#include <aputils.h>
#include <commdb.h>
#include <StringLoader.h>
#include <downloadmgrclient.h>
#include <VpnApItem.h>
#include <aknnotewrappers.h>

#include "ApiProvider.h"
#include "SettingsContainer.h"
#include "BrowserPreferences.h"
#include "Display.h"
#include "CommsModel.h"
#include "Browser.hrh"
#include "CommonConstants.h"
#include "BrowserAppUi.h"
#include "BrowserAppViewBase.h"
#include "BrowserUtil.h"
#include "BrowserUiVariant.hrh"
#include "BrowserDialogs.h"
#include "Logger.h"

#include "BrowserWindowManager.h"
#include "BrowserWindow.h"
// Browser as a plugin
#include <brctlinterface.h>
#include <brctldefs.h>

#include <browserplugininterface.h>
//Zsolt
#include <cmmanager.h>
#include <cmdestination.h>
#include <cmapplicationsettingsui.h>


#ifdef __SERIES60_HELP
// Context-Sensitve Help File
#include "BrowserApplication.h"
#include <csxhelp/browser.hlp.hrh>
#endif // __SERIES60_HELP

// CONSTANTS
const TInt KSettingCategoryMaxLength = 50;
_LIT( KSettingsCategoryListBoxItemNoIcon, " " );
const TInt KDoesntExist = -1;
const TInt KGranularity = 1;
const TInt KMaxTitleLength = 512;


_LIT( KWmlSettingsListBoxItemPrefix, " \t" );
_LIT( KWmlSettingsListBoxItemPostfix, "\t\t" );

// ========================= MEMBER FUNCTIONS ================================

// -----------------------------------------------------------------------------
// CSettingsContainer::CSettingsContainer
// -----------------------------------------------------------------------------
//
CSettingsContainer::CSettingsContainer( MApiProvider& aApiProvider,
                                        MObjectProvider& aMopParent )
    : iApiProvider( aApiProvider ),
      iMainSettingCurrentIndex( KWmlSettingsIndexNone ),
      iSubSettingCurrentIndex( KWmlSettingsIndexNone )
    {
    SetMopParent( &aMopParent );
    }

// ----------------------------------------------------------------------------
// CSettingsContainer::NewLC
// ----------------------------------------------------------------------------
//
CSettingsContainer* CSettingsContainer::NewLC ( const TRect& aRect,
                                                MApiProvider& aApiProvider,
                                                MObjectProvider& aMopParent )
    {
    CSettingsContainer* result;
    result = new (ELeave) CSettingsContainer( aApiProvider, aMopParent );
    CleanupStack::PushL( result );
    result->ConstructL( aRect );
    return result;
    }


// ----------------------------------------------------------------------------
// CSettingsContainer::NewL
// ----------------------------------------------------------------------------
//
CSettingsContainer* CSettingsContainer::NewL (  const TRect &aRect,
                                                MApiProvider& aApiProvider,
                                                MObjectProvider& aMopParent )
    {
    CSettingsContainer* result;
    result = CSettingsContainer::NewLC( aRect, aApiProvider, aMopParent );
    CleanupStack::Pop(); // result
    return result;
    }


// -----------------------------------------------------------------------------
// CSettingsContainer::~CSettingsContainer
// -----------------------------------------------------------------------------
//
CSettingsContainer::~CSettingsContainer()
    {
    delete iNaviDecorator; // This pops off the Navi Pane (if pushed).
    delete iSettingListBox;
    delete iSettingIndex;
    delete iEncodingArray;
    }


// -----------------------------------------------------------------------------
// CSettingsContainer::ConstructL
// -----------------------------------------------------------------------------
//
void CSettingsContainer::ConstructL( const TRect& aRect )
    {
    ApiProvider().Display().SetTitleL( R_WMLBROWSER_SETTINGS_TITLE );
    // Push empty Navi Pane. Do not use PushDefault; that must be popped and
    // that is problematic on exit (MDisplay may not be available).
    // Create a blank navi pane control, its destructor handles it safely.
    CAknNavigationControlContainer* naviPane =
      ApiProvider().Display().NaviPaneL();
    iNaviDecorator = naviPane->CreateNavigationLabelL();
    naviPane->PushL( *iNaviDecorator );

    iSettingIndex = new (ELeave) CArrayFixFlat<TUint>( 4 );

    // construct listbox from resources
    TResourceReader rr;

    iEikonEnv->CreateResourceReaderLC( rr, R_BROWSER_SETTING_CATEGORIES_LISTBOX );

    CreateWindowL();
    iSettingListBox = new( ELeave )CAknSettingStyleListBox;
    iSettingListBox->SetContainerWindowL( *this ) ;
    iSettingListBox->ConstructFromResourceL( rr );
    iPenEnabled = AknLayoutUtils::PenEnabled();
    if (iPenEnabled)
        {
        iSettingListBox->SetListBoxObserver( this );    
        }
    CleanupStack::PopAndDestroy(); // resource reader
    iSettingListBox->CreateScrollBarFrameL( ETrue );
    iSettingListBox->ScrollBarFrame()->SetScrollBarVisibilityL( CEikScrollBarFrame::EOff, CEikScrollBarFrame::EAuto );

    iEncodingArray = new(ELeave) CArrayFixFlat<TEncodingStruct>( KGranularity );
    CreateEncodingArrayL();
    RemoveUnsupportedEncodingsL();

    // Add Unicode
    AddEncodingL( KCharacterSetIdentifierUCS2, R_WMLBROWSER_SETTINGS_ENCODING_UCS_2 );

    // Preferences becomes active, WindowsManager is an observer,
    // it provides some local settings
    ApiProvider().Preferences().NotifyObserversL( EPreferencesActivate, TBrCtlDefs::ESettingsUnknown );

    // These settings cause page relayouts and should only be
    // updated upon leaving the settings page, this is why they are stored as
    // member data rather than getting the values from CBrowserPreferences
    iFontSize = ApiProvider().Preferences().FontSize();
    iEncoding = ApiProvider().Preferences().Encoding();
    iTextWrap = ApiProvider().Preferences().TextWrap();

    // Check if flash plugin is present and set variable
    iFlashPluginPresent = ApiProvider().FlashPresent();
    
    // Updating listbox content
    DisplaySettingCategoriesL();

    SetRect( aRect );
    ActivateL();
    iSettingListBox->ScrollBarFrame()->MoveVertThumbTo( iSettingListBox->CurrentItemIndex() );
    }

// -----------------------------------------------------------------------------
// CSettingsContainer::DisplaySettingCategoriesL
// -----------------------------------------------------------------------------
//
void CSettingsContainer::DisplaySettingCategoriesL()
    {
    StoreListBoxIndexL();
    // Flag which setting category we are entering
    iCurrentSettingCategory = EMain;

    ApiProvider().Display().SetTitleL( R_WMLBROWSER_SETTINGS_TITLE );

    ClearListBoxContentL();

    MDesCArray* itemList = iSettingListBox->Model()->ItemTextArray();
    CDesCArray* itemArray = ( CDesCArray* ) itemList;
    TBuf<KSettingCategoryMaxLength> itemText;

    // General Category item
    AddCategoryListBoxItemL( R_SETTINGS_CATEGORY_GENERAL, *itemArray );
    iSettingIndex->AppendL( EWmlSettingsGeneral );

    // Page Category item
    AddCategoryListBoxItemL( R_SETTINGS_CATEGORY_PAGE, *itemArray );
    iSettingIndex->AppendL( EWmlSettingsPage );

    // Privacy Category item
    AddCategoryListBoxItemL( R_SETTINGS_CATEGORY_PRIVACY, *itemArray );
    iSettingIndex->AppendL( EWmlSettingsPrivacy );

#ifdef __RSS_FEEDS
    // Web feeds Category item
    if (!ApiProvider().IsEmbeddedModeOn())
        {
        AddCategoryListBoxItemL( R_SETTINGS_CATEGORY_WEBFEEDS, *itemArray );
        iSettingIndex->AppendL( EWmlSettingsWebFeeds );
        }
#endif // __RSS_FEEDS

    
    if (!iPenEnabled)
    {
        // Toolbar Category item
        AddCategoryListBoxItemL( R_SETTINGS_CATEGORY_TOOLBAR, *itemArray );
        iSettingIndex->AppendL( EWmlSettingsToolbar );

        // Shortcut Keys Category item
        if(!ApiProvider().Preferences().ShortcutKeysForQwerty())
            {
            AddCategoryListBoxItemL( R_SETTINGS_CATEGORY_SHORTCUTS, *itemArray );
            iSettingIndex->AppendL( EWmlSettingsShortcuts );        
            }
    }
        

    RestoreListBoxIndexL();
    
    }

// -----------------------------------------------------------------------------
// CSettingsContainer::DisplayCorrectSettingCategoryListL
// -----------------------------------------------------------------------------
//
void CSettingsContainer::DisplayCorrectSettingCategoryListL()
    {
    switch ( iSettingIndex->At( iSettingListBox->CurrentItemIndex() ) )
        {
        // Open General Browser Settings
        case EWmlSettingsGeneral:
            {
            DisplayGeneralSettingsL();
            break;
            }
        // Open Privacy Browser Settings
        case EWmlSettingsPrivacy:
            {
            DisplayPrivacySettingsL();
            break;
            }

        // Open Page Browser Settings
        case EWmlSettingsPage:
            {
            DisplayPageSettingsL();
            break;
            }

        // Open Web Feeds Settings
        case EWmlSettingsWebFeeds:
            {
            DisplayWebFeedsSettingsL();
            break;
            }
            
        // Open Toolbar Settings
        case EWmlSettingsToolbar:
            {
            DisplayToolbarSettingsL();
            break;
            }
            
         // Open Shortcuts Settings
        case EWmlSettingsShortcuts:
            {
            DisplayShortcutsSettingsL();
            break;
            }           

        default:
            {
            DisplaySettingCategoriesL();    
            break;
            }
        }
    }

// -----------------------------------------------------------------------------
// CSettingsContainer::AddCategoryListBoxItemL
// -----------------------------------------------------------------------------
//
void CSettingsContainer::AddCategoryListBoxItemL( TInt aResourceId, CDesCArray& aItemArray )
    {
    TBuf<KSettingCategoryMaxLength> itemText;
    itemText.Zero();
    itemText.Append( KSettingsCategoryListBoxItemNoIcon );
    itemText.Append(KWmlSettingsListBoxItemPrefix);
    HBufC* listItemText = iCoeEnv->AllocReadResourceLC( aResourceId );
    itemText.Append( *listItemText );
    CleanupStack::PopAndDestroy(); // listItemText
    aItemArray.AppendL( itemText );
    }

// -----------------------------------------------------------------------------
// CSettingsContainer::DisplayGeneralSettingsL
// -----------------------------------------------------------------------------
//
void CSettingsContainer::DisplayGeneralSettingsL()
    {
    StoreListBoxIndexL();
    // Flag which setting category we are entering
    iCurrentSettingCategory = EGeneral;

    ApiProvider().Display().SetTitleL( R_WMLBROWSER_SETTINGS_TITLE_GENERAL );

    ClearListBoxContentL();

    CDesCArray* itemArray =
            ( CDesCArray* ) ( iSettingListBox->Model()->ItemTextArray() );

    TBuf<KWmlSettingsItemMaxLength> itemText;

    //=========================================================================
    // Add individual settings belonging to the 'General' Settings Category
    //

    // Default Access Point
    if ( !ApiProvider().Preferences().CustomAccessPointDefined() )
        {
        AppendDefaultAccessPointL( itemArray, itemText );
        }

    // Home Page
    if( ApiProvider().Preferences().
                UiLocalFeatureSupported( KBrowserUiHomePageSetting ) &&
                !ApiProvider().IsEmbeddedModeOn() )
        {
        AppendHomePageL( itemArray, itemText );
        }

    // Minimap not supported on touch devices
    if (!iPenEnabled)
        {
        //MiniMap feature is determined by the PageScaler dll, not by the preference setting.
        //We should query webkit for the feature.
        if(ApiProvider().WindowMgr().CurrentWindow()->IsPageOverviewSupportedL()) 
            {
            // Page Overview
            AppendPageOverviewL( itemArray, itemText );
            }           
        }


    if( iApiProvider.Preferences().UiLocalFeatureSupported( KBrowserGraphicalHistory ))
        {
        // Back List
        AppendBackListL( itemArray, itemText );
        }

    if (!ApiProvider().IsEmbeddedModeOn())
        {
        AppendURLSuffixListL(itemArray, itemText);
        }
    // Http Security Warnings
    if ( !ApiProvider().Preferences().HttpSecurityWarningsStatSupressed() )
        {
        AppendHttpSecurityWarningsL( itemArray, itemText );
        }

    // Ecma
    AppendEcmaL( itemArray, itemText );

    // Script Logging
    AppendScriptLogL( itemArray, itemText );

    // Downloads open settings 
    if( PROGRESSIVE_DOWNLOAD )
        {
        AppendDownloadsOpenL( itemArray, itemText );
        }
    
    if( ApiProvider().Preferences().SearchFeature() )
        {
        AppendSearchProviderL( itemArray, itemText );
        }

    RestoreListBoxIndexL();
    iSettingListBox->DrawNow();
    ActivateL();
    }

// -----------------------------------------------------------------------------
// CSettingsContainer::DisplayPageSettingsL
// -----------------------------------------------------------------------------
//
void CSettingsContainer::DisplayPageSettingsL()
    {
    StoreListBoxIndexL();
    // Flag which setting category we are entering
    iCurrentSettingCategory = EPage;

    ApiProvider().Display().SetTitleL( R_WMLBROWSER_SETTINGS_TITLE_PAGE );

    ClearListBoxContentL();

    CDesCArray* itemArray =
            ( CDesCArray* ) ( iSettingListBox->Model()->ItemTextArray() );

    TBuf<KWmlSettingsItemMaxLength> itemText;

    //=========================================================================
    // Add individual settings belonging to the 'General' Settings Category
    //

    // AutoLoad Content
    AppendAutoLoadContentL( itemArray, itemText );
 
    // Media Volume
    if( !HAS_SIDE_VOLUME_KEYS )  //some phones don't have side volume keys
        {
        AppendAutoMediaVolumeL( itemArray, itemText );
        }

    // Encoding
    AppendEncodingL( itemArray, itemText );

    // Pop-up Blocking
    if ( ApiProvider().Preferences().UiLocalFeatureSupported(
                                    KBrowserMultipleWindows ) &&
                                    !ApiProvider().IsEmbeddedModeOn() )
        {
        AppendPopupBlockingL( itemArray, itemText );
        }

    // Auto Refresh
    AppendAutoRefreshL( itemArray, itemText );

    // Font Size
    AppendFontSizeL( itemArray, itemText );
 
    RestoreListBoxIndexL();
    iSettingListBox->DrawNow();
    ActivateL();
    }

// -----------------------------------------------------------------------------
// CSettingsContainer::DisplayPrivacySettingsL
// -----------------------------------------------------------------------------
//
void CSettingsContainer::DisplayPrivacySettingsL()
    {
    StoreListBoxIndexL();
    // Flag which setting category we are entering
    iCurrentSettingCategory = EPrivacy;

    ApiProvider().Display().SetTitleL( R_WMLBROWSER_SETTINGS_TITLE_PRIVACY );

    ClearListBoxContentL();

    CDesCArray* itemArray =
            ( CDesCArray* ) ( iSettingListBox->Model()->ItemTextArray() );

    TBuf<KWmlSettingsItemMaxLength> itemText;

    //=========================================================================
    // Add individual settings belonging to the 'General' Settings Category
    //

    // Adaptive Bookmarks
    if ( ADAPTIVEBOOKMARKS )
        {
        AppendAdaptiveBookmarksL( itemArray, itemText );
        }

    if (iApiProvider.Preferences().UiLocalFeatureSupported(KBrowserAutoFormFill) )
        {
        // Form Data Saving
        AppendFormDataSavingL( itemArray, itemText );
        }

    // Cookies
    AppendCookiesL( itemArray, itemText );

    // IMEI
    if ( IMEI_NOTIFICATION && !ApiProvider().IsEmbeddedModeOn() )
        {
        AppendIMEIL( itemArray, itemText );
        }

    RestoreListBoxIndexL();
    iSettingListBox->DrawNow();
    ActivateL();
    }

// -----------------------------------------------------------------------------
// CSettingsContainer::DisplayWebFeedsSettingsL
// -----------------------------------------------------------------------------
//
void CSettingsContainer::DisplayWebFeedsSettingsL()
    {
    MPreferences& preferences = ApiProvider().Preferences();

    StoreListBoxIndexL();
    // Flag which setting category we are entering
    iCurrentSettingCategory = EWebFeeds;
    ApiProvider().Display().SetTitleL( R_SETTINGS_CATEGORY_WEBFEEDS );
    ClearListBoxContentL();

    CDesCArray* itemArray =
            ( CDesCArray* ) ( iSettingListBox->Model()->ItemTextArray() );
    TBuf<KWmlSettingsItemMaxLength> itemText;

    // Add setting items
    // Show the "Auto Update AP" option
    AppendAutomaticUpdatingAPL( itemArray, itemText );
        
    // Show the "Automatic update while roaming" option
    AppendAutomaticUpdatingWhileRoamingL( itemArray, itemText );
    
    RestoreListBoxIndexL();
    iSettingListBox->DrawNow();
    ActivateL();
    }

// CSettingsContainer::DisplayToolbarSettingsL
// -----------------------------------------------------------------------------
//
void CSettingsContainer::DisplayToolbarSettingsL()
    {
    MPreferences& preferences = ApiProvider().Preferences();

    StoreListBoxIndexL();
    // Flag which setting category we are entering
    iCurrentSettingCategory = EToolbar;
    ApiProvider().Display().SetTitleL( R_SETTINGS_CATEGORY_TOOLBAR );
    ClearListBoxContentL();

    CDesCArray* itemArray =
            ( CDesCArray* ) ( iSettingListBox->Model()->ItemTextArray() );
    TBuf<KWmlSettingsItemMaxLength> itemText;
    
    // Toolbar On/Off Setting
    AppendToolbarOnOffL(itemArray, itemText );
    
    // Toolbar Buttons
    AppendToolbarButtonsL(itemArray, itemText );

    RestoreListBoxIndexL();
    iSettingListBox->DrawNow();
    ActivateL();
    }

// -----------------------------------------------------------------------------
// CSettingsContainer::DisplayShortcutsSettingsL
// -----------------------------------------------------------------------------
//
void CSettingsContainer::DisplayShortcutsSettingsL()
    {
    MPreferences& preferences = ApiProvider().Preferences();

    StoreListBoxIndexL();
    // Flag which setting category we are entering
    iCurrentSettingCategory = EShortCuts;
    ApiProvider().Display().SetTitleL( R_SETTINGS_CATEGORY_SHORTCUTS);
    ClearListBoxContentL();

    CDesCArray* itemArray =
            ( CDesCArray* ) ( iSettingListBox->Model()->ItemTextArray() );
    TBuf<KWmlSettingsItemMaxLength> itemText;
    
    // Shortcut Keys
    AppendShortcutKeysL (itemArray, itemText );

    RestoreListBoxIndexL();
    iSettingListBox->DrawNow();
    ActivateL();
    }

// -----------------------------------------------------------------------------
// CSettingsContainer::CloseSettingsListL
// -----------------------------------------------------------------------------
//
TBool CSettingsContainer::CloseSettingsListL()
    {
    TBool ret( EFalse );
    // Check which list we are in
    switch ( iCurrentSettingCategory )
        {
        // About to leave
        case EMain:
            {
            SaveChangesL();
            ret = ETrue;
            break;
            }
        // Return to Setting Category List
        case EGeneral:
        case EPrivacy:
        case EWebFeeds:
        case EPage:
        case EToolbar:
        case EShortCuts:
            {
            DisplaySettingCategoriesL();
            ret = EFalse;
            break;
            }
        default:
            break;
        }
    return ret;
    }
    
    
// -----------------------------------------------------------------------------
// CSettingsContainer::SettingsError
// -----------------------------------------------------------------------------
//
TBool CSettingsContainer::SettingsError()
    {
    TBool ret( EFalse );
    if ( iCurrentSettingCategory == EToolbar )
        {
        // Return true is all toolbar buttons set to None for command (which is 0 in value)
        MPreferences& preferences = ApiProvider().Preferences();
        if ((preferences.ShowToolbarButton1Cmd() +
             preferences.ShowToolbarButton2Cmd() +
             preferences.ShowToolbarButton3Cmd() +
             preferences.ShowToolbarButton4Cmd() +
             preferences.ShowToolbarButton5Cmd() +
             preferences.ShowToolbarButton6Cmd() +
             preferences.ShowToolbarButton7Cmd() ) == 0 )
            {
            ret = ETrue;
            }
        }
    return ret;
    }    

// -----------------------------------------------------------------------------
// CSettingsContainer::AppendDefaultAccessPointL
// -----------------------------------------------------------------------------
//
void CSettingsContainer::AppendDefaultAccessPointL( CDesCArray*& aItemArray,
                                                    TBuf<KWmlSettingsItemMaxLength>& aItemText )
    {
    //get the connection identifier based on the connection type
    switch (ApiProvider().Preferences().AccessPointSelectionMode())
        {
        case EAlwaysAsk:
            {
            #if !defined(BRDO_BROWSER_50_FF)        
                CreateItemFromTwoStringsL( 
                R_WMLBROWSER_SETTINGS_DEFAULT_AP, 
                R_WML_SETTINGS_ACCESS_POINT_ALWAYS_ASK, 
                aItemText );
            #else
                CreateItemFromTwoStringsL( 
                R_WMLBROWSER_SETTINGS_DEFAULT_AP, 
                R_WML_SETTINGS_ACCESS_POINT_ASK_WHEN_NEEDED, 
                aItemText );
            #endif //BRDO_BROWSER_50_FF
            break;  
            }
            
        //get the destination identifyer based on the snap Id
        case EDestination:
            {
            aItemText.Zero();
            aItemText.Append( KWmlSettingsListBoxItemPrefix );
            HBufC* defaultAp = iCoeEnv->AllocReadResourceLC( 
                                    R_WMLBROWSER_SETTINGS_DEFAULT_AP );
            aItemText.Append( *defaultAp );
            CleanupStack::PopAndDestroy(); // defaultAp
            aItemText.Append( KWmlSettingsListBoxItemPostfix );
            
            RCmManager        cmManager;
            cmManager.OpenL();

            TInt snapId = ApiProvider().Preferences().DefaultSnapId();
            if (snapId != KWmlNoDefaultSnapId)   
                {            
                TRAPD(err, RCmDestination dest = cmManager.DestinationL( snapId ));
                if (err == KErrNone)
                    {
                    RCmDestination dest = cmManager.DestinationL( snapId );                 
                    CleanupClosePushL( dest );                  
                    HBufC* destName = dest.NameLC();                                        
                    aItemText.Append(*destName);
                    CleanupStack::PopAndDestroy( 2 ); // destName, dest
                    cmManager.Close();
                    break;
                    }   
                }
            #if !defined(BRDO_BROWSER_50_FF)
                HBufC* name = iCoeEnv->AllocReadResourceLC( R_WML_SETTINGS_ACCESS_POINT_ALWAYS_ASK );
            #else
                HBufC* name = iCoeEnv->AllocReadResourceLC( R_WML_SETTINGS_ACCESS_POINT_ASK_WHEN_NEEDED );
            #endif // BRDO_BROWSER_50_FF
            aItemText.Append( *name );
            CleanupStack::PopAndDestroy(); // name
                
            ApiProvider().Preferences().SetAccessPointSelectionModeL( EAlwaysAsk );
            cmManager.Close();

            break;
            }
            
            //if connecting with Iap Id
        case EConnectionMethod:
            {
            aItemText.Zero();
            aItemText.Append( KWmlSettingsListBoxItemPrefix );
            HBufC* defaultAp = iCoeEnv->AllocReadResourceLC(
                                    R_WMLBROWSER_SETTINGS_DEFAULT_AP );
            aItemText.Append( *defaultAp );
            CleanupStack::PopAndDestroy(); // defaultAp
            aItemText.Append( KWmlSettingsListBoxItemPostfix );

            TUint id = ApiProvider().Preferences().DefaultAccessPoint();
            if ( id != KWmlNoDefaultAccessPoint )
                {
                TBuf< KCommsDbSvrMaxFieldLength > name;
                CApUtils* au = CApUtils::NewLC( ApiProvider().CommsModel().CommsDb() );
                au->NameL( id, name );
                aItemText.Append( name );
                CleanupStack::PopAndDestroy();  // au
                }
            else
                {
                #if !defined(BRDO_BROWSER_50_FF)
                    HBufC* name = iCoeEnv->AllocReadResourceLC( R_WML_SETTINGS_ACCESS_POINT_ALWAYS_ASK );
                #else
                    HBufC* name = iCoeEnv->AllocReadResourceLC( R_WML_SETTINGS_ACCESS_POINT_ASK_WHEN_NEEDED );
                #endif // BRDO_BROWSER_50_FF
                aItemText.Append( *name );
                CleanupStack::PopAndDestroy(); // name
                
                ApiProvider().Preferences().SetAccessPointSelectionModeL( EAlwaysAsk  );
                }

            break;  
            }

        default:
            {
            User::Leave( KErrNotSupported); 
            }
        }
    
    aItemArray->AppendL( aItemText );
    iSettingIndex->AppendL( EWmlSettingsAccesspoint );
    }

// -----------------------------------------------------------------------------
// CSettingsContainer::AppendHomePageL
// -----------------------------------------------------------------------------
//
void CSettingsContainer::AppendHomePageL( CDesCArray*& aItemArray,
                                          TBuf<KWmlSettingsItemMaxLength>& aItemText)
    {
LOG_ENTERFN( "CSettingsContainer::AppendHomePageL" );
    aItemText.Zero();
    aItemText.Append( KWmlSettingsListBoxItemPrefix );
    HBufC* first = iCoeEnv->AllocReadResourceLC(
                                 R_WMLBROWSER_SETTINGS_HOMEPAGE );
    aItemText.Append( first->Des() );
    CleanupStack::PopAndDestroy(); // first
    aItemText.Append( KWmlSettingsListBoxItemPostfix );

    TWmlSettingsHomePage homepageType = ApiProvider().Preferences().HomePageType();
BROWSER_LOG( ( _L( " pgtype: %d" ), homepageType ) );
    if( homepageType == EWmlSettingsHomePageBookmarks )
        {
        HBufC* second = iCoeEnv->AllocReadResourceLC(
                            R_WMLBROWSER_SETTINGS_HOMEPAGE_BOOKMARKS );
        aItemText.Append( second->Des() );
        CleanupStack::PopAndDestroy();  // second
        }
    else
        {
        HBufC* second = HBufC::NewLC( KMaxHomePgUrlLength );
        TPtr ptr( second->Des() );
        if( ApiProvider().Preferences().HomePageUrlL( ptr ) == KErrNone )
            {
            TInt itemLength( KWmlSettingsItemMaxLength - aItemText.Length() );
            if( ptr.Length() > itemLength )
                {
                aItemText.Append( ptr.Left( itemLength ) );
                }
            else
                {
                aItemText.Append( ptr );
                }
            }
        else
            {
            // If access point homepage is to be used, but no homepage URL is 
            // defined, then display the bookmarks view instead. 
            if( homepageType == EWmlSettingsHomePageAccessPoint )
                {
                iCoeEnv->ReadResourceL( ptr, 
                                    R_WMLBROWSER_SETTINGS_HOMEPAGE_BOOKMARKS );
                aItemText.Append( ptr );
                }
            else 
                {
                aItemText.Append( KWmlSettingsListBoxItemPostfix );
                }
            }
        CleanupStack::PopAndDestroy(); // second     
        }
    aItemArray->AppendL( aItemText );
    iSettingIndex->AppendL( EWmlSettingsHomePage );
    }

// -----------------------------------------------------------------------------
// CSettingsContainer::AppendPageOverviewL
// -----------------------------------------------------------------------------
//
void CSettingsContainer::AppendPageOverviewL( CDesCArray*& aItemArray,
                                              TBuf<KWmlSettingsItemMaxLength>& aItemText)
    {
    if ( iApiProvider.Preferences().PageOverview() )
        {
        CreateItemFromTwoStringsL(
        R_WMLBROWSER_SETTINGS_PAGEOVERVIEW,
        R_WMLBROWSER_SETTINGS_PAGEOVERVIEW_ON,
        aItemText );
        }
    else
        {
        CreateItemFromTwoStringsL(
            R_WMLBROWSER_SETTINGS_PAGEOVERVIEW,
            R_WMLBROWSER_SETTINGS_PAGEOVERVIEW_OFF,
            aItemText );
        }
    aItemArray->AppendL( aItemText );
    iSettingIndex->AppendL( EWmlSettingsPageOverview );
    }


// -----------------------------------------------------------------------------
// CSettingsContainer::AppendToolbarOnOffL
// -----------------------------------------------------------------------------
//
void CSettingsContainer::AppendToolbarOnOffL( CDesCArray*& aItemArray,
                                              TBuf<KWmlSettingsItemMaxLength>& aItemText)
    {
    if ( iApiProvider.Preferences().ShowToolbarOnOff() )
        {
        CreateItemFromTwoStringsL(
        R_BROWSER_SETTING_TOOLBAR_ON_OFF,
        R_BROWSER_SETTING_TOOLBAR_ON,
        aItemText );
        }
    else
        {
        CreateItemFromTwoStringsL(
            R_BROWSER_SETTING_TOOLBAR_ON_OFF,
            R_BROWSER_SETTING_TOOLBAR_OFF,
            aItemText );
        }
    aItemArray->AppendL( aItemText );
    iSettingIndex->AppendL( EWmlSettingsToolbarOnOff );
    }


// -----------------------------------------------------------------------------
// CSettingsContainer::AppendBackListL
// -----------------------------------------------------------------------------
//
void CSettingsContainer::AppendBackListL( CDesCArray*& aItemArray,
                                          TBuf<KWmlSettingsItemMaxLength>& aItemText)
    {
    //-------------------------------------------------------------------------
    // BackList
   if ( iApiProvider.Preferences().BackList() )
        {
        CreateItemFromTwoStringsL(
        R_WMLBROWSER_SETTINGS_BACK_LIST,
        R_WMLBROWSER_SETTINGS_BACK_LIST_ON,
        aItemText );
        }
    else
        {
        CreateItemFromTwoStringsL(
            R_WMLBROWSER_SETTINGS_BACK_LIST,
            R_WMLBROWSER_SETTINGS_BACK_LIST_OFF,
            aItemText );
        }
    aItemArray->AppendL( aItemText );
    iSettingIndex->AppendL( EWmlSettingsBackList );
    }

void CSettingsContainer::AppendURLSuffixListL( CDesCArray*& aItemArray, 
                                    TBuf<KWmlSettingsItemMaxLength>& aItemText)
    {
        aItemText.Zero();
        aItemText.Append( KWmlSettingsListBoxItemPrefix );
        HBufC* first = iCoeEnv->AllocReadResourceLC( R_WMLBROWSER_SETTINGS_URL_SUFFIX );
        aItemText.Append( first->Des() );
        CleanupStack::PopAndDestroy(); // first
        aItemText.Append( KWmlSettingsListBoxItemPostfix );

        HBufC* buf = iApiProvider.Preferences().URLSuffixList();
        TPtrC suffix(buf->Des());
        
        if ( !suffix.Length() )
            {
            // leave line empty
            aItemText.Append( KWmlSettingsListBoxItemPostfix );
            }
        else
            {
            TInt itemLength( KWmlSettingsItemMaxLength - aItemText.Length() );
            if( suffix.Length() > itemLength )
                {
                aItemText.Append( suffix.Left( itemLength ) );
                }
            else
                {
                aItemText.Append( suffix );
                }
            }
        
        aItemArray->AppendL( aItemText );
        iSettingIndex->AppendL( EWmlSettingsUrlSuffix );
    }
// -----------------------------------------------------------------------------
// CSettingsContainer::AppendHttpSecurityWarningsL
// -----------------------------------------------------------------------------
//
void CSettingsContainer::AppendHttpSecurityWarningsL(
                                                     CDesCArray*& aItemArray,
                                                     TBuf<KWmlSettingsItemMaxLength>& aItemText)
    {
    CreateItemFromTwoStringsL(
        R_WMLBROWSER_SETTINGS_HTTP_SEC_WRNS,
        ApiProvider().Preferences().HttpSecurityWarnings() ?
        R_WMLBROWSER_SETTINGS_WRN_VALUE_SHOW :
        R_WMLBROWSER_SETTINGS_WRN_VALUE_HIDE,
        aItemText );
    aItemArray->AppendL( aItemText );
    iSettingIndex->AppendL( EWmlSettingsHttpSecurityWarnings );
    }

// -----------------------------------------------------------------------------
// CSettingsContainer::AppendEcmaL
// -----------------------------------------------------------------------------
//
void CSettingsContainer::AppendEcmaL(
                                     CDesCArray*& aItemArray,
                                     TBuf<KWmlSettingsItemMaxLength>& aItemText)
    {
    // Add Ecma
    switch ( ApiProvider().Preferences().Ecma() )
        {
        case EWmlSettingsECMAEnable:
           {
           CreateItemFromTwoStringsL(
           R_WMLBROWSER_SETTINGS_ECMA,
           R_WMLBROWSER_SETTINGS_ECMA_VALUE_ENABLE,
           aItemText );
           break;
           }
        case EWmlSettingsECMADisable:
           {
           CreateItemFromTwoStringsL(
           R_WMLBROWSER_SETTINGS_ECMA,
           R_WMLBROWSER_SETTINGS_ECMA_VALUE_DISABLE,
           aItemText );
           break;
           }
        default:
        break;
        }
    aItemArray->AppendL( aItemText );
    iSettingIndex->AppendL( EWmlSettingsEcma );
    }

// -----------------------------------------------------------------------------
// CSettingsContainer::AppendScriptLogL
// -----------------------------------------------------------------------------
//
void CSettingsContainer::AppendScriptLogL(
                                         CDesCArray*& aItemArray,
                                         TBuf<KWmlSettingsItemMaxLength>& aItemText)
    {
    TInt scriptLog = ApiProvider().Preferences().ScriptLog();
    CRepository*  repo = CRepository::NewLC( KCRUidBrowser );
    TInt newVal;
    if ( repo )
        {
        if ( ( repo->Get( KBrowserNGScriptLog, newVal ) == KErrNone ) &&
              newVal != scriptLog )
            {
            ApiProvider().Preferences().SetScriptLogL( newVal );
            scriptLog = newVal;
            }
        }
    CleanupStack::PopAndDestroy( repo );

    switch ( scriptLog )
        {
        case EWmlSettingsScriptLogDisable:
            {
            CreateItemFromTwoStringsL(
                R_WMLBROWSER_SETTINGS_SCRIPTLOG,
                R_WMLBROWSER_SETTINGS_SCRIPTLOG_VALUE_DISABLE,
                aItemText );
            break;
            }
        case EWmlSettingsScriptLogToFile:
            {
            CreateItemFromTwoStringsL(
                R_WMLBROWSER_SETTINGS_SCRIPTLOG,
                R_WMLBROWSER_SETTINGS_SCRIPTLOG_VALUE_TO_FILE,
                aItemText );                    
            break;
            }
        case EWmlSettingsScriptLogToConsole:
            {
            CreateItemFromTwoStringsL(
                R_WMLBROWSER_SETTINGS_SCRIPTLOG,
                R_WMLBROWSER_SETTINGS_SCRIPTLOG_VALUE_TO_CONSOLE,
                aItemText );
            break;
            }  
        case EWmlSettingsScriptLogToConsoleFile:
            {
                CreateItemFromTwoStringsL(
                    R_WMLBROWSER_SETTINGS_SCRIPTLOG,
                    R_WMLBROWSER_SETTINGS_SCRIPTLOG_VALUE_TO_CONSOLE_FILE,
                    aItemText );
                break;
            }  
        default:
            CreateItemFromTwoStringsL(
                R_WMLBROWSER_SETTINGS_SCRIPTLOG,
                R_WMLBROWSER_SETTINGS_SCRIPTLOG_VALUE_DISABLE,
                aItemText );
            break;
        }    

    aItemArray->AppendL( aItemText );
    iSettingIndex->AppendL( EWmlSettingsScriptLog );
    }


// -----------------------------------------------------------------------------
// CSettingsContainer::AppendDownloadsOpenL
// -----------------------------------------------------------------------------
void CSettingsContainer::AppendDownloadsOpenL(
                                                     CDesCArray*& aItemArray,
                                                     TBuf<KWmlSettingsItemMaxLength>& aItemText)
    {
    CreateItemFromTwoStringsL(
        R_WMLBROWSER_SETTINGS_DOWNLOAD_OPEN,
        ApiProvider().Preferences().DownloadsOpen() ?
        R_WMLBROWSER_SETTINGS_DOWNLOAD_OPEN_YES :
        R_WMLBROWSER_SETTINGS_DOWNLOAD_OPEN_NO,
        aItemText );
    aItemArray->AppendL( aItemText );
    iSettingIndex->AppendL( EWmlSettingsDownloadsOpen );
    }



// -----------------------------------------------------------------------------
// CSettingsContainer::AppendSearchProviderL
// -----------------------------------------------------------------------------
void CSettingsContainer::AppendSearchProviderL(
                                                      CDesCArray*& aItemArray,
                                                      TBuf<KWmlSettingsItemMaxLength>& aItemText)
    {
    aItemText.Zero();
    aItemText.Append( KWmlSettingsListBoxItemPrefix );
    HBufC* settingTitle = iCoeEnv->AllocReadResourceLC( R_BROWSERS_SETT_WEB_SEARCH_PROVIDER );
    aItemText.Append( *settingTitle );
    CleanupStack::PopAndDestroy(); // settingTitle
    aItemText.Append( KWmlSettingsListBoxItemPostfix );
    
    HBufC* searchProvider = HBufC::NewLC( KMaxTitleLength );
    TPtr searchProviderPtr = searchProvider->Des();
    
    ApiProvider().Preferences().GetStringValueL( KBrowserSearchProviderTitle,
            KMaxTitleLength , searchProviderPtr);
    
    if(searchProvider->Compare(KNullDesC()) == 0)
        {
        CleanupStack::PopAndDestroy(searchProvider);
        searchProvider = iCoeEnv->AllocReadResourceLC( R_IS_LABEL_NOT_SELECTED );
        }
    
    aItemText.Append( *searchProvider );
    CleanupStack::PopAndDestroy( searchProvider );
    
    aItemArray->AppendL( aItemText );
    iSettingIndex->AppendL( EWmlSettingsSearchProvider );
    }

// -----------------------------------------------------------------------------
// CSettingsContainer::AppendAutoLoadContentL
// -----------------------------------------------------------------------------
//
void CSettingsContainer::AppendAutoLoadContentL(
                                               CDesCArray*& aItemArray,
                                               TBuf<KWmlSettingsItemMaxLength>& aItemText)
    {
   
    TInt autoLoadContent = ApiProvider().Preferences().AutoLoadContent();
      
    switch ( autoLoadContent )
        {
        case EWmlSettingsAutoloadText:
            {
            CreateItemFromTwoStringsL(
                R_WMLBROWSER_SETTINGS_AUTOLOAD_CONTENT,
                R_WMLBROWSER_SETTINGS_AUTOLOAD_TEXT,
                aItemText );
            break;
            }
        case EWmlSettingsAutoloadImagesNoFlash:
            {
            if (iFlashPluginPresent)
                {
                CreateItemFromTwoStringsL(
                    R_WMLBROWSER_SETTINGS_AUTOLOAD_CONTENT,
                    R_WMLBROWSER_SETTINGS_AUTOLOAD_IMAGES_NO_FLASH,
                    aItemText );                    
                }
            else
                {
                CreateItemFromTwoStringsL(
                    R_WMLBROWSER_SETTINGS_AUTOLOAD_CONTENT,
                    R_WMLBROWSER_SETTINGS_AUTOLOAD_ALL,
                    aItemText );                    
                }
            break;
            }
        case EWmlSettingsAutoloadAll:
            {
            CreateItemFromTwoStringsL(
                R_WMLBROWSER_SETTINGS_AUTOLOAD_CONTENT,
                R_WMLBROWSER_SETTINGS_AUTOLOAD_ALL,
                aItemText );
            break;
            }  
        default:
            break;
        }    

    aItemArray->AppendL( aItemText );
    iSettingIndex->AppendL( EWmlSettingsAutoLoadContent );
    }
 

// -----------------------------------------------------------------------------
// CSettingsContainer::AppendAutoFullScreenL
// -----------------------------------------------------------------------------
//
void CSettingsContainer::AppendAutoFullScreenL(CDesCArray*& aItemArray,
                                               TBuf<KWmlSettingsItemMaxLength>& aItemText)
    {
    switch ( ApiProvider().Preferences().FullScreen() )
        {
        case EWmlSettingsFullScreenSoftkeysOnly:
            {
            CreateItemFromTwoStringsL(
                    R_WMLBROWSER_SETTINGS_SOFTKEYS,
                    R_WMLBROWSER_SETTINGS_SOFTKEYS_ON,
                    aItemText );
            break;
            }
        case EWmlSettingsFullScreenFullScreen:
            {
            CreateItemFromTwoStringsL(
                    R_WMLBROWSER_SETTINGS_SOFTKEYS,
                    R_WMLBROWSER_SETTINGS_SOFTKEYS_OFF,
                    aItemText );
            break;
            }        
        default:
            break;
        }

    aItemArray->AppendL( aItemText );
    iSettingIndex->AppendL( EWmlSettingsFullScreen );
    }

// -----------------------------------------------------------------------------
// CSettingsContainer::AppendAutomaticUpdatingAPL
// -----------------------------------------------------------------------------
//
void CSettingsContainer::AppendAutomaticUpdatingAPL(
                                            CDesCArray*& aItemArray,
                                            TBuf<KWmlSettingsItemMaxLength>& aItemText)
    {
    TUint id = ApiProvider().Preferences().AutomaticUpdatingAP();
    aItemText.Zero();
    aItemText.Append( KWmlSettingsListBoxItemPrefix );
    HBufC* defaultAp = iCoeEnv->AllocReadResourceLC(
                            R_WMLBROWSER_SETTINGS_AUTOUPDATING_AP );
    aItemText.Append( *defaultAp );
    CleanupStack::PopAndDestroy(); // defaultAp
    aItemText.Append( KWmlSettingsListBoxItemPostfix );
    
    if ( id == -1 )
        {
        // If user hasn't selected an AP, show a blank list entry. Once user
        // selects an AP, it will be displayed.  Note: The browser's default AP
        // is used, until user makes a different AP selection.
        aItemText.Append( KNullDesC );
        }
    else
        {
        // User (or configuration) has already selected an AP, display its name
        TBuf< KCommsDbSvrMaxFieldLength > name;
        CApUtils* au = CApUtils::NewLC( ApiProvider().CommsModel().CommsDb() );
        TRAP_IGNORE(au->NameL( id, name ));
        aItemText.Append( name );
        CleanupStack::PopAndDestroy();  // au
        }

    aItemArray->AppendL( aItemText );
    iSettingIndex->AppendL( EWmlSettingsAutomaticUpdatingAP );
    }

// -----------------------------------------------------------------------------
// CSettingsContainer::AppendAutomaticUpdatingWhileRoamingL
// -----------------------------------------------------------------------------
//
void CSettingsContainer::AppendAutomaticUpdatingWhileRoamingL(
                                            CDesCArray*& aItemArray,
                                            TBuf<KWmlSettingsItemMaxLength>& aItemText)
    {
    
    if ( iApiProvider.Preferences().AutomaticUpdatingWhileRoaming() )
        {
        CreateItemFromTwoStringsL(
        R_WMLBROWSER_SETTINGS_AUTOUPDATING_ROAMING,
        R_WMLBROWSER_SETTINGS_AUTOUPDATING_ROAMING_ON,
        aItemText );
        }
    else
        {
        CreateItemFromTwoStringsL(
            R_WMLBROWSER_SETTINGS_AUTOUPDATING_ROAMING,
            R_WMLBROWSER_SETTINGS_AUTOUPDATING_ROAMING_OFF,
            aItemText );
        }
    aItemArray->AppendL( aItemText );
    
    iSettingIndex->AppendL( EWmlSettingsAutomaticUpdatingWhileRoaming );
    }
    
// -----------------------------------------------------------------------------
// CSettingsContainer::AppendAutoMediaVolumeL
// -----------------------------------------------------------------------------
//
void CSettingsContainer::AppendAutoMediaVolumeL(
                                                CDesCArray*& aItemArray,
                                                TBuf<KWmlSettingsItemMaxLength>& aItemText)
    {
    TInt volumeLevel = ApiProvider().Preferences().MediaVolume();

    switch ( volumeLevel )
        {
        case EWmlSettingsVolumeMuted:
            {
            CreateItemFromTwoStringsL(
                R_WMLBROWSER_SETTINGS_MEDIA_VOLUME,
                R_WMLBROWSER_SETTINGS_MEDIA_VOLUME_MUTED,
                aItemText );
            break;
            }
        case EWmlSettingsVolume1:
            {
            CreateItemFromTwoStringsL(
                R_WMLBROWSER_SETTINGS_MEDIA_VOLUME,
                R_WMLBROWSER_SETTINGS_MEDIA_VOLUME_1,
                aItemText );
            break;
            }
        case EWmlSettingsVolume2:
            {
            CreateItemFromTwoStringsL(
                R_WMLBROWSER_SETTINGS_MEDIA_VOLUME,
                R_WMLBROWSER_SETTINGS_MEDIA_VOLUME_2,
                aItemText );
            break;
            }
        case EWmlSettingsVolume3:
            {
            CreateItemFromTwoStringsL(
                R_WMLBROWSER_SETTINGS_MEDIA_VOLUME,
                R_WMLBROWSER_SETTINGS_MEDIA_VOLUME_3,
                aItemText );
            break;
            }
        case EWmlSettingsVolume4:
            {
            CreateItemFromTwoStringsL(
                R_WMLBROWSER_SETTINGS_MEDIA_VOLUME,
                R_WMLBROWSER_SETTINGS_MEDIA_VOLUME_4,
                aItemText );
            break;
            }
        default:
            break;
        }

    aItemArray->AppendL( aItemText );
    iSettingIndex->AppendL( EWmlSettingsMediaVolume );
    }

// -----------------------------------------------------------------------------
// CSettingsContainer::AppendEncodingL
// -----------------------------------------------------------------------------
//
void CSettingsContainer::AppendEncodingL(
                                         CDesCArray*& aItemArray,
                                         TBuf<KWmlSettingsItemMaxLength>& aItemText)
    {
    TBool currentEncodingFound = EFalse;
    for( int i = 0; i < iEncodingArray->Count(); i++ )
        {
        if( iEncodingArray->At(i).iSystemId == iEncoding )
            {
            CreateItemFromTwoStringsL(
                        R_WMLBROWSER_SETTINGS_ENCODING,
                        iEncodingArray->At(i).iResId,
                        aItemText );
            currentEncodingFound = ETrue;
            break;
            }
        }

    if( !currentEncodingFound )
        {
        __ASSERT_DEBUG( EFalse, Util::Panic( Util::EOutOfRange ));
        }

    aItemArray->AppendL( aItemText );
    iSettingIndex->AppendL( EWmlSettingsEncoding );
    }

// -----------------------------------------------------------------------------
// CSettingsContainer::AppendPopupBlockingL
// -----------------------------------------------------------------------------
//
void CSettingsContainer::AppendPopupBlockingL(
                                              CDesCArray*& aItemArray,
                                              TBuf<KWmlSettingsItemMaxLength>& aItemText)
    {
    if ( ApiProvider().Preferences().PopupBlocking() )
        {
        CreateItemFromTwoStringsL(
            R_WMLBROWSER_SETTINGS_POPUP_BLOCKING,
            R_WMLBROWSER_SETTINGS_POPUP_BLOCKING_ON,
            aItemText );
        }
    else
        {
        CreateItemFromTwoStringsL(
            R_WMLBROWSER_SETTINGS_POPUP_BLOCKING,
            R_WMLBROWSER_SETTINGS_POPUP_BLOCKING_OFF,
            aItemText );
        }
    aItemArray->AppendL( aItemText );
    iSettingIndex->AppendL( EWmlSettingsPopupBlocking );
    }

// -----------------------------------------------------------------------------
// CSettingsContainer::AppendAutoRefreshL
// -----------------------------------------------------------------------------
//
void CSettingsContainer::AppendAutoRefreshL(
                                            CDesCArray*& aItemArray,
                                            TBuf<KWmlSettingsItemMaxLength>& aItemText)
    {
    if ( iApiProvider.Preferences().AutoRefresh() )
        {
        CreateItemFromTwoStringsL(
        R_WMLBROWSER_SETTINGS_AUTO_REFRESH,
        R_WMLBROWSER_SETTINGS_AUTO_REFRESH_ON,
        aItemText );
        }
    else
        {
        CreateItemFromTwoStringsL(
            R_WMLBROWSER_SETTINGS_AUTO_REFRESH,
            R_WMLBROWSER_SETTINGS_AUTO_REFRESH_OFF,
            aItemText );
        }
    aItemArray->AppendL( aItemText );
    iSettingIndex->AppendL( EWmlSettingsAutoRefresh );
    }

// -----------------------------------------------------------------------------
// CSettingsContainer::AppendAdaptiveBookmarksL
// -----------------------------------------------------------------------------
//
void CSettingsContainer::AppendAdaptiveBookmarksL(
                                                  CDesCArray*& aItemArray,
                                                  TBuf<KWmlSettingsItemMaxLength>& aItemText)
    {
    switch ( ApiProvider().Preferences().AdaptiveBookmarks() )
        {
        case EWmlSettingsAdaptiveBookmarksOn:
            {
            CreateItemFromTwoStringsL(
                R_WMLBROWSER_SETTINGS_ADAPTIVE_BOOKMARKS,
                R_WMLBROWSER_SETTINGS_ADAPTIVE_BOOKMARKS_VALUE_ON,
                aItemText );
            break;
            }
        case EWmlSettingsAdaptiveBookmarksHideFolder:
            {
            CreateItemFromTwoStringsL(
                    R_WMLBROWSER_SETTINGS_ADAPTIVE_BOOKMARKS,
                    R_WMLBROWSER_SETTINGS_ADAPTIVE_BOOKMARKS_VALUE_HIDE_FOLDER,
                    aItemText );
            break;
            }
        case EWmlSettingsAdaptiveBookmarksOff:
            {
            CreateItemFromTwoStringsL(
              R_WMLBROWSER_SETTINGS_ADAPTIVE_BOOKMARKS,
              R_WMLBROWSER_SETTINGS_ADAPTIVE_BOOKMARKS_VALUE_OFF,
              aItemText );
            break;
            }
        default:
            break;
        }

    aItemArray->AppendL( aItemText );
    iSettingIndex->AppendL( EWmlSettingsAdaptiveBookmarks );
    }

// -----------------------------------------------------------------------------
// CSettingsContainer::AppendFormDataSavingL
// -----------------------------------------------------------------------------
//
void CSettingsContainer::AppendFormDataSavingL(
                                               CDesCArray*& aItemArray,
                                               TBuf<KWmlSettingsItemMaxLength>& aItemText)
    {
    switch ( ApiProvider().Preferences().FormDataSaving() )
        {
        case EWmlSettingsFormDataPlusPassword:
            {
            CreateItemFromTwoStringsL(
                R_WMLBROWSER_SETTINGS_FORM_DATA_SAVING,
                R_WMLBROWSER_SETTINGS_FORM_DATA_SAVING_ON,
                aItemText );
            break;
            }
        case EWmlSettingsFormDataOnly:
            {
            CreateItemFromTwoStringsL(
                R_WMLBROWSER_SETTINGS_FORM_DATA_SAVING,
                R_WMLBROWSER_SETTINGS_FORM_DATA_SAVING_ONLY,
                aItemText );
            break;
            }
        case EWmlSettingsFormDataOff:
            {
            CreateItemFromTwoStringsL(
                R_WMLBROWSER_SETTINGS_FORM_DATA_SAVING,
                R_WMLBROWSER_SETTINGS_FORM_DATA_SAVING_OFF,
                aItemText );
            break;
            }

        default:
            break;
        }
    aItemArray->AppendL( aItemText );
    iSettingIndex->AppendL( EWmlSettingsFormDataSaving );
    }

// -----------------------------------------------------------------------------
// CSettingsContainer::AppendCookiesL
// -----------------------------------------------------------------------------
//
void CSettingsContainer::AppendCookiesL(
                                        CDesCArray*& aItemArray,
                                        TBuf<KWmlSettingsItemMaxLength>& aItemText)
    {
    switch ( ApiProvider().Preferences().Cookies() )
        {
        case EWmlSettingsCookieAllow:
            {
            CreateItemFromTwoStringsL(
                  R_WMLBROWSER_SETTINGS_COOKIES,
                  R_WMLBROWSER_SETTINGS_C_VALUE_ALLOW,
                  aItemText );
            break;
            }
        case EWmlSettingsCookieReject:
            {
            CreateItemFromTwoStringsL(
                  R_WMLBROWSER_SETTINGS_COOKIES,
                  R_WMLBROWSER_SETTINGS_C_VALUE_REJECT,
                  aItemText );
            break;
            }
        default:
            break;
        }

    aItemArray->AppendL( aItemText );
    iSettingIndex->AppendL( EWmlSettingsCookies );
    }

// -----------------------------------------------------------------------------
// CSettingsContainer::AppendIMEIL
// -----------------------------------------------------------------------------
//
void CSettingsContainer::AppendIMEIL(
                                     CDesCArray*& aItemArray,
                                     TBuf<KWmlSettingsItemMaxLength>& aItemText)
    {
    switch ( ApiProvider().Preferences().IMEINotification() )
        {
        case EWmlSettingsIMEIEnable:
            CreateItemFromTwoStringsL(
                R_WMLBROWSER_SETTINGS_IMEI,
                R_WMLBROWSER_SETTINGS_IMEI_VALUE_ENABLED,
                aItemText );
            break;
        case EWmlSettingsIMEIDisable:
            CreateItemFromTwoStringsL(
                R_WMLBROWSER_SETTINGS_IMEI,
                R_WMLBROWSER_SETTINGS_IMEI_VALUE_DISABLED,
                aItemText );
            break;
        default:
            break;
        }
    aItemArray->AppendL( aItemText );
    iSettingIndex->AppendL( EWmlSettingsIMEINotification );

    }
// -----------------------------------------------------------------------------
// CSettingsContainer::AppendFontSize
// -----------------------------------------------------------------------------
//
void CSettingsContainer::AppendFontSizeL( CDesCArray*& aItemArray,
                                         TBuf<KWmlSettingsItemMaxLength>& aItemText)
    {
    CreateItemFromTwoStringsL(R_WMLBROWSER_SETTINGS_FONT_SIZE,
                              GetFontSizeString(),
                              aItemText );
    aItemArray->AppendL( aItemText );
    iSettingIndex->AppendL( EWmlSettingsFontSize );
    }


// -----------------------------------------------------------------------------
// CSettingsContainer::GetFontSizeString
//   Convenience utility. Note that many of the settings duplicate the
//   code to generate this sort od string. This pattern could be applied
//   to many of the settings to reduce the code size. A side effect of this
//   code is that the iFontSize variable may be modified in the Chinese
//   Build case. This code is in one place now and not duplicated.
// -----------------------------------------------------------------------------
//
TInt CSettingsContainer::GetFontSizeString()
   {
    // China build (__AVKON_APAC__) has only three font sizes.
    // If for some reason smaller/larger is selected, display it as "normal"
    if (!AVKONAPAC)
        {
        switch (iFontSize)
            {
            case 0:
              return R_WMLBROWSER_SETTINGS_FS_VALUE_ALL_SMALL;
            case 1:
              return R_WMLBROWSER_SETTINGS_FS_VALUE_SMALLER;
            case 2:
              return R_WMLBROWSER_SETTINGS_FS_VALUE_NORMAL;
            case 3:
              return R_WMLBROWSER_SETTINGS_FS_VALUE_LARGER;
            case 4:
              return R_WMLBROWSER_SETTINGS_FS_VALUE_ALL_LARGE;
            default:
              return R_WMLBROWSER_SETTINGS_FS_VALUE_NORMAL;
            }
        }
    else
        {
        switch (iFontSize)
            {
            case 0:
              return R_WMLBROWSER_SETTINGS_FS_VALUE_ALL_SMALL;
            case 1:
            case 2:
            case 3:
              //These three map to same size.
              iFontSize = 2;
              return R_WMLBROWSER_SETTINGS_FS_VALUE_NORMAL;
            case 4:
              return R_WMLBROWSER_SETTINGS_FS_VALUE_ALL_LARGE;
            default:
              return R_WMLBROWSER_SETTINGS_FS_VALUE_NORMAL;
            }
        }
   }

// -----------------------------------------------------------------------------
// CSettingsContainer::AppendToolbarButtonsL
// -----------------------------------------------------------------------------
//
void CSettingsContainer::AppendToolbarButtonsL( CDesCArray*& aItemArray,
                                                TBuf<KWmlSettingsItemMaxLength>& aItemText)
    {
   
    CreateItemFromTwoStringsL(
        R_BROWSER_SETTING_TOOLBAR_BUTTON1,
        ToolbarCommandDecodeMatrix[iApiProvider.Preferences().ShowToolbarButton1Cmd()][KToolbarCommandDecodeMatrixStringId],
        aItemText );
    aItemArray->AppendL( aItemText );
    iSettingIndex->AppendL( EWmlSettingsToolbarButton1Cmd );    
    
    CreateItemFromTwoStringsL(
        R_BROWSER_SETTING_TOOLBAR_BUTTON2,
        ToolbarCommandDecodeMatrix[iApiProvider.Preferences().ShowToolbarButton2Cmd()][KToolbarCommandDecodeMatrixStringId],
        aItemText );
    aItemArray->AppendL( aItemText );
    iSettingIndex->AppendL( EWmlSettingsToolbarButton2Cmd );     
    
    CreateItemFromTwoStringsL(
        R_BROWSER_SETTING_TOOLBAR_BUTTON3,
        ToolbarCommandDecodeMatrix[iApiProvider.Preferences().ShowToolbarButton3Cmd()][KToolbarCommandDecodeMatrixStringId],
        aItemText );
    aItemArray->AppendL( aItemText );
    iSettingIndex->AppendL( EWmlSettingsToolbarButton3Cmd );     
    
    CreateItemFromTwoStringsL(
        R_BROWSER_SETTING_TOOLBAR_BUTTON4,
        ToolbarCommandDecodeMatrix[iApiProvider.Preferences().ShowToolbarButton4Cmd()][KToolbarCommandDecodeMatrixStringId],
        aItemText );
    aItemArray->AppendL( aItemText );
    iSettingIndex->AppendL( EWmlSettingsToolbarButton4Cmd );     
    
    CreateItemFromTwoStringsL(
        R_BROWSER_SETTING_TOOLBAR_BUTTON5,
        ToolbarCommandDecodeMatrix[iApiProvider.Preferences().ShowToolbarButton5Cmd()][KToolbarCommandDecodeMatrixStringId],
        aItemText );
    aItemArray->AppendL( aItemText );
    iSettingIndex->AppendL( EWmlSettingsToolbarButton5Cmd );     
     
    
    CreateItemFromTwoStringsL(
        R_BROWSER_SETTING_TOOLBAR_BUTTON6,
        ToolbarCommandDecodeMatrix[iApiProvider.Preferences().ShowToolbarButton6Cmd()][KToolbarCommandDecodeMatrixStringId],
        aItemText );
    aItemArray->AppendL( aItemText );
    iSettingIndex->AppendL( EWmlSettingsToolbarButton6Cmd );     
    
    
    CreateItemFromTwoStringsL(
        R_BROWSER_SETTING_TOOLBAR_BUTTON7,
        ToolbarCommandDecodeMatrix[iApiProvider.Preferences().ShowToolbarButton7Cmd()][KToolbarCommandDecodeMatrixStringId],
        aItemText );
    aItemArray->AppendL( aItemText );
    iSettingIndex->AppendL( EWmlSettingsToolbarButton7Cmd );
    }

// -----------------------------------------------------------------------------
// CSettingsContainer::AppendShortcutKeysL
// -----------------------------------------------------------------------------
//
void CSettingsContainer::AppendShortcutKeysL( CDesCArray*& aItemArray,
                                                TBuf<KWmlSettingsItemMaxLength>& aItemText)
    {
   
    CreateItemFromTwoStringsL(
        R_BROWSER_SETTING_SHORTCUT_KEY1,
        ShortcutsCommandDecodeMatrix[iApiProvider.Preferences().ShortcutKey1Cmd()][KShortcutsCommandDecodeMatrixStringId],
        aItemText );
    aItemArray->AppendL( aItemText );
    iSettingIndex->AppendL( EWmlSettingsShortCutKey1Cmd );    
    

    CreateItemFromTwoStringsL(
        R_BROWSER_SETTING_SHORTCUT_KEY2,
        ShortcutsCommandDecodeMatrix[iApiProvider.Preferences().ShortcutKey2Cmd()][KShortcutsCommandDecodeMatrixStringId],
        aItemText );
    aItemArray->AppendL( aItemText );
    iSettingIndex->AppendL( EWmlSettingsShortCutKey2Cmd );    


    CreateItemFromTwoStringsL(
        R_BROWSER_SETTING_SHORTCUT_KEY3,
        ShortcutsCommandDecodeMatrix[iApiProvider.Preferences().ShortcutKey3Cmd()][KShortcutsCommandDecodeMatrixStringId],
        aItemText );
    aItemArray->AppendL( aItemText );
    iSettingIndex->AppendL( EWmlSettingsShortCutKey3Cmd );    

    CreateItemFromTwoStringsL(
        R_BROWSER_SETTING_SHORTCUT_KEY4,
        ShortcutsCommandDecodeMatrix[iApiProvider.Preferences().ShortcutKey4Cmd()][KShortcutsCommandDecodeMatrixStringId],
        aItemText );
    aItemArray->AppendL( aItemText );
    iSettingIndex->AppendL( EWmlSettingsShortCutKey4Cmd );    

    CreateItemFromTwoStringsL(
        R_BROWSER_SETTING_SHORTCUT_KEY5,
        ShortcutsCommandDecodeMatrix[iApiProvider.Preferences().ShortcutKey5Cmd()][KShortcutsCommandDecodeMatrixStringId],
        aItemText );
    aItemArray->AppendL( aItemText );
    iSettingIndex->AppendL( EWmlSettingsShortCutKey5Cmd );    


    CreateItemFromTwoStringsL(
        R_BROWSER_SETTING_SHORTCUT_KEY6,
        ShortcutsCommandDecodeMatrix[iApiProvider.Preferences().ShortcutKey6Cmd()][KShortcutsCommandDecodeMatrixStringId],
        aItemText );
    aItemArray->AppendL( aItemText );
    iSettingIndex->AppendL( EWmlSettingsShortCutKey6Cmd );    

    CreateItemFromTwoStringsL(
        R_BROWSER_SETTING_SHORTCUT_KEY7,
        ShortcutsCommandDecodeMatrix[iApiProvider.Preferences().ShortcutKey7Cmd()][KShortcutsCommandDecodeMatrixStringId],
        aItemText );
    aItemArray->AppendL( aItemText );
    iSettingIndex->AppendL( EWmlSettingsShortCutKey7Cmd );    

    CreateItemFromTwoStringsL(
        R_BROWSER_SETTING_SHORTCUT_KEY8,
        ShortcutsCommandDecodeMatrix[iApiProvider.Preferences().ShortcutKey8Cmd()][KShortcutsCommandDecodeMatrixStringId],
        aItemText );
    aItemArray->AppendL( aItemText );
    iSettingIndex->AppendL( EWmlSettingsShortCutKey8Cmd );    
    
    CreateItemFromTwoStringsL(
        R_BROWSER_SETTING_SHORTCUT_KEY9,
        ShortcutsCommandDecodeMatrix[iApiProvider.Preferences().ShortcutKey9Cmd()][KShortcutsCommandDecodeMatrixStringId],
        aItemText );
    aItemArray->AppendL( aItemText );
    iSettingIndex->AppendL( EWmlSettingsShortCutKey9Cmd );    

    CreateItemFromTwoStringsL(
        R_BROWSER_SETTING_SHORTCUT_KEY0,
        ShortcutsCommandDecodeMatrix[iApiProvider.Preferences().ShortcutKey0Cmd()][KShortcutsCommandDecodeMatrixStringId],
        aItemText );
    aItemArray->AppendL( aItemText );
    iSettingIndex->AppendL( EWmlSettingsShortCutKey0Cmd );    

    CreateItemFromTwoStringsL(
        R_BROWSER_SETTING_SHORTCUT_KEYSTAR,
        ShortcutsCommandDecodeMatrix[iApiProvider.Preferences().ShortcutKeyStarCmd()][KShortcutsCommandDecodeMatrixStringId],
        aItemText );
    aItemArray->AppendL( aItemText );
    iSettingIndex->AppendL( EWmlSettingsShortCutKeyStarCmd );    

    CreateItemFromTwoStringsL(
        R_BROWSER_SETTING_SHORTCUT_KEYHASH,
        ShortcutsCommandDecodeMatrix[iApiProvider.Preferences().ShortcutKeyHashCmd()][KShortcutsCommandDecodeMatrixStringId],
        aItemText );
    aItemArray->AppendL( aItemText );
    iSettingIndex->AppendL( EWmlSettingsShortCutKeyHashCmd );    
   
    }


// -----------------------------------------------------------------------------
// CSettingsContainer::ClearListBoxContentL
// -----------------------------------------------------------------------------
//
void CSettingsContainer::ClearListBoxContentL()
    {
    CDesCArray* itemArray =
            ( CDesCArray* ) ( iSettingListBox->Model()->ItemTextArray() );

    // Clear out the listbox content - it is reusable
    while ( itemArray->Count() )
        {
        itemArray->Delete( 0 );
        }
    while ( iSettingIndex->Count() )
        {
        iSettingIndex->Delete( 0 );
        }
    }

// -----------------------------------------------------------------------------
// CSettingsContainer::StoreListBoxIndexL
// -----------------------------------------------------------------------------
//
void CSettingsContainer::StoreListBoxIndexL()
    {
    // We only use one list box for all of the lists in the Settings,
    // we need to keep track of the current & top indices so that focus
    // is updated correctly

    if ( iCurrentSettingCategory == EMain )
        {
        iMainSettingCurrentIndex = iSettingListBox->CurrentItemIndex();
        iMainSettingTopPosIndex = iSettingListBox->TopItemIndex();

        // Reinitialise the subsetting indices
        iSubSettingCurrentIndex = KWmlSettingsIndexNone;
        iSubSettingTopPosIndex = 0;
        }
    else
        {
        iSubSettingCurrentIndex = iSettingListBox->CurrentItemIndex();
        iSubSettingTopPosIndex = iSettingListBox->TopItemIndex();
        }
    }

// -----------------------------------------------------------------------------
// CSettingsContainer::RestoreListBoxIndexL
// -----------------------------------------------------------------------------
//
void CSettingsContainer::RestoreListBoxIndexL()
    {
    iSettingListBox->Reset();

    // Listbox might not have index
    if ( iCurrentSettingCategory == EMain )
        {
        if ( iMainSettingCurrentIndex != KWmlSettingsIndexNone )
            {
            iSettingListBox->SetCurrentItemIndex( iMainSettingCurrentIndex );
            iSettingListBox->SetTopItemIndex( iMainSettingTopPosIndex );
            }
        }
    else
        {
        if ( iSubSettingCurrentIndex != KWmlSettingsIndexNone )
            {
            iSettingListBox->SetCurrentItemIndex( iSubSettingCurrentIndex );
            iSettingListBox->SetTopItemIndex( iSubSettingTopPosIndex );
            }
        }
    iSettingListBox->HandleItemAdditionL();
    iSettingListBox->DrawNow();
    }

// -----------------------------------------------------------------------------
// CSettingsContainer::ChangeItemL
// -----------------------------------------------------------------------------
//
void CSettingsContainer::ChangeItemL( TBool aSelectKeyWasPressed )
    {
    LOG_ENTERFN("CSettingsContainer::ChangeItemL");
    // item is selected for modification
    TInt currentItem( iSettingListBox->CurrentItemIndex() );


    
    // creating array to hold resource ids of options
    CArrayFixFlat<TInt> *values=new(ELeave)CArrayFixFlat<TInt>( 4 );

    // creating an array to hold mapping between displayed choices and resource ids
    // in the case of Toolbar/Shortcut key assignments
    // where some commands (like ROTATE) are not visible.
    CArrayFixFlat<TInt> *valueDisplayMapping=new(ELeave)CArrayFixFlat<TInt>(4);


    CleanupStack::PushL( values );
    CleanupStack::PushL( valueDisplayMapping );
    TInt value;
    MPreferences& preferences = ApiProvider().Preferences();
    TInt settingsItem = MapCurrentItem( currentItem );
    switch ( settingsItem )
        {
        case EWmlSettingsAccesspoint:
            {
            TInt err(KErrNone);
            TInt retVal( KErrNone );
            //start the connection  dialog
            CCmApplicationSettingsUi* settings = CCmApplicationSettingsUi::NewL();
            CleanupStack::PushL( settings );
            
            TCmSettingSelection selection;
            
            //
            // In Avkon tradition,
            // set the id of selection from the existing setting in preferences
            // so it is shown that way in the selection dialog box.
            //
            // Note the CCmApplicationSettingsUi API should ensure that if
            // "Connection Method" was previously selected, it should show up as
            // "Uncategorized" being shown selected
            //
            switch (preferences.AccessPointSelectionMode())
                {   
                case EAlwaysAsk:
                    {                       
                    selection.iResult = EAlwaysAsk;
                    selection.iId = preferences.DefaultSnapId();
                    break;
                    }
                case EDestination:
                    {                       
                    selection.iResult = EDestination;
                    selection.iId = preferences.DefaultSnapId();
                    break;
                    }
                case EConnectionMethod:
                    {
                    selection.iResult = EConnectionMethod;
                    selection.iId = Util::IapIdFromWapIdL( ApiProvider(), preferences.DefaultAccessPoint() );
                    break;
                    }
                default:
                    // default case, set it to nothing
                    selection.iResult = EDefaultConnection;
                    break;
                }
                        
            // empty filter array because no filtering is wanted
            TBearerFilterArray filterArray;
            TRAP ( err, retVal = settings->RunApplicationSettingsL( selection, 
                                                EShowAlwaysAsk | EShowDestinations | EShowConnectionMethods,
                                                // turn off SNAP
//                                                EShowAlwaysAsk | EShowConnectionMethods,
                                                filterArray ) );
            CleanupStack::PopAndDestroy();//settings
            
            //if something has been selected
            if (retVal) 
                {
                //set the selection mode
                preferences.SetAccessPointSelectionModeL( 
                        STATIC_CAST( TCmSettingSelectionMode, selection.iResult ) );

                //based on the chosen connection type, store the connection identifier(iapid, snap id, always ask)
                //in the preferences list
                switch (preferences.AccessPointSelectionMode())
                    {           
                    case EConnectionMethod:
                        {
                        BROWSER_LOG( ( _L( "CSettingsContainer::ChangeItemL CApSettingsHandler" ) ) );
                            
                        TUint32 id = preferences.DefaultAccessPoint();
                        // CMManager gives us IAPid, need to translate to WAPid
                        if (selection.iId != 0)
                            {                           
                            id = Util::WapIdFromIapIdL( ApiProvider(), selection.iId ); 
                            }                               
                        preferences.SetDefaultAccessPointL( id );
                        BROWSER_LOG( ( _L( " SetDefaultAccessPointL OK" ) ) );
                        // Requested AP is preset for PushMtm
                        ApiProvider().SetRequestedAP( preferences.DefaultAccessPoint() );
                        break;  
                        }

                    case EDestination:
                        {
                        TUint32 snapId = preferences.DefaultSnapId();
                        if (selection.iId != 0)
                            {                           
                            snapId = selection.iId;
                            }                   
                        preferences.SetDefaultSnapId( snapId );

                        break;  
                        }

                    case EAlwaysAsk:
                        {
                        break;  
                        }

                    default:
                        {
                        LOG_ENTERFN(" Wrong value in CSettingsContainer::ChangeItemL");
                        }
                    }           
                }

            DisplayGeneralSettingsL();
            BROWSER_LOG( ( _L( "DisplayGeneralSettingsL OK" ) ) );

            break;
            }

        case EWmlSettingsAutoLoadContent:
            {
            if(iFlashPluginPresent)
                {
                value = 2 - preferences.AutoLoadContent();
                
                values->AppendL( R_WMLBROWSER_SETTINGS_AUTOLOAD_ALL );              // 2
                values->AppendL( R_WMLBROWSER_SETTINGS_AUTOLOAD_IMAGES_NO_FLASH );  // 1
                values->AppendL( R_WMLBROWSER_SETTINGS_AUTOLOAD_TEXT );             // 0
                
                value = 2 - (ShowRadioButtonSettingPageL(R_WMLBROWSER_SETTINGS_AUTOLOAD_CONTENT,
                                                            values,
                                                            value ));
                }
            else
                {
                // Below is cenrep values for Load Content for cenrep key KBrowserNGImagesEnabled
                // Load Content 
                // Valid values: 
                // If flash plugin is present: 0 = Text only, 1 = Images,No flash, 2 = All
                // If flash plugin is not present 0 = Text only, 1 = All
                
                // Based on this we have to manually translate the value of 1 from Images,No flash to All 
                // in case where the flash plugin is not present.


                value = 1 - preferences.AutoLoadContent();

                if ( value < 0 )
                    {
                    // for case of EWmlSettingsAutoloadAll value would be negative ;
                    // reset to 0 to prevent crash later
                    value = 0; 
                    }
                    
                values->AppendL( R_WMLBROWSER_SETTINGS_AUTOLOAD_ALL );  // 1 (translated from 2 when flash plugin is present)
                values->AppendL( R_WMLBROWSER_SETTINGS_AUTOLOAD_TEXT ); // 0
                
                value = 1 - (ShowRadioButtonSettingPageL(R_WMLBROWSER_SETTINGS_AUTOLOAD_CONTENT,
                                                              values,
                                                              value ));                
                                                              
                if (value == EWmlSettingsAutoloadImagesNoFlash)
                    // Flash plugin not present so reset value from 1 to 2
                    {
                    value = EWmlSettingsAutoloadAll;
                    }
                }

            preferences.SetAutoLoadContentL( value );
                DisplayPageSettingsL();
            break;
            }

        case EWmlSettingsFontSize:
             {
             //The relationship between index and fontsize is awkward because 
             //the radio buttons reverse the order and the routine called tries
             //to straighten that out. 
             TInt fontIndex = 4 - iFontSize; // (0,1,2,3,4 >> 4,3,2,1,0)
             if (!AVKONAPAC)
                 {
                 values->AppendL( R_WMLBROWSER_SETTINGS_FS_VALUE_ALL_LARGE );
                 values->AppendL( R_WMLBROWSER_SETTINGS_FS_VALUE_LARGER );
                 values->AppendL( R_WMLBROWSER_SETTINGS_FS_VALUE_NORMAL );
                 values->AppendL( R_WMLBROWSER_SETTINGS_FS_VALUE_SMALLER );
                 values->AppendL( R_WMLBROWSER_SETTINGS_FS_VALUE_ALL_SMALL );
                 //Convert result to (4,3,2,1,0 >> 0,1,2,3,4)
                 iFontSize = 4 - (ShowRadioButtonSettingPageL(R_WMLBROWSER_SETTINGS_FONT_SIZE,
                                                              values,
                                                              fontIndex ));
                 }
             else
                 {
                 // China build has only three font sizes.  Radio button
                 // indices are 0, 1, 2. Map these to 0, 2, 4 (all small,
                 // normal, all large) for preferences.
                 values->AppendL( R_WMLBROWSER_SETTINGS_FS_VALUE_ALL_LARGE );
                 values->AppendL( R_WMLBROWSER_SETTINGS_FS_VALUE_NORMAL );
                 values->AppendL( R_WMLBROWSER_SETTINGS_FS_VALUE_ALL_SMALL );
                 //Convert result to (4,3,2,1,0 >> 0,1,2,3,4)
                 iFontSize =  4 - (ShowRadioButtonSettingPageL(R_WMLBROWSER_SETTINGS_FONT_SIZE,
                                                               values,
                                                               fontIndex >> 1 ) << 1);
                 }
             preferences.SetFontSizeL( iFontSize );
             DisplayPageSettingsL();
             break;
             }

        case EWmlSettingsPageOverview:
            {
            if( ApiProvider().WindowMgr().CurrentWindow()->IsPageOverviewSupportedL() )
                {
                values->AppendL( R_WMLBROWSER_SETTINGS_PAGEOVERVIEW_OFF );
                values->AppendL( R_WMLBROWSER_SETTINGS_PAGEOVERVIEW_ON );
                value = preferences.PageOverview();
                value = aSelectKeyWasPressed ?
                1 - value :
                ShowRadioButtonSettingPageL(
                    R_WMLBROWSER_SETTINGS_PAGEOVERVIEW,
                    values,
                    value );
                preferences.SetPageOverviewL( value );
                DisplayGeneralSettingsL();
                }
            break;
            }

        case EWmlSettingsBackList:
            {
            values->AppendL( R_WMLBROWSER_SETTINGS_BACK_LIST_OFF );
            values->AppendL( R_WMLBROWSER_SETTINGS_BACK_LIST_ON );
            value = preferences.BackList();
            value = aSelectKeyWasPressed ?
            1 - value :
            ShowRadioButtonSettingPageL(
                R_WMLBROWSER_SETTINGS_BACK_LIST,
                values,
                value );
            preferences.SetBackListL( value );
                DisplayGeneralSettingsL();
            break;
            }
        case EWmlSettingsUrlSuffix:
            {
            TInt ret;

            HBufC* suffixBuf = iApiProvider.Preferences().URLSuffixList();
            HBufC* suffixToPass = HBufC::NewLC( KMaxSearchPgUrlLength );
            suffixToPass = suffixBuf;
            TPtr ptr = suffixToPass->Des();

            TInt resource = R_URL_SUFFIX_PAGE;

            CAknSettingPage* dlg = new ( ELeave ) CAknTextSettingPage( 
                    resource, ptr, CAknTextSettingPage::EZeroLengthAllowed );

            ret = dlg->ExecuteLD( CAknSettingPage::EUpdateWhenChanged );

            if ( ret )
                {
                iApiProvider.Preferences().SetURLSuffixList(suffixToPass);
                }
            
            CleanupStack::PopAndDestroy(); // suffixToPass
            DisplayGeneralSettingsL();
            break;
            }
        case EWmlSettingsAutoRefresh:
            {
            values->AppendL( R_WMLBROWSER_SETTINGS_AUTO_REFRESH_OFF );
            values->AppendL( R_WMLBROWSER_SETTINGS_AUTO_REFRESH_ON );
            value = preferences.AutoRefresh();
            value = aSelectKeyWasPressed ?
            1 - value :
            ShowRadioButtonSettingPageL(
                R_WMLBROWSER_SETTINGS_AUTO_REFRESH,
                values,
                value );
            preferences.SetAutoRefreshL( value );
                DisplayPageSettingsL();
            break;
            }

    case EWmlSettingsEncoding:
        {
        TInt highlightedItemIndex = KDoesntExist;
        TInt lathinIndex = KDoesntExist;
        for( int i = iEncodingArray->Count() - 1; i >= 0; i-- )
            {
            TEncodingStruct encoding = iEncodingArray->At(i);

            if ( encoding.iSystemId == KCharacterSetIdentifierIso88591 )
                {
                lathinIndex = i;
                }

            values->AppendL( encoding.iResId );

            if ( encoding.iSystemId == iEncoding )
                {
                highlightedItemIndex = i;
                }
            }

        if( highlightedItemIndex == KDoesntExist )
            {
            if( lathinIndex != KDoesntExist )
                {
                highlightedItemIndex = lathinIndex;
                }
            else
                {
                highlightedItemIndex = 0;
                }
            }

        TInt selectedIndex =
            iEncodingArray->Count() - 1 -
                ShowRadioButtonSettingPageL(
                        R_WMLBROWSER_SETTINGS_ENCODING,
                        values,
                        iEncodingArray->Count() - 1 - highlightedItemIndex );

            TEncodingStruct selectedEncoding =
                                        iEncodingArray->At( selectedIndex );

        iEncoding = selectedEncoding.iSystemId;
            DisplayPageSettingsL();
        break;
        }

        case EWmlSettingsAdaptiveBookmarks:
            {
            values->AppendL(
                    R_WMLBROWSER_SETTINGS_ADAPTIVE_BOOKMARKS_VALUE_OFF );
            values->AppendL(
                    R_WMLBROWSER_SETTINGS_ADAPTIVE_BOOKMARKS_VALUE_HIDE_FOLDER );
            values->AppendL(
                    R_WMLBROWSER_SETTINGS_ADAPTIVE_BOOKMARKS_VALUE_ON );

            value = 2-preferences.AdaptiveBookmarks();
            value = ShowRadioButtonSettingPageL(
                R_WMLBROWSER_SETTINGS_ADAPTIVE_BOOKMARKS,
                values,
                value );
            preferences.SetAdaptiveBookmarksL(
                                ( TWmlSettingsAdaptiveBookmarks )(2-value) );
            DisplayPrivacySettingsL();
            break;
            }

        case EWmlSettingsFormDataSaving:
            {
            values->AppendL( R_WMLBROWSER_SETTINGS_FORM_DATA_SAVING_OFF );
            values->AppendL( R_WMLBROWSER_SETTINGS_FORM_DATA_SAVING_ONLY );
            values->AppendL( R_WMLBROWSER_SETTINGS_FORM_DATA_SAVING_ON );

            value = preferences.FormDataSaving();
            TInt origValue(value);
            value = ShowRadioButtonSettingPageL(
                    R_WMLBROWSER_SETTINGS_FORM_DATA_SAVING,
                    values,
                    value );
            preferences.SetFormDataSavingL( (TWmlSettingsFormData) value );

            if (origValue > value)
                {
                // query to see if we need to delete stored form data
                if (origValue == EWmlSettingsFormDataPlusPassword && value == EWmlSettingsFormDataOnly)
                    {
                    // delete password data?
                    ShowFormDataDeletePasswordQueryDialogL();
                    }
                else if (origValue == EWmlSettingsFormDataPlusPassword && value == EWmlSettingsFormDataOff)
                    {
                    // delete password and form data
                    ShowFormDataDeleteAllQueryDialogL();
                    }
                else if (origValue == EWmlSettingsFormDataOnly && value == EWmlSettingsFormDataOff)
                    {
                    // delete form data
                    ShowFormDataDeleteAllQueryDialogL();
                    }
                }

            DisplayPrivacySettingsL();
            break;
            }

        case EWmlSettingsFullScreen:
            {
            
            values->AppendL( R_WMLBROWSER_SETTINGS_SOFTKEYS_OFF );
            values->AppendL( R_WMLBROWSER_SETTINGS_SOFTKEYS_ON );
            value = preferences.FullScreen(); 
            value = aSelectKeyWasPressed ?
            1 - value :
            ShowRadioButtonSettingPageL(
                R_WMLBROWSER_SETTINGS_SOFTKEYS,
                values,
                value );
            preferences.SetFullScreenL( ( TWmlSettingsFullScreen )( value ) );
            DisplayPageSettingsL();
            break;
            }

    case EWmlSettingsHomePage:
       {
       values->AppendL( R_WMLBROWSER_SETTINGS_HOMEPAGE_BOOKMARKS );

       // "Use current page" is available only if a page is loaded.
       // so there might be a hole in the array!
       HBufC *url = CBrowserAppUi::Static()->BrCtlInterface().PageInfoLC(
           TBrCtlDefs::EPageInfoUrl );
                      
       TInt useCurrent( 0 );
       if( url && url->Length() )
           {
           values->AppendL( R_WMLBROWSER_SETTINGS_HOMEPAGE_USE_CURRENT );
           ++useCurrent;
           }

       values->AppendL( R_WMLBROWSER_SETTINGS_HOMEPAGE_NEW_ADDRESS );
       values->AppendL( R_WMLBROWSER_SETTINGS_HOMEPAGE_ACCESS_POINT );
       TBool dialogCancel( ETrue );
       while( dialogCancel )
           {
           value = preferences.HomePageType();
           if( !useCurrent )
                {
                // 'Use current' is not shown as current item
                if( (TWmlSettingsHomePage)value == EWmlSettingsHomePageUseCurrent )
                    {
                    value = EWmlSettingsHomePageAddress;
                    }
                // items behind UseCurrent must be shifted below
                else if( (TWmlSettingsHomePage)value > EWmlSettingsHomePageUseCurrent )
                    {
                    --value;
                    }
                }
           // sanity check
           if( value >= 7 + useCurrent )
               {
               value = 7 + useCurrent;
               }
           if( ShowRadioButtonSettingPageBoolL(
               R_WMLBROWSER_SETTINGS_STARTPAGE, values, &value ) )
               {  // OK was pressed on RadioSetting
               // shift value if needed
               if( !useCurrent &&
                   ( (TWmlSettingsHomePage)value >= EWmlSettingsHomePageUseCurrent) )
                   {
                   ++value;
                   }
               if( (TWmlSettingsHomePage)value == EWmlSettingsHomePageAddress )
                   {
                   HBufC* prompt = StringLoader::LoadLC( R_BROWSER_PROMPT_HOMEPAGE_URL );
                   HBufC* retBuf = HBufC::NewL( KMaxHomePgUrlLength );
                   TPtr retString = retBuf->Des();
                   retString.Copy( KNullDesC );

                   if( preferences.HomePageUrlL( retString, ETrue ) != KErrNone )
                       {
                       retString.Zero();
                       retString.Copy( KNullDesC );
                       }
                   TInt result = TBrowserDialogs::DialogPromptReqL(
                        prompt->Des(),
                        &retString,
                        ETrue,
                        KMaxHomePgUrlLength );
                   CleanupStack::PopAndDestroy();  // prompt
                   if( result )  // URL was accepted
                       {
                       preferences.SetHomePageUrlL( retString );
                       preferences.SetHomePageTypeL( (TWmlSettingsHomePage) value );
                       dialogCancel = EFalse;
                       }
                   if( retBuf )
                       {
                       delete retBuf;
                       }
                   // else URL was cancelled! one more round
                   }
               else if( (TWmlSettingsHomePage)value == EWmlSettingsHomePageAccessPoint )
                   {
                   preferences.SetHomePageTypeL( (TWmlSettingsHomePage) value );
                   dialogCancel = EFalse;
                   }
               else if( (TWmlSettingsHomePage)value == EWmlSettingsHomePageUseCurrent )
                   {
                   // url was already checked. it exists.
                   TPtr urlPtr = url->Des();
                   preferences.SetHomePageUrlL( urlPtr );
                   preferences.SetHomePageTypeL( (TWmlSettingsHomePage) value );
                   dialogCancel = EFalse;
                   }
               else if( (TWmlSettingsHomePage)value == EWmlSettingsHomePageBookmarks )
                   {
                   preferences.SetHomePageTypeL( (TWmlSettingsHomePage) value );
                   dialogCancel = EFalse;
                   }
               }
           else  // Cancel was pressed on RadioSetting, nothing to do
               {
               dialogCancel = EFalse;
               }
           DisplayGeneralSettingsL();
           }
       CleanupStack::PopAndDestroy( url );  // url
       break;
       }

    case EWmlSettingsAutomaticUpdatingAP:
        {
        TUint32 id = preferences.AutomaticUpdatingAP();
        SelectUserDefinedAPL( id );

        preferences.SetAutomaticUpdatingApL( id );
        BROWSER_LOG( ( _L( "EWmlSettingsAutomaticUpdatingAP SetAutomaticUpdatingApL()" ) ) );

        DisplayWebFeedsSettingsL();
        BROWSER_LOG( ( _L( "EWmlSettingsAutomaticUpdatingAP DisplayWebFeedsSettingsL()" ) ) );
        break;
        }

    case EWmlSettingsMediaVolume:
        {
        // Add option list items
        values->AppendL( R_WMLBROWSER_SETTINGS_MEDIA_VOLUME_4 );
        values->AppendL( R_WMLBROWSER_SETTINGS_MEDIA_VOLUME_3 );
        values->AppendL( R_WMLBROWSER_SETTINGS_MEDIA_VOLUME_2 );
        values->AppendL( R_WMLBROWSER_SETTINGS_MEDIA_VOLUME_1 );
        values->AppendL( R_WMLBROWSER_SETTINGS_MEDIA_VOLUME_MUTED );

        TInt itemcount = values->Count();

        // Get Media Volume Value
        value = preferences.MediaVolume();

        // Convert volume to list index
        switch ( value )
            {
            case EWmlSettingsVolumeMuted:
                value = EVolumeMute;
                break;
            case EWmlSettingsVolume1:
                value = EVolumeLevel1;
                break;
            case EWmlSettingsVolume2:
                value = EVolumeLevel2;
                break;
            case EWmlSettingsVolume3:
                value = EVolumeLevel3;
                break;
            case EWmlSettingsVolume4:
                value = EVolumeLevel4;
                break;
            default:
                break;
            }

        // Open Volume Option List Page
        value = ShowRadioButtonSettingPageL(
                    R_WMLBROWSER_SETTINGS_MEDIA_VOLUME, values,
                        ( itemcount - 1 ) - value );

        // Reconvert list index to volume
        value = ( itemcount - 1 ) - value;
        switch ( value )
            {
            case EVolumeMute:
                value = EWmlSettingsVolumeMuted;
                break;
            case EVolumeLevel1:
                value = EWmlSettingsVolume1;
                break;
            case EVolumeLevel2:
                value = EWmlSettingsVolume2;
                break;
            case EVolumeLevel3:
                value = EWmlSettingsVolume3;
                break;
            case EVolumeLevel4:
                value = EWmlSettingsVolume4;
                break;
            default:
                break;
            }

        // Save Media Volume Value
        preferences.SetMediaVolumeL( value );
        DisplayPageSettingsL();
        break;
        }

        case EWmlSettingsCookies:
            {
            values->AppendL( R_WMLBROWSER_SETTINGS_C_VALUE_REJECT );
            values->AppendL( R_WMLBROWSER_SETTINGS_C_VALUE_ALLOW );
            value = preferences.Cookies();
            value = aSelectKeyWasPressed ? 1 - value : ShowRadioButtonSettingPageL(
                R_WMLBROWSER_SETTINGS_COOKIES,
                values,
                value );
            preferences.SetCookiesL( ( TWmlSettingsCookies )value );
            DisplayPrivacySettingsL();
            break;
            }

        case EWmlSettingsEcma:
            {
            values->AppendL( R_WMLBROWSER_SETTINGS_ECMA_VALUE_DISABLE );
            values->AppendL( R_WMLBROWSER_SETTINGS_ECMA_VALUE_ENABLE );
            value = preferences.Ecma();//items need to be appended in reverse order
            value = aSelectKeyWasPressed ? 1 - value : ShowRadioButtonSettingPageL(
                    R_WMLBROWSER_SETTINGS_ECMA,
                    values,
                    value );
            preferences.SetEcmaL( ( TWmlSettingsECMA ) value );
            DisplayGeneralSettingsL();
            break;
            }

        case EWmlSettingsScriptLog:
            {
            value = 3 - preferences.ScriptLog();
            
            values->AppendL( R_WMLBROWSER_SETTINGS_SCRIPTLOG_VALUE_TO_CONSOLE_FILE );
            values->AppendL( R_WMLBROWSER_SETTINGS_SCRIPTLOG_VALUE_TO_CONSOLE );
            values->AppendL( R_WMLBROWSER_SETTINGS_SCRIPTLOG_VALUE_TO_FILE );
            values->AppendL( R_WMLBROWSER_SETTINGS_SCRIPTLOG_VALUE_DISABLE );
            
            value = 3 - (ShowRadioButtonSettingPageL(R_WMLBROWSER_SETTINGS_SCRIPTLOG,
                                                        values,
                                                        value ));
            preferences.SetScriptLogL( value );
            DisplayGeneralSettingsL();

            break;
            }

        case EWmlSettingsDownloadsOpen:
            {
            values->AppendL( R_WMLBROWSER_SETTINGS_DOWNLOAD_OPEN_YES );
            values->AppendL( R_WMLBROWSER_SETTINGS_DOWNLOAD_OPEN_NO );
            value = preferences.DownloadsOpen();
            value = aSelectKeyWasPressed ? value :
            ShowRadioButtonSettingPageL(
                  R_WMLBROWSER_SETTINGS_DOWNLOAD_OPEN,
                  values,
                  1 - value );
            preferences.SetDownloadsOpenL( 1 - value ); 
            DisplayGeneralSettingsL();
            break;
            }

        case EWmlSettingsHttpSecurityWarnings:
            {
            values->AppendL( R_WMLBROWSER_SETTINGS_WRN_VALUE_SHOW );
            values->AppendL( R_WMLBROWSER_SETTINGS_WRN_VALUE_HIDE );
            value = preferences.HttpSecurityWarnings();
            value = aSelectKeyWasPressed ? value :
            ShowRadioButtonSettingPageL(
                  R_WMLBROWSER_SETTINGS_HTTP_SEC_WRNS,
                  values,
                  1 - value );
            preferences.SetHttpSecurityWarningsL( 1 - value ); // show=1, hide=0 in preferences
            DisplayGeneralSettingsL();
            break;
            }
            
        case EWmlSettingsSearchProvider:
            {
            RunSearchSettingsL();
            break;
            }



        case EWmlSettingsIMEINotification:
            {
            if ( IMEI_NOTIFICATION )
                {
                values->AppendL( R_WMLBROWSER_SETTINGS_IMEI_VALUE_DISABLED );
                values->AppendL( R_WMLBROWSER_SETTINGS_IMEI_VALUE_ENABLED );
                value = preferences.IMEINotification();
                //items need to be appended in reverse order
                value = aSelectKeyWasPressed ? 1 - value :
                                ShowRadioButtonSettingPageL(
                                                R_WMLBROWSER_SETTINGS_IMEI,
                                                values,
                                                value );
                preferences.SetIMEINotificationL( ( TWmlSettingsIMEI ) value );
                DisplayPrivacySettingsL();
                }
            break;
            }

        case EWmlSettingsSendReferrer:
            {
            values->AppendL( R_WMLBROWSER_SETTINGS_SEND_REFERRER_OFF );
            values->AppendL( R_WMLBROWSER_SETTINGS_SEND_REFERRER_ON );
            value = preferences.SendReferrer();
            value = aSelectKeyWasPressed ? 1 - value : ShowRadioButtonSettingPageL(
              R_WMLBROWSER_SETTINGS_SEND_REFERRER,
              values,
              value );
            DisplayPrivacySettingsL();
            break;
        }

        case EWmlSettingsPopupBlocking:
            {
            values->AppendL( R_WMLBROWSER_SETTINGS_POPUP_BLOCKING_OFF );
            values->AppendL( R_WMLBROWSER_SETTINGS_POPUP_BLOCKING_ON );
            value = preferences.PopupBlocking();
            value = aSelectKeyWasPressed ? 1 - value : ShowRadioButtonSettingPageL(
                    R_WMLBROWSER_SETTINGS_POPUP_BLOCKING,
                    values,
                    value );
            preferences.SetPopupBlockingL( value );
            DisplayPageSettingsL();
            break;
            }


        case EWmlSettingsAutomaticUpdatingWhileRoaming:
            {
            values->AppendL( R_WMLBROWSER_SETTINGS_AUTOUPDATING_ROAMING_OFF );
            values->AppendL( R_WMLBROWSER_SETTINGS_AUTOUPDATING_ROAMING_ON );

            value = preferences.AutomaticUpdatingWhileRoaming();
            value = aSelectKeyWasPressed ?
            1 - value :
            ShowRadioButtonSettingPageL(
                R_WMLBROWSER_SETTINGS_AUTOUPDATING_ROAMING,
                values,
                value );
            preferences.SetAutomaticUpdatingWhileRoamingL( value );         

            DisplayWebFeedsSettingsL();
            break;
            }


        case EWmlSettingsToolbarOnOff:
            {
            values->AppendL( R_BROWSER_SETTING_TOOLBAR_OFF );
            values->AppendL( R_BROWSER_SETTING_TOOLBAR_ON );

            value = preferences.ShowToolbarOnOff();
            value = aSelectKeyWasPressed ?
            1 - value :
            ShowRadioButtonSettingPageL(
                R_BROWSER_SETTING_TOOLBAR_ON_OFF,
                values,
                value );
            preferences.SetToolbarOnOffL( value );          

            DisplayToolbarSettingsL();
            break;
            }


        case EWmlSettingsToolbarButton1Cmd:
            {
            value = preferences.ShowToolbarButton1Cmd();
            // set toolbar values will build values into an array to be displayed to the user
            // this may mean filtering out some items, so we have to know the mapping between our 
            // fixed array of commands as well as the current value from settings
            value = SetToolbarButtonValues(values, value, valueDisplayMapping);
            value = ShowRadioButtonSettingPageL(    R_BROWSER_SETTING_TOOLBAR_BUTTON1,
                                                    values,
                                                    value );
                                                        
            preferences.SetToolbarButton1CmdL( valueDisplayMapping->At(value) );
            DisplayToolbarSettingsL();
            break;
            }

        case EWmlSettingsToolbarButton2Cmd:
            {
            value = preferences.ShowToolbarButton2Cmd();
            // set toolbar values will build values into an array to be displayed to the user
            // this may mean filtering out some items, so we have to know the mapping between our 
            // fixed array of commands as well as the current value from settings
            value = SetToolbarButtonValues(values, value, valueDisplayMapping);            
            value = ShowRadioButtonSettingPageL(    R_BROWSER_SETTING_TOOLBAR_BUTTON2,
                                                    values,
                                                    value );
                                                    
            preferences.SetToolbarButton2CmdL( valueDisplayMapping->At(value) );
            DisplayToolbarSettingsL();
            break;
            }

        case EWmlSettingsToolbarButton3Cmd:
            {
            value = preferences.ShowToolbarButton3Cmd();
            // set toolbar values will build values into an array to be displayed to the user
            // this may mean filtering out some items, so we have to know the mapping between our 
            // fixed array of commands as well as the current value from settings
            value = SetToolbarButtonValues(values, value, valueDisplayMapping);             
            value = ShowRadioButtonSettingPageL(    R_BROWSER_SETTING_TOOLBAR_BUTTON3,
                                                    values,
                                                    value );
                                                    
            preferences.SetToolbarButton3CmdL( valueDisplayMapping->At(value) );
            DisplayToolbarSettingsL();
            break;
            }

        case EWmlSettingsToolbarButton4Cmd:
            {
            value = preferences.ShowToolbarButton4Cmd();
            // set toolbar values will build values into an array to be displayed to the user
            // this may mean filtering out some items, so we have to know the mapping between our 
            // fixed array of commands as well as the current value from settings
            value = SetToolbarButtonValues(values, value, valueDisplayMapping);              
            value = ShowRadioButtonSettingPageL(    R_BROWSER_SETTING_TOOLBAR_BUTTON4,
                                                    values,
                                                    value );
                                                    
            preferences.SetToolbarButton4CmdL( valueDisplayMapping->At(value) );
            DisplayToolbarSettingsL();
            break;
            }

        case EWmlSettingsToolbarButton5Cmd:
            {
            value = preferences.ShowToolbarButton5Cmd();
            // set toolbar values will build values into an array to be displayed to the user
            // this may mean filtering out some items, so we have to know the mapping between our 
            // fixed array of commands as well as the current value from settings
            value = SetToolbarButtonValues(values, value, valueDisplayMapping); 
             
            value = ShowRadioButtonSettingPageL(    R_BROWSER_SETTING_TOOLBAR_BUTTON5,
                                                    values,
                                                    value );
                                                    
            preferences.SetToolbarButton5CmdL( valueDisplayMapping->At(value) );
            DisplayToolbarSettingsL();
            break;
            }

        case EWmlSettingsToolbarButton6Cmd:
            {
            value = preferences.ShowToolbarButton6Cmd();
            // set toolbar values will build values into an array to be displayed to the user
            // this may mean filtering out some items, so we have to know the mapping between our 
            // fixed array of commands as well as the current value from settings
            value = SetToolbarButtonValues(values, value, valueDisplayMapping); 
             
            value = ShowRadioButtonSettingPageL(    R_BROWSER_SETTING_TOOLBAR_BUTTON6,
                                                    values,
                                                    value );
                                                    
            preferences.SetToolbarButton6CmdL( valueDisplayMapping->At(value) );
            DisplayToolbarSettingsL();
            break;
            }

        case EWmlSettingsToolbarButton7Cmd:
            {
            value = preferences.ShowToolbarButton7Cmd();
            // set toolbar values will build values into an array to be displayed to the user
            // this may mean filtering out some items, so we have to know the mapping between our 
            // fixed array of commands as well as the current value from settings
            value = SetToolbarButtonValues(values, value, valueDisplayMapping); 
             
            value = ShowRadioButtonSettingPageL(    R_BROWSER_SETTING_TOOLBAR_BUTTON7,
                                                    values,
                                                    value );
                                                    
            preferences.SetToolbarButton7CmdL( valueDisplayMapping->At(value) );
            DisplayToolbarSettingsL();
            break;
            }

        case EWmlSettingsShortCutKey1Cmd:
            {
            value = preferences.ShortcutKey1Cmd();
            // set toolbar values will build values into an array to be displayed to the user
            // this may mean filtering out some items, so we have to know the mapping between our 
            // fixed array of commands as well as the current value from settings
            value = SetShortcutKeysValues(values, value, valueDisplayMapping); 
            value = ShowRadioButtonSettingPageL(    R_BROWSER_SETTING_SHORTCUT_KEY1,
                                                    values,
                                                    value );
            preferences.SetShortcutKey1CmdL( valueDisplayMapping->At(value) );
            DisplayShortcutsSettingsL();
            break;
            }

        case EWmlSettingsShortCutKey2Cmd:
            {
            value = preferences.ShortcutKey2Cmd();
            // set toolbar values will build values into an array to be displayed to the user
            // this may mean filtering out some items, so we have to know the mapping between our 
            // fixed array of commands as well as the current value from settings
            value = SetShortcutKeysValues(values, value, valueDisplayMapping);              
            value = ShowRadioButtonSettingPageL(    R_BROWSER_SETTING_SHORTCUT_KEY2,
                                                    values,
                                                    value );
            preferences.SetShortcutKey2CmdL( valueDisplayMapping->At(value) );
            DisplayShortcutsSettingsL();
            break;
            }

        case EWmlSettingsShortCutKey3Cmd:
            {
            value = preferences.ShortcutKey3Cmd();
            // set toolbar values will build values into an array to be displayed to the user
            // this may mean filtering out some items, so we have to know the mapping between our 
            // fixed array of commands as well as the current value from settings
            value = SetShortcutKeysValues(values, value, valueDisplayMapping);              
            value = ShowRadioButtonSettingPageL(    R_BROWSER_SETTING_SHORTCUT_KEY3,
                                                    values,
                                                    value );
            preferences.SetShortcutKey3CmdL( valueDisplayMapping->At(value) );
            DisplayShortcutsSettingsL();
            break;
            }

        case EWmlSettingsShortCutKey4Cmd:
            {
            value = preferences.ShortcutKey4Cmd();
            // set toolbar values will build values into an array to be displayed to the user
            // this may mean filtering out some items, so we have to know the mapping between our 
            // fixed array of commands as well as the current value from settings
            value = SetShortcutKeysValues(values, value, valueDisplayMapping);             
            value = ShowRadioButtonSettingPageL(    R_BROWSER_SETTING_SHORTCUT_KEY4,
                                                    values,
                                                    value );
            preferences.SetShortcutKey4CmdL( valueDisplayMapping->At(value) );
            DisplayShortcutsSettingsL();
            break;
            }

        case EWmlSettingsShortCutKey5Cmd:
            {
            value = preferences.ShortcutKey5Cmd();
            // set toolbar values will build values into an array to be displayed to the user
            // this may mean filtering out some items, so we have to know the mapping between our 
            // fixed array of commands as well as the current value from settings
            value = SetShortcutKeysValues(values, value, valueDisplayMapping);              
            value = ShowRadioButtonSettingPageL(    R_BROWSER_SETTING_SHORTCUT_KEY5,
                                                    values,
                                                    value );
            preferences.SetShortcutKey5CmdL( valueDisplayMapping->At(value) );
            DisplayShortcutsSettingsL();
            break;
            }

        case EWmlSettingsShortCutKey6Cmd:
            {
            value = preferences.ShortcutKey6Cmd();
            // set toolbar values will build values into an array to be displayed to the user
            // this may mean filtering out some items, so we have to know the mapping between our 
            // fixed array of commands as well as the current value from settings
            value = SetShortcutKeysValues(values, value, valueDisplayMapping);              
            value = ShowRadioButtonSettingPageL(    R_BROWSER_SETTING_SHORTCUT_KEY6,
                                                    values,
                                                    value );
            preferences.SetShortcutKey6CmdL( valueDisplayMapping->At(value) );
            DisplayShortcutsSettingsL();
            break;
            }

        case EWmlSettingsShortCutKey7Cmd:
            {
            value = preferences.ShortcutKey7Cmd();
            // set toolbar values will build values into an array to be displayed to the user
            // this may mean filtering out some items, so we have to know the mapping between our 
            // fixed array of commands as well as the current value from settings
            value = SetShortcutKeysValues(values, value, valueDisplayMapping);              
            value = ShowRadioButtonSettingPageL(    R_BROWSER_SETTING_SHORTCUT_KEY7,
                                                    values,
                                                    value );
            preferences.SetShortcutKey7CmdL( valueDisplayMapping->At(value) );
            DisplayShortcutsSettingsL();
            break;
            }

        case EWmlSettingsShortCutKey8Cmd:
            {
            value = preferences.ShortcutKey8Cmd();
            // set toolbar values will build values into an array to be displayed to the user
            // this may mean filtering out some items, so we have to know the mapping between our 
            // fixed array of commands as well as the current value from settings
            value = SetShortcutKeysValues(values, value, valueDisplayMapping); 
            value = ShowRadioButtonSettingPageL(    R_BROWSER_SETTING_SHORTCUT_KEY8,
                                                    values,
                                                    value );
            preferences.SetShortcutKey8CmdL( valueDisplayMapping->At(value) );
            DisplayShortcutsSettingsL();
            break;
            }

        case EWmlSettingsShortCutKey9Cmd:
            {
            value = preferences.ShortcutKey9Cmd();
            // set toolbar values will build values into an array to be displayed to the user
            // this may mean filtering out some items, so we have to know the mapping between our 
            // fixed array of commands as well as the current value from settings
            value = SetShortcutKeysValues(values, value, valueDisplayMapping);              
            value = ShowRadioButtonSettingPageL(    R_BROWSER_SETTING_SHORTCUT_KEY9,
                                                    values,
                                                    value );
            preferences.SetShortcutKey9CmdL( valueDisplayMapping->At(value) );
            DisplayShortcutsSettingsL();
            break;
            }

        case EWmlSettingsShortCutKey0Cmd:
            {
            value = preferences.ShortcutKey0Cmd();
            // set toolbar values will build values into an array to be displayed to the user
            // this may mean filtering out some items, so we have to know the mapping between our 
            // fixed array of commands as well as the current value from settings
            value = SetShortcutKeysValues(values, value, valueDisplayMapping);              
            value = ShowRadioButtonSettingPageL(    R_BROWSER_SETTING_SHORTCUT_KEY0,
                                                    values,
                                                    value );
            preferences.SetShortcutKey0CmdL( valueDisplayMapping->At(value) );
            DisplayShortcutsSettingsL();
            break;
            }

        case EWmlSettingsShortCutKeyStarCmd:
            {
            value = preferences.ShortcutKeyStarCmd();
            // set toolbar values will build values into an array to be displayed to the user
            // this may mean filtering out some items, so we have to know the mapping between our 
            // fixed array of commands as well as the current value from settings
            value = SetShortcutKeysValues(values, value, valueDisplayMapping);              
            value = ShowRadioButtonSettingPageL(    R_BROWSER_SETTING_SHORTCUT_KEYSTAR,
                                                    values,
                                                    value );
            preferences.SetShortcutKeyStarCmdL( valueDisplayMapping->At(value) );
            DisplayShortcutsSettingsL();
            break;
            }

        case EWmlSettingsShortCutKeyHashCmd:
            {
            value = preferences.ShortcutKeyHashCmd();
            // set toolbar values will build values into an array to be displayed to the user
            // this may mean filtering out some items, so we have to know the mapping between our 
            // fixed array of commands as well as the current value from settings
            value = SetShortcutKeysValues(values, value, valueDisplayMapping);              
            value = ShowRadioButtonSettingPageL(    R_BROWSER_SETTING_SHORTCUT_KEYHASH,
                                                    values,
                                                    value );
            preferences.SetShortcutKeyHashCmdL( valueDisplayMapping->At(value) );
            DisplayShortcutsSettingsL();
            break;
            }

        default:
            break;
        }
    CleanupStack::PopAndDestroy(2); // values, valueDisplayMapping
    }


// -----------------------------------------------------------------------------
// CSettingsContainer::SetToolbarButtonValues
// -----------------------------------------------------------------------------
//
TInt CSettingsContainer::SetToolbarButtonValues(CArrayFixFlat<TInt> *values, TInt value, CArrayFixFlat<TInt> *displaymap)
    {
    TInt valueNewMapping = 0;
    
    for ( TInt i=0 ; i < KToolbarCommandDecodeMatrixCount ; i++ )      
        {        
        TInt stringId = ToolbarCommandDecodeMatrix[i][KToolbarCommandDecodeMatrixStringId];

        if ((stringId == R_BROWSER_SETTING_ROTATE_SCREEN) && (!ApiProvider().Preferences().RotateDisplay()) )
        {   // Rotate Display is not available so don't show it as a toolbar button command
            continue;   
        }
  
#ifndef __RSS_FEEDS
        if (stringId == R_BROWSER_SETTING_SUBSCRIBE_TO_FEEDS)
        {   // Subscribe to Feeds is not available so don't show it as a toolbar button command
            continue;   
        }        
#endif

        // Disable save page until it's officially supported 
        if (stringId == R_BROWSER_SETTING_SAVE_PAGE )
        {   
            continue;   
        }        

        // make a note at the index within the new array that the current value is at
        // before appending (Count will then correspond to the index, rather than needing to use
        // Count() - 1 AFTER we add it
        if(i == value)
            {
            valueNewMapping = displaymap->Count();
            }      
        displaymap->AppendL( i );
        values->AppendL( stringId );
        }

    return valueNewMapping;
    }

            
// -----------------------------------------------------------------------------
// CSettingsContainer::SetShortcutKeysValues
// -----------------------------------------------------------------------------
//
TInt CSettingsContainer::SetShortcutKeysValues(CArrayFixFlat<TInt> *values, TInt value, CArrayFixFlat<TInt> *displaymap)
    {
    TInt valueNewMapping = 0;
        
    for ( TInt i=0 ; i < KShortcutsCommandDecodeMatrixCount ; i++ )      
        {
        
        TInt stringId = ShortcutsCommandDecodeMatrix[i][KShortcutsCommandDecodeMatrixStringId]; 
        
        if ((stringId == R_BROWSER_SETTING_ROTATE_SCREEN) && (!ApiProvider().Preferences().RotateDisplay()) )
        {   // Rotate Display is not available so don't show it as a shortcut key command
            continue;   
        }
  
#ifndef __RSS_FEEDS
        if (stringId == R_BROWSER_SETTING_SUBSCRIBE_TO_FEEDS)
        {   // Subscribe to Feeds is not available so don't show it as a shortcut key command
            continue;   
        }        
#endif


       // Disable save page until it's officially supported 
        if (stringId == R_BROWSER_SETTING_SAVE_PAGE )
        {   
            continue;   
        }        

        // make a note at the index within the new array that the current value is at
        // before appending (Count will then correspond to the index, rather than needing to use
        // Count() - 1 AFTER we add it
        if(i == value)
            {
            valueNewMapping = displaymap->Count();
            }      
        
        displaymap->AppendL( i );
        values->AppendL(stringId );
        }
        
    return valueNewMapping;
    }


// -----------------------------------------------------------------------------
// CSettingsContainer::IsSettingModifiable
// -----------------------------------------------------------------------------
//
TBool CSettingsContainer::IsSettingModifiable() const
    {
    TBool ret( EFalse );

    switch ( iCurrentSettingCategory )
        {
        // Only these setting lists contain items which can be modified
        case EGeneral:
        case EPage:
        case EWebFeeds:
        case EPrivacy:
        case EToolbar:
        case EShortCuts:
            {
            ret = ETrue;
            break;
            }
        default:
            break;
        }
    return ret;
    }

// -----------------------------------------------------------------------------
// CSettingsContainer::ShowRadioButtonSettingPageL
// -----------------------------------------------------------------------------
//
TInt CSettingsContainer::ShowRadioButtonSettingPageL(
                                                     TInt aTitle,
                                                     CArrayFixFlat<TInt>* aValues,
                                                     TInt aCurrentItem )
    {
    // title of the dialog
    HBufC* title = iCoeEnv->AllocReadResourceLC( aTitle );

    __ASSERT_DEBUG( (aValues != NULL), Util::Panic( Util::EUninitializedData ));


    // options array
    CDesCArrayFlat* values = new( ELeave )CDesCArrayFlat(1);
    CleanupStack::PushL( values );

    for ( TInt i = aValues->Count() - 1; i >= 0; i-- )
        {
        HBufC* value = iCoeEnv->AllocReadResourceLC( aValues->At( i ) );
        values->AppendL( *value );
        CleanupStack::PopAndDestroy(); // value
        }

    // index must be turned upside down, because options list is upside down
    TInt newItem = aCurrentItem = aValues->Count() - 1 - aCurrentItem;

    // We have everything to create dialog
    CAknRadioButtonSettingPage* dlg = new ( ELeave )CAknRadioButtonSettingPage(
        R_RADIO_BUTTON_SETTING_PAGE, newItem, values );

    CleanupStack::PushL( dlg );
    dlg->SetSettingTextL( *title );
    //dlg->ConstructL();
    //dlg->ListBoxControl()->SetCurrentItemIndex( newItem );
    CleanupStack::Pop(); // dlg
    iActionCancelled = EFalse;
    if ( !dlg->ExecuteLD( CAknSettingPage::EUpdateWhenChanged ) )
        {
        // Changes confirmed
        newItem = aCurrentItem;
        iActionCancelled = ETrue;
        }

    CleanupStack::PopAndDestroy( 2 ); // title, values

    // index must be re-turned upside down, because options list is upside down
    return aValues->Count() - 1 - newItem;
    }


// -----------------------------------------------------------------------------
// CSettingsContainer::ShowRadioButtonSettingPageL
// -----------------------------------------------------------------------------
//
TBool CSettingsContainer::ShowRadioButtonSettingPageBoolL(
                                                     TInt aTitle,
                                                     CArrayFixFlat<TInt>* aValues,
                                                     TInt* aCurrentItem )
    {
    // title of the dialog
    HBufC* title = iCoeEnv->AllocReadResourceLC( aTitle );

    __ASSERT_DEBUG( (aValues != NULL), Util::Panic( Util::EUninitializedData ));
    __ASSERT_DEBUG( (aCurrentItem != NULL), Util::Panic( Util::EUninitializedData ));


    // options array
    CDesCArrayFlat* values = new( ELeave )CDesCArrayFlat(1);
    CleanupStack::PushL( values );

    for ( TInt i = aValues->Count() - 1; i >= 0; i-- )
        {
        HBufC* value = iCoeEnv->AllocReadResourceLC( aValues->At( i ) );
        values->AppendL( *value );
        CleanupStack::PopAndDestroy(); // value
        }

    TInt newItem = *aCurrentItem;
    CAknRadioButtonSettingPage* dlg = new ( ELeave )CAknRadioButtonSettingPage(
        R_RADIO_BUTTON_SETTING_PAGE, newItem, values );

    CleanupStack::PushL( dlg );
    dlg->SetSettingTextL( *title );
    CleanupStack::Pop(); // dlg

    TBool retval = dlg->ExecuteLD( CAknSettingPage::EUpdateWhenChanged );

    if ( retval )  // OK was selected, store new value
        {
        *aCurrentItem = newItem;
        }
    CleanupStack::PopAndDestroy( 2 ); // title, values 
    return retval;
    }


// -----------------------------------------------------------------------------
// CSettingsContainer::SaveChangesL
// -----------------------------------------------------------------------------
//
void CSettingsContainer::SaveChangesL()
    {
    // These settings result in page layout changes, and changes to these
    // settings should only be relayed once the settings page is closed
    MPreferences& preferences = ApiProvider().Preferences();

    if ( iFontSize != preferences.FontSize() )
        {
    preferences.SetFontSizeL( iFontSize );
        }

    if ( iEncoding != preferences.Encoding() )
        {
        preferences.SetEncodingL( iEncoding );
        }

    if ( iTextWrap != preferences.TextWrap() )
        {
        preferences.SetTextWrapL( iTextWrap );
        }
    preferences.FlushSD();

    preferences.NotifyObserversL( EPreferencesDeactivate, TBrCtlDefs::ESettingsUnknown );
    }


// -----------------------------------------------------------------------------
// CSettingsContainer::OfferKeyEventL
// -----------------------------------------------------------------------------
//
TKeyResponse CSettingsContainer::OfferKeyEventL( const TKeyEvent& aKeyEvent, TEventCode aType )
    {
    TKeyResponse ret = EKeyWasNotConsumed;
    TKeyEvent keyEvent(aKeyEvent);
    
    // Special case for MSK enabled mode - turn MSK KeyUp to MSK KeyEvent to process Settings toggle function
    // This is what is done in BookmarksContainer::OfferKeyEventL
    if (( keyEvent.iScanCode == EStdKeyDevice3 ) || (keyEvent.iScanCode == EStdKeyEnter))
        {
        if( aType == EEventKeyDown )
            {
            iSelectionKeyDownPressed = ETrue;
            ret = EKeyWasConsumed;
            }
        else if( aType == EEventKeyUp )
            {
            if( iSelectionKeyDownPressed )
                {
                keyEvent.iCode = EKeyOK;
                ret = EKeyWasNotConsumed;
                iSelectionKeyDownPressed = EFalse;
                }
            else
                {
                // long press of Selection key was already processed
                ret = EKeyWasConsumed;
                }
            }
        }


    switch ( keyEvent.iCode )
        {
        // Open List Box item
        case EKeyEnter:
            {
            switch ( iSettingIndex->At( iSettingListBox->CurrentItemIndex() ) )
                {
                case EWmlSettingsGeneral:
                case EWmlSettingsPrivacy:
                case EWmlSettingsPage:
                case EWmlSettingsWebFeeds:
                case EWmlSettingsToolbar:
                case EWmlSettingsShortcuts:
                {
                    DisplayCorrectSettingCategoryListL();
                    break;
                }
                
                // Individual Settings
                case EWmlSettingsAccesspoint:
                case EWmlSettingsHomePage:
                case EWmlSettingsBackList:
                case EWmlSettingsHttpSecurityWarnings:
                case EWmlSettingsEcma:
                case EWmlSettingsScriptLog:
                case EWmlSettingsDownloadsOpen:
                case EWmlSettingsAutoLoadContent:
                case EWmlSettingsFullScreen:
                case EWmlSettingsPageOverview:
                case EWmlSettingsMediaVolume:
                case EWmlSettingsEncoding:
                case EWmlSettingsPopupBlocking:
                case EWmlSettingsAutoRefresh:
                case EWmlSettingsAdaptiveBookmarks:
                case EWmlSettingsFormDataSaving:
#ifdef __WIM
                case EWmlSettingsSaveReceipt:
#endif
                case EWmlSettingsCookies:
                case EWmlSettingsIMEINotification:
                case EWmlSettingsAutomaticUpdatingAP:
                case EWmlSettingsAutomaticUpdatingWhileRoaming:
                case EWmlSettingsUrlSuffix:
                case EWmlSettingsFontSize:
                case EWmlSettingsToolbarButton1Cmd:
                case EWmlSettingsToolbarButton2Cmd:
                case EWmlSettingsToolbarButton3Cmd:
                case EWmlSettingsToolbarButton4Cmd:
                case EWmlSettingsToolbarButton5Cmd:
                case EWmlSettingsToolbarButton6Cmd:
                case EWmlSettingsToolbarButton7Cmd:
                case EWmlSettingsShortCutKey1Cmd:
                case EWmlSettingsShortCutKey2Cmd:
                case EWmlSettingsShortCutKey3Cmd:
                case EWmlSettingsShortCutKey4Cmd:
                case EWmlSettingsShortCutKey5Cmd:
                case EWmlSettingsShortCutKey6Cmd:
                case EWmlSettingsShortCutKey7Cmd:
                case EWmlSettingsShortCutKey8Cmd:
                case EWmlSettingsShortCutKey9Cmd:
                case EWmlSettingsShortCutKey0Cmd:
                case EWmlSettingsShortCutKeyStarCmd:
                case EWmlSettingsShortCutKeyHashCmd:
                case EWmlSettingsSearchProvider:
                {
                    ChangeItemL( ETrue );
                    ret = EKeyWasConsumed;
                    break;
                }
                default:
                    break;
                }
            break;
            }
            
        case EKeyOK:
            {
            // MSK command handles the commands - no need for key handling
            ret = EKeyWasConsumed;
            break; 
            }
            
          default:
            break;
        }

    if ( iSettingListBox && ( ret == EKeyWasNotConsumed || iSettingListBox ) )
        {
        ret = iSettingListBox->OfferKeyEventL( keyEvent, aType );
        }
    return ret;
    }


// -----------------------------------------------------------------------------
// CSettingsContainer::HandleListBoxEventL
// -----------------------------------------------------------------------------
//
void CSettingsContainer::HandleListBoxEventL(CEikListBox* aListBox,TListBoxEvent aEventType)
    {
    if (iPenEnabled)
        {
        if (iSettingListBox && aListBox == iSettingListBox)
            {
            switch (aEventType)
                {
                //case MEikListBoxObserver::EEventEnterKeyPressed:
                case MEikListBoxObserver::EEventItemClicked:
                    break;
                case MEikListBoxObserver::EEventItemDoubleClicked:
                    {
                    TKeyEvent keyEvent;
                    keyEvent.iModifiers = 0;
                    keyEvent.iRepeats = 0;
                    keyEvent.iCode = EKeyDevice3;
                    keyEvent.iScanCode = EStdKeyDevice3;
                    CCoeEnv::Static()->SimulateKeyEventL( keyEvent, EEventKey );
                    }
                    break;
                default:
                    break;
                }
            }
        }
    }


// -----------------------------------------------------------------------------
// CSettingsContainer::HandleGainingForegroundL
// -----------------------------------------------------------------------------
//
void CSettingsContainer::HandleGainingForegroundL()
    {
    switch(iCurrentSettingCategory)
        {
        case EGeneral:
            DisplayGeneralSettingsL();
            break;
        default: // do nothing
            break;
        }
    }

// -----------------------------------------------------------------------------
// CSettingsContainer::CreateItemFromTwoStringsL
// -----------------------------------------------------------------------------
//
void CSettingsContainer::CreateItemFromTwoStringsL(
                                                   TInt aFirst,
                                                   TInt aSecond,
                                                   TBuf<KWmlSettingsItemMaxLength>& aResult )
    {
    // Appends two text from resources next to each other
    aResult.Zero();
    aResult.Append( KWmlSettingsListBoxItemPrefix );
    HBufC* first = iCoeEnv->AllocReadResourceLC(  aFirst );
    aResult.Append( first->Des() );
    CleanupStack::PopAndDestroy(); // first
    aResult.Append( KWmlSettingsListBoxItemPostfix );
    HBufC* second = iCoeEnv->AllocReadResourceLC(  aSecond );
    aResult.Append( second->Des() );
    CleanupStack::PopAndDestroy(); //second
    }


// ----------------------------------------------------------------------------
// CSettingsContainer::AddEncodingL()
// ----------------------------------------------------------------------------
//
void CSettingsContainer::AddEncodingL( TUint32 aSystemId, TUint32 aResId )
    {
    TEncodingStruct encoding;

    encoding.iSystemId = aSystemId;
    encoding.iResId = aResId;

    iEncodingArray->AppendL( encoding );
    }


// -----------------------------------------------------------------------------
// CSettingsContainer::RemoveUnsupportedEncodingsL()
// -----------------------------------------------------------------------------
//
void CSettingsContainer::RemoveUnsupportedEncodingsL()
    {
    RFs fsSession;
    User::LeaveIfError( fsSession.Connect() );
    CleanupClosePushL<RFs>( fsSession );

    CCnvCharacterSetConverter* charConv;
    charConv = CCnvCharacterSetConverter::NewLC();

    CArrayFix<CCnvCharacterSetConverter::SCharacterSet>* charSets;
    charSets = charConv->CreateArrayOfCharacterSetsAvailableLC(fsSession);

    TInt lastIndex = iEncodingArray->Count()-1;

    for ( int i = lastIndex; i >= 0; i-- )
        {
        TBool remove = ETrue;
        TEncodingStruct encoding = iEncodingArray->At(i);
        for ( int j = 0; j < charSets->Count(); j++ )
            {
            TUint32 identifier = charSets->At(j).Identifier();

            if ( identifier == encoding.iSystemId )
                {
                remove = EFalse;
                break;
                }
            }

        if ( remove &&
            ( encoding.iSystemId != KCharacterSetIdentifierAutomatic ) )
            {
            iEncodingArray->Delete(i);
            }
        }

    fsSession.Close();
    CleanupStack::PopAndDestroy(3); // charConv, charSets, fsSession
    }


// -----------------------------------------------------------------------------
// CSettingsContainer::CreateEncodingArrayL()
// -----------------------------------------------------------------------------
//
void CSettingsContainer::CreateEncodingArrayL()
    {
    AddEncodingL( KCharacterSetIdentifierGb2312,        R_WMLBROWSER_SETTINGS_ENCODING_GB2312 );
    AddEncodingL( KCharacterSetIdentifierIso88591,      R_WMLBROWSER_SETTINGS_ENCODING_ISO8859_1 );
    AddEncodingL( KCharacterSetIdentifierBig5,          R_WMLBROWSER_SETTINGS_ENCODING_BIG5 );
    AddEncodingL( KCharacterSetIdentifierUtf8,          R_WMLBROWSER_SETTINGS_ENCODING_UTF8 );
    AddEncodingL( KCharacterSetIdentifierIso88592,      R_WMLBROWSER_SETTINGS_ENCODING_ISO8859_2 );
    AddEncodingL( KCharacterSetIdentifierIso88594,      R_WMLBROWSER_SETTINGS_ENCODING_ISO8859_4 );
    AddEncodingL( KCharacterSetIdentifierIso88595,      R_WMLBROWSER_SETTINGS_ENCODING_ISO8859_5 );
    AddEncodingL( KCharacterSetIdentifierIso88597,      R_WMLBROWSER_SETTINGS_ENCODING_ISO8859_7 );
    AddEncodingL( KCharacterSetIdentifierIso88599,      R_WMLBROWSER_SETTINGS_ENCODING_ISO8859_9 );
    AddEncodingL( KCharacterSetIdentifierIso88598,      R_WMLBROWSER_SETTINGS_ENCODING_ISO8859_8 );
    AddEncodingL( KCharacterSetIdentifierIso88596,      R_WMLBROWSER_SETTINGS_ENCODING_ISO8859_6 );
    AddEncodingL( KCharacterSetIdentifierWindows1256,   R_WMLBROWSER_SETTINGS_ENCODING_WINDOWS_1256 );
    AddEncodingL( KCharacterSetIdentifierWindows1255,   R_WMLBROWSER_SETTINGS_ENCODING_WINDOWS_1255 );
    AddEncodingL( KCharacterSetIdentifierWindows1250,   R_WMLBROWSER_SETTINGS_ENCODING_WINDOWS_1250 );
    AddEncodingL( KCharacterSetIdentifierWindows1251,   R_WMLBROWSER_SETTINGS_ENCODING_WINDOWS_1251 );
    AddEncodingL( KCharacterSetIdentifierWindows1253,   R_WMLBROWSER_SETTINGS_ENCODING_WINDOWS_1253 );
    AddEncodingL( KCharacterSetIdentifierWindows1254,   R_WMLBROWSER_SETTINGS_ENCODING_WINDOWS_1254 );
    AddEncodingL( KCharacterSetIdentifierWindows1257,   R_WMLBROWSER_SETTINGS_ENCODING_WINDOWS_1257 );
    AddEncodingL( KCharacterSetIdentifierWindows1258,   R_WMLBROWSER_SETTINGS_ENCODING_WINDOWS_1258 );
    AddEncodingL( KCharacterSetIdentifierTis620,        R_WMLBROWSER_SETTINGS_ENCODING_TIS_620 );       // Thai
    AddEncodingL( KCharacterSetIdentifierWindows874,    R_WMLBROWSER_SETTINGS_ENCODING_WINDOWS_874 );   // Thai
    AddEncodingL( KCharacterSetIdentifierEucJpPacked,   R_WMLBROWSER_SETTINGS_ENCODING_EUC_JP );
    AddEncodingL( KCharacterSetIdentifierJis,           R_WMLBROWSER_SETTINGS_ENCODING_ISO_2022_JP );
    AddEncodingL( KCharacterSetIdentifierShiftJis,      R_WMLBROWSER_SETTINGS_ENCODING_SHIFT_JIS );
    AddEncodingL( KCharacterSetIdentifierKoi8_r,        R_WMLBROWSER_SETTINGS_ENCODING_KOI8_R );
    AddEncodingL( KCharacterSetIdentifierKoi8_u,        R_WMLBROWSER_SETTINGS_ENCODING_KOI8_U );
	AddEncodingL( KCharacterSetIdentifierIscii_temp,			R_WMLBROWSER_SETTINGS_ENCODING_ISCII);
    AddEncodingL( KCharacterSetIdentifierEucKr,         R_WMLBROWSER_SETTINGS_ENCODING_EUC_KR );
    AddEncodingL( KCharacterSetIdentifierKsc5601_temp,       R_WMLBROWSER_SETTINGS_ENCODING_KSC_5601 );
    AddEncodingL( KCharacterSetIdentifierAutomatic,     R_WMLBROWSER_SETTINGS_ENCODING_AUTOMATIC );
    }

#ifdef __SERIES60_HELP
// -----------------------------------------------------------------------------
// CSettingsContainer::GetHelpContext()
// -----------------------------------------------------------------------------
//
void CSettingsContainer::GetHelpContext( TCoeHelpContext& aContext ) const
    {
    aContext.iMajor = KUidBrowserApplication;
    /*
    context is dependent on what Category we are in
    enum TSettingCategory
            {
            EMain = 0,
            EGeneral,
            EPrivacy,
            EPage,
            EWebFeeds,
            EToolbar,
            EShortCuts,
            ENone
            };
    */
    switch ( iCurrentSettingCategory )
        {
        case EMain:
            aContext.iContext = KOSS_HLP_SETTINGS_FOLDERS;
            break;
        case EGeneral:
            aContext.iContext = KOSS_HLP_SETTINGS_GENERAL;
            break;
        case EPrivacy:
            aContext.iContext = KOSS_HLP_SETTINGS_PRIVACY;
            break;
        case EPage:
            aContext.iContext = KOSS_HLP_SETTINGS_PAGE;
            break;
        case EWebFeeds:
            aContext.iContext = KOSS_HLP_SETTINGS_RSS;
            break;
        case ENone:
            aContext.iContext = KOSS_HLP_SETTINGS_FOLDERS;
            break;
        default:
            aContext.iContext = KOSS_HLP_SETTINGS_FOLDERS;
            break;


        }

    }
#endif // __SERIES60_HELP


// -----------------------------------------------------------------------------
// CSettingsContainer::MapCurrentItem
// -----------------------------------------------------------------------------
//
TInt CSettingsContainer::MapCurrentItem( TUint aCurrentItem ) const
    {
    return iSettingIndex->At( aCurrentItem );
    }


// -----------------------------------------------------------------------------
// CSettingsContainer::CountComponentControls
// -----------------------------------------------------------------------------
//
TInt CSettingsContainer::CountComponentControls() const
  {
  return iSettingListBox ? 1 : 0;
  }


// -----------------------------------------------------------------------------
// CSettingsContainer::ComponentControl
// -----------------------------------------------------------------------------
//
CCoeControl* CSettingsContainer::ComponentControl( TInt aIndex ) const
  {
  return ( aIndex ? NULL : iSettingListBox );
  }


// -----------------------------------------------------------------------------
// CSettingsContainer::SizeChanged
// -----------------------------------------------------------------------------
//
void CSettingsContainer::SizeChanged()
    {
    if ( iSettingListBox )
        {
        iSettingListBox->SetRect( Rect() );
        }
    }


// ----------------------------------------------------------------------------
// CSettingsContainer::FocusChanged
// ----------------------------------------------------------------------------
//
void CSettingsContainer::FocusChanged( TDrawNow aDrawNow )
    {
    if ( iSettingListBox && iSettingListBox->IsVisible() )
        {
        iSettingListBox->SetFocus( IsFocused(), aDrawNow );
        }
    }

// -----------------------------------------------------------------------------
// CSettingsContainer::HandleResourceChange
// -----------------------------------------------------------------------------
//
void CSettingsContainer::HandleResourceChange( TInt aType )
    {
    if ( iSettingListBox )
        {
        iSettingListBox->HandleResourceChange( aType );
        }
        
    if (aType == KEikDynamicLayoutVariantSwitch)
        {
        TRect  rect;
        if (AknLayoutUtils::LayoutMetricsRect(AknLayoutUtils::EMainPane, rect))
            {
            SetRect(rect);
            }
        DrawDeferred();
        }    
        
    }

// ---------------------------------------------------------
// CSettingsContainer::ShowFormDataDeleteAllQueryDialogL
// ---------------------------------------------------------
//
void CSettingsContainer::ShowFormDataDeleteAllQueryDialogL( )
    {
    CAknQueryDialog* dlg = new (ELeave) CAknQueryDialog(CAknQueryDialog::EConfirmationTone);
    HBufC* deleteQuery = StringLoader::LoadLC(R_WMLBROWSER_SETTINGS_FORM_DATA_DELETE_ALL);
    dlg->SetPromptL(*deleteQuery);
    dlg->PrepareLC( R_SETTINGS_QUERY_LINES);
    TInt ret = dlg->RunLD();
    if( ret == EAknSoftkeyYes )
        {
        // call brctl to clear data
        CBrowserAppUi::Static()->BrCtlInterface().HandleCommandL((TInt)TBrCtlDefs::ECommandIdBase + (TInt)TBrCtlDefs::ECommandClearAutoFormFillData);
        CBrowserAppUi::Static()->BrCtlInterface().HandleCommandL((TInt)TBrCtlDefs::ECommandIdBase + (TInt)TBrCtlDefs::ECommandClearAutoFormFillPasswordData);
        }
    CleanupStack::PopAndDestroy(1); // deleteQuery
    }

// ---------------------------------------------------------
// CSettingsContainer::ShowFormDataDeletePasswordQueryDialogL
// ---------------------------------------------------------
//
void CSettingsContainer::ShowFormDataDeletePasswordQueryDialogL( )
    {
    CAknQueryDialog* dlg = new (ELeave) CAknQueryDialog(CAknQueryDialog::EConfirmationTone);
    HBufC* deleteQuery = StringLoader::LoadLC(R_WMLBROWSER_SETTINGS_FORM_DATA_DELETE_PASSWORD);
    dlg->SetPromptL(*deleteQuery);
    dlg->PrepareLC( R_SETTINGS_QUERY_LINES);
    TInt ret = dlg->RunLD();
    if( ret == EAknSoftkeyYes )
        {
        // call brctl to clear data
        CBrowserAppUi::Static()->BrCtlInterface().HandleCommandL((TInt)TBrCtlDefs::ECommandIdBase + (TInt)TBrCtlDefs::ECommandClearAutoFormFillPasswordData);
        }
    CleanupStack::PopAndDestroy(1); // deleteQuery
    }

// ---------------------------------------------------------
// CSettingsContainer::SelectUserDefinedAPL
// ---------------------------------------------------------
//
void CSettingsContainer::SelectUserDefinedAPL( TUint32& id )
    {
    LOG_ENTERFN( "CSettingsContainer::SelectUserDefinedAPL" );

    BROWSER_LOG( ( _L( "CApSettingsHandler()" ) ) );
    CApSettingsHandler *ApUi = CApSettingsHandler::NewLC(
                                ETrue,
                                EApSettingsSelListIsPopUp,
                                EApSettingsSelMenuSelectNormal,
                                KEApIspTypeAll,
                                EApBearerTypeAll,
                                KEApSortNameAscending,
                                EIPv4 | EIPv6
                                );
    BROWSER_LOG( ( _L( "EWmlSettingsAutomaticUpdatingAP RunSettingsL()" ) ) );
    TInt ret = ApUi->RunSettingsL( id, id );
    CleanupStack::PopAndDestroy( ApUi ); //ApUi
    if ( ret & KApUiEventExitRequested ) // & because it is a bit-mask...
        {
        CBrowserAppUi::Static()->ExitBrowser( EFalse );
        }

    CApSelect* ApSel = CApSelect::NewLC(
                iApiProvider.CommsModel().CommsDb(),
                KEApIspTypeAll, //KEApIspTypeWAPMandatory,
                EApBearerTypeAll,
                KEApSortNameAscending,
                EIPv4 | EIPv6 );
    TInt apSelCount = ApSel->Count();
    BROWSER_LOG( ( _L( " ApSel->Count(): %d" ), apSelCount ) );
    CleanupStack::PopAndDestroy( ApSel ); //ApSel

    CApUtils* au = CApUtils::NewLC( iApiProvider.CommsModel().CommsDb() );
    TBool apExist = au->WapApExistsL( id );
    CleanupStack::PopAndDestroy( au );  // au

    if( ( 0 == apSelCount ) || ( EFalse == apExist ) )
        {
        id =  KWmlNoDefaultAccessPoint;
        }
    }

// -----------------------------------------------------------------------------
// CSettingsContainer::RunSearchSettingsL
// -----------------------------------------------------------------------------
//
void CSettingsContainer::RunSearchSettingsL()
    {
    // Get Search application UID from CenRep 
    TInt id = ApiProvider().Preferences().GetIntValue( KBrowserSearchAppUid );
    TUid searchAppId( TUid::Uid( id ) );
    id = ApiProvider().Preferences().GetIntValue( KBrowserSearchProviderSettingViewId );
    TUid settingViewId( TUid::Uid( id ) );
    TVwsViewId viewToOpen(searchAppId, settingViewId);
    CBrowserAppUi::Static()->ActivateViewL(viewToOpen);
    } 

// End of File
