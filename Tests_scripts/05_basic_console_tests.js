require("Consoles");

name = "Test2";
if (consoleExists(name)) consoleClose(name);
if (consoleExists(name)) throw("consoleClose failed");
consoleAdd(name);
if (!consoleExists(name)) throw("consoleAdd failed");
consoleLoad(name, "20_print_tests.js");
consoleRun(name);
onSeconds(result, 1);

function result(){
	output = consoleGetOutput(name).split("\n");
	consoleClose(name);
	scriptResult("Basic console OK");
	}
