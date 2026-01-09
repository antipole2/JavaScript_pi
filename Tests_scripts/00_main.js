// test suite

require("Consoles");
consolePark();
consoleName("PluginTests");

try { readTextFile("textFile.txt");}
catch(error){
	throw("You need to set the current directory to the test scripts folder\n"+
		"Use the Directory tab in the tools\n" +
		"Then try again");
	}

message = "";
var buttons = [];
var testIndex;	// index of test being run
var runAll;	// true: run all tests  false: run just textIndex
var errorCount;
var timerCount;
var name = "Test";	//The test console name;
var timedOut;	// true if test timed out, else false
control = [];
control.push({type:"caption", value:"Test control"});
control.push({type:"text", value:"Run the tests from here"});
control.push({type:"button", label:["Run all", "Quit"]});	// the Run all button
control.push({type:"hLine"});
control.push({type:"text", value:"or run indivually..."});

abortDialogue = [{type:"caption", value:"Test control"},
	{type:"text", value:"Use this button to abort tests"},
	{type:"colours", background:"MAROON", text:"white"},
	{type:"button", label:"Abort"}];

// test table format
// (0) Test name	(1) function to run (not used at present) (2) script name (3) max seconds to wait (4) expected last line of output

/*
tests = [ // short list for testing this script
	{name:"Simple script", functionToUse:loadRunWait, script:"01_simple_script.js", time:2, result:{type:2, value:"All done OK"}},
	{name:"Alarm", functionToUse:loadRunWait, script:"72_alarm.js", time:10, result:{type:2, value:"Alarm OK"}},
	{name:"Context menu", functionToUse:loadRunWait, script:"76_contextMenu_test.js", time:40, result:{type:4, value:"OK"}},
	{name:"Waypoint, route & track", functionToUse:loadRunWait, script:"90_waypoint_route_track_tests.js", time:3, result:{type:2, value:"No errors found"}}
	];
*/

tests = [ // test table format
	// (0) Test name	(1) function to run	(2) script name (3) max seconds to wait (4) expected last line of output
	{name:"Simple script", functionToUse:loadRunWait, script:"01_simple_script.js", time:2, result:{type:2, value:"All done OK"}},
	{name:"Message", functionToUse:loadRunWait, script:"71_message_test.js", time:10, result:{type:2, value:"OK"}},
	{name:"Alarm", functionToUse:loadRunWait, script:"72_alarm.js", time:10, result:{type:2, value:"Alarm OK"}},
	{name:"Basic console", functionToUse:loadRunWait, script:"05_basic_console_tests.js", time:2, result:{type:2, value:"Basic console OK"}},
	{name:"Compile error", functionToUse:loadRunWait, script:"10_error_compile.js", time:1, result:{type:0, value:"SyntaxError: unterminated string"}},
	{name:"Throw from main", functionToUse:loadRunWait, script:"11_throw_from_main.js", time:1, result:{type:0, value:"Thrown from main"}},
	{name:"Runtime error main", functionToUse:loadRunWait, script:"12_runtime_error_main.js", time:3, result:{type:0, value:"line 3 Error: Console Test - Cancel onTimer callbacks has invalid call"}},
	{name:"Error trace", functionToUse:loadRunWait, script:"16a_error_message_checks.js", time:3, result:{type:2, value:"All OK"}},
	{name:"Throw from callback", functionToUse:loadRunWait, script:"14_error_from_callback.js", time:3, result:{type:0, value:"doit line 6 Error: Console Test - Cancel onTimer callbacks has invalid call"}},
	{name:"Printing", functionToUse:loadRunWait, script:"20_print_tests.js", time:30, result:{type:4, value:"Print tests completed OK"}},
	{name:"Position/Waypoint/Route", functionToUse:loadRunWait, script:"02_PosWayRouteTests.js", time:2, result:{type:2, value:"Tests ran to completion"}},
	{name:"Timeout main", functionToUse:loadRunWait, script:"22_main_timeout.js", time:21, result:{type:1, value:"Script timed out"}},
	{name:"Timeout onExit", functionToUse:loadRunWait, script:"24_onExit_timeout.js", time:21, result:{type:1, value:"Test->wrapUpWorks() onExit handler timed out"}},
	{name:"Timer + callbacks", functionToUse:loadRunWait, script:"30_timer_tests.js", time:6, result:{type:2, value:"12346789AM"}},
	{name:"Callback cancellations", functionToUse:loadRunWait, script:"31_callbackCancellations.js", time:2, result:{type:2, value:"Callback cancellations OK"}},
	{name:"Messaging", functionToUse:loadRunWait, script:"32_messaging.js", time:15, result:{type:4, value:"Messaging OK"}},
	{name:"NMEA", functionToUse:loadRunWait, script:"34_NMEA.js", time:4, result:{type:4, value:"NMEA OK"}},
	{name:"Navigation", functionToUse:loadRunWait, script:"36_navigation.js", time:2, result:{type:2, value:"Navigation OK"}},
	{name:"Read text file", functionToUse:loadRunWait, script:"40_read_text_file.js", time:3, result:{type:2, value:"Read local text file matched"}},
	{name:"Read remote text file", functionToUse:loadRunWait, script:"41_read_remote_text_file.js", time:3, result:{type:2, value:"Read remote text file matched"}},
	{name:"Load from URL", functionToUse:loadRunWait, script:"48_load_from_url.js", time:30, result:{type:2, value:"Load from URL OK"}},
	{name:"Files object", functionToUse:loadRunWait, script:"43_files.js", time:3, result:{type:2, value:"Files OK"}},
	{name:"Park console", functionToUse:loadRunWait, script:"44_parking.js", time:50, result:{type:2, value:"Parking completed"}},
	{name:"Keyboard states", functionToUse:loadRunWait, script:"120_keyboard_states.js", time:45, result:{type:4, value:"States pass"}},
	{name:"onCloseButton", functionToUse:loadRunWait, script:"46_close_button.js", time:18, result:{type:2, value:"Succeeded"}},
	{name:"notifications", functionToUse:loadRunWait, script:"48_notifications.js", time:15, result:{type:2, value:"Notes OK"}},
	{name:"Stop main", functionToUse:loadRunWait, script:"50_stop_main.js", time:1, result:{type:4, value:"Script was stopped"}},
	{name:"Stop result", functionToUse:loadRunWait, script:"52_stop_result_main.js", time:1, result:{type:4, value:"Stop result"}},
	{name:"Stop callback", functionToUse:loadRunWait, script:"54_stop_explicit_callback.js", time:3, result:{type:4, value:"Explicit stop callback"}},
	{name:"Explicit main explicit stop callback", functionToUse:loadRunWait, script:"56_result_explicit_callback_stop_explicit.js", time:3, result:{type:4, value:"Explicit stop callback"}},
	{name:"Explicit main", functionToUse:loadRunWait, script:"60_explicit_result_main.js", time:1, result:{type:2, value:"Explicit result"}},
	{name:"Explicit result stop", functionToUse:loadRunWait, script:"62_explict_result_stopped.js", time:1, result:{type:4, value:"Explicit result"}},
	{name:"Explicit main callback stop", functionToUse:loadRunWait, script:"64_explicit_main_callback_stop.js", time:3, result:{type:4, value:"Explicit result"}},
	{name:"Alert", functionToUse:loadRunWait, script:"70_alerts.js", time:20, result:{type:2, value:"Alert OK"}},
	{name:"Dialogue", functionToUse:loadRunWait, script:"74_dialogue_test.js", time:75, result:{type:4, value:"Dialogue tests good"}},
	{name:"Context menu", functionToUse:loadRunWait, script:"76_contextMenu_test.js", time:40, result:{type:4, value:"OK"}},
	{name:"onConsoleResult", functionToUse:loadRunWait, script:"81_on_console_result.js", time:2, result:{type:4, value:"onConsoleResult OK"}},
	{name:"Chain_no_brief", functionToUse:loadRunWait, script:"80_chain_no_brief.js", time:1, result:{type:0, value:"getBrief found no brief"}},
	{name:"Chain_with_brief", functionToUse:loadRunWait, script:"82_chain_with_brief.js", time:5, result:{type:2, value:"Found brief This is the brief"}},
	{name:"Call no brief", functionToUse:loadRunWait, script:"84_call_no_brief.js", time:1, result:{type:2, value:"getBrief found no brief"}},
	{name:"Call with brief", functionToUse:loadRunWait, script:"86_call_with_brief.js", time:1, result:{type:2, value:"Found brief This is the brief"}},
	{name:"Call chain with brief", functionToUse:loadRunWait, script:"88_call_chained.js", time:1, result:{type:2, value:"Found brief This is the brief"}},
	{name:"_remember", functionToUse:loadRunWait, script:"89 _remember_test.js", time:1, result:{type:4, value:"_remember OK"}},
	{name:"Waypoint, route & track", functionToUse:loadRunWait, script:"90_waypoint_route_track_tests.js", time:3, result:{type:2, value:"No errors found"}},
	{name:"GetGUIDs", functionToUse:loadRunWait, script:"100_getGuids.js", time:3, result:{type:2, value:"No errors"}},
	{name:"N2k decode/encode", functionToUse:loadRunWait, script:"110_n2k_encode_decode.js", time:5, result:{type:2, value:"All tests passed"}},
	{name:"Sockets", functionToUse:loadRunWait, script:"130_sockets.js", time:2, result:{type:4, value:"Sockets good"}},
	{name:"Sockets CBOR", functionToUse:loadRunWait, script:"131_sockets_CBOR.js", time:2, result:{type:4, value:"Sockets CBOR test good"}},
	{name:"socketChunker", functionToUse:loadRunWait, script:"132_socketChunker.js", time:2, result:{type:4, value:"socketChunker OK"}},
	];

config = OCPNgetPluginConfig();
messageBox(JSON.stringify(config, null, "    "),"OK","Plugin Configuration");
pluginVersion = config.PluginVersionMajor*100 + config.PluginVersionMinor;

// check essentials working
testName = "Test";
if (consoleExists(testName)){
	outcome = consoleClose(testName);
	if (outcome) throw("Unble to close a test console - " + outcome);
	}
if (consoleExists(testName)) stopScript("Unable to close a console");
consoleAdd(testName);
if (!consoleExists(testName)) stopScript("Unable to add a console");
outcome = consoleClose(testName);
if (outcome) throw("Unble to close a test console - " + outcome);

// we will construct the button rows dynamically from the tests table.
// we do this so we can display them over severl rows - hold on!
buttonCount = tests.length;

var buttonsArray = [[]];	// will be an array of arrays

for (i = 0, buttonsRow = 0; i < buttonCount; i++){//  construct the array of button names
	buttonsArray[buttonsRow].push(tests[i].name);
	if (((i+1)%5) == 0) { 
		buttonsArray.push([]);	// new row
		buttonsRow++;
		}
	}
if (buttonsArray[buttonsArray.length-1].length == 0) buttonsArray.pop();	// Remove last if empty
for (i = 0; i < buttonsArray.length; i++){
	control.push({type:"button", label:buttonsArray[i]});
	}
control.push({type:"text", value:message});		// space for message
onDialogue(action, control);
// end of buttons construction

function action(dialog){	// match button to tests
//	consoleClearOutput();
	button = dialog[dialog.length-1].label;
	for (i = 0; i < tests.length; i++){
		if (button == tests[i].name){
			runAll = false;
			testIndex = i;
			callOne(testIndex);
			return;
			}
		}
	if (button == "Run all"){	// to run all tests
			runAll = true;
			testIndex = 0;
			errorCount = 0;
			callAll(testIndex);
			return;
		}
	}

function callOne(which){
	// run one test
	timedOut = false;
	if (!consoleExists(name)) consoleAdd(name);
	else if (consoleBusy(name)){
		consoleClose(name);
		consoleAdd(name);
		}
	consoleLoad(name, tests[which].script);
	timerCount = 0;
	onSeconds(oneTimeout, tests[which].time);
	onConsoleResult(name, oneDone);
	}

function callAll(which){
	// run next test
	timedOut = false;
	onDialogue(stopTest, abortDialogue);

	if (consoleExists(name)) consoleClose(name);
	consoleAdd(name);
	consoleLoad(name, tests[which].script);
	timerCount = 0;
	onSeconds(allTimeout, tests[which].time);
	onConsoleResult(name, nextDone);
	}

function oneDone(handback){
	if (timedOut) return;	// ignore
	onSeconds();	// cancel timeer
	lastLine = handback.value;
	shouldBe = tests[testIndex].result.value;
	success = ((handback.type == tests[testIndex].result.type) && (lastLine.indexOf(shouldBe) >= 0));	// compare with expected result
	message = tests[testIndex].name + " test   ";
	if (lastLine == "Skipped") message += "SKIPPED";
	else if (success) message += "PASSED";
	else message += "FAILED - type:"+ handback.type + " value: " + lastLine;
	control[control.length-1].value = message;
	onDialogue(action, control);
	}

function oneTimeout(){
	timedOut = true;
	message = tests[testIndex].name + " test   timed out";
	control[control.length-1].value = message;
	consoleClose(name);
	onDialogue(action, control);
	}

function nextDone(handback){
	onDialogue(false);
	onSeconds();	// cancel timer
//	print("In nextDone with handback:", handback, "\n");
	if (handback.type == 8){ // this is a callAfter from a consoleClose() - ignore
		return;
		}
	lastLine = handback.value;
	shouldBe = tests[testIndex].result.value;
	success = ((handback.type == tests[testIndex].result.type) && (lastLine.indexOf(shouldBe) >= 0));	// compare with expected result
	message = "Test " + tests[testIndex].name;
	print((tests[testIndex].name + "                                    ").slice(0, 40));
	if (handback.type == 3){ // special fix for this
		message += handback.value;
		printOrange(handback.value, "\n");
		}
	else if (success) {
		message += "PASSED";
		printGreen("PASSED\n");
		}
	else {
		message += "FAILED - " + lastLine;
		printRed("FAILED - ", lastLine, "\n");
		errorCount++;
		}
	control[control.length-1].value = message;
	onSeconds(more, 0.01);	// give it time for things to wrap up
//	more();
	}


function allTimeout(){
	timedOut = true;
	nextDone({type:3, value:"TIMED OUT"});
	errorCount++;
	}

function more(){	// decide if more to run
	// print("In more() with old testIndex ", testIndex, "\n");
	if (++testIndex < tests.length) callAll(testIndex);
	else {
		print("Run all done - ");
		consoleClose(name);
		if (errorCount > 0) printRed(errorCount, " errors\n");
		else printGreen("No errors\n");
		message = "Run all done - "+ (errorCount> 0?(errorCount + " errors"):"No errors");
		control[control.length-1].value = message;
		onDialogue(action, control);
		}
	}


function loadRunWait(){};	// not used any more but keep compiler happy

function stopTest(){
	consoleClose(name);
	stopScript("Tests aborted");
	}
