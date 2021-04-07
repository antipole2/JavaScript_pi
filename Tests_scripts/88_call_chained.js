require("Consoles");

name = "x5b79l8q^bui";
if (!consoleExists(name)) consoleAdd(name);
consoleLoad(name, "82_chain_with_brief.js");
onConsoleResult(name, done);

function done(result){
	print("Done called\n");
	scriptResult(result.value);
	consoleClose(name);
	}
