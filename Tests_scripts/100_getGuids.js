const ALL = -1;	// -> OBJECTS_ALL
const NORMAL = 0;	// -> OBJECTS_NO_LAYERS
const LAYER = 1	// -> OBJECTS_ONLY_LAYERS

errors = 0;

type = "Waypoint";
allGuidsCount	= OCPNgetWaypointGUIDs(ALL).length;
normalGuidsCount = OCPNgetWaypointGUIDs(NORMAL).length;
layerGuidsCount = OCPNgetWaypointGUIDs(LAYER).length;
report(type);

type = "Route";
allGuidsCount	= OCPNgetRouteGUIDs(ALL).length;
normalGuidsCount = OCPNgetRouteGUIDs(NORMAL).length;
layerGuidsCount = OCPNgetRouteGUIDs(LAYER).length;
report(type);

type = "Track";
allGuidsCount	= OCPNgetTrackGUIDs(ALL).length;
normalGuidsCount = OCPNgetTrackGUIDs(NORMAL).length;
layerGuidsCount = OCPNgetTrackGUIDs(LAYER).length;
report(type);

scriptResult((errors > 0)? (errors + " errors"): "No errors");

function report(type){
	printUnderlined(type, " counts\n");
	print("All: ", allGuidsCount, "\tNormal: ", normalGuidsCount,
		"\tLayer: ", layerGuidsCount, "\n");
	sumCheckOk = (normalGuidsCount+layerGuidsCount) == allGuidsCount;
	print("Sum check is ", sumCheckOk, "\n\n");
	if (!sumCheckOk) errors++;
	}