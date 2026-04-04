// updated 2 Nov 2020 to accept position pair and to fix bug in formatted when minutes < 1 
// updated 18 Mar 2024 to add parsing formatted position
// updated 28 Jul 2025 to use new OCPNformatLatLon and hence user format choice to fix error in parsing position
// updated 04 Apr 2026
function Position(lat, lon){
	if (arguments.length == 1){
		if (typeof arguments[0] != "string"){
			this.latitude = arguments[0].latitude;
			this.longitude = arguments[0].longitude;
			}
		else {
			pos = parsePosition(arguments[0]);
			this.latitude = pos.latitude;
			this.longitude = pos.longitude;
			}
		}
	else {
		this.latitude= lat;
		this.longitude = lon;
		}
	
	Object.defineProperty(this, "formatted",{  // format for human eye
			enumerable: false,
			configurable: false,
			get: function () {
				return(niceify(OCPNformatDMS(1, this.latitude,true)) + " " + niceify(OCPNformatDMS(2, this.longitude,true)));
				}
			});
	
	Object.defineProperty(this, "NMEA",{   // format as in NMEA sentence
			enumerable: false,
			configurable: false,
			get: function () {
				latAbs = Math.abs(this.latitude);  wholeLatAbs =  Math.floor(latAbs);
				lonAbs = Math.abs(this.longitude);  wholeLonAbs =  Math.floor(lonAbs);
				return (
					((wholeLatAbs*100) + ((latAbs%1)*60)).toFixed(5) + "," + ((this.latitude < 0) ? "S," : "N,") +
					("00" + (((wholeLonAbs*100) + ((lonAbs%1)*60)).toFixed(5))).slice(-11) + "," + ((this.longitude < 0) ? "W" : "E")
					)
				}
			});
	
	this.NMEAdecode = function(string, n) {  // decodes n'th position from NMEA string
		n -= 1;  // base 0
		decoded = string.match(/\d+\.\d+,[NS],\d+\.\d+,[EW]/g);
		if (!decoded) return(false);
		poses = decoded[n].match(/\d+\.\d+/g);
		if (poses.length != 2) return false;
		whole = parseInt(poses[0]/100);  // Latitude
		rest = poses[0] - whole*100;
		this.latitude = whole + rest/60;
		whole = parseInt(poses[1]/100);  // Longitude
		rest = poses[1] - whole*100;
		this.longitude = whole + rest/60;
		poses = decoded[n].match(/[NSEW]/g);  // hemispheres
		if (poses[0] == "S") this.latitude *= -1;
		if (poses[1] == "W") this.longitude *= -1;
		return(true);
		}
	
	this.latest = function(){
		fix = OCPNgetNavigation();
		this.latitude = fix.position.latitude;
		this.longitude = fix.position.longitude;
		this.time = fix.fixTime;
		}
	
	this.parse = function(text){
		return parsePosition(text);
		}
	
	return this;
	
	function parsePosition(text){
		// parse lat & long into position object
		trace = false;
		title = arguments.callee.name;	// remember for error reporting
		phase = "";	// for error reporting
		partPat = /(.*)(N|S)(.*)(E|W)/i;
		parts = text.match(partPat);	// into parts
		if (parts == null) bail("not lat long pair");	
		if (trace) print("Halves: ", parts, "\n");
		if (parts.length != 5) bail("parse error 1");
		position = {};
		position.latitude = OCPNparseDMS(parts[1]+parts[2]);
		position.longitude = OCPNparseDMS(parts[3]+parts[4]);
		return position;
		
		function bail(message){
			throw(title + " " + phase + " " + message);
			};
		}
	
	function niceify(string){	// ajust precision to my liking
		parts = string.split(" ");
		switch (parts.length){
			case 1:	// Decimal degrees - do nothing
			return(string);
			case 2:	// should not be
			return("Nonsense formatting of decimal degrees");
			case 3:	// degrees and decimal minutes
			value = parts[1].slice(0,-1)* 1;
			parts[1] = value.toFixed(3) + "'";			
			return parts.join(" ");
			case 4:	// degrees minutes and decimal seconds
			value = parts[2].slice(0,-1)* 1;
			parts[2] = value.toFixed(1) + "\"";			
			return parts.join(" ");
			default: break;
			}			
		return("Invalid formatted string from OCPN");;
		}
	
	};
