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

#include <algorithm>

#include "ColourUtility.h"
#include "Convert.h"
#include "ExportMonoBi.h"
#include "ExportUtility.h"
#include "Formatting.h"
#include "Matrix.h"
#include "SystemSettings.h"
#include "Utility.h"

extern SystemSettings *GSystemSettings;


namespace ExportMonoBi
{
	bool CreateExportAnimation(TheMatrix *matrix, ExportOptions teo, std::vector<std::wstring> &output, int &entrycount, std::vector<std::wstring> &unique_items)
	{
		std::wstring cdescription = L"";
		std::wstring op = L"";
		std::wstring prefix = L"";
		std::wstring vartype = L"";
		std::wstring spacingstring = L"";
		DataOut tdo;

		entrycount = 0; // total of all entries added to data variable in output

		int lc = 0;
		int rc = 0;

		int MatrixDataCount = std::max(matrix->Details.Height, matrix->Details.Width);

		std::vector<std::wstring> *MatrixData[MatrixDataCount];

		for (int t = 0; t < MatrixDataCount; t++)
		{
			MatrixData[t] = new std::vector<std::wstring>;
		}

		auto baaAddContentBySize = [cdescription, teo](const std::wstring s, int frame, int rowcount) -> std::wstring
		{
			std::wstring m = s.substr(0, s.length() - 2); // trims last (and unnecessary) ", " from data

			switch(teo.Code.Language)
			{
			case ExportLanguage::kCSV:
				return GSystemSettings->App.OpenBracket + m + GSystemSettings->App.CloseBracket + L";";
			case ExportLanguage::kPICAXE:
				return L"EEPROM (" + m + L")";
			case ExportLanguage::kC1Dim:
				return teo.DataPadding + s;
			case ExportLanguage::kC2Dim:
				if (rowcount == 0)
					return teo.DataPadding + L"{" + m + L",  // " + cdescription + L" " + std::to_wstring(frame);
				else if (rowcount == -1)
					return teo.DataPadding + L" " + m + L"},";

				return teo.DataPadding + L" " + s;
			case ExportLanguage::kCFastLED:
				return teo.DataPadding + s;
			case ExportLanguage::kPython1Dim:
				return teo.DataPadding + s + L"";
			case ExportLanguage::kPython2Dim:
				if (rowcount == 0)
					return teo.DataPadding + L"[" + s + L"  # " + cdescription + L" " + std::to_wstring(frame);
				else if (rowcount == -1)
					return teo.DataPadding + L" " + m + L"],";

			   	return L" " + s;
			case ExportLanguage::kMicrochip:
				return L"dt " + m;
			case ExportLanguage::kPascal:
				return L"data : array[0..__LEDCount] of integer = (" + m + L");";
			case ExportLanguage::kSpecial:
				return s;
			}

			return L"";
		};

		auto baaProcessUnique = [unique_items](const std::wstring s) -> std::wstring
		{
			if (unique_items.size() == 0)
			{
				return s;
			}
			else
			{
				std::wstring m = s;

				for (int t = 0; t < unique_items.size(); t++)
				{
					m = Utility::ReplaceString(m, unique_items[t], std::to_wstring(t));
				}

				return m;
			}
		};

		// ===========================================================================

		prefix = ExportUtility::GetNumberFormat(teo.Code.Language, teo.Code.Format);

		if (teo.Code.CleanMode)
		{
			spacingstring = L" ";

			teo.Code.Language = ExportLanguage::kSpecial;
		}
		else
		{
			spacingstring = L", ";
		}

		// ===========================================================================

		if (teo.Code.IncludePreamble)
		{
			if (teo.ExportMode == ExportSource::kAnimation)
			{
				cdescription = GLanguageHandler->Text[kFrame];
			}
			else
			{
				cdescription = GLanguageHandler->Text[kMemory];
			}

			// =========================================================================

			ExportUtility::GetPreamble(teo, output, false, matrix->Details.Comment);

			ExportUtility::GetSpacerLine(teo.Code.Language, output);
			output.push_back(L"");
		}

		// =========================================================================
		// =========================================================================
		// =========================================================================

		op = L"";
		lc = 0;
		rc = 0;

		vartype = ExportUtility::GetVariableType(teo.Code.Language, teo.Code.Size) +
			 ExportUtility::GetVariableID(teo.Code.Language);

		if (!vartype.empty())
		{
			output.push_back(vartype);
		}

		teo.DataPadding = Formatting::PadString(L' ', vartype.length());

		entrycount = 0; // total of all entries added to data variable in output

		// =========================================================================
		// =========================================================================

		for (int t = teo.Code.StartFrame; t <= teo.Code.EndFrame; t++)
		{
			if (teo.Code.Language == ExportLanguage::kCFastLED)
			{
				output.push_back(ExportUtility::GetVariableIDFrameIn(teo.Code.Language, t));
			}

			// =========================================================================

			for (int i = 0; i < MatrixDataCount; i++)
			{
				MatrixData[i]->clear();
			}

			if (teo.Code.Source == ReadSource::kRows)
			{
				for (int y = teo.Code.SelectiveStart - 1; y < teo.Code.SelectiveEnd; y++)
				{
					tdo = ExportRowData(matrix, teo, t, y, spacingstring);

					for (int i = 0; i < tdo.Count; i++)
					{
						if (!tdo.Data[i].empty())
						{
							MatrixData[y]->push_back(baaProcessUnique(prefix + tdo.Data[i]) + spacingstring);
						}
					}

					entrycount += tdo.Count;
				}
			}

			if (teo.Code.Source == ReadSource::kColumns)
			{
				for (int x = teo.Code.SelectiveStart - 1; x < teo.Code.SelectiveEnd; x++)
				{
					tdo = ExportColumnData(matrix, teo, t, x, spacingstring);

					for (int i = 0; i < tdo.Count; i++)
					{
						if (!tdo.Data[i].empty())
						{
							MatrixData[x]->push_back(baaProcessUnique(prefix + tdo.Data[i]) + spacingstring);
						}
					}

					entrycount += tdo.Count;
				}
			}

			// ===========================================================================
			// ===========================================================================
			// row data
			// ===========================================================================
			// ===========================================================================

			if (teo.Code.Content != LineContent::kBytes)	// maintain data when saving in blocks
			{
				op = L"";
			}

			if (teo.Code.Source == ReadSource::kRows)
			{
				int start = 0;
				int end = 0;
				int delta = 0;

				if (teo.Code.Orientation == InputOrientation::kTopBottomLeftRight)
				{
					start = teo.Code.SelectiveStart - 1;
					end   = teo.Code.SelectiveEnd - 1;
					delta   = 1;
				}
				else
				{
					start = teo.Code.SelectiveEnd - 1;
					end   = teo.Code.SelectiveStart - 1;
					delta   = -1;
				}

			    int	y = start;

				while (y != 999)
				{
					if (teo.Code.Content == LineContent::kRowCol)
					{
						op = L"";
					}

					for (int z = 0; z < MatrixData[y]->size(); z++)
					{
						op = op + (*MatrixData[y])[z];

						if (teo.Code.Content == LineContent::kBytes)
						{
							lc++;

							if (lc == teo.Code.LineCount)
							{
								if (!op.empty())
								{
									output.push_back(baaAddContentBySize(op, t, rc));
                                }

								lc = 0;
								op = L"";
								rc++;
							}
						}
					}

					if (teo.Code.Content == LineContent::kRowCol)
					{
						ExportUtility::AddContentByRowCol(teo, op, output);
					}

					y += delta;

					if (teo.Code.Orientation == InputOrientation::kTopBottomLeftRight)
					{
						if (y > end || y < 0) y = 999;
					}
					else
					{
						if (y < end || y < 0) y = 999;
					}
				}

				switch (teo.Code.Content)
				{
				case LineContent::kFrame:
					ExportUtility::AddContentByFrame(teo, op, t, output);

				case LineContent::kBytes:

					switch (teo.Code.Language)
					{
					case ExportLanguage::kC2Dim:
					case ExportLanguage::kPython2Dim:
						if (!op.empty())
						{
							output.push_back(baaAddContentBySize(op, t, -1));
                        }

						op = L"";
						lc = 0;
						rc = 0;
					}
				}
			}

			// ===========================================================================
			// col data
			// ===========================================================================

			if (teo.Code.Source == ReadSource::kColumns)
			{
				switch (teo.Code.Orientation)
				{
				case InputOrientation::kTopBottomLeftRight:
				case InputOrientation::kBottomTopRightLeft:
				{
					int start = 0;
					int end = 0;
					int delta = 0;

					if (teo.Code.Orientation == InputOrientation::kTopBottomLeftRight)
					{
						start = teo.Code.SelectiveStart - 1;
						end   = teo.Code.SelectiveEnd - 1;
						delta = 1;
					}
					else
					{
						start = teo.Code.SelectiveEnd - 1;
						end   = teo.Code.SelectiveStart - 1;
						delta = -1;
					}

					int y = start;

					while (y != 999)
					{
						if (teo.Code.Content == LineContent::kRowCol)
						{
							op = L"";
						}

						for (int z = 0; z < MatrixData[y]->size(); z++)
						{
							op += (*MatrixData[y])[z];

							if (teo.Code.Content == LineContent::kBytes)
							{
								lc++;

								if (lc == tdo.Count)
								{
									if (!op.empty())
									{
										output.push_back(baaAddContentBySize(op, t, rc));
                                    }

									lc = 0;
									op = L"";
									rc++;
								}
							}
						}

						if (teo.Code.Content == LineContent::kRowCol)
						{
							if (!op.empty())
							{
								ExportUtility::AddContentByRowCol(teo, op, output);
							}
						}

						y += delta;

						if (teo.Code.Orientation == InputOrientation::kTopBottomLeftRight)
						{
							if (y > end || y < 0) y = 999;
						}
						else
						{
							if (y < end || y < 0) y = 999;
						}
					}
				}
				case InputOrientation::kSure24x16:
					for (int y = 7; y >= 0; y--)
					{
						for (int z = 0; z < MatrixData[y]->size(); z++)
						{
							op += (*MatrixData[y])[z];// + spacingstring;
						}
					}

					for (int y = 15; y >= 8; y--)
					{
						for (int z = 0; z < MatrixData[y]->size(); z++)
						{
							op += (*MatrixData[y])[z];// + spacingstring;
						}
					}

					for (int y = 23; y >= 16; y--)
					{
						for (int z = 0; z < MatrixData[y]->size(); z++)
						{
							op += (*MatrixData[y])[z];// + spacingstring;
						}
					}
                    break;
				}

				switch (teo.Code.Content)
				{
				case LineContent::kFrame:
					ExportUtility::AddContentByFrame(teo, op, t, output);
					break;
				case LineContent::kBytes:
					switch (teo.Code.Language)
					{
					case ExportLanguage::kC2Dim:
					case ExportLanguage::kPython2Dim:
						if (!op.empty())
						{
							output.push_back(baaAddContentBySize(op, t, -1));
						}

						op = L"";
						lc = 0;
						rc = 0;
					}
				}
			}

			if (teo.Code.Language == ExportLanguage::kCFastLED)
			{
				output.push_back(ExportUtility::GetVariableIDFrameOut(teo.Code.Language));

				output.push_back(L"");
			}
		}

		switch (teo.Code.Content)
		{
		case LineContent::kBytes:
			if (!op.empty())
			{
				output.push_back(baaAddContentBySize(op, 0, rc));
			}
			break;
		}

		switch (teo.Code.Language)
		{
		case ExportLanguage::kC1Dim:
		case ExportLanguage::kC2Dim:
			output.push_back(teo.DataPadding + L"};");
			break;
		case ExportLanguage::kCFastLED:
			break;
		case ExportLanguage::kPython1Dim:
		case ExportLanguage::kPython2Dim:
			output.push_back(teo.DataPadding + L"]");
			break;
		}

		for (int t = 0; t < MatrixDataCount; t++)
		{
			delete MatrixData[t];
		}

		//delete[] MatrixData;

		if (teo.Code.IncludePreamble)
        {
			ExportUtility::GetSpacerLine(teo.Code.Language, output);
		}

		return true;
	}


	DataOut ExportColumnData(TheMatrix *matrix, ExportOptions teo, int frame, int col, const std::wstring spacingchar)
	{
		InternalArray ia;
		ia.Clear();

		Matrix *selectedmatrix;

		std::wstring s = L"";
		DataOut dataout;
		ScanDirection direction = teo.Code.Direction;

		int bitcounter = 0;
		int dataindex = 0;
		ia.Data[dataindex] = 0;

		for (int y = 0; y < DataOutDataMax; y++)
		{
			dataout.Data[y] = L"";
        }

		int bits = teo.GetNumberSizeLength(teo.Code.Size);
		int pads = teo.GetNumberSizePadLength(teo.Code.Size);

		// ===========================================================================

		if (teo.ExportMode == ExportSource::kAnimation)
		{
			if (matrix->MatrixLayers.size() == 1)
			{
				selectedmatrix = matrix->MatrixLayers[0]->Cells[frame];
			}
			else
			{
				matrix->BuildMergedFrame(frame, MergeFrameMode::kRetainGridValue);

				selectedmatrix = matrix->MatrixMerge;
			}
		}
		else
		{
			selectedmatrix = matrix->MatrixUser[frame];
		}

		// ===========================================================================

		if (teo.Code.Orientation == InputOrientation::kTopBottomLeftRight)
		{
			switch (direction)
			{
			case ScanDirection::kColAltDownUp:
				if (col % 2 == 0)
				{
					direction = ScanDirection::kColTopToBottom;
				}
				else
				{
					direction = ScanDirection::kColBottomToTop;
				}
				break;
			case ScanDirection::kColAltUpDown:
				if (col % 2 == 0)
				{
					direction = ScanDirection::kColBottomToTop;
				}
				else
				{
					direction = ScanDirection::kColTopToBottom;
				}
				break;
			}
		}
		else if (teo.Code.Orientation == InputOrientation::kBottomTopRightLeft)
		{
			switch (direction)
			{
			case ScanDirection::kColAltDownUp:
				if ((matrix->Details.Width - col - 1) % 2 == 0)
				{
					direction = ScanDirection::kColTopToBottom;
				}
				else
				{
					direction = ScanDirection::kColBottomToTop;
				}
				break;
			case ScanDirection::kColAltUpDown:
				if ((matrix->Details.Width - col - 1) % 2 == 0)
				{
					direction = ScanDirection::kColBottomToTop;
				}
				else
				{
					direction = ScanDirection::kColTopToBottom;
				}
				break;
			}
		}

		// ===========================================================================

		if (direction == ScanDirection::kColTopToBottom)
		{
			for (int y = 0; y < matrix->Details.Height; y++)
			{
				if (matrix->MatrixDeadLayout->Grid[y * matrix->Details.Width + col] == PixelAlive)
				{
					if (selectedmatrix->Grid[y * matrix->Details.Width + col] == 1)
					{
						if (teo.Code.LSB == LeastSignificantBit::kTopLeft)
						{
							ia.Data[dataindex] += powers[bitcounter];
						}
						else
						{
							ia.Data[dataindex] += powers[bits - bitcounter];
						}
					}

					if (bitcounter == bits)
					{
						bitcounter = 0;
						dataindex++;

						if (y != matrix->Details.Height - 1)
						{
							ia.Data[dataindex] = 0;
						}
					}
					else
					{
						bitcounter++;
					}
				}
			}
		}
		else if (direction == ScanDirection::kColBottomToTop)
		{
			for (int y = matrix->Details.Height - 1; y >= 0; y--)
			{
				if (matrix->MatrixDeadLayout->Grid[y * matrix->Details.Width + col] == PixelAlive)
				{
					if (selectedmatrix->Grid[y * matrix->Details.Width + col] == 1)
					{
						if (teo.Code.LSB == LeastSignificantBit::kTopLeft)
						{
							ia.Data[dataindex] += powers[bitcounter];
						}
						else
						{
							ia.Data[dataindex] += powers[bits - bitcounter];
						}
					}

					if (bitcounter == bits)
					{
						bitcounter = 0;
						dataindex++;

						if (y != 0)
						{
							ia.Data[dataindex] = 0;
						}
					}
					else
					{
						bitcounter++;
					}
				}
			}
		}

		dataout.Count = dataindex;

		// ===========================================================================

		for (int y = 0; y < dataout.Count; y++)
		{
			if (ia.Data[y] != -1)
			{
				dataout.Count++;

				switch (teo.Code.Size)
				{
				case NumberSize::k8bitSwap:		// swap nybbles
				{
					std::wstring b = L"XX";

					s = IntToHex(ia.Data[y], 2);

					b[0] = s[1];
					b[1] = s[0];

					ia.Data[y] = Convert::HexToInt(b);
					break;
				}
				case NumberSize::k16bitSwap:	// swap bytes
				{
					std::wstring b = L"XXXX";

					s = IntToHex(ia.Data[y], 4);

					b[0] = s[2];
					b[1] = s[3];
					b[2] = s[0];
					b[3] = s[1];

					ia.Data[y] = Convert::HexToInt(b);
					break;
				}
				}

				switch (teo.Code.Format)
				{
				case NumberFormat::kDecimal:
					dataout.Data[y] = std::to_wstring(ia.Data[y]);
					break;
				case NumberFormat::kBinary:
					dataout.Data[y] = Convert::IntegerToBinary(bits, ia.Data[y]);
					break;
				case NumberFormat::kHex:
					dataout.Data[y] = IntToHex(ia.Data[y], pads);
					break;
				}
			}
		}

		// ===========================================================================

		return dataout;
	}


	DataOut ExportRowData(TheMatrix *matrix, ExportOptions teo, int frame, int row, const std::wstring spacingchar)
	{
		InternalArray ia;
		ia.Clear();
		std::wstring s = L"";
		DataOut dataout;

		Matrix *selectedmatrix;

		for (int x = 0; x < DataOutDataMax; x++)
		{
			dataout.Data[x]     = L"";
		}

		int bits = teo.GetNumberSizeLength(teo.Code.Size);
		int pads = teo.GetNumberSizePadLength(teo.Code.Size);

		ScanDirection direction = teo.Code.Direction;

		int bitcounter = 0;
		int dataindex = 0;
		ia.Data[dataindex] = 0;

		// ===========================================================================

		if (teo.ExportMode == ExportSource::kAnimation)
		{
			if (matrix->MatrixLayers.size() == 1)
			{
				selectedmatrix = matrix->MatrixLayers[0]->Cells[frame];
			}
			else
			{
				matrix->BuildMergedFrame(frame, MergeFrameMode::kRetainGridValue);

				selectedmatrix = matrix->MatrixMerge;
			}
		}
		else
		{
			selectedmatrix = matrix->MatrixUser[frame];
		}

		// ===========================================================================

		if (teo.Code.Orientation == InputOrientation::kTopBottomLeftRight)
		{
			switch (direction)
			{
			case ScanDirection::kRowAltLeftRight:
				if (row % 2 == 0)
					direction = ScanDirection::kRowLeftToRight;
				else
					direction = ScanDirection::kRowRightToLeft;
				break;
			case ScanDirection::kRowAltRightLeft:
				if (row % 2 == 0)
					direction = ScanDirection::kRowRightToLeft;
				else
					direction = ScanDirection::kRowLeftToRight;
				break;
			}
		}
		else if (teo.Code.Orientation == InputOrientation::kBottomTopRightLeft)
		{
			switch (direction)
			{
			case ScanDirection::kRowAltLeftRight:
				if ((matrix->Details.Height - row - 1) % 2 == 0)
					direction = ScanDirection::kRowLeftToRight;
				else
					direction = ScanDirection::kRowRightToLeft;
				break;
			case ScanDirection::kRowAltRightLeft:
				if ((matrix->Details.Height - row - 1) % 2 == 0)
					direction = ScanDirection::kRowRightToLeft;
				else
					direction = ScanDirection::kRowLeftToRight;
				break;
			}
		}

		// ===========================================================================

		if (direction == ScanDirection::kRowLeftToRight)	// left to right
		{
			for (int x = 0; x < matrix->Details.Width; x++)
			{
				if (matrix->MatrixDeadLayout->Grid[row * matrix->Details.Height + x] == PixelAlive)
				{
					if (selectedmatrix->Grid[row * matrix->Details.Height + x] == 1)
					{
						if (teo.Code.LSB == LeastSignificantBit::kTopLeft)
							ia.Data[dataindex] += powers[bitcounter];
						else
							ia.Data[dataindex] += powers[bits - bitcounter];
					}

					if (bitcounter == bits)
					{
						bitcounter = 0;
						dataindex++;

						if (x != matrix->Details.Width - 1)
						{
							ia.Data[dataindex] = 0;
						}
					}
					else
					{
						bitcounter++;
					}
				}
			}
		}
		else if (direction == ScanDirection::kRowRightToLeft)	// right to left
		{
			for (int x = matrix->Details.Width - 1; x >= 0; x--)
			{
				if (matrix->MatrixDeadLayout->Grid[row * matrix->Details.Height + x] == PixelAlive)
				{
					if (selectedmatrix->Grid[row * matrix->Details.Height + x] == 1)
					{
						if (teo.Code.LSB == LeastSignificantBit::kTopLeft)
						{
							ia.Data[dataindex] += powers[bitcounter];
						}
						else
						{
							ia.Data[dataindex] += powers[bits - bitcounter];
						}
					}

					if (bitcounter == bits)
					{
						bitcounter = 0;
						dataindex++;

						if (x != 0)
						{
							ia.Data[dataindex] = 0;
						}
					}
					else
					{
						bitcounter++;
					}
				}
			}
		}

		dataout.Count = dataindex;

		// ===========================================================================

		for (int x = 0; x < dataout.Count; x++)
		{
			if (ia.Data[x] != -1)
			{
				dataout.Count++;

				switch (teo.Code.Size)
				{
				case NumberSize::k8bitSwap:		// swap nybbles
				{
					std::wstring b = L"XX";

					s = IntToHex(ia.Data[x], 2);

					b[0] = s[1];
					b[1] = s[0];

					ia.Data[x] = Convert::HexToInt(b);
					break;
				}
				case NumberSize::k16bitSwap:	// swap bytes
				{
					std::wstring b = L"XXXX";

					s = IntToHex(ia.Data[x], 4);

					b[0] = s[2];
					b[1] = s[3];
					b[2] = s[0];
					b[3] = s[1];

					ia.Data[x] = Convert::HexToInt(b);
					break;
				}
				}

				switch (teo.Code.Format)
				{
				case NumberFormat::kDecimal:
					dataout.Data[x] = std::to_wstring(ia.Data[x]);
					break;
				case NumberFormat::kBinary:
					dataout.Data[x] = Convert::IntegerToBinary(bits, ia.Data[x]);
					break;
				case NumberFormat::kHex:
					dataout.Data[x] = IntToHex(ia.Data[x], pads);
					break;
				}
			}
		}

		// ===========================================================================

		return dataout;
	}


	DataOutDisplay SimpleExportMono(TheMatrix *matrix, int frame, int sourceLSB, int source, int sourcedirection, bool hexformat, bool combinenibbles, bool sourcedisplayvisible)
	{
		DataOutDisplay dod;

		int mydata = 0;
		std::wstring s = L"";

		for (int y = 0; y < matrix->Details.Height; y++)
		{
			mydata = 0;

			for (int x = 0; x < matrix->Details.Width; x++)
			{
				if (matrix->MatrixLayers[0]->Cells[frame]->Grid[y * matrix->Details.Width + x] == 1)
				{
					if (sourceLSB == 0)
					{
						mydata = mydata + (powers[x]);
					}
					else
					{
						mydata = mydata + (powers[matrix->Details.Width - x - 1]);
                    }
				}
			}

			if (hexformat)
			{
				s = IntToHex(mydata, GSystemSettings->App.PadModeHexRow);
			}
			else
			{
				s = std::to_wstring(mydata);
			}

			dod.RowData[y] = s;
		}

		for (int x = 0; x < matrix->Details.Width; x++)
		{
			mydata = 0;

			for (int y = 0; y < matrix->Details.Height; y++)
			{
				if (matrix->MatrixLayers[0]->Cells[frame]->Grid[y * matrix->Details.Width + x] == 1)
				{
					if (sourceLSB == 0)
					{
						mydata = mydata + (powers[y]);
					}
					else
					{
						mydata = mydata + (powers[matrix->Details.Height - y - 1]);
					}
				}
			}

			if (hexformat)
			{
				s = IntToHex(mydata, GSystemSettings->App.PadModeHexCol);
			}
			else
			{
				s = std::to_wstring(mydata);
			}

			dod.ColumnData[x] = s;
		}

		// ===========================================================================
		// Need to display anything?
		// ===========================================================================

		if (!sourcedisplayvisible) return dod;

		s = L"";

		if (source == 0)
		{

			// =================================================================
			// Row data
			// =================================================================

			if (sourcedirection == 0)
			{
				s = GSystemSettings->App.OpenBracket;

				for (int y = 0; y <= matrix->Details.Height - 2; y++)
				{
					s += GSystemSettings->App.HexPrefix + dod.RowData[y] + L", ";
				}

				s = s + GSystemSettings->App.HexPrefix + dod.RowData[matrix->Details.Height - 1] + GSystemSettings->App.CloseBracket;
			}
			else
			{
				s = GSystemSettings->App.OpenBracket;

				for (int y = matrix->Details.Height - 1; y >= 1; y--)
				{
					s += GSystemSettings->App.HexPrefix + dod.RowData[y] + L", ";
				}

				s += GSystemSettings->App.HexPrefix + dod.RowData[0] + GSystemSettings->App.CloseBracket;
			}

			dod.Text = s;
		}
		else
		{
			// =================================================================
			// Column data
			// =================================================================

			switch (sourcedirection)
			{
			case 0:
				s = GSystemSettings->App.OpenBracket;

				if (combinenibbles)
				{
					int column = 0;

					while (column <= matrix->Details.Width - 2)
					{
						s += GSystemSettings->App.HexPrefix + dod.ColumnData[column] + dod.ColumnData[column + 1] + L", ";

						column += 2;
					}

					s = s.substr(0, s.length() - 2) + GSystemSettings->App.CloseBracket;
				}
				else
				{
					for (int x = 0; x <= matrix->Details.Width - 2; x++)
					{
						s += GSystemSettings->App.HexPrefix + dod.ColumnData[x] + L", ";
					}
				}

				s += GSystemSettings->App.HexPrefix + dod.ColumnData[matrix->Details.Width - 1] + GSystemSettings->App.CloseBracket;
				break;
			case 1:
				s = GSystemSettings->App.OpenBracket;

				if (combinenibbles)
				{
					int column = matrix->Details.Width - 1;

					while (column >= 0)
					{
						s += GSystemSettings->App.HexPrefix + dod.ColumnData[column] + dod.ColumnData[column - 1] + L", ";

						column -= 2;
					}

					s = s.substr(0, s.length() - 2) + GSystemSettings->App.CloseBracket;
				}
				else
				{
					for (int x = matrix->Details.Width - 1; x >= 1; x++)
					{
						s += GSystemSettings->App.HexPrefix + dod.ColumnData[x] + L", ";
					}

					s += GSystemSettings->App.HexPrefix + dod.ColumnData[0] + GSystemSettings->App.CloseBracket;
				}
				break;
			case 2:
				s = GSystemSettings->App.OpenBracket;

				for (int x = 7; x >= 0; x--)
				{
					s += GSystemSettings->App.HexPrefix + dod.ColumnData[x] + L", ";
				}

				for (int x = 15; x >= 8; x--)
				{
					s += GSystemSettings->App.HexPrefix + dod.ColumnData[x] + L", ";
				}

				for (int x = 23; x >= 17; x--)
				{
					s += GSystemSettings->App.HexPrefix + dod.ColumnData[x] + L", ";
				}

				s += GSystemSettings->App.HexPrefix + dod.ColumnData[16] + GSystemSettings->App.CloseBracket;
				break;
			}

			dod.Text = s;
		}

		return dod;
	}


	DataOutDisplay SimpleExportBiSequential(TheMatrix *matrix, int frame, int sourceLSB, int source, int sourcedirection, bool hexformat, bool sourcedisplayvisible)
	{
		DataOutDisplay dod;
		std::wstring s = L"";
		std::wstring temp = L"";

		for (int y = 0; y < matrix->Details.Height; y++)
		{
			temp = L"";

			for (int x = 0; x < matrix->Details.Width; x++)
			{
				if (sourceLSB == 0)
				{
					temp += ColourUtility::BiColoursLSBLeft[matrix->MatrixLayers[0]->Cells[frame]->Grid[y * matrix->Details.Width + x]];
				}
				else
				{
					temp += ColourUtility::BiColoursLSBRight[matrix->MatrixLayers[0]->Cells[frame]->Grid[y * matrix->Details.Width + (matrix->Details.Width - x - 1)]];
				}
			}

			if (hexformat)
			{
				s = IntToHex(Convert::BinToInt(temp), GSystemSettings->App.PadModeHexRow);
			}
			else
			{
				s = std::to_wstring(Convert::BinToInt(temp));
			}

			dod.RowData[y] = s;
		}

		for (int x = 0; x < matrix->Details.Width; x++)
		{
			temp = L"";

			for (int y = 0; y < matrix->Details.Height; y++)
			{
				if (sourceLSB == 0)
				{
					temp = temp + ColourUtility::BiColoursLSBLeft[matrix->MatrixLayers[0]->Cells[frame]->Grid[y * matrix->Details.Width + x]];
				}
				else
				{
					temp = temp + ColourUtility::BiColoursLSBRight[matrix->MatrixLayers[0]->Cells[frame]->Grid[(matrix->Details.Height - y - 1) * matrix->Details.Width + x]];
				}
			}

			if (hexformat)
			{
				s = IntToHex(Convert::BinToInt(temp), GSystemSettings->App.PadModeHexCol);
			}
			else
			{
				s = std::to_wstring(Convert::BinToInt(temp));
            }

			dod.ColumnData[x] = s;
		}

		// ===========================================================================
		// Need to display anything?
		// ===========================================================================

		if (!sourcedisplayvisible) return dod;

		s = L"";

		if (source == 0)
		{
			// ===========================================================================
			// Row data
			// ===========================================================================

			if (sourcedirection == 0)
			{
				s = GSystemSettings->App.OpenBracket;

				for (int y = 0; y <= matrix->Details.Height - 2; y++)
				{
					s += GSystemSettings->App.HexPrefix + dod.RowData[y] + L", ";
				}

				s += GSystemSettings->App.HexPrefix + dod.RowData[matrix->Details.Height - 1] + GSystemSettings->App.CloseBracket;
			}
			else
			{
				s = GSystemSettings->App.OpenBracket;

				for (int y = matrix->Details.Height - 1; y >= 1; y--)
				{
					s += GSystemSettings->App.HexPrefix + dod.RowData[y] + L", ";
				}

				s += GSystemSettings->App.HexPrefix + dod.RowData[0] + GSystemSettings->App.CloseBracket;
			}

			dod.Text = s;
		}
		else
		{
			// ===========================================================================
			// Column data
			// ===========================================================================

			switch (sourcedirection)
			{
			case 0:
				s = GSystemSettings->App.OpenBracket;

				for (int x = 0; x < matrix->Details.Width - 1; x++)
				{
					s += GSystemSettings->App.HexPrefix + dod.ColumnData[x] + L", ";
				}

				s += GSystemSettings->App.HexPrefix + dod.ColumnData[matrix->Details.Width - 1] + GSystemSettings->App.CloseBracket;
				break;
			case 1:
				s = GSystemSettings->App.OpenBracket;

				for (int x = matrix->Details.Width - 1; x >= 1; x--)
				{
					s += GSystemSettings->App.HexPrefix + dod.ColumnData[x] + L", ";
				}

				s += GSystemSettings->App.HexPrefix + dod.ColumnData[0] + GSystemSettings->App.CloseBracket;
				break;
			case 2:
				s = GSystemSettings->App.OpenBracket;

				for (int x = 7; x >= 0; x--)
				{
					s += GSystemSettings->App.HexPrefix + dod.ColumnData[x] + L", ";
				}

				for (int x = 15; x >= 8; x--)
				{
					s += GSystemSettings->App.HexPrefix + dod.ColumnData[x] + L", ";
				}

				for (int x = 23; x >= 17; x--)
				{
					s += GSystemSettings->App.HexPrefix + dod.ColumnData[x] + L", ";
				}

				s += GSystemSettings->App.HexPrefix + dod.ColumnData[16] + GSystemSettings->App.CloseBracket;
				break;
			}

			dod.Text = s;
		}

        return dod;
	}


	DataOutDisplay SimpleExportBiBitplanes(TheMatrix *matrix, int frame, int sourceLSB, int source, int source_direction, bool hexformat, bool sourcedisplayvisible)
	{
        DataOutDisplay dod;
		int bitplane = 0;
		std::wstring s = L"";

		for (int y = 0; y < matrix->Details.Height; y++)
		{
			bitplane = 0;

			for (int x = 0; x < matrix->Details.Width; x++)
			{
				if (sourceLSB == 0)
				{
					switch (matrix->MatrixLayers[0]->Cells[frame]->Grid[y * matrix->Details.Width + x])
					{
					case 0:
						break;
					case 1:
						bitplane = bitplane + (powers[x]);
						break;
					case 2:
						bitplane = bitplane + (powers[x + (matrix->Details.Width - 1)]);
						break;
					case 3:
						bitplane = bitplane + (powers[x]);
						bitplane = bitplane + (powers[x + (matrix->Details.Width - 1)]);
						break;
					}
				}
				else
				{
					switch (matrix->MatrixLayers[0]->Cells[frame]->Grid[y * matrix->Details.Width + x])
					{
					case 0:
						break;
					case 1:
						bitplane = bitplane + (powers[matrix->Details.Width - x - 1]);
						break;
					case 2:
						bitplane = bitplane + (powers[matrix->Details.Width - x - 1 + (matrix->Details.Width - 1)]);
						break;
					case 3:
						bitplane = bitplane + (powers[matrix->Details.Width - x - 1]);
						bitplane = bitplane + (powers[matrix->Details.Width - x - 1 + (matrix->Details.Width - 1)]);
						break;
					}
				}
			}

			if (hexformat)
				s = IntToHex(bitplane, GSystemSettings->App.PadModeHexRow);
			else
			  	s = std::to_wstring(bitplane);

			dod.RowData[y] = s;
		}

		for (int x = 0; x < matrix->Details.Width; x++)
		{
			bitplane = 0;

			for (int y = 0; y < matrix->Details.Height; y++)
			{
				if (sourceLSB == 0)
				{
					switch (matrix->MatrixLayers[0]->Cells[frame]->Grid[y * matrix->Details.Width + x])
					{
					case 0:
						break;
					case 1:
						bitplane = bitplane + (powers[y]);
						break;
					case 2:
						bitplane = bitplane + (powers[y + (matrix->Details.Height - 1)]);
						break;
					case 3:
						bitplane = bitplane + (powers[y]);
						bitplane = bitplane + (powers[y + (matrix->Details.Height - 1)]);
						break;
					}
				}
				else
				{
					switch (matrix->MatrixLayers[0]->Cells[frame]->Grid[y * matrix->Details.Width + x])
					{
					case 0:
						break;
					case 1:
						bitplane = bitplane + (powers[matrix->Details.Height - y - 1]);
						break;
					case 2:
						bitplane = bitplane + (powers[matrix->Details.Height - y - 1 + (matrix->Details.Height - 1)]);
						break;
					case 3:
						bitplane = bitplane + (powers[matrix->Details.Height - y - 1]);
						bitplane = bitplane + (powers[matrix->Details.Height - y - 1 + (matrix->Details.Height - 1)]);
						break;
					}
                }
			}

			if (hexformat)
			{
				s = IntToHex(bitplane, GSystemSettings->App.PadModeHexCol);
			}
			else
			{
				s = std::to_wstring(bitplane);
            }

			dod.ColumnData[x] = s;
		}

		// ===========================================================================
		// Need to display anything?
		// ===========================================================================

		if (!sourcedisplayvisible) return dod;

		s = L"";

		if (source == 0)
		{
			// ===========================================================================
			// Row data
			// ===========================================================================

			if (source_direction == 0)
			{
				s = GSystemSettings->App.OpenBracket;

				for (int y = 0; y <= matrix->Details.Height - 2; y++)
				{
					s += GSystemSettings->App.HexPrefix + dod.RowData[y] + L", ";
				}

				s += GSystemSettings->App.HexPrefix + dod.RowData[matrix->Details.Height - 1] + GSystemSettings->App.CloseBracket;
			}
			else
			{
				s = GSystemSettings->App.OpenBracket;

				for (int y = matrix->Details.Height - 1; y >= 1; y--)
				{
					s += GSystemSettings->App.HexPrefix + dod.RowData[y] + L", ";
				}

				s += GSystemSettings->App.HexPrefix + dod.RowData[0] + GSystemSettings->App.CloseBracket;
			}

			dod.Text = s;
		}
		else
		{
			// ===========================================================================
			// Column data
			// ===========================================================================

			switch (source_direction)
			{
			case 0:
				s = GSystemSettings->App.OpenBracket;

				for (int x = 0; x <= matrix->Details.Width - 2; x++)
				{
					s += GSystemSettings->App.HexPrefix + dod.ColumnData[x] + L", ";
				}

				s +=  GSystemSettings->App.HexPrefix + dod.ColumnData[matrix->Details.Width - 1] + GSystemSettings->App.CloseBracket;
				break;
			case 1:
				s = GSystemSettings->App.OpenBracket;

				for (int x = matrix->Details.Width - 1; x >= 1; x++)
				{
					s +=  GSystemSettings->App.HexPrefix + dod.ColumnData[x] + L", ";
				}

				s + GSystemSettings->App.HexPrefix + dod.ColumnData[0] + GSystemSettings->App.CloseBracket;
				break;
			case 2:
				s = GSystemSettings->App.OpenBracket;

				for (int x = 7; x >= 0; x--)
				{
					s +=  GSystemSettings->App.HexPrefix + dod.ColumnData[x] + L", ";
				}

				for (int x = 15; x >= 8; x--)
				{
					s += GSystemSettings->App.HexPrefix + dod.ColumnData[x] + L", ";
				}

				for (int x = 23; x >= 17; x--)
				{
					s +=  GSystemSettings->App.HexPrefix + dod.ColumnData[x] + L", ";
				}

				s += GSystemSettings->App.HexPrefix + dod.ColumnData[16] + GSystemSettings->App.CloseBracket;
                break;
			}

			dod.Text = s;
		}

        return dod;
	}
}
