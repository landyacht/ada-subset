procedure OrderOfOperation4 (X: Double := 3; Y: Double := 6; Z: Double := 45; Total: Double := 0) is
begin
	
	Total:= X * Y + Z; -- Should be  with correct order of operations 63
	
	PutLn(Total);
	

end :OrderOfOperation4;