require("Consoles");

name = "azbycxdveu";
if (!consoleExists(name)) consoleAdd(name);
consoleLoad(name, "82_chain_with_brief.js");
onConsoleResult(name, done);

function done(result){
	print("Done called\n");
	scriptResult(result.value);
	onSeconds(function(){consoleClose(name);}, 0.01);
	}

