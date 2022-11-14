alert("Do not dismiss this alert yet.\n");

onSeconds(three, 3);
onSeconds(five, 5);
onSeconds(seven, 7);
onSeconds(longStop, 20);

function three(){
	alert("This line added to alert.\nDo not dismiss yet.");
	}

function five(){
	alert(false);
	}

function seven(){
	OCPNsoundAlarm();
	alert("**** Dismiss this alert now ****");
	onSeconds(check, 1);
	}

function check(){
	if (!alert()) {
		stopScript("Alert OK");
		}
	else onSeconds(check, 1);
	}

function longStop() {
	throw("User failed to dismiss alert");
	}
