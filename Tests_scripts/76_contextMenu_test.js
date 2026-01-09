contextSubMenu = require("contextSubMenu");

var testWaypointGUID = false;
scriptResult("ContextMenu testing failed");
onExit(wrapup);
id = OCPNonContextMenu(handleResponse1, "Select this");
alert("Use canvas context menu to select item 'Select this'");

function handleResponse1(data){
	alert(false);
	if (data.menuName =! "Select this") stopScript("Wrong selection");
	subMenu = new contextSubMenu();
	subMenu.add(handleWrongOne, "Not this");
	subMenu.add(handleWrongOne, "Nor this");
	subMenu.add(handleResponse2, "But this one!");
	id = OCPNonAllContextMenu(subMenu, "Select from submenus");
	alert("Now use the canvas context menu to select 'But this one!' in the submenues");
	}
function handleWrongOne(){
	alert("\n*** You picked the wrong one! ***");
	};

function handleResponse2(data){
	if (data.menuName != "But this one!")stopScript("Error handling response 2");
	alert(false);
	OCPNonContextMenu(id);
	testWaypoint = {markName:"Test waypoint", position:data, isVisible:true, isNameVisible:true, nRanges:2};
	testWaypointGUID = OCPNaddSingleWaypoint(testWaypoint);
	OCPNonWaypointContextMenu(handleResponse3, "Select this");
	alert("Now using a waypoint such as 'Test waypoint',\nselect the waypoint context menu item 'Select this'");
	}

function handleResponse3(data){
	alert(false);
	if (data.menuName != "Select this") stopScript("Wrong selection");
	subMenu = new contextSubMenu();
	subMenu.add(handleWrongOne, "Not this");
	subMenu.add(handleWrongOne, "Nor this");
	subMenu.add(handleResponse4, "But this one!");
	id = OCPNonAllWaypointContextMenu(subMenu, "Submenus");
	alert("Now use the waypoint context submenus to select 'But this one!' in the submenus");
	}

function handleResponse4(data){
	if (data.menuName != "But this one!") stopScript("Wrong selection");
	alert(false);
	OCPNonAllWaypointContextMenu();
	stopScript("Context tests OK");
	}

function wrapup(){
	if (testWaypointGUID) OCPNdeleteSingleWaypoint(testWaypointGUID);
	}
