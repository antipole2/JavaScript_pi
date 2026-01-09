// decode NMEA2000 payload or encode NMEA2000 object v1.0
// tracing options as per value of tracechoice  NMEA2000(arg, arg, {trace: tracechoice}}
//	1	    1	call parameters and high level flow
//	2	   10	print descriptor
//	4	 0100	parse
//	8	 1000	decode
//	16	10000	byte/bit decoding
//	32  100000	byte/bit encoding

function NMEA2000(arg, data, options){	
	// do not include the following when enumerating the NMEA2000 object
	Object.defineProperty(this, "descriptor", {enumerable: false, writable: true});
	Object.defineProperty(this, "trace", {enumerable: false, writable: true});
	Object.defineProperty(this, "undefined", {enumerable: false, writable: true});
	this.trace = false;
	this.undefined = false;	// normally omit undefined values

	// examine the arguments
	this.descriptor = false;	// no descriptor yet
	// printOrange("NMEA2000 options arg:", options, "\ttypeof: ", typeof(options),"\n");
	if (typeof(options) == "object"){
		if (options.hasOwnProperty("trace")) this.trace = options.trace;
		if (options.hasOwnProperty("undefined")) this.undefined = options.undefined;
		}
	var trace = this.trace;	// for convenience
	// printOrange("NMEA2000 called with trace:", this.trace, "\n");
	if (trace & 1) printOrange("NMEA2000 called with arg type ", typeof(arg), "\n");
	switch (typeof(arg)){
		case "number":
			if (trace & 1) printOrange("arg is number ", arg, "\n");
			this.descriptor = require("pgnDescriptors")(arg);	// load descriptor
			break;
		case "object":	// custom descriptor
			if (trace & 1) printOrange("Have custom descriptor?\n");
			if (arg.constructor.name == "Array") throw("NMEA2000 called with invalid first arg");
			pgn = arg.PGN;
			if (typeof(pgn) != "number") throw("NMEA2000 called with 1st arg invalid descriptor");
			if (trace & 1) printOrange("NMEA2000 called with custom descriptor for pgn " + arg.PGN);
			this.descriptor = arg;	// the provided custom descriptor
			break;
		default: throw("NMEA2000 called with invalid arg type " + typeof(arg));
		}
	desc = this.descriptor;	// desc is shorthand for convenience
	// if not already done, shift Fields up so we can index from 1 rather than zero
	if (desc.Fields[0] != 0) desc.Fields.unshift(0);
	if (desc.Fields[1].Order != 1) throw("NMEA2000 invalid descriptor");	// now order should match index
	if (trace & 2) printOrange("\n", JSON.stringify(this.descriptor, null, "\t"), "\n\n");
	this.PGN = desc.PGN;
	// register the repeating fields, if any
	repeat1Size = desc.RepeatingFieldSet1Size;
	repeat1StartField = desc.RepeatingFieldSet1StartField;
	repeat1CountField = desc.RepeatingFieldSet1CountField;
	if (desc.RepeatingFieldSet2Size != void 0){
		printRed("Descriptor ", desc.Id, " has second repeating field group", "\n");
		throw("Unsupported descriptor");
		}
	if ((repeat1Size != void 0) && (repeat1StartField != void 0) && (repeat1CountField != void 0)){
		repeat1 = true;
		repeatName = desc.Fields[repeat1CountField].Id;	// name of repetition
		if (trace & 1) printOrange("Repeating fields - repeat1Size:", repeat1Size, "\trepeat1StartField:",
			repeat1StartField, "\trepeat1CountField:", repeat1CountField, "\trepeatName:", repeatName, "\n");
		thisRepeat = {};
		}
	else repeat1 = false;
	if ((typeof(data) == void 0) || (data == null)) buildBare(this);
	else if (Array.isArray(data)) parse(this, data);
	else if (typeof (data) == "object") throw("NME2000 constructor called with object for 2nd argument");
	else throw("NMEA2000 constructor - 2nd arg unknown type: " + typeof data);

	// now for the methods

	this.decode = function(data){
		var trace = this.trace;
		desc = this.descriptor;
		repeat1Size = desc.RepeatingFieldSet1Size;
		repeat1StartField = desc.RepeatingFieldSet1StartField;
		repeat1CountField = desc.RepeatingFieldSet1CountField;
		parse(this, data);
		return this;
		}

	this.encode = function(options){
		var trace = this.trace;
		if (typeof(options) == "object") undefineTimestamp = true;
		else undefineTimestamp = false;
		desc = this.descriptor;
		repeat1Size = desc.RepeatingFieldSet1Size;
		repeat1StartField = desc.RepeatingFieldSet1StartField;
		repeat1CountField = desc.RepeatingFieldSet1CountField;
		result = [147, 19];	// build the standard OCPN header
		result.push(checkUndefined(this.priority, 8));
		result = result.concat(encodeBytes(this.PGN, 3));	// the pgn
		result.push(checkUndefined(this.destination, 8), checkUndefined(this.origin, 8));
		if (undefineTimestamp) result.push(255,255,255,255);
		else result = result.concat(encodeBytes(new Date(), 4));
		if (trace & 1) printOrange("Encoding - started encloding - base is:", result, "\n");

		data = [];	// will hold data portion
		var nextBitIndex = 0;
		for (order = 1; order < desc.Fields.length; order++){
			field = desc.Fields[order];
			if (trace & 4) printOrange("Starting on field order ", order, "\tId:", field.Id, "\n");
			if (!repeat1 || order < repeat1StartField){	// not a repeating field
				if (trace & 4) printOrange("Non-repeating Order:", order, "\n");
				if (repeat1 && (order == repeat1CountField)){
					repeatName = field.Id;
					repeatCount = this[repeatName + "Count"];
					if (trace & 4) printOrange("Repeat count:", repeatCount, "\n");
					encodeBits(repeatCount, field.BitOffset, field.BitLength, field.BitStart);
					}
				else {

					if (trace & 4) printOrange("About to encode with this:", this, "\nfield:", field, "\n");
					encodeValue(this, field);
					}
				}
			else {	// we have reached the start of the repeating fields
				if (trace & 4) printOrange("Repeating order:", order, "\n");
				if (repeatCount == "undefined"){
					if (trace & 4) printOrange("NMEA2000 encode repeat count not present in object\n");
					break;	// we stop here
					}
				repeatId = field.Id;
				repeats = [];
				if (trace & 4) printOrange("Field order ", order, " has repeat of ", repeatId, " count ", repeatCount, "\n");
				for (var r = 0; r < repeatCount; r++){	// for each repetition...
					if (trace & 4) printOrange("Starting on repetition ", r, "\n");
					thisRepeat = {};
					for (s = 0; s < repeat1Size; s++){	// for each in set
						field = desc.Fields[order++];	// descriptor for this parameter
						if (trace & 4) printOrange("Processing set ", s, "\tin repetition ", r, "\torder: ", order, "\n");
						Id = field.Id;
						if (trace & 4) printOrange("\t\t\t\tfield.Id ",  Id, "\n");
						if (Id.slice(0, 8) == "reserved"){
							encodeBits(0xffffffff, field.BitOffset, field.BitLength, field.BitStart);
							continue;
							}
						else value = this[repeatName][r][Id];
						if (trace & 4) printOrange("r:", r,"\tId:", Id, "\tvalue:", value, "\n");
						encodeValue(this[repeatName][r], field);
						}
					order = repeat1StartField;	// back to start of descriptors for this repetition
					}
				order = repeat1StartField + repeat1Size;	// finished repeats, so set order to next thereafter
				}
			}
		result = result.concat(data.length, data);
		return result;
		}

	this.push = function(handle){
		if ((handle == void 0) && (this.handle == void 0)){	// need to find the handle
			handles = OCPNgetActiveDriverHandles();
			var n2kHandles = [];
			for (h = 0; h < handles.length; h++){
				attributes = OCPNgetDriverAttributes(handles[h]);
				if (attributes.protocol == "nmea2000") n2kHandles.push(handles[h]);
				}
			switch (n2kHandles.length){
				case 0:	throw("NMEA2000 no nmea2000 handle");
				case 1:	this.handle = n2kHandles[0];
						break;
				default:	printRed("Multiple nmea2000 handles: ", n2kHandles, "\n");
						throw("Handles ambiguous");
				}
			}
		else this.handle = handle;
		// this.handle now contains our cached handle
		var payload = this.encode();
		payload = payload.slice(13);	// drop Actisense header
		if ((this.destination == void 0) || (this.destination == "undefined"))
			destination = 255;
		else destination = this.destination;
		if ((this.priority == void 0) || (this.priority == "undefined"))
			priority = 3;
		else priority = this.destination;
		if (this.trace & 1) print("NMEA2000 about to push with handle:", this.handle, "\tPGN:", this.PGN, "\tdest:", destination, "\tpri:", priority, "\npayload:", payload, "\n");
		outcome = OCPNpushNMEA2000(this.handle, this.PGN, destination, priority, payload);
		return outcome;
		}

	return this;	// end of constructor ---------------------------------------------------

	function buildBare(us){	// create empty attributes
		var trace = us.trace;
		if (trace & 1) printOrange("buildEmpty for pgn ", us.PGN, "\n");
		ud = "undefined";
		us["id"] = desc.Id;
		us["description"] = desc.Description;
		us["timestamp"] = ud;
		us["priority"] = ud;
		us["destination"] = ud;
		us["origin"] = ud;
		for (field = 1; field < desc.Fields.length; field++){
			if (desc.Fields[field].Id.slice(0,8) == "reserved") continue;	// ignore
			if (desc.Fields[field].Id == "spare") continue;	// ignore this too
			if (field == repeat1StartField - 1){	// set up the the repeating field
				repeats = [];	// for the repeating values
				us[repeatName+"Count"] = ud;
				us[repeatName] = [thisRepeat];
				}
			if (!repeat1 || (field < repeat1StartField) || (field > repeat1StartField + repeat1Size)){
				// not a repeating field
				if (field == repeat1CountField) continue;	// ignore - we already have the repeatNname
				Id = desc.Fields[field].Id;
				us[Id] = ud;
				}
			else {	// repeating field - add to repeating Ids
				Id = desc.Fields[field].Id;
				thisRepeat[Id] = ud;
				}
			}
		}

	function parse(us, data){	// parse
		var trace = us.trace;
		if (trace & 1) printOrange("parse for pgn ", us.PGN, "\n");
		us["id"] = desc.Id;
		us["description"] = desc.Description;
		count = data[12];
		if (trace & 4) printOrange("count in data: ", count, " data length is ", data.length-13, "\n");
//		if (count != data.length-13) throw ("NMEA2000 byte count in data does not match actuality");
		us["timestamp"] = 0;	// get this in now to establish position in enumeration
		nextByte = 2;
		us["priority"] = data[nextByte++];
		pgn = getBytes(data, nextByte,3);
		if (pgn != this.pgn) throw("NMEA2000 pgn in data does not match descriptor pgn");
		us["destination"] = data[nextByte++];
		if ( data[0]==/*MsgTypeN2kData*/0x93 ) {
			us["origin"] = data[nextByte++];
			stamp = getBytes(data, nextByte,4);
			stamp = checkNumber(stamp, false, 32);
			if (trace & 4) {
				if (stamp == void 0) printOrange("No header timestamp\n");
				else printOrange("Have header timestamp ", stamp, "\n");
				}
			us["timestamp"] = stamp;
			}
		  else {
			us["origin"] = data[i++]; /*DefaultSource*/;
			us["timestamp"] = new Date();
		  }
		data = data.slice(13);	// now dispense with actisense header and start on NMEA2000 data proper
		nextBitIndex = 0;	// the index within data of the next bit to be processed
		canCheckBitOffset = true;	// BitOffset should tally with nextBitIndex.  Not possible after variable length or first repeating field
		for (order = 1; order < desc.Fields.length; order++){
			field = desc.Fields[order];
			if (trace & 4) printOrange("Starting on field order ", order, "\tId:", field.Id, "\tnextBitIndex:", nextBitIndex, "\n");
			if (!repeat1 || order < repeat1StartField){	// not a repeating field
				if (trace & 4) printOrange("Non-repeating order:", order, "\n");
				if (field.Id.slice(0, 8) == "reserved") {	// we will skip this
					nextBitIndex += field.BitLength;
					continue;
					}
				if (field.Id == "spare") {	// we will skip this also
					nextBitIndex += field.BitLength;
					continue;
					}
				Id = field.Id;
				if (field.BitOffset == void 0) canCheckBitOffset = false;	// no BitOffset so stop checking
				if (canCheckBitOffset) if (field.BitOffset != nextBitIndex)
					throw("NMEA2000 parse BitOffset " + field.BitOffset + " nextBitIndex " + nextBitIndex + " conflict");
				value = decodeValue(us, data, field);
				if (repeat1 && (order == repeat1CountField)){
					repeatCount = value;	// remember repeat count
					repeatName = field.Id;
					if (trace & 4) printOrange("Repeat count:", repeatCount, "\n");
					if (repeatCount == void 0) repeatCount = 0;	// no repeats present
					us[repeatName + "Count"] = repeatCount;
					}
				else us[Id] = maybeUndefine(value, us.undefined);
				if (trace & 4) printBlue("Field order:", order, "\tId:", Id, "\tValue type:", typeof value, "\n");
				if (trace & 4) printBlue("us:", us, "\n");
				}
			else {	// we have reached the start of the repeating fields
				if (trace & 4) printOrange("Repeating order:", order, "\n");
				if (repeatCount == void 0){
					if (trace & 4) printOrange("NMEA2000 parse repeat count not present in data\n");
					break;	// we stop here
					}
				repeatId = field.Id;
				repeats = [];
				us[repeatName] = repeats;
				if (trace & 4) printOrange("Field order ", order, " has repeat of ", repeatId, " count ", repeatCount, "\n");
				for (r = 1; r <= repeatCount; r++){	// for each repetition...
					if (trace & 4) printOrange("Starting on repetition ", r, "\n");
					thisRepeat = {};
					for (s = 0; s < repeat1Size; s++){	// for each in set
						field = desc.Fields[order++];	// descriptor for this parameter
						if (trace & 4) printOrange("Processing set ", s, "\tin repetition ", r, "\torder: ", order, "\n");
						Id = field.Id;
						if (trace & 4) printOrange("\t\t\t\tfield.Id ",  Id, "\n");
						if (Id.slice(0, 8) == "reserved") {	// we will skip this
							nextBitIndex += field.BitLength;
							canCheckBitOffset = false;
							continue;
							}
						if (canCheckBitOffset) if (field.BitOffset != nextBitIndex)
							throw("NMEA2000 parse BitOffset " + field.BitOffset + " nextBitIndex " + nextBitIndex + " conflict");
						startByteIndex = Math.floor(nextBitIndex/8);
						BytesToGet = Math.ceil(field.bitLength/8);
						if (startByteIndex + BytesToGet > data.length) break;	// off end of data
						thisRepeat[Id] =  maybeUndefine(decodeValue(us, data, field), us.undefined);
						if (trace & 32) print("r:", r, "\ts:", s, "\t nextBitIndex:", nextBitIndex,  " mod8:", nextBitIndex%8, "\n");
						}
					repeats.push(thisRepeat);
					order = repeat1StartField;	// back to start of descriptors for this repetition
					canCheckBitOffset = false;
					}
				order = repeat1StartField + repeat1Size;	// finished repeats, so set order to next thereafter
				}
			}
		}

	function decodeValue(us, data, desc) {
		// to understand how values are expressed, see https://canboat.github.io/canboat/canboat.html#ft-NUMBER
		var trace = us.trace;
		if (trace & 4) printBlue("Decode data:", data, "\ndesc:", desc, "\nnextBitIndex:", nextBitIndex, "\n");
		switch (desc.FieldType){
			case "Integer":
throw("Type integer");
				value = getBits(data, nextBitIndex, desc.BitLength, desc.BitStart);
				nextBitIndex += desc.BitLength;
				value = checkNumber(value, desc.Signed, desc.BitLength);
/* should be fixed so allow this
				if (this.PGN != 60928){	// Address claim regularly fails, so omit
					if (value > desc.RangeMax) printRed("Data for " + desc.Id + " is " + value + " exceeds maximum");
					else if (value < desc.RangeMin) printRed("Data for " + desc.Id + " is " + value + " below minimum");
					}
*/
				return value;
			case "NUMBER":
				value = getBits(data, nextBitIndex, desc.BitLength, desc.BitStart);
				nextBitIndex += desc.BitLength;
				value = checkNumber(value, desc.Signed, desc.BitLength);
				if (value == void 0) return value;
				if (trace & 8) printOrange("Number/Lat/Long: ", value, "\n");
				decimalPlaces = -Math.log10(desc.Resolution);
				return parseFloat((value * desc.Resolution).toFixed(decimalPlaces)); // desc.Resolution
			case "MMSI":
				value = getBits(data, nextBitIndex, desc.BitLength, desc.BitStart);
				nextBitIndex += desc.BitLength;
				value = checkNumber(value, desc.Signed, desc.BitLength);
				if (trace & 8) printOrange("MMSI: ", value, "\n");
				return value;
			case "DATE":	// in days since 1970
				value = getBits(data, nextBitIndex, desc.BitLength, desc.BitStart);
				nextBitIndex += desc.BitLength;
				value = checkNumber(value, desc.Signed, desc.BitLength);
				if (value == "undefined") return value;
				moment = new Date(value * 86400 * 1000);	// date to msec since 1 Jan 1970
				// have to work to getdate into same format as canboat yyyy.mm.dd
				date = date = moment.toUTCString().slice(0,10);
				return date.replace(/-/g, '.');	// e.g. 2023.10.22
			case	"TIME":	// in milliseconds since midnight
				value = getBits(data, nextBitIndex, desc.BitLength, desc.BitStart);
				nextBitIndex += desc.BitLength;
				value = checkNumber(value, desc.Signed, desc.BitLength);
				if (value == "undefined") return value;
				value = value*desc.Resolution;
				time = new Date(value*1000);	// need msec
				return time.toUTCString().slice(11,19);	// e.g 17:35:00
			case "LOOKUP":
				value = getBits(data, nextBitIndex, desc.BitLength, desc.BitStart);
				nextBitIndex += desc.BitLength;
				if (trace & 8) printOrange("Looking up '", desc.BitLength, "' for value ", value, "\n");
				return lookupEnumeration(desc.LookupEnumeration, value);
			case "STRING_FIX":
				string = "";
				toDo = desc.BitLength/8;
				while (toDo-- > 0) {
					nextByte = getNextByte(data);
					if ((nextByte == 0x00) || (nextByte == 0xff)) continue;	// ignore
					nextChar = String.fromCharCode(nextByte);
					string += nextChar;
					}
				while (string.slice(-1) == "@") string.length--;	// drop trailing @s
				return string.trim();				
			case "STRING_LAU":
				count = getNextByte(data);
				code = getNextByte(data);
				if (trace & 8) {
					printOrange(data, "\n"); printBlue("nextBitIndex:", 	nextBitIndex);
					printOrange("\tcount:", count, "\tcode:", code, "\n");
					}
				if (code != 1) throw("In " + + desc.Id + " the string data is not ASCII");
				string = "";
				toDo = count - 3;	// count included itself, the code and the extra crc - adjust
				while (toDo-- > 0) {
					nextByte = getNextByte(data);
					nextChar = String.fromCharCode(nextByte);
					string += nextChar;
					}
				nullByte = getNextByte(data);	// the null byte terminator
				if (trace & 8) printOrange("String: ", string, "\tnull byte:", nullByte, "\n");
				if (nullByte != 0) throw("In " + + desc.Description + " the string null terminator was not null");
				if (trace & 8) printOrange("string:", string, "\n");
				return string;
			case "BINARY":
				value = getBits(data, nextBitIndex, desc.BitLength, desc.BitStart);
				nextBitIndex += desc.BitLength;
				if (trace & 8) printOrange("Decoding binary data.  value:", value, "\tlength:", desc.BitLength, "\n");
				return numberToBinary(value, desc.BitLength);
			case "SPARE":
				value = getBits(data, nextBitIndex, desc.BitLength, desc.BitStart);
				nextBitIndex += desc.BitLength;
				if (trace & 8) printOrange("Spare  value:", value, "\tlength:", desc.BitLength, "\n");
				return value;
			default: { printGreen("Unsupported field type:", desc, "\n");throw("Unsupported field type " + desc.FieldType);}
			}
		result = "The result";
		return result;
		}

	function getBits(data, BitOffset, bitLength, bitStart){	// extract bits from data
		if (bitStart == void 0) bitStart = 0;	// for when no bitStart in descriptor
		startByteIndex = Math.floor(BitOffset/8);
		BytesToGet = Math.ceil(bitLength/8);
		if (startByteIndex + BytesToGet > data.length) throw("NMEA2000 getBits ran off end of data");
		if (trace & 16) printOrange("getBits BitOffset:", BitOffset, "\tbitLength:", bitLength, "\tbitStart:", bitStart,
			"\tstartByteIndex:", startByteIndex, "\tBytesToGet:", BytesToGet, "\n");
		chunk = getBytes(data, startByteIndex, BytesToGet);
		// bit ops are good for 32 bits only, so only do this next if needed - will leave 64 bits untouched
		if (bitLength <= 8){	// nibbles are little endon
			nibble = chunk >> bitStart;
			mask = (1 << bitLength) - 1;
			nibble &= mask;
			}
		else if (bitLength <= 31){	// added for issue #108 13 Dec 2024
			nibble = chunk >> bitStart;
			mask = (1 << bitLength) - 1;
			nibble &= mask;
			}
		else nibble = chunk;
		if (trace & 16) printOrange("BitOffset:", BitOffset, "\tbitLength:", bitLength, "\tbitStart:", bitStart, "\tchunk: ", chunk, "\tnibble:", nibble, "\n");
		return nibble;
		}

	function getNextByte(data){	// get next byte from data		
		byteOffset = nextBitIndex/8;
		if (byteOffset > data.length) throw("NMEA2000 getNextByte ran off end of data");
		if (trace & 16) printOrange("nextBitIndex:", nextBitIndex, "\tByteOffset: ", byteOffset, "\n", data, "\n");
		theByte = data[byteOffset];
		nextBitIndex += 8;
		return theByte;
		}
		
	function getBytes(v, start, bytes){	// little endean!
		var trace = this.trace;
		offset = start+bytes-1;
		result = v[offset--];
		if (trace & 16) {
			printOrange("getBytes start:", start, "\tbytes:", bytes, "\toffset:", offset);
			printOrange("\tresult:", result, "\n");
				}
		for ( ; offset >= start; offset--){
			result = (result * 256) + v[offset];	// shift left 64 bit safe
			if (trace & 16) printOrange("getBytes loop offset:", offset, "\tresult:", result, "\n");
			}
		nextByte = start + bytes;
		if (trace & 16) printOrange("getBytes final result:", result, "\n");
		return result;
		};

	function encodeBytes(what, nbytes){// encode into nbytes little endean
		var trace = this.trace;
		if (trace & 32) printOrange("encodeBytes what:", what, " nbytes:", nbytes, "\n");
		var result = [];
		while (nbytes--){
			result.push(what & 0xff);
			what /= 256;
			}
		return result;
		}

	function encodeBits(what, BitOffset, bitLength, bitStart){	// put bits into encoded array
		var trace = this.trace;
		if (bitStart == void 0) bitStart = 0;	// for when no bitStart in descriptor
		if (BitOffset == void 0) BitOffset = 0;	// for when no bitOffset in descriptor
		startByteIndex = Math.floor(BitOffset/8);
		BytesToPut = Math.ceil(bitLength/8);
		if (trace & 32) printOrange("encodeBits what:", what, " BitOffset:", BitOffset, "\tbitLength:", bitLength, "\tbitStart:", bitStart,
			"\tstartByteIndex:", startByteIndex, "\tBytesToPut:", BytesToPut, "\n");
		if ((BitOffset%8 == 0) && (bitStart == 0) && (bitLength%8 == 0)){	// whole bytes - this is easy
			data = data.concat(encodeBytes(what, bitLength/8));
			return;
			}
		if (bitLength <= 8){	// nibbles are little endon
			mask = (1 << bitLength) - 1;	// just to be safe
			what &= mask;
			if (bitStart == 0){	// this is first nibble
				data.push(what);
				return;
				}
			else {
				chunk = data.pop();	// recover partially formed byte
				nibble = what << bitStart;
				chunk |= nibble;
				data.push(chunk);
				return;
				}
			}
		// greater than 8 bits
		if (bitStart == 0) {
			encodeBytes(what, bitLength/8);
			return;
			}
		if (bitLength + bitStart == 8){	// completing byte
			chunk = data.pop();	// recover partially formed byte
			nibble = what << bitStart;
			chunk |= nibble;
			data.push(chunk);
			return;
			}
		if (what = 0xffff){ // just padding
			chunk = data.pop();	// recover partially formed byte
			nibble = what << bitStart;
			nibble &= 0xff;
			chunk |= nibble;
			data.push(chunk);
			bitLength -= bitLength%8;	// reduce to bytes remaining
			encodeBytes(what, bitLength/8);
			return;
			}
		throw("encodebits unsupported bitLength of " + bitLength + " when not padding");
		}

	function encodeValue(us, field){
		var trace = us.trace;
		Id = field.Id;
		if (trace & 32) printOrange("encodeValue us:", us, "\tfield:", field, "\Id:", field.Id, "\n");
		if ((Id.slice(0,8) == "reserved")|| (Id == "spare")){
			encodeBits(0xffff, field.BitOffset, field.BitLength, field.BitStart);
			return;
			}
		else value = us[Id];
		if (trace & 32) printOrange("encodeValue value: ", value, "\tType:", field.FieldType, "\n");
		if ((value == void 0) || (value == "undefined")){	// no value - undefined
			if (field.BitLength <= 3) value = 0xfe;	// special case
			else {	// need to enode undefined 
				toShift = field.BitLength;
				if (field.Signed) toShift--;	// signed numbers have one less max value
				value = checkUndefined(value, toShift);
				}
			encodeBits(value, field.BitOffset, field.BitLength, field.BitStart);
			return;
			}
 		switch (field.FieldType){
			case "Integer":
				value = value/field.Resolution.toFixed(0);
				encodeBits(value, field.BitOffset, field.BitLength, field.BitStart);
				return;
			case "LOOKUP":
				if (typeof value == "object"){
					if (trace & 32) printOrange("field.Type is object with value:", value.value, "\n");
					encodeBits(value.value, field.BitOffset, field.BitLength, field.BitStart);
					return;
					}
				else if (typeof value == "number"){
					if (trace & 32) printOrange("field.Type is number\n");
					encodeBits(value, field.BitOffset, field.BitLength, field.BitStart);
					return;
					}
/*
				if (((typeof field.EnumValues) != "undefined") &&
					((typeof value) == "string")){ // only if lookup table included in desc and it is a string
					if (trace & 32) printOrange("field.EnumValues.length: " + field.EnumValues.length, "\n");
					for (i = 0; i < field.EnumValues.length; i++){
						if (trace & 32) printOrange("Table name: ", field.EnumValues[i].name, "\n");
						if (value == field.EnumValues[i].name){
							encodeBits(field.EnumValues[i].value, field.BitOffset, field.BitLength, field.BitStart);
							return;
							}
						}
					throw("encode unknown lookup table name for field " + Id);
					}
*/
				else throw("LOOKUP unknown type:", typeof value);
			case "MMSI":
				encodeBits(value, field.BitOffset, field.BitLength, field.BitStart);
				return;
//			case "Date":
			case "DATE":
				value = value.replace(/\./g, '-');	// change . to - separator
				date = new Date(value);
				day = date.getTime()/(86400 * 1000);	// day number
				encodeBits(day, field.BitOffset, field.BitLength, field.BitStart);
				return;
			case "TIME":
				time = new Date("1970-01-01T" + value + "Z");	// msec since start epoch
				time = time.getTime()/1000;	// seconds since midnight
				time = time/field.Resolution;	// sat time
				encodeBits(time, field.BitOffset, field.BitLength, field.BitStart);
				return;
			case "NUMBER":
				if (trace & 16) printOrange("Encoding number:", value, "\n");
				value = Math.round(value/field.Resolution);	// convert to integer
				if (value < 0){
					if (!field.Signed) throw("encode Value ", value, " but unsigned type");
					bits = field.BitLength;
					value = (value*-1 - 2**bits) * -1;	// make positive;
					}
				encodeBits(value, field.BitOffset, field.BitLength, field.BitStart);
				return;
			case "STRING_LAU": //ASCII or UNICODE string starting with length and control byte
				data.push(value.length+3, 1);	// count (includes itself, code and terminating zero) + ASCII code
				for (var c = 0; c < value.length; c++) data.push(value.charCodeAt(c));
				data.push(0);
				return;
			case "STRING_FIX":	// fixedlength text
				var c = field.BitLength/8;	// number of chars allowed
				var s = value.length;		// number of chars provided
				if (trace & 16) printOrange("Encode ASCII c: ", c, "\ts:", s, "\n");
				for (var i = 0; ((c > 0) && (s > 0));  i++, c--, s--)
					data.push(value.charCodeAt(i));
				while (c-- > 0) data.push(0x00);	// pad if needed
				return;
			case "BINARY":
				value = binaryToNumber(value);
				encodeBits(value.toString(), field.BitOffset, field.BitLength, field.BitStart);
				return;
			default: throw("field " + field + "\tencode Value unsupported type: " + field.Type);
			}
		return value;
		}

	function checkNumber(number, isSigned, bits){ // check decoded number is valid and fix up sign
		// See here for special meaning of some numbers https://canboat.github.io/canboat/canboat.html#field-types
		// in JavaScript, bit ops are only available up to 32 bits
		// so to be 64 bit safe, we have to manage without bit shift and bit flip (:-#)
		var trace = this.trace;
		toShift = bits;
		if (isSigned) toShift--;	// signed numbers have one less max value
		max = (2 ** toShift) - 1;	// maximum possible value
		if (number == max) return void 0;
		else if ((bits >= 4) && (number == max-1)) number = "has error";
		else if (isSigned && (number > max)){ // must be negative
			number = (2**bits - number) * -1;
			}
		if (trace & 16) printOrange("checkNumber - number:", number, "\tbits:", bits, "\tmax:", max,"\n");
		return number;
		}

	function checkUndefined(value, bits){
		if ((value == void 0) || (value == "undefined")) return (2 ** bits) - 1;	// required number of bits all on
		else return value;
		}

	function maybeUndefine(value, ifUndefined){
		// if to display undefined, substitute text for void 0
		if (ifUndefined && value == void 0) return "undefined";
		else return value;
		}

	function lookupEnumeration(name, value){
		if (_canboat == undefined){ // load the canboat object
			_canboat = JSON.parse(require("canboat"));
			}		
		enums = _canboat.LookupEnumerations;
		for (var i = 0; i < enums.length; i++){
			e = enums[i];
			if (e.Name == name){
				evalues = e.EnumValues;
				for (var j = 0; j < evalues.length; j++){
					if (evalues[j].Value == value) return {value:value, name:evalues[j].Name};
					}
				return{value:value, name:"no name entry for this value"};
				}
			}
		throw("Lookup of '" + name + "' - not known");
		}

	function numberToBinary(data, bitLength){	// convert to binary string
		if (bitLength > 32) return "numberToBinary for > 32 bits not supported";
		result = "";
		for (var i = bitLength; i > 0; i--){
			mask = 1 << i-1;
			onOff = data & mask;
			result += onOff?"1":"0";
			}
		return result;
		}

	function binaryToNumber(binary){	// convert binary string to number
		if (binary.length > 32) return "binaryToNumber for > 32 bits not supported";
		var result = 0;
		value = 1;
		for (var i = binary.length-1; i >= 0; i--){
			if (binary[i] == "1") result += value;
			value *= 2; // shifts left 1 but no effect first time as resukt is 0

			}
		return result;
		}

	}




