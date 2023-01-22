Position = require("Position");
testString = "xyz";
menuItem = "Test menu item";

function processA(arg){
	position = new Position(arg);
	print("Context menu responded from position ", position.formatted, " with arg  of '", arg.info, "'\n");
	if (arg.info == testString) scriptResult("OK");
	else scriptResult("Failed");
	alert(false);
	}

OCPNonContextMenu(processA, menuItem, testString);
OCPNsoundAlarm();
alert("Select the context menu '", menuItem, "'");
