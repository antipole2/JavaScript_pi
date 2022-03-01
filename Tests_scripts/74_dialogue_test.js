myDialogue = [];
myDialogue.push({type:"caption", value:"Dialogue test"});
myDialogue.push({type:"text", value:"Complete the form and read back the entries"});
myDialogue.push({type:"field", label:"Name:", value:"(place holder)"});
nameI = myDialogue.length-1;
myDialogue.push({type:"field", label:"Area test:", multiLine:true,
	width:200, height:100, value:"line1\nline2\nA very long line that might be wrapped depending on field behaviour"});
areaI = myDialogue.length-1;
myDialogue.push({type:"tick", value:"*Yes?"});
tickI = myDialogue.length-1;
myDialogue.push({type:"field", label:"Age:",suffix:"years"});
ageI = myDialogue.length-1;
myDialogue.push({type:"radio", label:"Gender", value:["Male","Female", "Not saying"]});
radioI = myDialogue.length-1;
myDialogue.push({type:"tickList", label:"Ticklist", value:["Tinker","Taylor", "Soldier", "Sailor"]});
ticklistI = myDialogue.length-1;
myDialogue.push({type:"hLine"});
myDialogue.push({type:"slider", range:[0,360], value:180, width:300, label:"Course " + String.fromCharCode(176)});
sliderI = myDialogue.length-1;
myDialogue.push({type:"spinner", range:[0,10], value:5, label:"Speed", width:80});
spinI = myDialogue.length-1;
myDialogue.push({type:"choice", value:["Tinker","Taylor", "Soldier", "Sailor"]});
choiceI = myDialogue.length-1;
myDialogue.push({type:"text", value:"This text is monospaced", style:{font:"monospace"}});
myDialogue.push({type:"button", label:["Read back", "Finish"]})

onDialogue(dialogAction, myDialogue);

function dialogAction(dialog){
	button = dialog[dialog.length-1].label;
	if (button == "Read back"){
		print("Name:\t", dialog[nameI].value, "\n");
		print("Area test:\t", dialog[areaI].value, "\n");
		print("Yes? is ticked\t", dialog[tickI].value, "\n");
		print("Age:\t\t", dialog[ageI].value, "\n");
		print("Gender\t", dialog[radioI].value, "\n");
		print("Ticklist:\t", dialog[ticklistI].value, "\n");
		print("Course:\t", dialog[sliderI].value, "\n");
		print("Speed:\t", dialog[spinI].value, "\n");
		print("Choice:\t", dialog[choiceI].value, "\n");
		onDialogue(dialogAction, myDialogue);
		}
	else scriptResult("Dialogue done");
	}
