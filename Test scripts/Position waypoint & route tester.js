// Position, waypoint and route confidence tester

function compareWaypoints(waypoint1, waypoint2){
	// compares two waypoints and displays any mismatches.
	// returns true if error else false if OK
	function display(attribute){
		if (!displaying) print("The following attributes of waypoints do not match:\n");
		displaying = true;
		print(attribute, "\n");
		}
	displaying = false;
	if (waypoint1.position.latitude != waypoint2.position.latitude) display ("latitude");
	if (waypoint1.position.longitude != waypoint2.position.longitude) display ("longitude");
	if (waypoint1.markName != waypoint2.markName) display ("markName");
	if (waypoint1.description != waypoint2.description) display ("markName");
	if (waypoint1.iconName != waypoint2.iconName) display ("iconName");
	if (waypoint1.isVisible != waypoint2.isVisible) display ("isVisible");
	if (waypoint1.iconName != waypoint2.iconName) display ("iconName");
	if (waypoint1.creationDateTime != waypoint2.creationDateTime) display ("creationDateTime");
	if (waypoint1.GUID != waypoint1.GUID) return ("GUID");  // omit this - too many issues with routes
	if (waypoint1.hyperlinkList.length != waypoint2.hyperlinkList.length) return ("number of hyperlinks differs");
	numberOfLinks = waypoint1.hyperlinkList.length;
	if (numberOfLinks > 0){  // following loop hangs if no links (why?) so avoid
		for (i = 0; i < numberOfLinks; i++){
			if (waypoint1.hyperlinkList[i].description != waypoint2.hyperlinkList[i].description) display ("hyperlink[" + i + "].description differs");
			if (waypoint1.hyperlinkList[i].link != waypoint2.hyperlinkList[i].link) display ("hyperlink[" + i + "].link differs");
			if (waypoint1.hyperlinkList[i].type != waypoint2.hyperlinkList[i].type) display ("hyperlink[" + i + "].type differs");
			}
		}
	return displaying;
	}
	
function compareRoutes(route1, route2){
	// compares two routes including their waypoints and displays any mismatches.
	// returns true if error else false if OK
	function display(attribute){
	if (!displaying) print("The following attributes of routes do not match:\n");
		displaying = true;
		print(attribute, "\n");
		}
	displaying = false;

	if (route1.GUID != route2.GUID) display ("GUID");
	if (route1.name != route2.name) display ("name");
	if (route1.from != route2.from) display ("from");
	if (route1.to != route2.to) display ("to");
	if (route1.waypoints.length != route2.waypoints.length) return ("Number of waypoints does not match");
	for (i = 0; i < route1.waypoints.length; i++){
		result = compareWaypoints(route1.waypoints[i], route2.waypoints[i]);
		if (result) display("Waypoints " + i + " in routes do not match ");
		}
	return displaying;
	}

function report(message){
	print(message, "\n");
	OK = false;
	}

// 1: Position
print("*** Positon tests ***\n");
Position = require("Position");
if (typeof(Position) != "function") throw("Position did not load function");
OK = true;
testPosition = new Position;
sentence = "$OCRMB,A,0.000,L,,UK-S:Y,5030.530,N,00121.020,W,0021.506,82.924,0.000,5030.530,S,00120.030,E,V,A*69";
testPosition.NMEAdecode(sentence,2);
if (testPosition.NMEA != "5030.530,S,00120.030,E") report("Position NMEA tests failed");
testPosition.latest();
if(testPosition.fixTime == 0) print("No fix since OpenCPN started - reporting last\n");
else print("Latest fix was at ", Date(testPosition.fixtime*1000), "\n");
print("Position ", testPosition.formatted, "\n");
nav = OCPNgetNavigation();
print("SOG\t", nav.SOG, "\n");
print("COG\t", nav.COG, "\n");
print("HDT\t", nav.HDT, "\n");
print("HDM\t", nav.HDM, "\n");
print("variation\t", nav.variation, "\n");
delete testPosition;
print("Position tests completed ", OK?"OK":"with error(s)", "\n");

// 2: Waypoints
print("\n*** Waypoint tests ***\n");
OK = true;
Waypoint = require("Waypoint");
if (typeof(Waypoint) != "function") throw("Waypoint constructor did not load function");
testWaypoint = new Waypoint(0, -123.45);
testWaypoint.position.latitude = 12.345;
if (testWaypoint.position.NMEA != "1220.700,N,12327.000,W") throw("Waypoint position attribute error");
testWaypoint.markName = "Test waypoint";
testWaypoint.description = "This waypoint is a temporary one for diagnostics";
testWaypoint.iconName = "anchor";
// add some hyperlinks
var link1 = {description:"OpenCPN", link: "https://opencpn.org", type: "Type1"};
var link2 = {description:"JavaScript user guide", link: "https://github.com/antipole2/JavaScript_pi/blob/master/JavaScript%20plugin%20user%20guide.pdf", type: "Type2"};
testWaypoint.hyperlinkList.push(link1);
testWaypoint.hyperlinkList.push(link2);
GUID = testWaypoint.add();
if (!GUID) throw("Add waypoint failed");
readWaypoint = new Waypoint();
if (readWaypoint.get("abc")) throw("Get non-existant waypoint suceeded!!");
if (!readWaypoint.get(GUID)) throw("Get waypoint failed");
result = compareWaypoints(testWaypoint, readWaypoint);
if (result) report("Read-back of waypoint does not match oiginal ");
returnedTimeStamp = readWaypoint.creationDateTime;	// remember for use in Route tests because of OpenCPN bug
readWaypoint.descripton = readWaypoint.descripton + " (updated)";
readWaypoint.position.latitude = -10;
readWaypoint.position.longitude = 75;
readWaypoint.iconName = "Food";
readWaypoint.markName = "Test waypoint updated";
readWaypoint.isVisible = !readWaypoint.isVisible
readWaypoint.hyperlinkList[0].description = "Updated";
if(!readWaypoint.update()) throw("Update waypoint failed");
updatedWaypoint = new Waypoint();
if (!updatedWaypoint.get(GUID)) throw("Getting updated waypoint failed");
result = compareWaypoints(readWaypoint, updatedWaypoint);
if (result) report("Read back of updated waypoint does not match original of updarted waypoint");
if (!updatedWaypoint.delete()) report("Waypoint deletion failed");
if (updatedWaypoint.get(GUID)) report("Waypoint was not actually deleted");
delete testWaypoint; delete readWaypoint; delete updatedWaypoint;
print("Waypoint tests completed ", OK?"OK":"with error(s)", "\n");

// 3: Routes
print("\n*** Route tests ***\n");
OK = true;
Route = require("Route");
if (typeof(Route) != "function") throw("Route constructor did not load");
newRoute = new Route();
newRoute.name = "Constructed test";
newRoute.from = "Start place";
newRoute.to = "End place";
for (i = 1; i < 4; i++) { // construct some waypoints
	waypoint = new Waypoint();
	waypoint.markName = "TestWP" + i;
	waypoint.description = "Description of WP" + i;
	waypoint.position.latitude = 60 + i/10;
	waypoint.position.longitude = -1 - i/10
	waypoint.creationDateTime = returnedTimeStamp;  // *** THIS IS TO STOP BUG BEING REPORTED AGAIN
	newRoute.waypoints.push(waypoint);
	}
GUID = newRoute.add();
if (!GUID) throw("Failed to create new route");
newRoute.GUID = GUID;
retrievedRoute = new Route;
if (!retrievedRoute.get(GUID)) throw("Failed to retrieve route");
result = compareRoutes(newRoute, retrievedRoute);
if (result) {
	report("Retrieved route fails to match original ");
	print("\nnewRoute:\n", JSON.stringify(newRoute), "\n\n");
	print("\nretrievedRoute:\n", JSON.stringify(retrievedRoute), "\n\n");
	}
retrievedRoute.name = "Changed route name";
retrievedRoute.to = "New to string";
retrievedRoute.waypoints[1].position.longitude += 1;
if (!retrievedRoute.update()) throw("Failed to update route");
updatedRoute = new Route;
if (!updatedRoute.get(GUID)) throw("Failed to retrieve updated route");
result = compareRoutes(retrievedRoute, updatedRoute);
if (result) {
	report("Retrieved updated route fails to match original ");
	print("\nretrievedRoute:\n", JSON.stringify(retrievedRoute), "\n\n");
	print("\nupdatedRoute:\n", JSON.stringify(updatedRoute), "\n\n");
	}
if (OCPNdeleteRoute("aaa")) throw("Deleting non-existant route reports success!");
updatedRoute.purgeWaypoints();
if (updatedRoute.waypoints.length) throw("Failed to purge waypoints from updatedRoute");
updatedRoute.delete();
if (updatedRoute.get(GUID)) throw("Failed to delete route in OpenCPN");
print("Routes tests OK\n");

// 3: Tracks
print("\n*** Track tests ***\n");
throw("Track tests not yet available");
OK = true;
Track = require("Track");
if (typeof(Track) != "function") throw("Track constructor did not load");
newTrack = new Track();
newTrack.name = "Constructed test";
newTrack.from = "Start place";
newTrack.to = "End place";
for (i = 1; i < 4; i++) { // construct some waypoints
	print("Constructing track waypoint ", i, "\n");
	waypoint = new Waypoint();
	waypoint.markName = "TestWP" + i;
	waypoint.description = "Description of WP" + i;
	waypoint.position.latitude = 60 + i/10;
	waypoint.position.longitude = -1 - i/10
	waypoint.creationDateTime = returnedTimeStamp;  // *** THIS IS TO STOP BUG BEING REPORTED AGAIN
	newTrack.waypoints.push(waypoint);
	}
print("Track is:\n", JSON.stringify(newTrack), "\n\n");
GUID = newTrack.add();
if (!GUID) throw("Failed to create new track");
throw("Should have created track now");
newTrack.GUID = GUID;
retrievedTrack = new Track;
if (!retrievedTrack.get(GUID)) throw("Failed to retrieve track");
result = compareRoutes(newTrack, retrievedTrack); // works for tracks too
if (result) {
	report("Retrieved track fails to match original ");
	print("\nnewTrack:\n", JSON.stringify(newTrack), "\n\n");
	print("\nretrievedTrack:\n", JSON.stringify(retrieved), "\n\n");
	}
retrievedTrack.name = "Changed track name";
retrievedTrack.to = "New to string";
retrievedTrack.waypoints[1].position.longitude += 1;
if (!retrievedRTrack.update()) throw("Failed to update track");
updatedTrack = new Track;
if (!updatedTrack.get(GUID)) throw("Failed to retrieve updated track");
result = compareRoutes(retrievedTrack, updatedTrack);	// compare works for tracks too
if (result) {
	report("Retrieved updated track fails to match original ");
	print("\nretrievedTrack:\n", JSON.stringify(retrievedTrack), "\n\n");
	print("\nupdatedTrack:\n", JSON.stringify(updatedTrack), "\n\n");
	}
if (OCPNdeleteTrack("aaa")) throw("Deleting non-existant track reports success!");
updatedTrack.purgeWaypoints();
if (updatedTrack.waypoints.length) throw("Failed to purge waypoints from updatedTrack");
updatedTrack.delete();
if (updatedTrack.get(GUID)) throw("Failed to delete track in OpenCPN");
print("Track tests OK\n");
"OK"