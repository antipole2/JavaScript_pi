print("This is a black string followed by number ", 123, " then boolean ", true, "\n");
printRed("This is a red string followed by number ", 123, " then boolean ", true, "\n");
printOrange("This is an orange string followed by number ", 123, " then boolean ", true, "\n");
printGreen("This is a green string followed by number ", 123, " then boolean ", true, "\n");
printBlue("This is a blue string followed by number ", 123, " then boolean ", true, "\n");
printUnderlined("This is an underlined string followed by number ", 123, " then boolean ", true, "\n");
var boatObject = {type:"yacht", length:12, beam:6, draught:2.1};
print("Object printing: ", boatObject, "\n");
result = 2;
result = messageBox("Results in output window\nLook correct?", "YesNo", "Print tests");
switch (result){
	case 0: scriptResult("Print test cancelled");
		break;
	case 1:
	case 2: scriptResult("Print tests completed OK");
		break;
	case 3: scriptResult("Print test reported failed");
		break;
	}
