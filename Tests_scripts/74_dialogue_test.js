myDialogue = [];
myDialogue.push({type:"caption", value:"Dialogue test"});
myDialogue.push({type:"text", value:"Complete the form and read back the entries"});
myDialogue.push({type:"field", label:"Name:", value:"(place holder)"});
myDialogue.push({type:"tick", value:"*Yes?"});
myDialogue.push({type:"field", label:"Age:",suffix:"years"});
myDialogue.push({type:"radio", label:"Gender", value:["Male","Female", "Not saying"]});
myDialogue.push({type:"tickList", label:"Ticklist", value:["Tinker","Taylor", "Soldier", "Sailor"]});
myDialogue.push({type:"hLine"});
myDialogue.push({type:"slider", range:[0,360], value:180, width:300, label:"Course " + String.fromCharCode(176)});
myDialogue.push({type:"spinner", range:[0,10], value:5, label:"Speed", width:80});
myDialogue.push({type:"choice", value:["Tinker","Taylor", "Soldier", "Sailor"]});
myDialogue.push({type:"text", value:"This text is monospaced", style:{font:"monospace"}});
myDialogue.push({type:"button", label:["Read back", "Finish"]})

onDialogue(dialogAction, myDialogue);

function dialogAction(dialog){
	button = dialog[dialog.length-1].label;
	if (button == "Read back"){
		print("Name:\t", dialog[2].value, "\n");
		print("Yes? is ticked\t", dialog[3].value, "\n");
		print("Age:\t\t", dialog[4].value, "\n");
		print("Gender\t", dialog[5].value, "\n");
		print("Ticklist:\t", dialog[6].value, "\n");
		print("Course:\t", dialog[8].value, "\n");
		print("Speed:\t", dialog[9].value, "\n");
		print("Choice:\t", dialog[10].value, "\n");
		onDialogue(dialogAction, myDialogue);
		}
	else scriptResult("Dialogue done");
	}
