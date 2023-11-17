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

#pragma once


static const std::wstring __LEDStudioDate = L"November 15th 2023";

#if _WIN64
static const std::wstring __LEDStudioVersion = L"0.20.0 (x64)";
#else
static const std::wstring __LEDStudioVersion = L"0.20.0 (x32)";
#endif

static const std::wstring __SimpleTextFull = L"(c) Paul Alan Freshney :: " + __LEDStudioDate + L" :: www.MaximumOctopus.com";

static const std::wstring __Caption = __LEDStudioVersion + L" :: " + __LEDStudioDate;