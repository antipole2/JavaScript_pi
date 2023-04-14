require("Consoles");

var frameDetails, Topband
title = "Configure parking for platform";
intro = "This utility helps establish the platform-dependant parameters for console parking";

step1prompt = "Do you wish to test parking with existing compiled Parameters?";
dialog = [
	{type:"caption", value:"Test control"},
	{type:"text", value:""},
	{type:"button", label:["Yes","No"]}];
consoleNames = ["M", "MMMMMMMMMMMMMM"];
consoleHide();
messageBox(intro, null, title);
cleanup();
dialog[1].value = step1prompt;
onDialogue(step1, dialog);

function step1(response){
	button = response[response.length-1].label;
	if (button == "Yes"){
		for (i = consoleNames.length-1; i >= 0; i--){
			name = consoleNames[i];
			consoleAdd(name);
			consolePark(name);
			}
		}
	for (i = 0; i < consoleNames.length; i++){
		if (consoleExists(consoleNames[i])) consoleClose(consoleNames[i]);
		}
	dialog.pop();	// get rid of buttons so use default "OK"
	for (i = consoleNames.length-1; i >= 0; i--){
		name = consoleNames[i];
		consoleAdd(name);
		detail = consoleDetails(name, true);	// set minSize very small
//		printOrange(JSON.stringify(detail, null, "\t"), "\n");
		}
	prompt = "Step 1: You should now see " + consoleNames.length + " consoles ";
	for (i = 0; i < consoleNames.length; i++){
		prompt += (consoleNames[i])
		if (i < consoleNames.length-1) prompt += " and ";
		}
	prompt += "\n\nShrink each of these to the desired minimum size.\nNote you can make them smaller than usual.\nYou should see the console top band only.\nReduce the width of each so that it is just wide enough for the console name to be visible.\nConsole M will be much narrower than the other";
	
	dialog[1].value = prompt;
	onDialogue(step2, dialog);
	}

function step2(){
	prompt = "Step 2: Drag console M into the desired parking place in the main window top band,\npositioning it in the first left-most position.\nDrag the other console into the parking place positioning it next right with the desired small space between them";
	dialog[1].value = prompt;
	onDialogue(step3, dialog);
	}

function step3(){
	//	now for the work
	details0 = consoleDetails(consoleNames[0]);
	details1 = consoleDetails(consoleNames[1]);
//	printBlue(JSON.stringify(details0, null, "\t"), "\n");
//	printGreen(JSON.stringify(details1, null, "\t"), "\n");
	consoleMinHeight = Math.round((details0.size.y + details1.size.y)/2);
	parkFirstX = details0.position.x;
	parkLevel = Math.round((details0.position.y + details1.position.y)/2);
	rightEdge0 = details0.position.x + details0.size.x;
	leftEdge1 = details1.position.x;
	parkSep = (leftEdge1 - rightEdge0).toFixed(1);
	consoleCharWidth = ((details1.size.x - details0.size.x) / (consoleNames[1].length - consoleNames[0].length)).toFixed(2);
	consoleStub = Math.round(details0.size.x - consoleCharWidth*consoleNames[0].length);
	print("Recommended settings for consolePositiong.h:\n\n");
	print("#define CONSOLE_STUB ", consoleStub, "\n");
	print("#define CONSOLE_CHAR_WIDTH ", consoleCharWidth, "\n");
	print("#define PARK_LEVEL ", parkLevel, "\n");
	print("#define PARK_FIRST_X ", parkFirstX, "\n");
	print("#define PARK_SEP ", parkSep, "\n\n");
	consoleDetails(consoleNames[0], false);	// restore normal min size
	consoleDetails(consoleNames[1], false);
	}

function cleanup(){	
	for (i = 0; i < consoleNames.length; i++){
		if (consoleExists(consoleNames[i])) consoleClose(consoleNames[i]);
		}
	}
