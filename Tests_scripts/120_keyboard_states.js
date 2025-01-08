prompts = [
	"Test with no keys down",
	"Test with shift key down",
	"Test with control key down\n(Windows/Linux: Windows)",
	"Test with option key down\n(Windows/Linux: Alt)",
	"Test with command key down\n(Windows/Linux: Ctrl)",
	"Test with shift, option & command keys down\nWindows/Linux: Shift & Alt & Ctrl)"
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
errors = false;
dialogue[1].value = prompts[which];

onDialogue(test, dialogue);

function test(x){
	button = x[x.length-1].label;
	if (button == "Test"){
		ok = check(states[which]);
		if (ok){
			print(prompts[which].split("\n")[0], " passed\n");
			}
		else {
			print("\nMismatch between \"", prompts[which], "\"\nGot:    \t", keyboardState(), "\nExpected:\t", states[which], "\n\n");
			errors = true;
			}
		if (++which >= states.length) stopScript(errors?"Failed states tests":"States pass");
		else {
			dialogue[1].value = prompts[which];
			onDialogue(test, dialogue);
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
