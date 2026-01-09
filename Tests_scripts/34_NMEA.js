handleTest = false;

sentence1 = "$ABJKL,sentence1";
sentence2 = "$ABGHI,sentence2";
sentence3 = "$ABXYZ,sentence2";
sentence4 = "$ABMLN,handleData";
OK1 = OK2 = OK3 = OK4 = false;

if (handleTest){
	// find NMEA0183 handle
	handles = OCPNgetActiveDriverHandles();
	for (h = 0; h < handles.length; h++){
		attributes = OCPNgetDriverAttributes(handles[h]);
		if (attributes.protocol == "nmea0183"){
			handle = handles[h];
			print("Sending handle port ", attributes.netPort, "\n");
			break;
			}
		}
	if (typeof handle == "undefined") throw("No NMEA0183 handle found");
	}

onSeconds(stage1, 1);
OCPNonNMEA0183(get1);

function stage1() {OCPNpushNMEA0183(sentence1);}

function get1(result){
	if (!result.OK) stopScript("NMEA stage 1 checksum error");
	else if (result.value != sentence1) stopScript("NMEA test 1 failed");
	OK1 = true
	print("Complete 1\n");
	OCPNonNMEA0183(receiveXYZ, "XYTRE");	// should not be received
	OCPNonNMEA0183(receiveXYZ, "ABXYZ");
	OCPNonNMEA0183(receiveGHI, "GHI");
	if (handleTest){
		OCPNonNMEA0183(receiveGHI, "MLN");
		OCPNpushText(sentence4, handle);
		}
	OCPNpushNMEA0183(sentence2);
	OCPNpushNMEA0183(sentence3);
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

function receiveMLN(result){
	if (!result.OK) stopScript("NMEA stage 4 checksum error");
	else if (result.value != sentence4) stopScript("NMEA test 4 failed");
	OK4 = true;
	print("Complete 4\n");
	}

function sumup(){
	if (OK1 && OK2 && OK3 && (handleTest?OK4:true)) stopScript("NMEA OK");
	else stopScript("NMEA sumup failed");
	}
