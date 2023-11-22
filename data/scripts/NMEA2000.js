// decode NMEA2000 payload
// tracing options as per value of tracechoice  NMEA2000(arg, {trace: tracechoice}}
//	1	    1	call parameters and high level flow
//	2	   10	print descriptor
//	4	 0100	parse
//	8	 1000	decode
//	16	10000	byte/bit decoding

function NMEA2000(arg, data, options){
	useBuiltIns = false;	// if using modules built into plugin, else modules in plugin current directory
	trace= false;
	// do not include the descriptor when enumerating the NMEA2000 object
	Object.defineProperty(this, "descriptor", {enumerable: false, writable: true});

	// examine the arguments
	this.descriptor = false;	// no descriptor yet
	if (typeof(options) == "object"){
		if (options.hasOwnProperty("trace")) trace = options.trace;
		}		
	if (trace & 1) printOrange("NMEA2000 called with arg type ", typeof(arg), "\n");
	switch (typeof(arg)){
		case "number":
			if (trace & 1) printOrange("arg is number ", arg, "\n");
			this.descriptor = require(includeString("pgnDescriptors"))(arg);	// load descriptor
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
	if (trace & 2) printOrange("\n", JSON.stringify(this.descriptor, null, "\t"), "\n\n");
	desc = this.descriptor;
	this.pgn = desc.PGN;
	desc.Fields.unshift(0);	// shift Fields up so we can index from 1 rather than zero

	// register the repeating fields, if any
	repeat1Size = desc.RepeatingFieldSet1Size;
	repeat1StartField = desc.RepeatingFieldSet1StartField;
	repeat1CountField = desc.RepeatingFieldSet1CountField;
	if (typeof(desc.RepeatingFieldSet2Size) != "undefined"){
		printRed("Descriptor ", desc.Id, " has second repeating field group", "\n");
		throw("Unsupported descriptor");
		}
	if ((repeat1Size != void 0) && (repeat1StartField != void 0) && (repeat1CountField != void 0)){
		repeat1 = true;
		repeatName = desc.Fields[repeat1CountField].Id;	// name of repetition
		if (trace & 1) printOrange("Repeating fields - repeat1Size:", repeat1Size, "\trepeat1StartField:", repeat1StartField, "\trepeat1CountField:", repeat1CountField, "\trepeatName:", repeatName, "\n");
		thisRepeat = {};
		}
	else repeat1 = false;

	if ((typeof(data) == "undefined") || (data == null)) buildBare(this);
	else parse(this, data);

	this.decode = function(data){
		desc = this.descriptor;
		repeat1Size = desc.RepeatingFieldSet1Size;
		repeat1StartField = desc.RepeatingFieldSet1StartField;
		repeat1CountField = desc.RepeatingFieldSet1CountField;
		parse(this, data);
		return this;
		}

	return this;	// end of constructor

	function buildBare(us){	// create empty attributes
		if (trace & 1) printOrange("buildEmpty for pgn ", us.pgn, "\n");
		ud = "undefined";
		us["id"] = desc.Id;
		us["description"] = desc.Description;
		us["timestamp"] = ud;
		us["prio"] = ud;
		us["dst"] = ud;
		us["src"] = ud;
		for (field = 1; field < desc.Fields.length; field++){
			if (desc.Fields[field].Id.slice(0,8) == "reserved") continue;	// ignore
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
		if (trace & 1) printOrange("parse for pgn ", us.pgn, "\n");
		us["id"] = desc.Id;
		us["description"] = desc.Description;
		count = data[12];
		if (trace & 4) printOrange("count in data: ", count, "\n");
		if (count != data.length-13) throw ("NMEA2000 byte count in data does not match actuality");
		us["timestamp"] = 0;	// get this in now to establish position in enumeration
		nextByte = 2;
		us["prio"] = data[nextByte++];
		pgn = getBytes(data, nextByte,3);
		if (pgn != this.pgn) throw("NMEA2000 pgn in data does not match descriptor pgn");
		us["dst"] = data[nextByte++];
		if ( data[0]==/*MsgTypeN2kData*/0x93 ) {
			us["src"] = data[nextByte++];
			stamp = getBytes(data, nextByte,4);
			stamp = checkNumber(stamp, false, 32);
			if (trace & 4) printOrange("Have header timestamp ", stamp, "\n");
			us["timestamp"] = stamp;
			}
		  else {
			us["src"] = data[i++]; /*DefaultSource*/;
			us["timestamp"] = new Date();
		  }

		data = data.slice(13);	// now dispense with actisense header

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
				Id = field.Id;
				if (field.BitOffset == void 0) canCheckBitOffset = false;	// no BitOffset so stop checking
				if (canCheckBitOffset) if (field.BitOffset != nextBitIndex)
					throw("NMEA2000 parse BitOffset " + field.BitOffset + " nextBitIndex " + nextBitIndex + " conflict");
				value = decode(data, field);
				if (repeat1 && (order == repeat1CountField)){
					repeatCount = value;	// remember repeat count
					repeatName = field.Id;
					if (trace & 4) printOrange("Repeat count:", repeatCount, "\n");
					us[repeatName + "Count"] = repeatCount;
					}
				else us[Id] = value;
				}
			else {	// we have reached the start of the repeating fields
				if (trace & 4) printOrange("Repeating order:", order, "\n");
				if (repeatCount == "undefined"){
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
						thisRepeat[Id] = decode(data, field);
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

	function decode(data, desc) {
		// to understand how values are expressed, see https://canboat.github.io/canboat/canboat.html#ft-NUMBER
		switch (desc.Type){
			case "Integer":
				value = getBits(data, nextBitIndex, desc.BitLength, desc.BitStart);
				nextBitIndex += desc.BitLength;
				value = checkNumber(value, desc.Signed, desc.BitLength);
				if (value > desc.RangeMax) printRed("Data for " + desc.Id + " is " + value + " exceeds maximum");
				else if (value < desc.RangeMin) printRed("Data for " + desc.Id + " is " + value + " below minimum");
				return value;
			case "Number":
			case "Latitude":
			case "Longitude":
			case "Temperature":
				// see https://canboat.github.io/canboat/canboat.html#ft-NUMBER
				value = getBits(data, nextBitIndex, desc.BitLength, desc.BitStart);
				nextBitIndex += desc.BitLength;
				value = checkNumber(value, desc.Signed, desc.BitLength);
				if (trace & 8) printOrange("Number/Lat/Long: ", value, "\n");
				return (value * desc.Resolution).toFixed(6); // desc.Resolution
			case "Date":	// in days since 1970
				value = getBits(data, nextBitIndex, desc.BitLength, desc.BitStart);
				nextBitIndex += desc.BitLength;
				value = checkNumber(value, desc.Signed, desc.BitLength);
				moment = new Date(value * 86400 * 1000);	// date to msec since 1 Jan 1970
				// have to work to getdate into same format as canboat yyyy.mm.dd
				date = date = moment.toUTCString().slice(0,10);
				return date.replace(/-/g, '.');	// e.g. 2023.10.22
			case	"Time":	// in milliseconds since midnight
				value = getBits(data, nextBitIndex, desc.BitLength, desc.BitStart);
				nextBitIndex += desc.BitLength;
				value = checkNumber(value, desc.Signed, desc.BitLength);
				value = value*desc.Resolution;
				time = new Date(value*1000);	// need msec
				return time.toUTCString().slice(11,19);	// e.g 17:35:00
			case "Lookup table":
				value = getBits(data, nextBitIndex, desc.BitLength, desc.BitStart);
				nextBitIndex += desc.BitLength;
				value = checkNumber(value, desc.Signed, desc.BitLength);
				if (trace & 8) printOrange(desc.Id, " Lookup table value ", value, "\n");
				if (typeof desc.EnumValues == "undefined") return "undefined";	// if no lookup values provided
				max = (1 << desc.BitLength) -1;	// maximumm possible value
				if (value == max) us[desc.Id]= "not available";
				else if ((desc.BitLength >= 4) && (value == max-1)) return "has error";
				else {
					name = false
					for (e = 0; e < desc.EnumValues.length; e++){
						if (desc.EnumValues[e].value == value) {
							name = desc.EnumValues[e].name;
							return name;
							}
						}
					if (name) return name;
					else return "invalid";
					}
			case "ASCII or UNICODE string starting with length and control byte":
				count = getNextByte(data);
				code = getNextByte(data);
				if (trace & 8) {
					printBlue(data, "\n"); printBlue("nextBitIndex:", 	nextBitIndex);
					printBlue("\tcount:", count, "\tcode:", code, "\n");
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
			case "Manufacturer code":
				value = getBits(data, nextBitIndex, desc.BitLength, desc.BitStart);
				nextBitIndex += desc.BitLength;
				value = checkNumber(value, desc.Signed, desc.BitLength);
				return value;
			default: throw("Unsupported field type " + desc.Type);
			}
			result = "The result";
			return result;
		}

	function getBits(data, BitOffset, bitLength, bitStart){	// extract bits from Data
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
			mask = (1 <<bitLength) - 1;
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
		if (trace & 16) printOrange("checkNumber - number:", number, "\tbits:", bits, "\tmax:", max,"\n");
		return number;
		}

	function includeString(module){
		if (useBuiltIns) return module;
		return module + ".js";
		}

	}



