verbose = false;

Route = require("Route");

function compareWaypoints(waypoint1, waypoint2, label, checkGUID){
	// compares two waypoints and displays any mismatches.
	// returns false if match else true if mismatch
	// if checkGUID is true, includes GUID in checks
	function displayW(attribute){
		if (!displaying) printOrange("The following attributes of waypoints ", label, " do not match:\n");
		displaying = true;
		printOrange(attribute, "\n");
		}
	if (verbose){
		print(label, " 1st:\t", waypoint1, "\n");
		print(label, " 2nd:\t", waypoint2, "\n");
		}
	displaying = false;
	if (checkGUID){	// we will check both GUID and creationDateTime
		if (waypoint1.GUID != waypoint2.GUID)
			displayW ("GUID\t" + waypoint1.GUID + "\t" + waypoint2.GUID);
		if ((waypoint2.creationDateTime - waypoint1.creationDateTime) > 2)	// allow a bit of lag
			displayW ("creationDateTime\t" + waypoint1.creationDateTime + "\t" + waypoint2.creationDateTime);
		}
	if (waypoint1.position.latitude != waypoint2.position.latitude)
		displayW ("latitude\t" + waypoint1.position.latitude + "\t" + waypoint2.position.latitude);
	if (waypoint1.position.longitude != waypoint2.position.longitude)
		displayW ("longitude\t" + waypoint1.position.longitude + "\t" + waypoint2.position.longitude);
	if (waypoint1.markName != waypoint2.markName)
		displayW ("markName\t" + waypoint1.markName + "\t" + waypoint2.markName);
	if (waypoint1.description != waypoint2.description)
		displayW ("description\t" + waypoint1.description + "\t" + waypoint2.description);
	if (waypoint1.iconName != waypoint2.iconName)
		displayW ("iconName\t" + waypoint1.iconName + "\t" + waypoint2.iconName);
	if (waypoint1.isVisible != waypoint2.isVisible)
		displayW ("isVisible\t" + waypoint1.isVisible + "\t" + waypoint2.isVisible);
	if (waypoint1.nRanges != waypoint2.nRanges)
		displayW ("nRanges\t" + waypoint1.nRanges + "\t" + waypoint2.nRanges);
	if (waypoint1.RangeRingSpace != waypoint2.RangeRingSpace)
		displayW ("rangeRingSpace\t" + waypoint1.rangeRingSpace + "\t" + waypoint2.rangeRingSpace);
	if (waypoint1.RangeRingColour != waypoint2.RangeRingColour)
		displayW ("rangeRingColour\t" + waypoint1.rangeRingColour + "\t" + waypoint2.rangeRingColour);
	if (waypoint1.minScale != waypoint2.minScale)
		displayW ("minScale\t" + waypoint1.minScale + "\t" + waypoint2.minScale);
	if (waypoint1.hyperlinkList.length == waypoint2.hyperlinkList.length){
		numberOfLinks = waypoint1.hyperlinkList.length;
		if (numberOfLinks > 0){
			for (i = 0; i < numberOfLinks; i++){
				if (waypoint1.hyperlinkList[i].description != waypoint2.hyperlinkList[i].description) displayW ("hyperlink[" + i + "].description differs");
				if (waypoint1.hyperlinkList[i].link != waypoint2.hyperlinkList[i].link) displayW ("hyperlink[" + i + "].link differs");
				// if (waypoint1.hyperlinkList[i].type != waypoint2.hyperlinkList[i].type) display ("hyperlink[" + i + "].type differs");
				}
			}
		}
	else displayW("Number of hyperlinks differs");
	return displaying;
	}
	
function compareRoutes(route1, route2, label, checkRouteGUID, doWaypoints, checkWaypointGUIDs){
	// compares two routes.  If doWaypoints true, then also compares their waypoints. D any mismatches.
	// returns true if mismatch else false if OK
	function displayR(attribute){
		if (!displaying) {
			printOrange("The following attributes of routes ", route1.name + "&" + route2.name, " do not match:\n");
			displaying = true;
			print(attribute, "\n");
			}
		}
	displaying = false;

	if (route1.waypoints.length != route2.waypoints.length) displayR("Number of points in routes differ\n");
	if (checkRouteGUID){
		if (route1.GUID != route2.GUID) displayR ("GUID\t" + route1.GUID + "\t" + route2.GUID);
		}
	if (route1.name != route2.name) displayR ("name\t" + route1.name + "\t" + route2.name);
	if (route1.from != route2.from) displayR ("from\t" + route1.from + "\t" + route2.from);
	if (route1.to != route2.to) displayR ("to\t" + route1.to + "\t" + route2.to);
	if (route1.waypoints.length != route2.waypoints.length) return ("Number of waypoints does not match");
	if (doWaypoints){
		for (k = 0; k < route1.waypoints.length; k++){
			mismatch = compareWaypoints(route1.waypoints[k], route2.waypoints[k], k, checkWaypointGUIDs=false);
			if (mismatch) displayR("Waypoints " + k + " in routes do not match ");
			}
		}
	return displaying;
	}
	

function report(message){
	printOrange(message, "\n");
	OK = false;
	}

function timeStampOK(waypoint){	// checks creationDateTime is within 1 second of start - returns true if OK
	diff = waypoint.creationDateTime - startTime;
	if ((diff > 2) || (diff < -1)) return false;
	else return true;
	}

startTime = new Date()/1000;
errorCount = 0;
var caught;

testWaypoint = {"position":{"latitude":50.5,"longitude":-2.2},"markName":"Test waypoint","description":"This waypoint is a temporary one for diagnostics",
"isVisible":true,"iconName":"anchor",
"minScale": 98765, "nRanges":6, "rangeRingSpace":6.543,"rangeRingColour":"#FFEE11",/* "creationDataTime": startTime, */
"hyperlinkList":[{"description":"OpenCPN","link":"https://opencpn.org","type":"Type1"},{"description":"JavaScript user guide","link":"https://github.com/antipole2/JavaScript_pi/blob/master/JavaScript%20plugin%20user%20guide.pdf","type":"Type2"}]};

printUnderlined("Add a waypoint and read it back\n");
wpGUID = OCPNaddSingleWaypoint(testWaypoint);
readBack = OCPNgetSingleWaypoint(wpGUID);
mismatch = compareWaypoints(testWaypoint, readBack, "A", compareGUID = false);
if (mismatch) {printRed("Readback does not match\n"); errorCount++;}
else printGreen("Readback matches\n");
if (!timeStampOK(readBack)) {printRed("Readback timestamp does not look right - ", readBack.creationDateTime, "\n"); errorCount++;}
print("Readback shows waypoint as ",  readBack.isFreeStanding?"Freestanding\n":"Not freestanding\n");
print("Readback shows ", readBack.routeCount, " route memberships\n");

printUnderlined("\nUpdate waypoint and read it back\n");
testWaypoint = {};
testWaypoint.GUID = wpGUID;
testWaypoint.position = {latitude:readBack.position.latitude + 0.1, longitude:readBack.position.longitude + 0.2};
testWaypoint.markName = "Testmark modified";
testWaypoint.description = "Updated description";
testWaypoint.isVisible = !testWaypoint.isVisible;
testWaypoint.iconName = "diamond";
testWaypoint.minScale = 123456;
testWaypoint.useMinScale = !testWaypoint.useMinScale
testWaypoint.nRanges = 2;
testWaypoint.rangeRingSpace = 0.2;
testWaypoint.rangeRingColour = "#ffDd22";
testWaypoint.hyperlinkList = [{"description":"OpenCPNupdated","link":"https://opencpn.org","type":"Type1"},
	{"description":"JavaScript user guide updated","link":"https://opencpn-manuals.github.io/plugins/javascript/0.1/index.html", "type":"Type2"}];

try {OCPNupdateSingleWaypoint(testWaypoint);}
catch (error) {
	printRed(error.message, " - continuing anyway\n");
	}
readBackUpdated = OCPNgetSingleWaypoint(wpGUID);
mismatch = compareWaypoints(testWaypoint, readBackUpdated, "B", compareGUID=false);
if (mismatch) {printRed("Readback of waypoint does not match\n"); errorCount++;}
else printGreen("Readback of waypoint matches\n");

// just check our waypoint still exists
caught = false;
try {refreshed = OCPNgetSingleWaypoint(wpGUID);}	// our original waypoint
catch(error){caught = true;}
if (caught) { throw("Waypoint has been lost - logic error A\n")}

printUnderlined("\nAdd Route A and read back\n");
testRoute = new Route();
testRoute.name = "Test Route A";
testRoute.from = "Start place";
testRoute.to = "End place";
for (i = 0; i < 4; i++) { // construct some waypoints
	waypoint = new Waypoint();
	name = "TestA WP" + i;
	waypoint.markName = name;
	waypoint.description = "Description of " + name;
	waypoint.position.latitude = 50 + i/10;
	waypoint.position.longitude = -2.5 - i/10;
	waypoint.iconName = "";
	waypoint.isVisible = true;
	waypoint.nRanges = i;
	waypoint.minScale = 10010+i;
	waypoint.hyperlinkList = [];
	testRoute.waypoints.push(waypoint);
	}
testRoute.waypoints.push(testWaypoint);	// add a free-standing waypoint
indexOfTestWaypoint = testRoute.waypoints.length-1;	// remember for later
try {routeGUID = OCPNaddRoute(testRoute);}
catch(error) {throw("Failed to add route A");}
try {readbackRoute = OCPNgetRoute(routeGUID);}
catch(error) {throw("Failed to get route A");}
if (verbose) print(readbackRoute, "\n");
mismatch = compareRoutes(testRoute, readbackRoute, "", checkGUID=false, doWaypoints=true);
if (mismatch) {printRed("Readback of Route A does not match\n"); errorCount++;}
else printGreen("Readback of Route A matches\n");
// make spot checks the other bits only available in readback
if (!timeStampOK(readbackRoute.waypoints[1])) {printRed("Created routepoint has bad createDateTime\n"); errorCount++;}
if (readbackRoute.waypoints[1].isFreeStanding == true) {printRed("Created routepoint marked as freestanding\n");; errorCount++;}
if (readbackRoute.waypoints[indexOfTestWaypoint].isFreeStanding == false) {printRed("Included waypoint not marked as freestanding\n");; errorCount++;}
if (readbackRoute.waypoints[1].routeCount != 1) {printRed("Created routepoint has wrong routeCount of ", readbackRoute.waypoints[1].routeCount, "\n");; errorCount++;}
if (!timeStampOK(readbackRoute.waypoints[indexOfTestWaypoint])) {printRed("Included waypoint has bad createDateTime\n");; errorCount++;}
if (readbackRoute.waypoints[indexOfTestWaypoint].routeCount != 1) {printRed("Included waypoint has wrong routeMembershipCount of ", readbackRoute.waypoints[indexOfTestWaypoint].routeCount, "\n");; errorCount++;}

printUnderlined("\nAdd Route B with shared routepoint and waypoint and read back\n");
sharedRoutepoint = readbackRoute.waypoints[2];	// Use this as shared routepoint
extraRoute = new Route();
extraRoute.name = "Test Route B";
extraRoute.from = "Start place";
extraRoute.to = "End place";
for (i = 0; i < 2; i++) { // construct some waypoints
	waypoint = new Waypoint();
	name = "TestB WP" + i;
	waypoint.markName = name;
	waypoint.description = "Description of " + name;
	waypoint.position.latitude = 50.2 + i/10;
	waypoint.position.longitude = -2.55 - i/10;
	waypoint.iconName = "";
	waypoint.isVisible = true;
	waypoint.nRanges = i;
	waypoint.minScale = 10010+i;
	extraRoute.waypoints.push(waypoint);
	}
extraRoute.waypoints.push(sharedRoutepoint);	// add shared routepoint
extraRoute.waypoints.push(testWaypoint);	// add the waypoint
if (verbose) print("\nextraRoute\n", extraRoute, "\n\n");
extraRouteGUID = OCPNaddRoute(extraRoute);
readbackExtra = OCPNgetRoute(extraRouteGUID);
if (compareRoutes(extraRoute, readbackExtra, "", checkRouteGUID= false, doWaypoints=true, checkWaypointGUIDs=false)) {
	printRed("Readback of Route B does not match\n"); ; errorCount++;
	}
else printGreen("Readback of Route B matches\n");
// check the shared routepoint
RPindex = extraRoute.waypoints.length-2;	// index of shared routepoint
if (readbackExtra.waypoints[RPindex].GUID != sharedRoutepoint.GUID) printRed("Shared routepoint has different GUID\n");
if (readbackExtra.waypoints[RPindex].routeCount != 2) printRed("Shared routepoint routeCount should be 2 - is ", readbackExtra.waypoints[index].routeCount, "\n");
// check shared waypoint
WPindex = extraRoute.waypoints.length-1;	// index of waypoint
if (readbackExtra.waypoints[WPindex].GUID != wpGUID) {
	printRed("Shared waypoint has different GUID\n");
	errorCount++;
	if (verbose) print("from readbackRoute:\n", readbackRoute.waypoints[index], "\nreadbackExtra:\n", readbackExtra.waypoints[index], "\n");
	}
if (readbackExtra.waypoints[RPindex].routeCount != 2) {
	printRed("Shared waypoint routeMembershipCount should be 2 - is ", readbackExtra.waypoints[index].routeCount, "\n");
	errorCount++;
	}

printUnderlined("\nUpdate Route B - route details only\n");
readbackExtra.name = "Updated name";
readbackExtra.from = "Updated from name";
readbackExtra.to = "Updated to name";
try {OCPNupdateRoute(readbackExtra);}
catch(error) {throw("Failed to update route - ", error.message);}

readbackUpdatedExtra = OCPNgetRoute(readbackExtra.GUID);
if (compareRoutes(readbackExtra, readbackUpdatedExtra, "readbackExtra & readbackUpdatedExtra", checkRouteGUID= true, doWaypoints=true, checkWaypointGUIDs=true)) {
	printRed("Readback of updated Route B does not match\n");
	errorCount++;
	}

// check the shared routepoint
if (readbackExtra.waypoints[RPindex].GUID != sharedRoutepoint.GUID) printRed("Shared routepoint now has different GUID\n");
if (readbackExtra.waypoints[RPindex].routeCount != 2) printRed("Shared routepoint routeCount should be 2 - is ", readbackExtra.waypoints[index].routeCount, "\n");
if (readbackExtra.waypoints[WPindex].GUID != wpGUID) printRed("Shared waypoint now has different GUID\n");
if (readbackExtra.waypoints[WPindex].routeCount != 2) {
	printRed("Shared waypoint routeCount should be 2 - is ", readbackExtra.waypoints[index].routeCount, "\n");
	errorCount++;
	}
else printGreen("Readback of Route B matches\n");

printUnderlined("\nUpdate Route B - changing points\n");
removedRoutePointGUID = readbackExtra.waypoints[0].GUID;	// remember the first routepoint which we are deleting
readbackExtra.waypoints.shift();	// remove first waypoint
waypoint = new Waypoint();
waypoint.markName = "Extra routepoint";
waypoint.description = "Description of extra";
waypoint.position.latitude = 49.90;
waypoint.position.longitude = -2.4;
waypoint.iconName = "diamond";
waypoint.isVisible = true;
waypoint.nRanges = 4;
waypoint.minScale = 10020;
readbackExtra.waypoints.push(waypoint);	// add extra routepoint on end
extraIndex = readbackExtra.waypoints.length-1;	// remember where it is
try {OCPNupdateRoute(readbackExtra);}
catch(error) {throw("Failed to update route");}
updatedRoute = OCPNgetRoute(readbackExtra.GUID);
if (verbose){
	print("\nreadbackRoute:\n", readbackRoute, "\n");
	print("updatedRoute:\n", updatedRoute, "\n\n");
	}

readbackUpdatedExtra = OCPNgetRoute(readbackExtra.GUID);
// fix up extra routepoint to allow comparison
readbackExtra.waypoints[extraIndex].GUID = readbackUpdatedExtra.waypoints[extraIndex].GUID;
readbackExtra.waypoints[extraIndex].creationDateTime = readbackUpdatedExtra.waypoints[extraIndex].creationDateTime;

if (compareRoutes(readbackExtra, readbackUpdatedExtra, "", checkRouteGUID= true, doWaypoints=true, checkWaypointGUIDs=true)) {
	printRed("Readback of updated Route B does not match\n");
	errorCount++;
	}
else printGreen("Readback of Route B matches\n");
// check the shared routepoint
RPindex--; WPindex--;	// we dropped first routepoint, so shared points have moved down
if (readbackExtra.waypoints[RPindex].GUID != sharedRoutepoint.GUID) printRed("Shared routepoint now has different GUID\n");
if (readbackExtra.waypoints[RPindex].routeCount != 2) printRed("Shared routepoint routeCount should be 2 - is ", readbackExtra.waypoints[RPindex].routeCount, "\n");
if (readbackExtra.waypoints[WPindex].GUID != wpGUID) printRed("Shared waypoint now has different GUID\n");
if (readbackExtra.waypoints[WPindex].routeCount != 2) {
	printRed("Shared waypoint routeCount should be 2 - is ", readbackExtra.waypoints[WPindex].routeCount, "\n");
	errorCount++;
	}

// The extra routepoint will not have a GUID or creationDataTime.  Fix them up before compare
extraPointIndex = readbackRoute.waypoints.length - 1;
// check for orphaned routepoint
caught = false;
try {orphan = OCPNgetSingleWaypoint(removedRoutePointGUID);}
catch(error){ caught = true;}
if (!caught) {printRed("Routepoint removed by update still exists as orphan\n"); errorCount++}

printUnderlined("\nDeletion tests\n");
// we will now delete the second route and check for consistency
print("Deleting route B\n");

// just check our waypoint still exists
caught = false;
try {refreshed = OCPNgetSingleWaypoint(wpGUID);}	// our original waypoint
catch(error){caught = true;}
if (caught) { throw("Waypoint has been lost - logic error C\n")}
 
firstRoutepointGUID = readbackExtra.waypoints[0].GUID;
sharedRoutepointGUID = readbackExtra.waypoints[RPindex].GUID;
refresh = OCPNgetRoute(readbackExtra.GUID);
OCPNdeleteRoute(readbackExtra.GUID);
 
// check route has gone
caught = false;
try{ OCPNgetRoute(readbackExtra.GUID);}
catch(error){ caught = true;}
if (!caught) { printRed("OCPNdeleteRoute failed to delete route B\n"); errorCount++;}
else{
	// check unshared routepoint has gone
	caught = false;
	try{ OCPNgetSingleWaypoint(firstRoutepointGUID);}
	catch(error){caught = true;}
	if (!caught) {
		printRed("OCPNdeleteRoute failed to delete unshared routepoint\n");
		errorCount++;
		}
	else	printGreen("Unshared routepoint was deleted\n");

	// check shared routepoint remains
	caught = false;
	try{ OCPNgetSingleWaypoint(sharedRoutepointGUID);}
	catch(error){printRed("OCPNdeleteRoute deleted a still-shared routepoint\n"); errorCount++;}
	if (!caught) printGreen("Shared routepoint was retained\n");

	// check waypoint remains
	caught = false;
	try{ OCPNgetSingleWaypoint(wpGUID);}
	catch(error){printRed("OCPNdeleteRoute deleted a waypoint\n"); errorCount++; }
	if (!caught) printGreen("Waypoint was retained\n");
	}

print("Deleting route A\n");
OCPNdeleteRoute(routeGUID);
// check waypoint still there
caught = false;
try{ OCPNgetSingleWaypoint(wpGUID);}
catch(error){printRed(error.message,  " - OCPNgetSingleWaypoint found waypoint has been deleted\n");  errorCount++; caught = true;}
if (!caught) printGreen("Waypoint was retained\n");

print("Deleting waypoint\n");
OCPNdeleteSingleWaypoint(wpGUID);
caught = false;
try{ OCPNgetSingleWaypoint(wpGUID);}
catch(error){caught = true;}
if (!caught) {printRed("OCPNgetSigleWaypoint failed to delete it\n");  errorCount++;}
else printGreen("Deleted succesfully\n");

printUnderlined("\nActivity tests\n");

activeRouteGUID = OCPNgetActiveRouteGUID();
if (!activeRouteGUID) printOrange("No active route\n");
else {
	activeRoute = OCPNgetRoute(activeRouteGUID);
	print("Active route is ", activeRoute.name, "\n");
	}

activeWaypointGUID = OCPNgetActiveWaypointGUID();
if (!activeWaypointGUID) printOrange("No active waypoint\n");
else {
	activeWaypoint = OCPNgetSingleWaypoint(activeWaypointGUID);
	print("Active waypoint is ", activeWaypoint.markName, "\n");
	}

printUnderlined("\nTrack tests\n");
print("Add Track and read back\n");
testTrack = new Route();	// we will use the route constructor as there is no track xonstructor 
testTrack.name = "Test Track";
testTrack.from = "Start place";
testTrack.to = "End place";
for (i = 0; i < 4; i++) { // construct some trackpoints
	waypoint = {"position":{"latitude":50 + i/10, "longitude":-2.5 - (i/10)*i}, "creationDateTime": startTime+i*1000 };
	testTrack.waypoints.push(waypoint);
	}
if (verbose) print("Track: ", testTrack, "\n");
try {trackGUID = OCPNaddTrack(testTrack);}
catch(error) {throw("Failed to add track - " + error.message);}

try {readbackTrack = OCPNgetTrack(trackGUID);}
catch(error) {throw("Failed to get track - " + error.message);}
if (verbose) print(readbackTrack, "\n");
mismatch = compareRoutes(testTrack, readbackTrack, "", checkGUID=false, doWaypoints=false);
// too problematic to have compared trackpoints using compareWaypoints, so we will do it here
errorCountNow = errorCount;
for (j = 0; j < testTrack.waypoints.length; j++){
	if (testTrack.waypoints[j].position.latitude != readbackTrack.waypoints[j].position.latitude){
		printRed("Trackpoint ", j, " does not match\n"); errorCount++
		}
	if (testTrack.waypoints[j].position.longitude != readbackTrack.waypoints[j].position.longitude){
		printRed("Trackpoint ", j, " does not match\n"); errorCount++
		}
	}
if (mismatch || errorCount != errorCountNow) {printRed("Readback of track does not match\n"); errorCount++;}
else printGreen("Readback of track matches\n");

print("Update Track and read back\n");
testTrack.name = "Updated track name";
testTrack.to = "Updated to";
testTrack.from = "Updated from";
testTrack.GUID = trackGUID;
try {outcome = OCPNupdateTrack(testTrack);}
catch(error) {throw("Failed to update track - " + error.message);}
try {readbackTrack = OCPNgetTrack(trackGUID);}
catch(error) {throw("Failed to get updated track - " + error.message);}
mismatch = compareRoutes(testTrack, readbackTrack, "", checkGUID=true, doWaypoints=false);
if (mismatch) {printRed("Readback of updated track does not match\n"); errorCount++;}
else printGreen("Readback of updated track matches\n");
print("Deleting Track\n");
try{ OCPNdeleteTrack(trackGUID);}
catch(error){printRed("OCPNdeleteTrack threw error ", error.message, "\n");  errorCount++;}
// make sure it has gone
caught = false;
try { OCPNgetTrack(trackGUID);}
catch(error){ caught = true;}
if (!caught) {printRed("Track was not deleted\n"); errorCount++;}
else printGreen("Track was deleted\n");

if (errorCount == 0){
	printGreen("\n** End of tests - no errors found **\n");
	scriptResult("No errors found");
	}
else {
	printRed("\n** Tests ended after ", errorCount, (errorCount==1)?" error\n":" errors\n");
	print("You may need to use the Route & Mark Manager clean up routes and waypoints tht include 'test' in their names\n");
	scriptResult(errorCount, "errors");
	}
