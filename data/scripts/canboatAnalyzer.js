function canboatAnalyzer(payload, shellString, analyzer){
	trace = false;
	shell = require("shell");
	if (trace) printOrange("cA1:payload ", payload, "\n");
	canboatString = reformToCanboat(payload);
	if (trace) printOrange("cA2:canboatString ", canboatString, "\n");
	pipeline = "echo \"" + canboatString + "\" | " + analyzer + " -json";
	if (trace) printOrange("cA3:pipeline ", pipeline, "\n");
	outcome = shell(pipeline, shellString, true);
	print("stderr: ", outcome.stderr, "\n");
	if (outcome.stderr.indexOf("ERROR") >= 0) return(outcome.stderr);
	outcome = outcome.stdout;
	if (trace) printOrange("outcome:", outcome, "\n");
	offset = outcome.indexOf("{", 3);	// where real stuff starts
	real = outcome.slice(offset);
	return JSON.parse(real);

	function reformToCanboat(payload){
		// given OCPN nmea2k payload, returns Actisense string as expected by canboat analyzer
		build = [];
		timeStamp = getBytes(payload, 8, 4);
		timeStamp = checkNumber(timeStamp, false, 32);
		if (timeStamp == "undefined") timeStamp = "2023-11-24-22:42:04.388";	// needs to be valid
		build.push(timeStamp);
		build.push(payload[2]);	// priority
		pgn = getBytes(payload, 3, 3);
		build.push(pgn);
		build.push(payload[7]);	//src
		build.push(payload[8]);	//dst
		build.push(payload[12]);	//data count
		for (i = 13; i < payload.length; i++) build.push( ("0" + payload[i].toString(16)).slice(-2)); // data to hex
		return (build.join(","));
		}

	function getBytes(v, start, bytes){	// little endean!
		offset = start+bytes-1;
		result = v[offset--];
		if (trace & 16) {printOrange("getBytes start:", start, "\tbytes:", bytes, "\toffset:", offset); printOrange("\tresult:", result, "\n");}
		for ( ; offset >= start; offset--){
			// result = (result << 8) | v[offset];  shift uses 32 bits, so to be 64 bit safe we use...
			result = (result * 256) + v[offset];
			if (trace & 16) printOrange("getBytes loop offset:", offset, "\tresult:", result, "\n");
			}
		nextByte = start + bytes;
		if (trace & 16) printOrange("getBytes final result:", result, "\n");
		return result;
		};

	function checkNumber(number, isSigned, bits){ // check number is valid and fix sign
		// See here for special meaning of some numbers https://canboat.github.io/canboat/canboat.html#field-types
		// in JavaScript, bit ops are only available up to 32 bits
		// so to be 64 bit safe, we have to manage without bit shift and bit flip (:-#)
		toShift = bits;
		if (isSigned) toShift--;	// signed numbers have one less max value
		max = (2 ** toShift) - 1;	// maximum possible value
		if (number == max) number = "undefined";
		else if ((bits >= 4) && (number == max-1)) number = "has error";
		else if (isSigned && (number > max)){// must be negative
			number = (2**bits - number) * -1;
			}
		// printOrange("checkNumber - number:", number, "\tbits:", bits, "\tmax:", max,"\n");
		return number;
		}
	}
