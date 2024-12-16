prompts = [
	"Test with no keys down",
	"Test with shift key down",
	"Test with control key down\n(Windows/Linux: shift + F10 or right-click",
	"Test with option key down\n(Windows/Linux: alt",
	"Test with command key down\n(Windows/Linux: ctl",
	"Test with shift, option & command keys down\nWindows/Linux: shift & alt & cntl"
	];

dialogue = [
	{type:"caption", value:"Keyboard states"},
	{type:"text", value:"Test with no keys down"},
	{type:"button", label:["Cancel","Skip tests", "Test"]}
	]

states = [
	{shift:false,control:false,option:false,menu:false},
	{shift:true,control:false,option:false,menu:false},
	{shift:false,control:true,option:false,menu:false},
	{shift:false,control:false,option:true,menu:false},
	{shift:false,control:false,option:false,menu:true},
	{shift:true,control:false,option:true,menu:true},
	];

which = 0;
dialogue[1].value = prompts[which];

onDialogue(test, dialogue);

function test(x){
	button = x[x.length-1].label;
	if (button == "Test"){
		ok = check(states[which]);
		if (ok){
			print(prompts[which].split("\n")[0], " passed\n");
			if (++which >= states.length) stopScript("States pass");
			dialogue[1].value = prompts[which];
			onDialogue(test, dialogue);
			}
		else {
			print("Mismatch between \"", prompts[which], "\" and ", JSON.stringify(states[which]), "\n");
			stopScript("Failed test");
			}
		}
	else if (button == "Skip tests") stopScript("Skipped");
	else stopScript("Cancelled");
	};

function check(which){
	state = keyboardState();
	if (
		(state.shift == which.shift) &&
		(state.option == which.option) &&
		(state.menu == which.menu)
		) return true;
	else return false;
	}
/*{
		if (++which > states.length) stopScript("States pass");
		}
	else {
		printRed("State ", states[which], " failed\n");
		stopScript("States failed");
		}
	};
*/