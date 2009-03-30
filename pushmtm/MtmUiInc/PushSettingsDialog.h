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
* Description:  Class definition of CPushSettingsDialog.
*
*/



#ifndef PUSHSETTINGSDIALOG_H
#define PUSHSETTINGSDIALOG_H

//  INCLUDES

#include "PushMtmSettings.h"
#include <AknForm.h>
#include <eiklbo.h>
#include <ConeResLoader.h>

// FORWARD DECLARATIONS

class CMsvSession;
class CAknSettingStyleListBox;
class CEikImage;

// CLASS DECLARATION

/**
* Avkon environment is required.
* Waiting dialog.
* Usage.
* #include "PushSettingsDialog.h"
* CPushSettingsDialog* dlg = new (ELeave) CPushSettingsDialog;
* dlg->ExecuteLD( msvSession );
*
* Note that we could use CMuiuSettingsArray defined in MuiuSettingsArray.h, 
* but it seems that our PI list handling (Ignore services from) makes 
* this dialog so complicated, that it is better not to use CMuiuSettingsArray, 
* which would make the desing more complicated, because it does not support 
* such functionality that PI list handling requires.
*/
class CPushSettingsDialog : public CAknDialog, 
                            public MEikListBoxObserver
    {
    public: // Constructors

        /**
        * Constructor.
        * @param aMsvSession Message Server session.
        */
        CPushSettingsDialog( CMsvSession& aMsvSession );
    
    public: // New functions

        /**
        * Execute the dialog with this member.
        * @return None.
        */
        void ExecuteLD();

    private: // New data types

        enum TSettingType
            {
            EServiceReception, ///< On or Off
            EServiceLoadingType ///< Automatic or Manual
            };

    private: // Constructors and destructor

        /**
        * Symbian OS constructor.
        */
        void ConstructL();

        /**
        * Destructor.
        */
        ~CPushSettingsDialog();

    private: // New functions
        
        /**
        * Set up title text.
        * @return none
        */
        void SetUpStatusPaneL();

        /**
        * Restore the previous title text.
        * @return none
        */
        void RestoreStatusPaneL();

        /**
        * Construct model.
        * @return none
        */
        void ConstructModelL();

        /**
        * Update setting page model.
        * @return none
        */
        void UpdateSettingListBoxModelL();

        /**
        * Update setting page view.
        * @return none
        */
        void UpdateSettingListBoxViewL() const;

        /**
        * Apply changes of the model.
        * @return None.
        */
        void ApplyModelChangesL();

        /**
        * Show radio button setting page for all 
        * items except for EIgnoreServicesFrom.
        * @return none
        */
		void ShowRadioButtonSettingPageL( const TSettingType aSettingType );

        /**
        * Show the confirmation dialog about the changing of the service loading 
        * type from 'manual' to 'automatic'.
        * @return ETrue if the user confirmed. EFalse otherwise.
        */
		TBool ShowServiceLoadingTypeConfirmationL() const;

        /**
        * Change current setting. No setting page usage is preferred.
        * @return none
        */
        void ChangeCurrentSettingWithoutSettingPageL();

        /**
        * Change current setting with the usage of setting page.
        * @return none
        */
        void ChangeCurrentSettingL();

        /**
        * Append an item to the list box' item text array.
        * @param aSettingType Setting item type.
        * @param aTitleResId Title resource id. If it is KErrNotFound, 
        *        then aTitleText is used.
        * @param aValueResId Value resource id. If it is KErrNotFound, 
        *        then aValueText is used.
        * @param aTitleText See above.
        * @param aValueText See above.
        * @return none
        */
        void ConstructAndAppendItemTextL
            ( const TSettingType aSettingType, 
            const TInt aTitleResId, const TInt aValueResId, 
            const TDesC& aTitleText = KNullDesC(), 
            const TDesC& aValueText = KNullDesC() );

        /**
        * Construct the title text for a setting.
        * param aSettingType Type of the setting.
        * @return The constructed title text.
        */
        HBufC* ConstructSettingTitleL( const TSettingType aSettingType ) const;

        /**
        * Get the resource ids of those strings that a given binary setting can have.
        * Note. The following functions have strong relations: 
        * ConstructSettingValueArray, CurrentlySelectedSettingValueResId, 
        * ConstructSettingValueArrayL, UpdateSettingL!
        * param aSettingType Type of the setting.
        * param aVal1ResId The id of the first value string.
        * param aVal2ResId The id of the second value string.
        * @return None.
        */
        void ConstructSettingValueArray
            ( const TSettingType aSettingType, TInt& aVal1ResId, TInt& aVal2ResId ) const;

        /**
        * Get the resource ids of the currently set value string.
        * Note. The following functions have strong relations: 
        * ConstructSettingValueArray, CurrentlySelectedSettingValueResId, 
        * ConstructSettingValueArrayL, UpdateSettingL!
        * param aSettingType Type of the setting.
        * param aIndex The index of this currently selected value is also returned.
        * @return Resource id.
        */
        TInt CurrentlySelectedSettingValueResId
            ( const TSettingType aSettingType, TInt& aIndex ) const;

        /**
        * Get the value strings of those strings that a given binary setting can have.
        * Note. The following functions have strong relations: 
        * ConstructSettingValueArray, CurrentlySelectedSettingValueResId, 
        * ConstructSettingValueArrayL, UpdateSettingL!
        * param aSettingType Type of the setting.
        * param aIndex The index of this currently selected value is also returned.
        * @return The value array.
        */
        CDesCArrayFlat* ConstructSettingValueArrayL
            ( const TSettingType aSettingType, TInt& index ) const;

        /**
        * Set in the model the right setting value using the given index.
        * Note. The following functions have strong relations: 
        * ConstructSettingValueArray, CurrentlySelectedSettingValueResId, 
        * ConstructSettingValueArrayL, UpdateSettingL!
        * param aSettingType Type of the setting.
        * param aIndex The index of the selected value.
        * @return None.
        */
        void UpdateSettingL
            ( const TSettingType aSettingType, const TInt index ) const;

    private: // Functions from base classes

        TKeyResponse OfferKeyEventL
            ( const TKeyEvent& aKeyEvent, TEventCode aType );

        void DynInitMenuPaneL( TInt aResourceId, CEikMenuPane* aMenuPane );

       	void PreLayoutDynInitL();

        void ProcessCommandL( TInt aCommand );

        TBool OkToExitL( TInt aButtonId );

        void HandleListBoxEventL( CEikListBox* aListBox, TListBoxEvent aEventType );

#ifdef __SERIES60_HELP

        void GetHelpContext( TCoeHelpContext& aContext ) const;

#endif //__SERIES60_HELP

    private: // Data members

        CPushMtmSettings*   iModel;                         ///< Settings: the model. Has.
        CMsvSession&        iMsvSession;                    ///< Message Server session.
        RConeResourceLoader iResourceLoader;                ///< Resource file loader.
        TFileName iResourceFile;
        CAknSettingStyleListBox*        iSettingListBox;    ///< List box contained by the dialog. Uses.
        CDesCArrayFlat*     iSettingListBoxItemTextArray;   ///< List box' item text array. Uses.
        CArrayFixFlat<TSettingType>*    iSettingType;       /**< A given array item 
                                                            * determines the type of 
                                                            * the item of the list 
                                                            * with the same index. Has.
                                                            */
        TBool               iNewTitleTextUsed;              ///< Indicate if new title text is used.
        HBufC*              iPreviousTitleText;             ///< Restore it on termination. Has.
        TBool               iRestoreSP;     /**< EFalse, if it is not necessary 
                                            * to restore status pane. */
    };

#endif // PUSHSETTINGSDIALOG_H
            
// End of file.
