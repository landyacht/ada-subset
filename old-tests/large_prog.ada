procedure Big (A, B: Integer; C, D: Float;) is
	E : Integer;
	F : String;
begin
	E := A * (B + -C) / D - A + 2 * (C / B);
	if E > 4 then
		E := 4;
		F := "Some text";
	else
		E := 0;
		F := "Some more " & "text";
		if A * B > 4 and A - B <= 4 then
			F := "Other text";
			return 13;
		end if;
	end if;
	SomeFunc(E, A + B, F);
	OtherFunc(E, A + -B, 12);
	return 12;
end Big;

procedure Big2 (A, B: Integer; C, D: Float;) is
	E : Integer;
	F : String;
begin
	E := A * (B + -C) / D - A + 2 * (C / B);
	if E > 4 then
		E := 4;
		F := "Some text";
	else
		E := 0;
		F := "Some more " & "text";
		if A * B > 4 and A - B <= 4 then
			F := "Other text";
			return 13;
		end if;
	end if;
	SomeFunc(E, A + B, F);
	OtherFunc(E, A + -B, 12);
	return 12;
end Big2;

procedure Big3 (A, B: Integer; C, D: Float;) is
	E : Integer;
	F : String;
begin
	E := A * (B + -C) / D - A + 2 * (C / B);
	if E > 4 then
		E := 4;
		F := "Some text";
	else
		E := 0;
		F := "Some more " & "text";
		if A * B > 4 and A - B <= 4 then
			F := "Other text";
			return 13;
		end if;
	end if;
	SomeFunc(E, A + B, F);
	OtherFunc(E, A + -B, 12);
	return 12;
end Big3;
