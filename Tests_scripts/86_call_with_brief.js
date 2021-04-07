require("Consoles");

name = "Test2"
if (!consoleExists(name)) consoleAdd(name);
consoleLoad(name, "testGetBrief.js");
onConsoleResult(name, receive, "This is the brief");

function receive(result){
	scriptResult(result.value);
	consoleClose(name);
	}
