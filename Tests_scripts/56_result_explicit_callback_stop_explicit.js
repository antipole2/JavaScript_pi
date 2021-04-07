onSeconds(doit, 1);
scriptResult("Explicit result main");

function doit(){
	stopScript("Explicit stop callback");
	print("Script not stopped\n");
	}