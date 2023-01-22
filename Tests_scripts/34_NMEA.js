require("Consoles");

other = "other";
name = "JSTEST";
testText = "$JSTST, Test NMEA data";
script = "OCPNpushNMEA('" + testText + "');";
onExit(clearUp);
onSeconds(timeOut, 3);	// longstop in case of no response
clearUp();
consoleAdd(other);
consoleLoad(other, script);
OCPNonNMEAsentence(getResult, name);
consoleRun(other);

function getResult(received){
//	print(received, "\n");
	if (received.value.slice(0,6) != testText.slice(0,6)){	// try again
		OCPNonNMEAsentence(getResult, name);
		return;
		}
	if (!received.OK) {
		scriptResult("NMEA data has checksum error");
		stopScript();
		}
	scriptResult((received.value == testText) ? "NMEA OK" : "NMEA failed");
	stopScript();
	}

function clearUp(){
	if (consoleExists(other)) consoleClose(other);
	}

function timeOut(){
	messageBox("NMEA data not seen\nCheck OCPN filtering allows receipt of NMEA $JSTST", "OK", "NMEA test");
	clearUp();
	}
