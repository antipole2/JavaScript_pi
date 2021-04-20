require("Consoles");
name = "errorChecks";
expected = "abc line 2 TypeError: object required, found 1 (stack index 0)\ncalled from outer line 6\ncalled from line 9\n";

try { consoleAdd(name);}
catch(err){};
consoleLoad(name, "16_error_message_checks.js");
consoleClearOutput(name);
consoleRun(name);
onSeconds(evaluate, 1);

function evaluate(){
	output = consoleGetOutput(name);
	consoleClose(name);
	if (output == expected) scriptResult("All OK");
	else scriptResult("Failed");
	}
