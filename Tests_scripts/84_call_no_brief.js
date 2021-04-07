require("Consoles");

name = "Test2"
if (consoleExists(name)) consoleClose(name);
if (!consoleExists(name)) consoleAdd(name);
consoleLoad(name, "testGetBrief.js");
print(consoleDump());
onConsoleResult(name, receive);
print(consoleDump());

function receive(result){
	print("In received\n");
	print(consoleDump());
	scriptResult(result.value);
	consoleClose(name);
	}
