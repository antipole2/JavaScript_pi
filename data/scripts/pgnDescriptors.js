 function getDescriptor(pgn, optionsArg){
	/*
	Descriptors copied from here: https://github.com/canboat/canboat/blob/master/docs/canboat.json

	To update, copy the json file.
	Edit out PGN 129541
	Then run this script to check it compiles without errors by copying into a console and running.
	For extra confidence, run the N2k decode/encode tests in the test scripts.
	*/

	unsupported = [
	{pgn:130824, reason:"multiple descriptions leave ambiguity"},
	{pgn:129541, reason:"This does not work on Windows - load as custom descriptor - see https://github.com/antipole2/JavaScripts-shared/blob/main/descriptor129541/descriptor129541.adoc"},
	];

	// load the canboat object if not already loaded
	if (_canboat == undefined)_canboat = JSON.parse(require("canboat"));
		
	descriptors = _canboat.PGNs;

	returnAll = false
	if (arguments.length < 1) return descriptors;
	if (arguments.length == 2){
		if (typeof(optionsArg) == "object"){
			if (optionsArg.hasOwnProperty("options")){
				if (optionsArg.options == "returnAll") returnAll = true;
				else throw("getDescriptor - invalid options argument");
				}
			else throw("getDescriptor - invalid options argument");
			}
		else throw("getDescriptor - 2nd argument not options");
		}
	if (!returnAll){
		for (var i = 0; i < unsupported.length; i++){
			if (unsupported[i].pgn == pgn) throw("pgn " + pgn + " unsupported - " + unsupported[i].reason);
			}
		}
	if (returnAll){
		toReturn = [];
		for (i = 0; i < descriptors.length; i++){
			if (descriptors[i].PGN == pgn) toReturn.push(descriptors[i]);
			}
		return toReturn;
		}
	else {
		for (var i = 0; i < descriptors.length; i++){
			if (descriptors[i].PGN == pgn){
				if (descriptors[i].RepeatingFieldSet2StartField != void 0)
					throw("Descriptor for pgn " + pgn + " has more than one repeating field - not supported");
				return(descriptors[i]);
				}
			}
		throw("getDescriptor - pgn " + pgn + " not found");
		}
	}