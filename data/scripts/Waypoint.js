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
		this.isVisible = false;
		this.hyperlinkList = [];
		break;
		case 1:	// argument could be waypoint or position
		var wp = arguments[0];
		if (!(wp.position  != undefined)){  // it is a position
			this.position = {};
			this.position.latitude = wp.latitude;
			this.position.longitude = wp.longitude;
			this.GUID = "";
			this.markName = "";
			this.description = "";
			this.isVisible = false;
			this.iconName = "diamond";
			this.hyperlinkList = [];
			}
		else{	// its a waypoint
			Object.assign(this, wp);
			this.position = new Position(wp.position);
			}
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
		var wp = OCPNgetSingleWaypoint(GUID);
		if (!wp) return false;
		Object.assign(this, wp);
		return (true);
		}
	this.delete = function(GUID){ // delete waypoint in OpenCPN
		if (typeof GUID == "undefined") GUID = this.GUID;
		if (typeof GUID == "undefined") throw("Attempting to delete waypoint without GUID");
		return(OCPNdeleteSingleWaypoint(GUID));
		}
	this.summary = function(){ return("Mark " + this.markName + " is at " + this.position.formatted);}
	};
