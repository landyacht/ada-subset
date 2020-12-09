procedure Main is
	X, Y: Integer;
	Z: Float;
begin
	X := 3;
	Y := X + 5;
	Z := 2.0 / Y;
	PutLn("Should be 3: ", X);
	PutLn("Should be 8: ", Y);
	PutLn("Should be 0.25: ", Z);
end Main;
