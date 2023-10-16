sentence1 = "$ABJKL,sentence1";
sentence2 = "$ABGHI,sentence2";
sentence3 = "$ABXYZ,sentence2";
OK1 = OK2 = OK3 = false;

onSeconds(stage1, 1);
OCPNonNMEAsentence(get1);

function stage1() {OCPNpushNMEA(sentence1);}

function get1(result){
	if (!result.OK) stopScript("NMEA stage 1 checksum error");
	else if (result.value != sentence1) stopScript("NMEA test 1 failed");
	OK1 = true
	print("Complete 1\n");
	OCPNonNMEAsentence(receiveXYZ, "XYTRE");	// should not be received
	OCPNonNMEAsentence(receiveXYZ, "ABXYZ");
	OCPNonNMEAsentence(receiveGHI, "GHI");
	OCPNpushNMEA(sentence2);
	OCPNpushNMEA(sentence3);
	onSeconds(sumup,1);
	}

function receiveXYZ(result){
	if (!result.OK) stopScript("NMEA stage 2 checksum error");
	else if (result.value != sentence3) stopScript("NMEA test 2 failed");
	OK2 = true;
	print("Complete 2\n");
	}

function receiveGHI(result){
	if (!result.OK) stopScript("NMEA stage 3 checksum error");
	else if (result.value != sentence2) stopScript("NMEA test 3 failed");
	OK3 = true;
	print("Complete 3\n");
	}

function sumup(){
	if (OK1 && OK2 && OK3) stopScript("NMEA OK");
	else stopscript("NMEA sumup failed");
	}