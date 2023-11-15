

#include <vcl.h>
#pragma hdrstop

#include <algorithm>

#include "FormNewProject.h"
#include "LanguageConstants.h"
#include "LanguageHandler.h"
#include "PresetHandler.h"
#include "SystemSettings.h"
#include "Utility.h"

#pragma package(smart_init)
#pragma resource "*.dfm"
TForm16 *Form16;

extern LanguageHandler *GLanguageHandler;
extern PresetHandler *GPresetHandler;
extern SystemSettings *GSystemSettings;

//---------------------------------------------------------------------------
//---------------------------------------------------------------------------

ProjectSettings OpenNewProject(ProjectSettings &OldProjectSettings, bool appstatus)
{
	TForm16 *Form16 = new TForm16(Application);

	Form16->ClearStatus = appstatus;

	ProjectSettings NewProjectSettings;

	NewProjectSettings.Valid = False;
	NewProjectSettings.Mode = MatrixMode::kMono;
	NewProjectSettings.Width = -1;
	NewProjectSettings.Height = -1;
	NewProjectSettings.Clear = false;
	NewProjectSettings.ShapeCustom = CustomShape::kNone;

	Form16->BuildFrom(OldProjectSettings);

	if (Form16->ShowModal() == mrOk)
	{
		Form16->SetTo(NewProjectSettings);
	}

	delete Form16;

	return NewProjectSettings;
}

//---------------------------------------------------------------------------
//---------------------------------------------------------------------------

__fastcall TForm16::TForm16(TComponent* Owner)
	: TForm(Owner)
{
}


void __fastcall TForm16::FormCreate(TObject *Sender)
{
	SetGUILanguageText();

	cbFrames->Items->Add("1");
	cbFrames->Items->Add("2");
	cbFrames->Items->Add("4");
	cbFrames->Items->Add("5");
	cbFrames->Items->Add("10");
	cbFrames->Items->Add("16");
	cbFrames->Items->Add("20");
	cbFrames->Items->Add("25");
	cbFrames->Items->Add("32");
	cbFrames->Items->Add("50");
	cbFrames->Items->Add("64");
	cbFrames->Items->Add("100");
}


void TForm16::SetGUILanguageText()
{
	Caption = GLanguageHandler->Text[kCreate].c_str();

	tsCustom->Caption = GLanguageHandler->Text[kCustom].c_str();

	gbMatrixOptions->Caption = GLanguageHandler->Text[kMatrixOptions].c_str();

	for (int t = 0; t < 5; t++)
	{
		cbMatrixType->Items->Add(Utility::MatrixModeAsStringFromInt(t).c_str());
	}

	cbMatrixType->ItemIndex = 0;


	cbCustomShape->Items->Add(GLanguageHandler->Text[kNoCustomShape].c_str());
	cbCustomShape->Items->Add(GLanguageHandler->Text[kCircle].c_str());
	cbCustomShape->Items->Add(GLanguageHandler->Text[kFrameBorderNoCentre].c_str());
	cbCustomShape->Items->Add(GLanguageHandler->Text[kTriangle].c_str());
	cbCustomShape->ItemIndex = 0;

	lBackground->Caption = GLanguageHandler->Text[kBackground].c_str();
	rbCommon->Caption = GLanguageHandler->Text[kCommon].c_str();
	rbAll->Caption = GLanguageHandler->Text[kAll].c_str();
	Label11->Caption = GLanguageHandler->Text[kBorder].c_str();


	tsFromPreset->Caption = GLanguageHandler->Text[kFromPreset].c_str();

	Label6->Caption = GLanguageHandler->Text[kType].c_str();
	Label7->Caption = GLanguageHandler->Text[kWidth].c_str();
	Label9->Caption = GLanguageHandler->Text[kHeight].c_str();



	gbPixelShape->Caption = GLanguageHandler->Text[kPixelShape].c_str();

	gbAnimation->Caption = GLanguageHandler->Text[kAnimation].c_str();
	Label3->Caption = GLanguageHandler->Text[kStartWith].c_str();
	Label5->Caption = GLanguageHandler->Text[kAnimationFrames].c_str();

	cbClearAll->Caption = GLanguageHandler->Text[kClearAllAnimationData].c_str();


	bOK->Caption = GLanguageHandler->Text[kCreate].c_str();
	bCancel->Caption = GLanguageHandler->Text[kCancel].c_str();
}


void TForm16::BuildFrom(ProjectSettings &ps)
{
	switch (ps.Pixel)
	{
	case PixelShape::kSquare:
		rbPixelSquare->Checked = true;
		break;
	case PixelShape::kCircle:
		rbPixelCircle->Checked = true;
		break;
	case PixelShape::kRoundRect:
		rbPixelRoundRect->Checked = true;
	}

	sBackground->Brush->Color = TColor(ps.Background);

	cbMatrixType->ItemIndex  = ps.MatrixModeToInt();

	cbMatrixTypeChange(nullptr);

	// ===========================================================================

	if (ps.Width < 1 || ps.Height < 1)
	{
		ps.Width  = 8;
		ps.Height = 8;
	}

	OldWidth  = ps.Width;
	OldHeight = ps.Height;

	if (ps.SizeType)
	{
		rbCommon->Checked = true;
	}
	else
	{
		rbAll->Checked = true;
	}

	rbCommonClick(nullptr);

//	BuildPresetList();  // to do
}


void TForm16::SetTo(ProjectSettings &ps)
{
	ps.Valid = true;

	if (pcNew->ActivePageIndex == 0)
	{
		ps.Mode = Utility::MatrixModeFromInt(cbMatrixType->ItemIndex);
		ps.Width = cbWidth->Text.ToInt();
		ps.Height = cbHeight->Text.ToInt();
	}
	else
	{
		ps.Mode = Utility::MatrixModeFromInt(lPresetType->Tag);
		ps.Width = lPresetWidth->Caption.ToInt();
		ps.Height = lPresetHeight->Caption.ToInt();
	}

	ps.CustomShapeFromInt(cbCustomShape->ItemIndex);
	ps.CustomShapeParam = cbCustomShapeParam->ItemIndex;

	ps.Background = sBackground->Brush->Color;

	ps.Clear = cbClearAll->Checked;
	ps.Special = cbFrames->Text.ToInt();

	if (rbPixelSquare->Checked)
	{
		ps.Pixel = PixelShape::kSquare;
	}
	else if (rbPixelCircle->Checked)
	{
		ps.Pixel = PixelShape::kCircle;
	}
	else
	{
		ps.Pixel = PixelShape::kRoundRect;
	}

	if (rbCommon->Checked)
	{
		ps.SizeType = true;
	}
	else
	{
		ps.SizeType = false;
	}
}


void __fastcall TForm16::bOKClick(TObject *Sender)
{
	if (Utility::ValidateNumber(cbFrames->Text.c_str(), 100000))
	{
		if (ClearStatus)
		{
			if (cbClearAll->Checked)
			{
				if (MessageDlg(GLanguageHandler->Text[kAreYouSureYouWantToDeleteTheCurrentMatrix].c_str(), mtWarning, mbYesNo, 0) == mrYes)
				{
					ModalResult = mrOk;
				}
			}
			else
			{
				ModalResult = mrOk;
			}
		}
		else
		{
			ModalResult = mrOk;
		}
	}

	cbFrames->SelectAll();
	cbFrames->SetFocus();
}


void __fastcall TForm16::cbCustomShapeChange(TObject *Sender)
{
	cbCustomShapeParam->Clear();

	switch (cbCustomShape->ItemIndex)
	{
	case customShapeNone:
		cbCustomShapeParam->Items->Add(GLanguageHandler->Text[kNA].c_str());
		break;
	case customShapeCircle:
		cbCustomShapeParam->Items->Add(GLanguageHandler->Text[kNA].c_str());
		break;
	case customShapeJustBorders:
	{
		int width = cbWidth->Text.ToInt();
		int height = cbWidth->Text.ToInt();

		int c = std::floor(std::min(width, height / 2));

		for (int t = 1; t <= c; t++)
		{
			cbCustomShapeParam->Items->Add(t);
		}

		break;
	}
	case customShapeTriangle:
		cbCustomShapeParam->Items->Add(GLanguageHandler->Text[kNA].c_str());
        break;
	}

	cbCustomShapeParam->ItemIndex = 0;
}


void __fastcall TForm16::cbMatrixTypeChange(TObject *Sender)
{
	int status = false;

	if (cbMatrixType->ItemIndex == 3)
	{
		status = true;
	}

	lBackground->Visible = status;
	sBackground->Visible = status;

	UpdateHelp(Utility::MatrixModeFromInt(cbMatrixType->ItemIndex));
}


void __fastcall TForm16::cbPresetsChange(TObject *Sender)
{
	std::wstring name = cbPresets->Text.c_str();

	MatrixPreset mp = GPresetHandler->Load(GSystemSettings->App.LMSFilePath + L"presets\\" + name + L".ledspreset");

	lPresetWidth->Caption  = mp.Width;
	lPresetHeight->Caption = mp.Height;

	lPresetType->Caption   = mp.MatrixModeText.c_str();
	lPresetType->Tag       = mp.MatrixModeTag;
}


void __fastcall TForm16::cbWidthChange(TObject *Sender)
{
	cbCustomShapeChange(nullptr);
}


void __fastcall TForm16::rbCommonClick(TObject *Sender)
{
	static const std::wstring CCommonSizes[] { L"1", L"2", L"4", L"5", L"7", L"8", L"12", L"16", L"24", L"32", L"48", L"60", L"64", L"128", L"256" };

	cbWidth->Clear();
	cbHeight->Clear();

	if (rbAll->Checked)
	{
		for (int x = 1; x <= 256; x++)
		{
			std::wstring s = std::to_wstring(x);

			cbWidth->Items->Add(s.c_str());
			cbHeight->Items->Add(s.c_str());
		}

		cbWidth->ItemIndex = cbWidth->Items->IndexOf(OldWidth);
		cbHeight->ItemIndex = cbHeight->Items->IndexOf(OldHeight);
	}
	else
	{
		for (int x = 0; x < 16; x++)
		{
			cbWidth->Items->Add(CCommonSizes[x].c_str());
			cbHeight->Items->Add(CCommonSizes[x].c_str());
		}

		cbWidth->ItemIndex  = cbWidth->Items->IndexOf(OldWidth);
		cbHeight->ItemIndex = cbHeight->Items->IndexOf(OldHeight);

		if (cbHeight->ItemIndex == -1)
		{
			cbHeight->ItemIndex = 7;
		}

		if (cbWidth->ItemIndex == -1)
		{
			cbWidth->ItemIndex = 7;
		}
	}

	OldWidth = cbWidth->Text;
	OldHeight = cbHeight->Text;
}


void __fastcall TForm16::sBackgroundMouseDown(TObject *Sender, TMouseButton Button,
		  TShiftState Shift, int X, int Y)
{
 //	if (cdNewProject->Execute)
//	{
//		sBackground->Brush->Color = cdNewProject->Color;
//	}
}


void __fastcall TForm16::rbPixelSquareClick(TObject *Sender)
{
	TShape *s = (TShape*)Sender;

	switch (s->Tag)
	{
	case 0:
		rbPixelSquare->Checked = true;
		break;
	case 1: rbPixelCircle->Checked = true;
		break;
	case 2: rbPixelRoundRect->Checked = true;
		break;
	}
}


void TForm16::UpdateHelp(MatrixMode mode)
{
	switch (mode)
	{
	case MatrixMode::kMono:
		mHelp->Text = GLanguageHandler->Text[kNPModeMono].c_str();
		break;
	case MatrixMode::kBiSequential:
		mHelp->Text = GLanguageHandler->Text[kNPModeBiSequential].c_str();
		break;
	case MatrixMode::kBiBitplanes:
		mHelp->Text = GLanguageHandler->Text[kNPModeBiBitplane].c_str();
		break;
	case MatrixMode::kRGB:
		mHelp->Text = GLanguageHandler->Text[kNPModeRGB].c_str();
		break;
	case MatrixMode::kRGB3BPP:
		mHelp->Text = GLanguageHandler->Text[kNPModeRGB3BPP].c_str();
		break;
	}
}