procedure Main is
	X: Integer;
	A: Float;
	Q: Boolean;
	S: String;
begin
	Put("Enter an integer: ");
	X := GetInt();
	Put("Enter a float: ");
	A := GetFloat();
	Put("Enter a boolean (1 or 0): ");
	Q := GetBoolean();
	Put("Enter a line of text: ");
	S := GetLn();
	PutLn("Your integer was ", X);
	PutLn("Your float was ", A);
	PutLn("Your boolean was ", Q);
	PutLn("Your text was ", S);
end Main;
