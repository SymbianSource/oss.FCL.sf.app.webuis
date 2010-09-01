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
*     Container of the information about the active settings
*
*/


#ifndef SETTINGS_CONTAINER_H
#define SETTINGS_CONTAINER_H

// INCLUDE FILES
#include <coecntrl.h>
#include <coemop.h>
#include <AknLists.h>

// CONSTANTS
const TInt KWmlSettingsIndexNone = -1;
const TInt KWmlSettingsItemMaxLength = 100;

// FORWARD DECLARATION
class MApiProvider;
class CAknNavigationDecorator;
class CAknSettingStyleListBox;


struct TEncodingStruct
    {
    TUint32 iSystemId;  // System id of the encoding (CHARCONV.h)
    TUint32 iResId;     // Resource string id
    };

// CLASS DECLARATION

/**
*  Container of the settings view
*  @since 1.2
*/
class CSettingsContainer : public CCoeControl, public MEikListBoxObserver
	{
	public:  // construction, destruction

        /**
        * Two-phased constructor.
		* @since 1.2
        * @param aRect Rectangle.
        * @param aPreferences Preferences.
        * @param aCommsModel Comms model.
        */
		static CSettingsContainer* NewLC(
            const TRect& aRect,
            MApiProvider& aApiProvider,
            MObjectProvider& aMopParent
            );

        /**
        * Two-phased constructor.
		* @since 1.2
        * @param aRect Rectangle.
        * @param aPreferences Preferences.
        * @param aCommsModel Comms model.
        */
		static CSettingsContainer* NewL(
            const TRect& aRect,
            MApiProvider& aApiProvider,
            MObjectProvider& aMopParent
            );

        /**
        * Destructor.
        */
		virtual ~CSettingsContainer();

	private: // constructor

        /**
        * Constructor.
        * @param aPreferences Preferences.
        * @param aCommsModel Comms model.
        */
		CSettingsContainer
            ( MApiProvider& aApiProvider, MObjectProvider& aMopParent );

        /**
        * By default constructor is private.
        */
		void ConstructL( const TRect& aRect );

    /**
    * Inherited functions
    */
	public:	// from CCoeControl

        /**
        * Count component controls.
		* @since 1.2
        * @return Number of component controls.
        */
		virtual TInt CountComponentControls() const;

        /**
        * Handles focus changing
        * @param aDrawNow
        */
        void FocusChanged( TDrawNow aDrawNow );

    	/**
        * Handle resource change.
        * @param aType Event type.
        */
        void HandleResourceChange( TInt aType );


        /**
        * Get a component control by index.
		* @since 1.2
        * @param aIndex Index of component control to be returned.
        * @return Component control or NULL.
        */
		virtual CCoeControl* ComponentControl( TInt aIndex ) const;

        /**
        * Handle size change, lay out and draw components.
		*  @since 1.2
        */
		virtual void SizeChanged();

        /**
        * Handle key event. Forwards all key events to the listbox, if present.
		* @since 1.2
        * @param aKeyEvent The key event.
        * @param aType Key event type.
        * @return Response (was the key event consumed?).
        */
		virtual TKeyResponse OfferKeyEventL( const TKeyEvent& aKeyEvent, TEventCode aType );

        /**
        * HandleListBoxEventL
        * From CCoeControl
        *
        */
        void HandleListBoxEventL(CEikListBox* aListBox,TListBoxEvent aEventType);
        
        /**
        * HandleGainingForegroundL
        * Handles Foreground Event.
        */
        void HandleGainingForegroundL();

    /**
    * New functions
    */
	public:

        /**
        * Handles selection key or menu command to change value of an item.
		* @since 1.2
        */
		void ChangeItemL( TBool aSelectKeyWasPressed );

		MApiProvider& ApiProvider() { return iApiProvider; }

		/**
        * Opens the appropriate setting list e.g. Page
        * @since 3.1
        */
		void DisplayCorrectSettingCategoryListL();

		/**
        * Closes the current setting list, if the main list,
        * then leaves settings
        * @return ETrue if settings lists are closed EFalse if still open
		* @since 3.1
        */
		TBool CloseSettingsListL();
		
		
		/**
        * Checks for errors in settings values
        * @return ETrue if settings values are set incorrectly
		* @since 5.0
        */		
		TBool SettingsError();


		/**
		* Indicates if a setting item is modifiable
		*
		* @return TInt
		* @since 3.1
		*/
		TBool IsSettingModifiable() const;
		
		
		/**
        * Saves changes made by the user to the ini file
		* @since 1.2
        */
        void SaveChangesL();



        /**
        * Shows the main setting categories in a list box
        * @since 3.1
        */
        void DisplaySettingCategoriesL();

        void DisplayGeneralSettingsL();
        void DisplayPageSettingsL();
        void DisplayPrivacySettingsL();
        void DisplayWebFeedsSettingsL();
        void DisplayToolbarSettingsL();
        void DisplayShortcutsSettingsL();
        
	/**
    * New functions
    */
    private:
        TInt SetToolbarButtonValues(CArrayFixFlat<TInt> *values, TInt value, CArrayFixFlat<TInt> *displaymap);
        TInt SetShortcutKeysValues(CArrayFixFlat<TInt> *values, TInt value, CArrayFixFlat<TInt> *displaymap);
              
        void AddCategoryListBoxItemL( TInt aResourceId, CDesCArray& aList );
        void ClearListBoxContentL();
        void StoreListBoxIndexL();
        void RestoreListBoxIndexL();

        // Methods for adding settings to the settings lists

        //---------------------------------------------------------------------
        // General Settings
        void AppendDefaultAccessPointL( CDesCArray*& aItemArray,
                                TBuf<KWmlSettingsItemMaxLength>& aItemText );

        void AppendHomePageL( CDesCArray*& aItemArray,
                                TBuf<KWmlSettingsItemMaxLength>& aItemText );

        void AppendPageOverviewL( CDesCArray*& aItemArray,
                                TBuf<KWmlSettingsItemMaxLength>& aItemText );

        void AppendBackListL( CDesCArray*& aItemArray,
                                TBuf<KWmlSettingsItemMaxLength>& aItemText );
        void AppendURLSuffixListL( CDesCArray*& aItemArray, 
                                TBuf<KWmlSettingsItemMaxLength>& aItemText );                                                      
        void AppendHttpSecurityWarningsL( CDesCArray*& aItemArray,
                                TBuf<KWmlSettingsItemMaxLength>& aItemText );

        void AppendEcmaL( CDesCArray*& aItemArray,
                                TBuf<KWmlSettingsItemMaxLength>& aItemText );

        void AppendScriptLogL( CDesCArray*& aItemArray,
                                TBuf<KWmlSettingsItemMaxLength>& aItemText );

        void AppendDownloadsOpenL( CDesCArray*& aItemArray,
                                TBuf<KWmlSettingsItemMaxLength>& aItemText );
                                
        void AppendSearchProviderL( CDesCArray*& aItemArray,
                                TBuf<KWmlSettingsItemMaxLength>& aItemText );
        //---------------------------------------------------------------------
        // Page Settings
        void AppendAutoLoadContentL( CDesCArray*& aItemArray,
                                    TBuf<KWmlSettingsItemMaxLength>& aItemText );

        void AppendAutoFullScreenL( CDesCArray*& aItemArray,
                                    TBuf<KWmlSettingsItemMaxLength>& aItemText );

        void AppendAutoMediaVolumeL( CDesCArray*& aItemArray,
                                     TBuf<KWmlSettingsItemMaxLength>& aItemText );

        void AppendEncodingL( CDesCArray*& aItemArray,
                              TBuf<KWmlSettingsItemMaxLength>& aItemText );

        void AppendPopupBlockingL( CDesCArray*& aItemArray,
                                   TBuf<KWmlSettingsItemMaxLength>& aItemText );

        void AppendAutoRefreshL( CDesCArray*& aItemArray,
                                 TBuf<KWmlSettingsItemMaxLength>& aItemText );

        void AppendFontSizeL( CDesCArray*& aItemArray,
                              TBuf<KWmlSettingsItemMaxLength>& aItemText );

        TInt GetFontSizeString();
        
        
        //---------------------------------------------------------------------
        // Toolbar Settings
        void AppendToolbarOnOffL( CDesCArray*& aItemArray,
                              		TBuf<KWmlSettingsItemMaxLength>& aItemText );
        
        void AppendToolbarButtonsL( CDesCArray*& aItemArray,
                              		TBuf<KWmlSettingsItemMaxLength>& aItemText );

        void AppendShortcutKeysL( CDesCArray*& aItemArray,
                              		TBuf<KWmlSettingsItemMaxLength>& aItemText );

        //---------------------------------------------------------------------
        // Privacy Settings
        void AppendAdaptiveBookmarksL( CDesCArray*& aItemArray,
                                TBuf<KWmlSettingsItemMaxLength>& aItemText );

        void AppendFormDataSavingL( CDesCArray*& aItemArray,
                                TBuf<KWmlSettingsItemMaxLength>& aItemText );

        void AppendCookiesL( CDesCArray*& aItemArray,
                                TBuf<KWmlSettingsItemMaxLength>& aItemText );

        void AppendIMEIL( CDesCArray*& aItemArray,
                                TBuf<KWmlSettingsItemMaxLength>& aItemText );

        //---------------------------------------------------------------------
        // Web feeds Settings
        void AppendAutomaticUpdatingAPL( CDesCArray*& aItemArray,
                                TBuf<KWmlSettingsItemMaxLength>& aItemText );

        void AppendAutomaticUpdatingWhileRoamingL( CDesCArray*& aItemArray,
                                TBuf<KWmlSettingsItemMaxLength>& aItemText );
        //---------------------------------------------------------------------
        // 
        /**
		* Pops up a custom dialog to allow user to set setting items
		*/
		TInt ShowRadioButtonSettingPageL( TInt aTitle,
		                                  CArrayFixFlat<TInt>* aValues,
		                                  TInt aCurrentItem );


        /**
		* Pops up a custom dialog to allow user to set setting items
		*/
		TBool ShowRadioButtonSettingPageBoolL( TInt aTitle,
                		                       CArrayFixFlat<TInt>* aValues,
                		                       TInt* aCurrentItem );

        /**
		* Appends two text from resources to display it on lisbox
		*/
		void CreateItemFromTwoStringsL(
        		                    TInt aFirst,
        		                    TInt aSecond,
        		                    TBuf<KWmlSettingsItemMaxLength>& aResult);

        TInt MapCurrentItem( TUint aCurrentItem ) const;

        /**
        * Gets encoding information as parameters.
        * Creates an encoding object and add the new object to iEncodingArray.
        */
        void AddEncodingL( TUint32 aSystemId, TUint32 aResId );

        /**
        * Add encodings here.
        * To add or remove encodings modify this function!
        */
        void CreateEncodingArrayL();

		/**
        * Check if the added encodings are supported by the system. Removes those that are not.
        */
        void RemoveUnsupportedEncodingsL();

        /**
        * Notify BrowserControl that a setting has been changed in the UI
        */
        void NotifySettingChanged( TInt aSettingItem, TInt aSettingValue );

		/**
		*  Query user to delete Form data plus Password data
		*/
		void ShowFormDataDeleteAllQueryDialogL( );
		/**
		*  Query user to delete Form Password data
		*/
		void ShowFormDataDeletePasswordQueryDialogL( );

		/**
		*  Query user to select fromm user defined AP
		*/
        void SelectUserDefinedAPL( TUint32& id );
        
        /**
        *  Launch Search Application Settings View.
        */
        void RunSearchSettingsL();

#ifdef __SERIES60_HELP
        /**
        * Get help context for the control.
        * @param aContext The context that is filled in.
        * @return None.
        */
        void GetHelpContext( TCoeHelpContext& aContext ) const;
           
#endif // __SERIES60_HELP

    private:

        // Media Volume indices
        enum TMediaVolumeIndices
	        {
	        EVolumeMute = 0,
	        EVolumeLevel1,
	        EVolumeLevel2,
	        EVolumeLevel3,
	        EVolumeLevel4
	        };

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

    private:	// members

		MApiProvider& iApiProvider;                         // not owned
		CAknNavigationDecorator* iNaviDecorator;            // not owned

        // Settings List Box related members
        CAknSettingStyleListBox* iSettingListBox;           // OWNED
        CArrayFixFlat<TUint>* iSettingIndex;                // OWNED
        TInt iMainSettingCurrentIndex; // hold the most recent index in the main setting list box
        TInt iMainSettingTopPosIndex;
        TInt iSubSettingCurrentIndex;  // hold the most recent index in the sub setting list boxes
        TInt iSubSettingTopPosIndex;
        TBool iActionCancelled;
        TBool iPenEnabled;    // stores platform pen enabled flag value

        TSettingCategory iCurrentSettingCategory;

        // The settings are only updated when we leave the settings
        // view to avoid unnecessary redraws
        TInt iFontSize;
        TUint32 iEncoding;
        TBool iTextWrap;

        // Holds a 'list' of all encodings
        CArrayFixFlat<TEncodingStruct>* iEncodingArray;     // OWNED
        TBool iSelectionKeyDownPressed;    
        
        TBool iFlashPluginPresent;
	};

#endif // SETTINGS_CONTAINER_H

// End of File
