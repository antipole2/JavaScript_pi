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
onSeconds(step1, 2);

function step1(){
	result = messageBox(consoleNames.length + " consoles should have been created and parked\nWidth of minimized windows with different name lengths  correct?", "YesNo");
	if (result == 0) stopScript("Parking tests cancelled");
	if (result != 2) stopScript("Parking tests reported wrong");

	console = consoleNames[1];
	consoleLoad(console, growYourself);
	consoleRun(console);
	consoleNames.push(maximumLengthName);	// this one might be hanging around so add to ones to be deleted
	onSeconds(step2,2);
	}

function step2(){
	result = messageBox("Console between '" + consoleNames[0] + "' and '" + consoleNames[2] + "' has been renamed from '" + consoleNames[1] + "' to '" + maximumLengthName + "'\nHave parked consoles adjusted to accommodate?", "YesNo");
	if (result == 0) throw("Parking tests cancelled");
	if (result != 2) throw("Parking tests reported wrong");
	console = maximumLengthName;
	consoleLoad(console, shrinkYourself);
	consoleRun(console);
	consoleNames.push(console);	// this one might be hanging around so add to ones to be deleted
	onSeconds(step3, 2);
	}

function step3(){
	result = messageBox("Console between '" + consoleNames[0] + "' and '" + consoleNames[2] + "' has been shortened from '" + maximumLengthName + "' to '" + extras[0] + "'\nHave parked consoles adjusted to accommodate this?", "YesNo");
	if (result == 0) throw("Parking tests cancelled");
	if (result != 2) throw("Parking tests reported wrong");
	consoleClose("ConsoleC__Long");
	onSeconds(step4, 2);
	}
function step4(){
	result = messageBox("Console ConsoleC__Long has been deleted\nOther parked consoles should have closed up", "YesNo");
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



