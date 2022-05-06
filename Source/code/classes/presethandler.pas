// ===================================================================
//
// (c) Paul Alan Freshney 2012-2022
// www.freshney.org :: paul@freshney.org :: maximumoctopus.com
//
// https://github.com/MaximumOctopus/LEDMatrixStudio
//
// Please do not modifiy this comment section

//
// ===================================================================

unit presethandler;


interface


uses SysUtils, Vcl.StdCtrls,

     matrixconstants,

     utility;


type
  TMatrixPreset = record
    Width          : integer;
    Height         : integer;
    PixelSize      : integer;
    PixelShape     : integer;

    MatrixMode     : TMatrixMode;
    MatrixModeText : string;
    MatrixModeTag  : integer;
  end;

  TMatrixPresetParameter = (ppUnknown, ppStructBegin, ppStructEnd, ppProjectWidth, ppProjectHeight, ppSource, ppSourceLSB, ppSourceDirection, ppUnused, ppPixelSize, ppMatrixType);


  TPresetHandler = class
    class function  LoadMatrixPreset(aFileName : string): TMatrixPreset;
    class function  SaveMatrixPreset(aFileName : string; aMPP : TMatrixPreset): boolean;
    class function  GetMatrixPresetParameterType(s : string): TMatrixPresetParameter;
    class function  GetMatrixPresetList(aPath : string; var aPresetList : TComboBox): boolean;
  end;


implementation


class function TPresetHandler.LoadMatrixPreset(aFileName : string): TMatrixPreset;
var
  tf   : TextFile;
  s, v : string;

begin
  AssignFile(tf, aFileName);
  Reset(tf);

  while not(eof(tf)) do begin
    readln(tf, s);

    if s <> '' then begin
      v := Copy(s, 3, length(s) - 2);

      case GetMatrixPresetParameterType(s) of
        ppProjectWidth    : Result.Width      := StrToInt(v);
        ppProjectHeight   : Result.Height     := StrToInt(v);
        ppUnused          : {};
        ppPixelSize       : Result.PixelSize  := StrToInt(v);
        ppMatrixType      : begin
                              Result.MatrixMode     := TMatrixMode(StrToInt(v));

                              Result.MatrixModeText := TUtility.GetTypeName(TMatrixMode(StrToInt(v)));

                              Result.MatrixModeTag  := StrToInt(v);
                            end;
      end;
    end;
  end;

  CloseFile(tf);
end;


class function TPresetHandler.SaveMatrixPreset(aFileName : string; aMPP : TMatrixPreset): boolean;
var
  tf   : TextFile;

begin
  Result := True;

  try
    AssignFile(tf, aFileName);
    Rewrite(tf);

    Writeln(tf, '{preset');
    Writeln(tf, 'w:' + IntToStr(aMPP.Width));
    Writeln(tf, 'h:' + IntToStr(aMPP.Height));
    Writeln(tf, 'e:' + IntToStr(aMPP.PixelSize));
    Writeln(tf, 'm:' + IntToStr(Ord(aMPP.MatrixMode)));
    Writeln(tf, 's:' + IntToStr(aMPP.PixelShape));
    Writeln(tf, '}');
  finally
    CloseFile(tf);
  end;
end;


class function TPresetHandler.GetMatrixPresetParameterType(s : string): TMatrixPresetParameter;
 begin
  if s[1] = '{' then
    Result := ppStructBegin
  else if s[1] = '}' then
    Result := ppStructEnd
  else if s[1] = 'w' then
    Result := ppProjectWidth
  else if s[1] = 'h' then
    Result := ppProjectHeight
  else if s[1] = 'a' then
    Result := ppSource
  else if s[1] = 'b' then
    Result := ppSourceLSB
  else if s[1] = 'c' then
    Result := ppSourceDirection
  else if s[1] = 'd' then
    Result := ppUnused
  else if s[1] = 'e' then
    Result := ppPixelSize
  else if s[1] = 'm' then
    Result := ppMatrixType
  else
    Result := ppUnknown;
end;


class function TPresetHandler.GetMatrixPresetList(aPath : string; var aPresetList : TComboBox): boolean;
var
  searchResult : TSearchRec;

begin
  Result := False;

  if FindFirst(aPath, faAnyFile, searchResult) = 0 then begin
    aPresetList.Clear;

    repeat
      aPresetList.Items.Add(TUtility.RemoveExtension(searchResult.Name));
    until FindNext(searchResult) <> 0;

    Result := True;
  end;
end;


end.
