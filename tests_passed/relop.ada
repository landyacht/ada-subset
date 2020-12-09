procedure Main is
	X, Y: Integer;
begin
	X := 5;
	Put("Enter an integer: ");
	Y := GetInt();
	if X > Y then
		return 1;
		PutLn("Should not print!");
	else
		return 0;
		PutLn("Should not print!");
	end if;
end Main;
