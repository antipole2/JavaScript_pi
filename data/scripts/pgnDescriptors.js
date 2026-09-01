 function getDescriptor(pgn, id){
	/*
	called as
	getDescriptor()			returns array of all descriptors
	getDescriptor(pgn)		returns first descriptor with matching pgn - error if no match
	getDescriptor(pgn, id)	returns first descriptor matching on pgn and id - error if no match
	getDescriptor(pgn, "")	returns array of descriptors matching on pgn - empty array if none
	
	Descriptors copied from here: https://github.com/canboat/canboat/blob/master/docs/canboat.json

	To update, copy the json file.
	Edit out PGN 129541
	Then run this script to check it compiles without errors by copying into a console and running.
	For extra confidence, run the N2k decode/encode tests in the test scripts.
	*/

	unsupported = [
//	{pgn:130824, reason:"multiple descriptions leave ambiguity"},
	{pgn:129541, reason:"This does not work on Windows - load as custom descriptor - see https://github.com/antipole2/JavaScripts-shared/blob/main/descriptor129541/descriptor129541.adoc"},
	];

	// load the canboat object if not already loaded
	if (_canboat == undefined)_canboat = JSON.parse(require("canboat"));
		
	descriptors = _canboat.PGNs;

	if (arguments.length == 0) return descriptors;

	for (var u in unsupported){	//check for unsupported pgn
		if (pgn == unsupported[u].pgn) throw("pgn " + pgn + " unsupported - " + unsupported[u].reason);
		}
	
	if (arguments.length == 1){
		for (var i = 0; i < descriptors.length; i++){
			if (descriptors[i].PGN == pgn){
				if (descriptors[i].RepeatingFieldSet2StartField != void 0)
					throw("Descriptor for pgn " + pgn + " has more than one repeating field - not supported");
				return(descriptors[i]);
				}
			}
		throw("No matching descriptor for pgn " + pgn);
		}
	
	if ((arguments.length == 2) && (id.length > 0)){
		for (var i = 0; i < descriptors.length; i++){
			if ((descriptors[i].PGN == pgn) && (descriptors[i].Id == id)){
				if (descriptors[i].RepeatingFieldSet2StartField != void 0)
					throw("Descriptor for pgn " + pgn + " and Id " + id + " has more than one repeating field - not supported");
				return(descriptors[i]);
				}
			}
		throw("No matching descriptor for pgn " + pgn + " and Id " + id);
		}
		
	if ((arguments.length == 2) && (id == "")){	// all matching
		var matches = [];
		for (var i = 0; i < descriptors.length; i++){
			if (descriptors[i].PGN == pgn){
				if (descriptors[i].RepeatingFieldSet2StartField != void 0)
					throw("Descriptor for pgn " + pgn + " and Id " + id + " has more than one repeating field - not supported");
				matches.push(descriptors[i]);
				}
			}
		return matches;
		}
	}