// test suite

require("Consoles");

try { readTextFile("textFile.txt");}
catch(error){
	throw("You need to set the current directory to the test scripts folder\n"+
		"Use the Directory tab in the tools\n" +
		"Then try again");
	}

message = "";
var buttons = [];
var testIndex;
var errorCount;
var name = "Test";	//The test console name;
control = [];
control.push({type:"caption", value:"Test control"});
control.push({type:"text", value:"Run the tests from here"});
control.push({type:"button", label:["Run all", "Quit"]});	// the Run all button
control.push({type:"hLine"});
control.push({type:"text", value:"or run indivually..."});

tests = [ // test table format
	// (0) Test name	(1) function to run	(2) script name (3) max seconds to wait (4) expected last line of output
	{name:"Basic console", functionToUse:load, script:"05_basic_console_tests.js", time:2, result:"Basic console OK"},
	{name:"Compile error", functionToUse:load, script:"10_error_compile.js", time:1, result:"SyntaxError: unterminated string"},
	{name:"Throw from main", functionToUse:load, script:"11_throw_from_main.js", time:1, result:"Thrown from main"},
	{name:"Runtime error main", functionToUse:load, script:"12_runtime_error_main.js", time:3, result:"onSeconds first argument must be function"},
	{name:"Error trace", functionToUse:load, script:"16a_error_message_checks.js", time:3, result:"All OK"},
	{name:"Throw from callback", functionToUse:load, script:"14_error_from_callback.js", time:3, result:"onSeconds first argument must be function"},
	{name:"Printing", functionToUse:load, script:"20_print_tests.js", time:1, result:"Print tests completed OK"},
	{name:"Position/Waypoint/Route", functionToUse:load, script:"02_PosWayRouteTests.js", time:2, result:"Tests ran to completion"},
	{name:"Timer", functionToUse:load, script:"30_timer_tests.js", time:21, result:"Timer tests ended OK"},
	{name:"Stop main", functionToUse:load, script:"50_stop_main.js", time:1, result:"undefined"},
	{name:"Stop result", functionToUse:load, script:"52_stop_result_main.js", time:1, result:"Stop result"},
	{name:"Stop callback", functionToUse:load, script:"54_stop_explicit_callback.js", time:3, result:"Explicit stop callback"},
	{name:"Explicit main explicit stop callback", functionToUse:load, script:"56_result_explicit_callback_stop_explicit.js", time:3, result:"Explicit stop callback"},
	{name:"Explicit main", functionToUse:load, script:"60_explicit_result_main.js", time:1, result:"Explicit result"},
	{name:"Explicit result stop", functionToUse:load, script:"62_explict_result_stopped.js", time:1, result:"Explicit result"},
	{name:"Explicit main callback stop", functionToUse:load, script:"64_explicit_main_callback_stop.js", time:3, result:"Explicit result"},
	{name:"Alert", functionToUse:load, script:"70_alerts.js", time:10, result:"result: Alert OK"},
	{name:"Dialogue", functionToUse:load, script:"74_dialogue_test.js", time:10, result:"Dialogue done"},
	{name:"Chain_no_brief", functionToUse:load, script:"80_chain_no_brief.js", time:1, result:"getBrief found no brief"},
	{name:"Chain_with_brief", functionToUse:load, script:"82_chain_with_brief.js", time:5, result:"Found brief This is the brief"},
	{name:"Call no brief", functionToUse:load, script:"84_call_no_brief.js", time:1, result:"getBrief found no brief"},
	{name:"Call with brief", functionToUse:load, script:"86_call_with_brief.js", time:1, result:"Found brief This is the brief"},
	{name:"Call chain with brief", functionToUse:load, script:"88_call_chained.js", time:1, result:"Found brief This is the brief"}
	
	];

// we will construct the button rows dynamically from the tests table.
// we do this so we can display them over severl rows - hold on!
buttonCount = tests.length;

var buttonsArray = [[]];	// will be an array of arrays
//buttonsArray[0] =[];
//stopScript("Stop here");
for (i = 0, buttonsRow = 0; i < buttonCount; i++){//  construct the array of button names
	buttonsArray[buttonsRow].push(tests[i].name);
	if (((i+1)%5) == 0) { 
		buttonsArray.push([]);	// new row
		buttonsRow++;
		}
	}
//stopScript("Place two");
for (i = 0; i < buttonsArray.length; i++){
	control.push({type:"button", label:buttonsArray[i]});
	}
control.push({type:"text", value:message});		// space for message
onDialogue(action, control);
// end of buttons construction

function action(dialog){	// match button to tests
	button = dialog[dialog.length-1].label;
	for (i = 0; i < tests.length; i++){
		if (button == tests[i].name){
			tests[i].functionToUse(tests[i].script);
			break;
			}
		}
	if (button == "Run all"){	// to run all tests
		consoleClearOutput();
		if (!consoleExists(name)) consoleAdd(name);
		else if (consoleBusy(name)){
			consoleClose(name);
			consoleAdd(name);
			}
		testIndex = 0; errorCount = 0;
		call(testIndex);
		return;
		}
	else if (button == "Quit") stopScript("Quitted");
	if (i >= tests.length) throw("Logic error - failed to match button");
	onSeconds(waitForIt, tests[i].time, i);
	}

function waitForIt(thisTest){	// test should have completed
	if (consoleBusy(name)){
//		print("Still waiting for test\n");
		onSeconds(waitForIt(2, thisTest));
		}
	else {
		output = consoleGetOutput(name).split("\n");
		lastLine = output[output.length-2];
		button = (tests[thisTest].name + "                                    ").slice(0, 40);
		success = lastLine.search(tests[thisTest].result);
		message = button + ((success>= 0)?"PASSED":"FAILED" + "\t\t\t" + lastLine);
		control[control.length-1].value = message;
		onDialogue(action, control);
		}
	}
		

function load(file){
	if (!consoleExists(name)) consoleAdd(name);
	if (consoleBusy(name)) alert("Console ", name, " is busy\n");
	else {
		consoleClearOutput(name);
		consoleLoad(name, file)
		consoleRun(name);
		}
	}

function dialogue(file){
	if (!consoleExists(name)) consoleAdd(name);
	consoleLoad(name,"dialogue_test.js")
	consoleRun(name);
	}

function call(index){
	buttonNamePadded = (tests[testIndex].name + "                                    ").slice(0, 40);
	print(buttonNamePadded);
	consoleClearOutput(name);
	consoleLoad(name, tests[index].script);
	onConsoleResult(name, callResult);
	onSeconds(callTimedOut, (tests[index].time)*2);	// give it twice usual times
	}

function callResult(handback){
	onSeconds();	// cancel longstop timer
	lastLine = handback.value;
	success = lastLine.search(tests[testIndex][4]);
	if (success >= 0) printGreen("PASSED\n");
	else {
		printOrange("FAILED");
		errorCount++;
		print("\t\t", lastLine, "\n");
		}
	if (++testIndex < tests.length ){
		call(testIndex);
		}
	else wrapup();
	}

function timedOut(){
	throw("Test " + tests[testIndex].name + ", timed out");
	}

function callTimedOut(){
	printOrange("TIMED OUT\n");
	consoleClose(name);
	consoleAdd(name);
	errorCount++;
		if (++testIndex < tests.length ){
		call(testIndex);
		}
	else wrapup();
	}

function wrapup(){
		message = "Run all done - "+ (errorCount> 0?(errorCount + " errors"):"No errors");
		print(message, "\n");
		consoleClose(name);
		control[control.length-1].value = message;
		onDialogue(action, control);
	}
