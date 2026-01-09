a = "aaa"; b= "bbb"; c = "ccc";
var count = 0;
var log = "";
onExit(wrapup);
expected = "Test A 0\nTest B 0\nTest B 0\nTest C 0\nTest A 1\nTest B 1\nTest B 1\nTest C 1\nTest A 2\nTest B 2\nTest B 2\nTest C 2\nTest A 3\nTest B 3\nTest B 3\nTest C 3\nTest A 4\nTest B 4\nTest C 4\nTest A 5\nTest B 5\nTest C 5\n";


id1 = OCPNonMessageName(catcha, a);
id2 = OCPNonAllMessageName(catchb, b);
id3 = OCPNonAllMessageName(catchb, b);
id4 = OCPNonAllMessageName(function(message){report(message);},c);
print("ids are: ", id1, " ", id2,  " ", id3, " ", id4, "\n");
onAllSeconds(tick, 1, count);

/*
try {
	OCPNonAllMessageName(0);	// should throw error
	printRed("Attempting to cancel OCPNonAllMessageName with invalid id not thrown\n");
	}
catch(err){
	printGreen("Attempting to cancel OCPNonAllMessageName with invalid id thrown correctly\n");
	}
*/

print("Attempting to cancel OCPNonAllMessageName with invalid id fails correctly: ",
	!OCPNonAllMessageName(0), "\n");

function tick(){
	OCPNsendMessage(a, "Test A " + count);
	OCPNsendMessage(b, "Test B " + count);
	OCPNsendMessage(c, "Test C " + count);
	if (count == 4) {
		print("Before cancel, ids are ", OCPNonMessageName("?"), "\n")
		print("About to cancel ", id2, "\n");
		OCPNonMessageName(id2);
		print("After cancel, ids are ", OCPNonMessageName("?"), "\n")
		}
	else if (count == 6) OCPNonMessageName(false);
	else if (count >= 8) stopScript();
	count++;
	}

function catcha(message){
	report(message);
	OCPNonMessageName(catcha, a);
	}

function catchb(message){
	report(message);
	}

function report(message){
	message += "\n";
	print(message);
	log += message; 
	}

function wrapup(){
	// printGreen(log, "\n");
	// printBlue(expected, "\n");
	scriptResult((log == expected)?"Messaging OK":"Messaging Had errors");
	}
