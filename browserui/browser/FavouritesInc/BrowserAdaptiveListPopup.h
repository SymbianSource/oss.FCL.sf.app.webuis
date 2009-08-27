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
* Description:  Header file for CBrowserPopupList and CBrowserAdaptiveListPopup
*
*
*/

#ifndef BROWSERADAPTIVELISTPOPUP_H
#define BROWSERADAPTIVELISTPOPUP_H

// INCLUDES
#include <aknpopuplayout.h>
#include <AknLists.h>

//for testing purposes you can define fake ahle engine
//wich will provide random results
// CONSTANTS
// UID of view
const TUid KViewId = {1};

enum TParentType
    {
    EGotoPane,
    EBookmarksGotoPane
    };

class CRecentUrlStore;


class CAknsListBoxBackgroundControlContext;

// CLASS DECLARATION
class CBrowserPopupList : public CAknSingleGraphicPopupMenuStyleListBox
{
public :
	CBrowserPopupList(TParentType aParentType);
    ~CBrowserPopupList();
	
	void SetHighLight(TBool aHighLight);

	TBool HighLightEnabled();
			
	void ConstructL(const CCoeControl* aParent, TInt aFlags);

    void SizeChanged();   
    void Draw(const TRect& /*aRect*/) const;

protected :
    TParentType iParentType;
    TBool iOnePartFrameBitmapUsed;
};



/**
*  CBrowserAdaptiveListPopup view class.
* 
*/
class CBrowserAdaptiveListPopup : 
	public CCoeControl, 
	public MCoeControlObserver, public MEikListBoxObserver

    {
    public: // Constructors and destructor

        /**
		*C++ Constructor
		*/
		CBrowserAdaptiveListPopup( CEikEdwin* aGotoPaneEdit, 
		        CCoeControl* aParent, 
		        TParentType aParentType,
		        TBool aSearchFeature  = EFalse );
			
		/**
        * default constructor.
        */
        void ConstructL();

        /**
        * Destructor.
        */
        virtual ~CBrowserAdaptiveListPopup();


    public: // Functions from base classes

		TKeyResponse OfferKeyEventL( const TKeyEvent& aKeyEvent, TEventCode aType );

		void HidePopupL();

		TBool IsInDirectoryMode();

		void SetDirectoryModeL(TBool aDirMode);

        TBool IsOpenDirToShow();

        TBool IsPoppedUp();	
		
		void HandleResourceChange( TInt aType );

        void HandleResourceChangeL( TInt aType );

        void SetUrlSuffixList(HBufC* urlSuffixList);

		void SetMaxRecentUrls (TInt maxRecentUrls);
		
        /**
        * Sets the OrdinalPosition of GotoPane
        * @param aPos The OrdinalPosition
        */
        void SetOrdinalPosition( TInt aPos );
        /**
         * From CCoeControl.     
         * Handles pointer events
         */
        void HandlePointerEventL(const TPointerEvent& aPointerEvent);
        void HandleListBoxEventL(CEikListBox* aListBox,TListBoxEvent aEventType);
	private: // Functions from base classes	
        
		void HandleControlEventL(CCoeControl* aControl,enum MCoeControlObserver::TCoeEvent aEventType);		
		
		TInt CountComponentControls() const;
		
		CCoeControl* ComponentControl( TInt aIndex ) const;
		
		void SetIconsL();

		void CreateURLCompletionListL();
		
	private:
	
	    /**
        * ShowPopupList handles displaying the adaptive list box
		* @since 
		* @param aRelayout Whether or not this popup list is being displayed as the result
		* 	of a layout changed event (in which case we need to make sure that the popup 
		*   is redrawn in the correct position) Set to EFalse by default.
        * @return void
        */
        void ShowPopupListL(TBool aRelayout = EFalse);
        void UpdateScrollBarPositionL(const TInt aPosition);
		void HandleItemClickedL(CEikListBox* aListBox );

		


    private: // from CCoeControl
		
		HBufC* iPrevGotoContent;//to compare if there were really any changes!   
		
    private: // private members
				
		CBrowserPopupList* iList;//owned
		CDesCArrayFlat* iItems;//owned
		CDesCArrayFlat* iItemNames;//owned
		CDesCArrayFlat* iItemNamesToShow;//owned
		CEikEdwin* iEditor;//Editor of the GotoPane
		TInt iItemNo;
		TInt iPrevItem;
		TBool iPoppedUp;//it shows wheter it is popped up
		TAknPopupWindowLayoutDef iLayout;
		HBufC* iFirstGotoContent;
		CRecentUrlStore*	iRecentUrlStore; // not owned
		TBool iDirectoryMode;//this shows wheter it is in a directory
        TBool iOpenCBA;//this shows whether Open directory shuold be showed
        CCoeControl *iParent;
        TAknLayoutRect iCover, iShadow, iOutline, iInside;
        CFormattedCellListBoxItemDrawer *iItemDrawer;
        TParentType iParentType;
        TBool iUrlCompletionMode;
        HBufC* iUrlSuffixBuf;
        TBool iTouchSupported;
        TInt iMaxRecentUrlsToShow;
        // shows whether search feature is enabled or not.
        TBool iSearchFeature; 
    };

#endif  // BROWSERADAPTIVELISTPOPUP_H

// End of File
