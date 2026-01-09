require("Consoles");

testConsoleName = "TestOnResult";
if (!consoleExists(testConsoleName)) consoleAdd(testConsoleName);
onExit(function(){print("In exit\n");consoleClose(testConsoleName)});
//onSeconds(function(){stopScript("onConsoleResult timed out");}, 1);
consoleLoad(testConsoleName, "scriptResult('Completed')");
onConsoleResult(
	function(){print("Got it\n");
	stopScript("onConsoleResult OK");
	},
	testConsoleName
	);