procedure OrderOfOperation2 (X: Double := 3; Y: Double := 6; Z: Double := 45; Total: Double := 0) is
begin
	
	Total:= Z - X / Y ; -- Should be  with correct order of operations -44.5
	
	PutLn(Total);
	

end :OrderOfOperation2;