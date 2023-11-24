// ===================================================================
//
//   (c) Paul Alan Freshney 2012-2023
//   www.freshney.org :: paul@freshney.org :: maximumoctopus.com
//
//   https://github.com/MaximumOctopus/LEDMatrixStudio
//
//   https://maximumoctopus.hashnode.dev/
//
//   C++ Rewrite October 11th 2023
//
// ===================================================================

#include <vcl.h>
#pragma hdrstop

#include "FormSaveRange.h"
#include "LanguageConstants.h"
#include "LanguageHandler.h"

#pragma package(smart_init)
#pragma resource "*.dfm"
TForm20 *Form20;

extern LanguageHandler *GLanguageHandler;

//---------------------------------------------------------------------------
//---------------------------------------------------------------------------

SaveFrameRangeObject OpenFrameRange(int frame_count)
{
	TForm20 *Form20 = new TForm20(Application);

	SaveFrameRangeObject sfto;

	Form20->MatrixFrameCount = frame_count;

	if (Form20->ShowModal() == mrOk)
	{
		sfto.Process = true;
		sfto.StartFrame = Form20->eStartFrame->Text.ToInt() - 1;
		sfto.EndFrame = Form20->eEndFrame->Text.ToInt() - 1;
	}

	delete Form20;

	return sfto;
}

//---------------------------------------------------------------------------
//---------------------------------------------------------------------------


__fastcall TForm20::TForm20(TComponent* Owner)
	: TForm(Owner)
{
	SetGuiLanguageText();
}


void __fastcall TForm20::eStartFrameChange(TObject *Sender)
{
	bOK->Enabled = ValidateInputs();
}


void TForm20::SetGuiLanguageText()
{
  Caption = GLanguageHandler->Text[kSaveARangeOfFrames].c_str();

  lStart->Caption = GLanguageHandler->Text[kStart].c_str();
  lEnd->Caption = GLanguageHandler->Text[kEnd].c_str();

  bOK->Caption = GLanguageHandler->Text[kOK].c_str();
  bCancel->Caption = GLanguageHandler->Text[kCancel].c_str();
}


bool TForm20::ValidateInputs()
{
	int sf = eStartFrame->Text.ToIntDef(-1);
	int ef = eEndFrame->Text.ToIntDef(-1);

	return (sf >= 1 && ef >= 1 && sf <= ef && sf <= MatrixFrameCount && ef <= MatrixFrameCount);
}
