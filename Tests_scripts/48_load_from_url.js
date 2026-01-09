require("Consoles");

url = "https://raw.githubusercontent.com/antipole2/JavaScript_pi/v3.0.0-beta/Tests_scripts/Load_from_URL.txt.js";
urlConsole = "loadTest";
if (!consoleExists(urlConsole)) consoleAdd(urlConsole);
toClipboard(url);
dialog = [
	{"type": "text", "value": "In console loadTest, use the Load button to load URL on clipboard\nthen click OK"},
	{"type": "button", "label": ["Failed to load script", "Loaded script OK"]}
	];
onDialogue(urlLoaded, dialog);

function urlLoaded(dialog){
	button = dialog[dialog.length-1].label;
	if (button == "Loaded script OK"){
		onConsoleResult(urlConsole, hasRun);
		}
	else scriptResult("Load from URL failed");
	}

function hasRun(result){
	print(result, "\n");
	consoleClose(urlConsole);
	if (result.type == 2) scriptResult(result.value);
	else scriptResult("Load from URL failed");
	}
