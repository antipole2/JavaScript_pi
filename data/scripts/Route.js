function Route(){
	var i;	// local scope only
	if (typeof Waypoint == "undefined") Waypoint = require("Waypoint");
	switch (arguments.length){
		case 0:
			this.name = "";
			this.from = "";
			this.to = "";
			this.GUID = "";
			this.description = "";
			this.isVisible = undefined;
			this.isActive = undefined;
			this.waypoints = [];
			break;
		case 1:	// argument should be route
			rt = arguments[0];
			this.name = rt.name;
			this.from = rt.from;
			this.to = rt.to;
			this.description = rt.description;
			this.isVisible = rt.isVisible;
			this.isActive = rt.isActive;
			this.GUID = rt.GUID;
			this.waypoints = rt.waypoints;
			break;
		default:	throw("Route constructor invalid call");
		}	
	this.get = function(GUID){
		if (typeof GUID == "undefined") GUID = this.GUID;
		if (typeof GUID == "undefined") throw("Attempting to get waypoint without GUID");
		gotRoute = OCPNgetRoute(GUID);
		if (!gotRoute) return false;
		// the gotRoute is a bare objct without methods.  Will copy into this object
		this.GUID = gotRoute.GUID;
		this.name = gotRoute.name;
		this.from = gotRoute.from;
		this.to = gotRoute.to;
		this.description = gotRoute.description;
		this.isVisible = gotRoute.isVisible;
		this.isActive = gotRoute.isActive;
		// need to construct the waypoint objects so they have their methods
		for (i = 0; i <gotRoute.waypoints.length; i++){
			waypoint = new Waypoint();
			waypoint.GUID = gotRoute.waypoints[i].GUID;
			waypoint.markName = gotRoute.waypoints[i].markName;
			waypoint.iconName = gotRoute.waypoints[i].iconName;
			waypoint.description = gotRoute.waypoints[i].description;
			waypoint.position.latitude = gotRoute.waypoints[i].position.latitude;
			waypoint.position.longitude = gotRoute.waypoints[i].position.longitude;
			waypoint.creationDateTime = gotRoute.waypoints[i].creationDateTime;
			waypoint.isVisible = gotRoute.waypoints[i].isVisible;
			waypoint.hyperlinkList = gotRoute.waypoints[i].hyperlinkList;
			this.waypoints.push(waypoint);
			delete waypoint;
			}
		delete gotRoute;
		return true;
		}
	this.update = function(){ // update route to OpenCPN
		if (typeof this.GUID == "undefined") throw("Updating route without GUID");
		return (OCPNupdateRoute(this));
		}
	this.add = function(GUID){ // add route to OpenCPN
		if (typeof GUID == "undefined") GUID = this.GUID;
		GUID = OCPNaddRoute(this);
		return (GUID);
		}
	this.purgeWaypoints = function(){
		for (i = 0; i <this.waypoints.length; i++){
			delete this.waypoints[i];
			}
		this.waypoints.length = 0;	// clears the array
		}
	this.delete = function(GUID){ // delete route in OpenCPN
		if (typeof GUID == "undefined") GUID = this.GUID;
		if (typeof GUID == "undefined") throw("Attempting to delete route without GUID");
		return(OCPNdeleteRoute(GUID));
		}
	}