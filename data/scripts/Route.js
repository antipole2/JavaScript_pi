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
		case 1:	// argument should be route or GUID of route
			rt = arguments[0];
			if (typeof rt == "string"){ // is GUID
				rt = OCPNgetRoute(rt);
				}
			//now it is a route
			Object.assign(this, rt);
			break;
		default:	throw("Route constructor invalid call");
		}	
	this.get = function(GUID){
		if (typeof GUID == "undefined") GUID = this.GUID;
		if (typeof GUID == "undefined") throw("Attempting to get waypoint without GUID");
		gotRoute = OCPNgetRoute(GUID);
		if (!gotRoute) return false;
		// the gotRoute is a bare objct without methods.  Will copy into this object
		Object.assign(this, gotRoute);
		// need to construct the waypoint objects so they have their methods
		this.waypoints = [];
		for (i = 0; i < gotRoute.waypoints.length; i++){
			waypoint = new Waypoint(gotRoute.waypoints[i]);
			this.waypoints.push(waypoint);
			}
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
