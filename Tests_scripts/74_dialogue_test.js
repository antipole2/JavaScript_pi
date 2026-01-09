testDialogue = [];
testDialogue.push({type:"caption", value:"Dialogue test"});
testDialogue.push({type:"text", value:"Complete the form and read back the entries"});
testDialogue.push({type:"field", label:"Name:", value:"(place holder)"});
nameI = testDialogue.length-1;
testDialogue.push({type:"field", label:"Area test:", multiLine:true,
	width:200, height:100, value:"line1\nline2\nA very long line that might be wrapped depending on field behaviour"});
areaI = testDialogue.length-1;
testDialogue.push({type:"tick", value:"*Yes?"});
tickI = testDialogue.length-1;
testDialogue.push({type:"field", label:"Age:",suffix:"years"});
ageI = testDialogue.length-1;
testDialogue.push({type:"radio", label:"Gender", value:["Male","Female", "Not saying"]});
radioI = testDialogue.length-1;
testDialogue.push({type:"tickList", label:"Ticklist", value:["Tinker","Taylor", "Soldier", "Sailor"]});
ticklistI = testDialogue.length-1;
testDialogue.push({type:"hLine"});
testDialogue.push({type:"slider", range:[0,360], value:180, width:300, label:"Course " + String.fromCharCode(176)});
sliderI = testDialogue.length-1;
testDialogue.push({type:"spinner", range:[0,10], value:5, label:"Speed", width:80});
spinI = testDialogue.length-1;
testDialogue.push({type:"choice", value:["Tinker","Taylor", "Soldier", "Sailor"]});
choiceI = testDialogue.length-1;
testDialogue.push({type:"text", value:"This text is monospaced", style:{font:"monospace"}});
testDialogue.push({type:"button", label:["Read back", "Next"]});

skipDialogue = [
	{type:"position", x:1000, y:300},
	{type:"colours", background:"ORANGE RED", text:"white"},
	{type:"caption", value:"Skip option"},
	{type:"button", label:"Skip tests"}
	];

summaryDialogue = [
	{type:"position", x:1000, y:300},
	{type:"colours", background:"PURPLE", text:"white"},
	{type:"caption", value:"Summary"},
	{type:"text", value:"Where all tests OK?"},
	{type:"button", label:["All OK", "Had errors", "Skip tests"]}
	];

var dialogId;
var lastRead;
var testCount = 0;
alertMessage = "Use the 'Read back' button to read back and print\nthe form contents and check all correct.\nWhen finished these tests, select 'Next'";

onDialogue(function(){stopScript("Dialogue tests skipped");}, skipDialogue);
onDialogue(dialogAction1, testDialogue);
alert(alertMessage);

function dialogAction1(dialogue){
	button = dialogue[dialogue.length-1].label;
	if (button == "Read back"){
		dialogDisplay(dialogue);
		onDialogue(dialogAction1, testDialogue);
		}
	else {	// next
		alert(false);
		alert("*** The form is now being displayed as modal. ***\n\n" + alertMessage);
		timerId = onSeconds(
			function(){
				ids = onDialogue("?");
				onDialogue(ids[ids.length-1]);
				}, 30);
		do {
			dialogue = modalDialogue(testDialogue);
			if (dialogue == undefined) break;
			button = dialogue[dialogue.length-1].label;
			if (button == "Next") break;
			dialogDisplay(dialogue);
			} while (true);
		onSeconds(timerId);
		dialogId = onDialogue(dialogAction2, testDialogue);
		lastRead = readDialogue(dialogId);
		timerId = onAllSeconds(readBack, 1);
		alert(false);
		alert("The form is now being read back without you needing to press a button\nMake changes to check they are printed.\nWhen complete, choose either button");
		}
	}

function dialogAction2(dialogue){	// any button pressed during read back tests
	onSeconds();
	onDialogue();
	alert(false);
	onSeconds(collectResult, 0.25);
	}

function readBack(){
	dialog = readDialogue(dialogId);
	if ( dialog[nameI].value != lastRead[nameI].value) print("Name: ", dialog[nameI].value, "\n");
	if ( dialog[areaI].value != lastRead[areaI].value) print("Area test: ", dialog[areaI].value, "\n");

	if ( dialog[tickI].value != lastRead[tickI].value) print("Yes? is ticked: ", dialog[tickI].value, "\n");
	if ( dialog[ageI].value != lastRead[ageI].value) print("Age: ", dialog[ageI].value, "\n");
	if ( dialog[radioI].value != lastRead[radioI].value) print("Gender: ", dialog[radioI].value, "\n");
	if ( JSON.stringify(dialog[ticklistI].value )!= JSON.stringify(lastRead[ticklistI].value)) print("Ticklist: ", dialog[ticklistI].value, "\n");
	if ( dialog[sliderI].value != lastRead[sliderI].value) print("Course: ", dialog[sliderI].value, "\n");
	if ( dialog[spinI].value != lastRead[spinI].value) print("Speed: ", dialog[spinI].value, "\n");
	if ( JSON.stringify(dialog[choiceI].value )!= JSON.stringify(lastRead[choiceI].value)) print("Choice: ", dialog[choiceI].value, "\n");

	lastRead = dialog; 
	}


function dialogDisplay(dialog){
	button = dialog[dialog.length-1].label;
	print("Name:\t", dialog[nameI].value, "\n");
	print("Area test:\t", dialog[areaI].value, "\n");
	print("Yes? is ticked\t", dialog[tickI].value, "\n");
	print("Age:\t\t", dialog[ageI].value, "\n");
	print("Gender\t", dialog[radioI].value, "\n");
	print("Ticklist:\t", dialog[ticklistI].value, "\n");
	print("Course:\t", dialog[sliderI].value, "\n");
	print("Speed:\t", dialog[spinI].value, "\n");
	print("Choice:\t", dialog[choiceI].value, "\n");
	testCount++;
	}

function collectResult(){
	outcome = modalDialogue(summaryDialogue);
	button = outcome[outcome.length-1].label;
	switch (button){
		case "All OK": result = "Dialogue tests good"; break;
		case "Had errors": result = "Dialogue tests had errors"; break;
		case "Skip tests": result = "Dialogue tests skipped"; break;
		}
	stopScript(result);
	}



