require("Consoles");
File = require("File");

name = "Test2"
if (consoleExists(name)) consoleClose(name);
if (!consoleExists(name)) consoleAdd(name);
fcb = new File("08_require_tests.js", READ);
script = "";
while (!fcb.eof()){
	textLine = fcb.getTextLine();
	textLine = textLine.replace(/^[\t ]*/g, "");
	script += textLine + "\n";
	}
consoleLoad(name, script);

message = "Make sure the focus is on console " + name + " and observe that the script is not indented.\nNow type ⌘+F (Apple) or Ctrl+F (Windows/Linux) and confirmthhathe script for loop is indented appropriately";
dialog = [
	{type: "caption", value:"Script reflow test"},
	{type:"colours", background:"ORANGE RED", text:"white"},
	{type:"text", value: message},
	{type: "button", label:["Correct", "Incorrect"]}
	];
onDialogue(action, dialog);

function action(dialog, label){
	if (label == "Correct") scriptResult("Reflow OK");
	else scriptResult("Reflow failed");
	consoleClose(name);
	}








