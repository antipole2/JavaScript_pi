function Waypoint(lat, lon){
	if (typeof Position == "undefined") Position = require("Position");
	this.position = new Position({latitude:undefined, logitude:undefined});
	switch (arguments.length){
		case 2:	// lat and long provided
			this.position.latitude = arguments[0];
			this.position.longitude = arguments[1];
			// fall through into case 0 !
		case 0:
			this.GUID = "";
			this.markName = "";
			this.description = "";
			this.creationDateTime = "";
			this.isVisible = true;
			this.iconName = "";
			this.hyperlinkList = [];
			break;
		case 1:	// argument should be  waypoint
			wp = arguments[0];
			this.position.latitude = wp.position.latitude;
			this.position.longitude = wp.position.longitude;
			this.GUID = wp.GUID;
			this.markName = wp.markName;
			this.description = wp.description;
			this.creationDateTime = wp.creationDateTime;
			this.isVisible = wp.isVisible;
			this.iconName = wp.iconName;
			this.hyperlinkList = wp.hyperlinkList;
			break;
		default:	throw("Waypoint constructor invalid call");
		}
	this.add = function(GUID){ // add waypoint to OpenCPN
		if (typeof GUID == "undefined") GUID = this.GUID;
		if (typeof this.creationDateTime == "undefined"){
			timeStamp = new Date;
			timeStamp.setTime(Date.now());
			this.creationDateTime = timeStamp/1000;  // in seconds
			delete timeStamp;	
			}
		GUID = OCPNaddSingleWaypoint(this);
		return (GUID);
		}
	this.update = function(){ // update waypoint to OpenCPN
		if (typeof this.GUID == "undefined") throw("Updating waypoint without GUID");
		return (OCPNupdateSingleWaypoint(this));
		}

	this.get = function(GUID){ // get waypoint by GUID
		if (typeof GUID == "undefined") GUID = this.GUID;
		if (typeof GUID == "undefined") throw("Attempting to get waypoint without GUID");
		waypoint = OCPNgetSingleWaypoint(GUID);
		if (!waypoint) return false;
		this.GUID = waypoint.GUID;
		this.markName = waypoint.markName;
		this.description = waypoint.description;
		this.position.latitude = waypoint.position.latitude;
		this.position.longitude = waypoint.position.longitude;
		this.creationDateTime = waypoint.creationDateTime;
		this.isVisible = waypoint.isVisible;
		this.iconName = waypoint.iconName;
		this.hyperlinkList = waypoint.hyperlinkList;
		delete waypoint;
		return (true);
		}
	this.delete = function(GUID){ // delete waypoint in OpenCPN
		if (typeof GUID == "undefined") GUID = this.GUID;
		if (typeof GUID == "undefined") throw("Attempting to delete waypoint without GUID");
		return(OCPNdeleteSingleWaypoint(GUID));
		}
	this.summary = function(){ return("Mark " + this.markName + " is at " + this.position.formatted);}
	};