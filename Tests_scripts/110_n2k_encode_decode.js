// NMEA2000 object tests

runAnalyser = false;
analyzer = "/Users/Tony/OpenCPN_project/canboat/rel/darwin-arm64/analyzer";// where to find analyzer

verbose = false;	// make true to print objects

tests = [
	{	// Test 0
	pgn: 126992,
	data: [147,19,
		3,		// priority
		16,240,1,	// pgn
		255,		// desination
		1,		// origin
		255,255,255,255, // timestamp
		8,		// count
		13,240,207,76,208,3,94,40	// data
		],
	expected: {"PGN":126992,"id":"systemTime","description":"System Time","priority":3,"desination":255,"origin":1,"sid":13,"source":"GPS","date":"2023.11.02","time":"18:48:45"}
		},
	{	// Test 1
	pgn: 129540,
	data: [147,19,3,4,250,1,	255,1,255,255,255,255,147,
	13,254,12,25,33,51,101,183,28,12,0,0,0,0,242,28,115,20,147,206,240,10,0,0,0,0,242,32,162,28,241,177,172,13,0,0,0,0,242,66,46,23,124,202,84,11,0,0,0,0,242,76,34,21,101,198,40,10,0,0,0,0,242,75,115,50,11,17,96,9,0,0,0,0,242,65,162,13,241,162,40,10,0,0,0,0,242,3,23,4,53,235,0,0,0,0,0,0,240,6,174,15,115,35,0,0,0,0,0,0,240,11,174,15,10,62,0,0,0,0,0,0,240,12,150,41,22,49,0,0,0,0,0,0,240,24,255,14,103,93,0,0,0,0,0,0,240],
	expected: {"PGN":129540,"id":"gnssSatsInView","description":"GNSS Sats in View","priority":3,"desination":255,"origin":1,"sid":13,"rangeResidualMode":"undefined","satsInViewCount":12,"satsInView":[{"prn":25,"elevation":"1.3089000","azimuth":"4.6949000","snr":"31.0000000","rangeResiduals":0,"status":"Used"},{"prn":28,"elevation":"0.5235000","azimuth":"5.2883000","snr":"28.0000000","rangeResiduals":0,"status":"Used"},{"prn":32,"elevation":"0.7330000","azimuth":"4.5553000","snr":"35.0000000","rangeResiduals":0,"status":"Used"},{"prn":66,"elevation":"0.5934000","azimuth":"5.1836000","snr":"29.0000000","rangeResiduals":0,"status":"Used"},{"prn":76,"elevation":"0.5410000","azimuth":"5.0789000","snr":"26.0000000","rangeResiduals":0,"status":"Used"},{"prn":75,"elevation":"1.2915000","azimuth":"0.4363000","snr":"24.0000000","rangeResiduals":0,"status":"Used"},{"prn":65,"elevation":"0.3490000","azimuth":"4.1713000","snr":"26.0000000","rangeResiduals":0,"status":"Used"},{"prn":3,"elevation":"0.1047000","azimuth":"6.0213000","snr":"0.0000000","rangeResiduals":0,"status":"Not tracked"},{"prn":6,"elevation":"0.4014000","azimuth":"0.9075000","snr":"0.0000000","rangeResiduals":0,"status":"Not tracked"},{"prn":11,"elevation":"0.4014000","azimuth":"1.5882000","snr":"0.0000000","rangeResiduals":0,"status":"Not tracked"},{"prn":12,"elevation":"1.0646000","azimuth":"1.2566000","snr":"0.0000000","rangeResiduals":0,"status":"Not tracked"},{"prn":24,"elevation":"0.3839000","azimuth":"2.3911000","snr":"0.0000000","rangeResiduals":0,"status":"Not tracked"}]}
		},
	{	// Test 2
	pgn: 129029,
	data: [147,19,3,5,248,1,	255,1,255,255,255,255,
		43,						// count
		96,						// sid
		196,76, 48,169,186,37,		// date & time
		0,244,115,40,109,103,77,7,	// latitude
		2,144,3,51,154,166,52, 0,		// longitude
		64,50,59,7,0,0,0,0,			// altitude
		16,						// type & method
		252,						// integrety
		12,						// number of satellites used
		150,0,					// hdop
		250,0,					// pdop
		91,18,0,0,				// geodal separation
		0						// reference stations
		],
	expected: {"PGN":129029,"id":"gnssPositionData","description":"GNSS Position Data","priority":3,"desination":255,"origin":1,"sid":96,"date":"2023.10.22","time":"17:34:59","latitude":"52.6190450","longitude":"1.4819880","altitude":"121.3200000","gnssType":"GPS","method":"GNSS fix","integrity":"No integrity checking","numberOfSvs":12,"hdop":"1.5000000","pdop":"2.5000000","geoidalSeparation":"46.9900000","referenceStationsCount":0,"referenceStations":[]}
		},
	{ // 3
	pgn: 129285,
	data: [147,19,3,5,249,1,	255,1,255,255,255,255,54,
		255,255,		// startv  RPS#
		2,0,			// nitems
		255,255,		// database ID
		255,255,		// routeID
		224,			// navigationDirectionInRoute + supplementaryRouteWpDataAvailable + reserved
		9,1,83,69,78,48,48,52,0,	// route name
		255,			// reserved
		0,0,			// wp ID 1
		3,1,0,		//wp name 1 (null)
		255,255,255,127,	// latitude 1
		255,255,255,127,	// longitude 1
		1,0,			// wp id 2
		12,1,80,111,111,108,101,32,66,97,114,0,// wp name 2
		88,80,49,30,	// latitude
		185,47,219,254	// longitude (negative!)
		],
	expected: {"PGN":129285,"id":"navigationRouteWpInformation","description":"Navigation - Route/WP Information","priority":3,"desination":255,"origin":1,"nitemsCount":2,"navigationDirectionInRoute":"Forward","supplementaryRouteWpDataAvailable":"Off","routeName":"SEN004","nitems":[{"wpId":0,"wpName":""},{"wpId":1,"wpName":"Poole Bar","wpLatitude":"50.6548312","wpLongitude":"-1.9189831"}]}
		},
	{ // 4
	pgn: 126996,
	data: [147,19,255,20,240,1,255,255,255,255,255,255,134,52,48,232,253,83,117,112,101,114,32,77,111,100,101,108,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,65,108,112,104,32,50,48,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,86,101,114,121,32,108,111,111,110,103,32,109,111,100,101,108,32,118,101,114,115,105,111,110,32,110,117,109,98,101,114,0,83,101,114,105,97,108,32,99,111,100,101,32,110,117,109,98,101,114,32,104,101,114,101,0,0,0,0,0,0,0,0,0,200,1],
	expected: {"PGN":126996,"id":"productInformation","description":"Product Information","priority":255,"desination":255,"origin":255,"nmea2000Version":"12.3400000","productCode":65000,"modelId":"Super Model","softwareVersionCode":"Alph 20","modelVersion":"Very loong model version number","modelSerialCode":"Serial code number here","certificationLevel":200,"loadEquivalency":1}
		}
	];	

Nmea2kConstructor = require("NMEA2000");
Position = require("Position");

NMEA2k = new Nmea2kConstructor(126992,null, {undefined:true});
print(JSON.stringify(NMEA2k, null, "\t"), "\n\n")

decodeErrorCount  = 0;
runTime = 0;
for (t = 0; t < tests.length; t++){
	test = tests[t];
	printUnderlined("\nTesting pgn ", test.pgn, "\n");
	if (verbose) print("Data count after header: ", test.data.length-13, "\n");
	startTime = new Date();
	NMEA2k = new Nmea2kConstructor(test.pgn, test.data, {undefined:false,trace:0});
	endTime= new Date();
	runTime += endTime - startTime;
	OK = (JSON.stringify(NMEA2k) == JSON.stringify(test.expected));
	if (!OK) decodeErrorCount++;
	print("Test ", t, " decode ", OK ? "passed":"failed", "\n");
	if (!OK){
		printBlue("Result:\n", NMEA2k, "\n");
		printGreen("Expected:\n",test.expected, "\n");
		}
	if (verbose) print(JSON.stringify(NMEA2k, null, "\t"), "\n");
	switch (NMEA2k.pgn){
	case 126992:	// system time
		print("System time: ",  NMEA2k.date + " " + NMEA2k.time, "\n");
		break;
	case 129029: // sat fix
		position = new Position(NMEA2k.latitude, NMEA2k.longitude);
		print("Position: ", position.formatted, "\n");
		print("Fix time: ",  NMEA2k.date + " " + NMEA2k.time, "\n");
		print("using ", NMEA2k.numberOfSvs, " satellites\n");
		break;
	case 129540:
		break;
		}
	}
print("\nTotal time in main decode tests ", runTime, "ms\n");

test = tests[1];
printUnderlined("\nTesting decode method with pgn ", test.pgn, "\n");
NMEA2k = new Nmea2kConstructor(test.pgn, null, {trace:0});
if (verbose) print("Bare object:\n", JSON.stringify(NMEA2k, null, "\t"), "\n");
outcome = NMEA2k.decode(test.data);
if (verbose) print("Decoded object:\n", JSON.stringify(outcome, null, "\t"), "\n");
if (!OK) decodeErrorCount++;
print("Decode method ", OK ? "passed":"failed", "\n");

printBlue((decodeErrorCount > 0) ? "\nDecode " + decodeErrorCount + " errors\n":"\nAll decode tests passed\n");

// encoding tests
encodeErrorCount = 0;
runTime = 0;
for (t = 0; t < tests.length; t++){
	test = tests[t];
	printUnderlined("\nEncoding pgn ", test.pgn, "\n");
	trace = 0;
	if (t == 1) trace = 32;
	startTime = new Date();
	NMEA2k = new Nmea2kConstructor(test.pgn, test.data, {trace:0});
	encoded = NMEA2k.encode({timeStamp:"undefined"});
	endTime= new Date();
	runTime += endTime - startTime;
	OK = matches(encoded, test.data);
	print("Test ", t, " encode ", OK ? "OK":"failed", "\n");
	if (!OK){
		encodeErrorCount++;
		printBlue("Result:\n", encoded, "\n");
		printGreen("Expected:\n", test.data, "\n"); 
		printBlue(JSON.stringify(NMEA2k, null, "\t"), "\n");
		printGreen(JSON.stringify(NMEA2k.descriptor, null, "\t"), "\n");
		}
	}
print("\nTotal time in encode tests ", runTime, "ms\n");
printBlue((encodeErrorCount > 0) ? "\nEncode " + encodeErrorCount + " errors\n":"All encode tests passed\n");

if (!runAnalyser){
	scriptResult((decodeErrorCount + encodeErrorCount == 0) ? "All tests passed" : "Had errors");
	stopScript();
	}

canboatAnalyzer = require("canboatAnalyzer");

runTime = 0;
for (t = 0; t < tests.length; t++){
	test = tests[t];
	printUnderlined("\nTesting pgn with canboat ", test.pgn, "\n");
	startTime = new Date();
	outcome = canboatAnalyzer(test.data, "bash", analyzer);
	endTime= new Date();
	runTime += endTime - startTime
	if (1) print(JSON.stringify(outcome, null, "\t"), "\n");
	}
print("\nTotal time in canboat tests ", runTime, "ms\n");

function matches(a, b){	// compare two simple arrays
	if (a.length != b.length) return false;
	for (var i = 0; i < a.length; i++){
		if (a[i] != b[i]) return false;
		}
	return true;
	}
