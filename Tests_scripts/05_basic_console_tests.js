require("Consoles");

name = "Test2";
if (consoleExists(name)) consoleClose(name);
if (consoleExists(name)) throw("consoleClose failed");
consoleAdd(name);
if (!consoleExists(name)) throw("consoleAdd failed");
consoleLoad(name, "01_simple_script.js");
consoleRun(name);
onSeconds(result, 1);

function result(){
	output = consoleGetOutput(name).split("\n");
	printResult = output[output.length-2];
	if (printResult == "result: All done OK") scriptResult("Basic console OK");
	else scriptResult("Running script in another console gave unexpected result");
	consoleClose(name);
	}
