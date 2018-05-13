{ compile with "fpc hashtable-pascal.pas -Sd" }

program Benchmark;

uses
	DateUtils,
	Sysutils,
	Classes,
	Contnrs;

const
	FILENAME = 'words.txt';

var
	Started: TDateTime;
	Map: TFPObjectHashTable;
	Sum: Integer;

procedure AddToMap(Word: String);

var
	Item: TObject;
	Count: Integer;

begin
	Item := Map.Items[Word];

	if Assigned(Item) then
		begin
			Count := Integer(Item) + 1;
			Map.Items[Word] := TObject(Count);
		end
	else
		begin
			Map.Add(Word, TObject(1));
		end;
end;

procedure CalcSumCallback(Item: TObject; const key: AnsiString; var continue: Boolean);

begin
	Sum := Sum + Integer(Item);
end;

procedure Benchmark;

var
	srcFile: TextFile;
	block: String[255];
	Word: String[64];
	Offset: Integer;
	Buffer: AnsiString;
	Completed: Boolean;

begin
	AssignFile(srcFile, FILENAME);

	reset(srcFile);

	Offset := 0;
	Buffer := '';

	while(not eof(srcFile)) and (not eoln(srcFile)) do
		begin
			read(srcFile, block);
			Completed := false;

			Buffer := Buffer + block;

			while(not Completed) and (Length(Buffer) > 0) do
				begin
					Offset := pos(' ', Buffer);

					if Offset = 0 then
						begin
							Completed := true;
						end
					else
						begin
							Word := copy(Buffer, 1, Offset);

							AddToMap(Word);

							Buffer := RightStr(Buffer, Length(Buffer) - Offset);
							Offset := Offset + 1;
						end;
				end;
		end;

	if(Length(Buffer) > 0) then
		begin
			AddToMap(Word);
		end;

	CloseFile(srcFile);

	writeln(Map.Count);

	Map.Iterate(CalcSumCallback);

	writeln(Sum);
end;

begin
	Started := Now;

	Map := TFPObjectHashTable.Create(false);
	Sum := 0;

	Benchmark;

	writeln(MillisecondsBetween(Now, Started));
end.
