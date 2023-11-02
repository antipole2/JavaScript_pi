print("This is a black string followed by number ", 123, " then boolean ", true, "\n");
printRed("This is a red string followed by number ", 123, " then boolean ", true, "\n");
printOrange("This is an orange string followed by number ", 123, " then boolean ", true, "\n");
printGreen("This is a green string followed by number ", 123, " then boolean ", true, "\n");
printBlue("This is a blue string followed by number ", 123, " then boolean ", true, "\n");
printUnderlined("This is an underlined string followed by number ", 123, " then boolean ", true, "\n");
var boatObject = {type:"yacht", length:12, beam:6, draught:2.1};
print("Object printing: ", boatObject, "\n");

onDialogue(reply, [
	{type:"text", value:"Results in output window look correct?"},
	{type:"button", label:["Cancel", "*Yes", "No"]}
	]);

function reply(outcome){
	switch (outcome[outcome.length-1].label){
		case "Cancel":	stopScript("Print test cancelled");
		case "Yes":	stopScript("Print tests completed OK");
		case "No":	stopScript("Print test reported failed");
		}
	}