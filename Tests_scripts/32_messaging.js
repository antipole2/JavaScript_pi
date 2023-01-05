require("Consoles");

other = "other";
name = "JSTEST";
testText = "Test text";
script = "OCPNsendMessage('" + name + "', '" + testText + "');";
printOrange(script, "\n");
onExit(clearUp);
onSeconds(clearUp, 3);	// longstop in case of no response
clearUp();
consoleAdd(other);
consoleLoad(other, script);
OCPNonMessageName(getResult, name);
consoleRun(other);

function getResult(received){
	scriptResult((received == testText) ? "Messaging OK" : "Messaging failed");
	stopScript();
	}

function clearUp(){
	if (consoleExists(other)) consoleClose(other);
	}
