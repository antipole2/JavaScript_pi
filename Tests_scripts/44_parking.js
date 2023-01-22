require("Consoles");
consoleHide();

consoleNames = ["ConsoleA", "1", "ConsoleC__Long", "ConsoleD", "ConsoleE"];
maximumLengthName = "MaximumLength"
shrinkYourself = "consoleName('2');scriptResult('');";
growYourself = "consoleName('" + maximumLengthName + "');scriptResult('');";
restoreName = "consoleName(\"" + consoleNames[consoleNames.length-1] + "\");scriptResult('');";
extras = ["2", maximumLengthName];	// we will also be creating these
consolesWithExtras = consoleNames.concat(extras);
var console;
deleteConsoles();	// clear out any consoles left from previous runs
onExit(deleteConsoles);

for (i = 0; i < consoleNames.length; i++){
	console = consoleNames[i];
	if (consoleExists(console)) consoleClose(console);
	consoleAdd(console);
	consolePark(console);
	}
onSeconds(step1, 1);

function step1(){
	result = messageBox(consoleNames.length + " consoles should have been created and parked\nWidth of minimized windows with different name lengths  correct?", "YesNo");
	if (result == 0) throw("Parking tests cancelled");
	if (result != 2) throw("Parking tests reported wrong");

	consoleLoad(console, shrinkYourself);
	consoleRun(console);
	consoleNames.push("2");	// this one might be hanging around so add to ones to be deleted
	onSeconds(step2,1);
	}

function step2(){
	result = messageBox("Rightmost console has been renamed to '2'\nHas width of minimized window shrunk correctly?", "YesNo");
	if (result == 0) throw("Parking tests cancelled");
	if (result != 2) throw("Parking tests reported wrong");
	console = "2";
	consoleLoad(console, growYourself);
	consoleRun(console);
	consoleNames.push(maximumLengthName);	// this one might be hanging around so add to ones to be deleted
	onSeconds(step3, 1);
	}

function step3(){
	result = messageBox("Rightmost console has been renamed to " + maximumLengthName + "\nHas width of minimized window grown as needed?", "YesNo");
	if (result == 0) throw("Parking tests cancelled");
	if (result != 2) throw("Parking tests reported wrong");
	onSeconds(waitforit, 1);
	scriptResult("Parking completed");
	}

function waitforit(){};

function deleteConsoles(){
	for (i = 0; i < consolesWithExtras.length; i++){
		console = consolesWithExtras[i];
		if (consoleExists(console)) consoleClose(console);
		}
	}
