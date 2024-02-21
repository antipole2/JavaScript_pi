// updated 2 Nov 2020 to accept position pair and to fix bug in formatted when minutes < 1 
// updated 20 Feb 2023 to add parsing formatted position
function position(lat, lon){
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
        latAbs = Math.abs(this.latitude);
        lonAbs = Math.abs(this.longitude);
        return (
            ("00" + parseInt(latAbs)).slice(-2) + String.fromCharCode(176) + " " +
            ("00" + (latAbs % 1 * 60).toFixed(3)).slice(-6) + "'" +
            ((this.latitude < 0) ? "S " : "N ") +
            ("00" + parseInt(lonAbs)).slice(-3) + String.fromCharCode(176) + " " +
            (lonAbs % 1 * 60).toFixed(3) + "'" +
            ((this.longitude < 0) ? "W" : "E")
	        )}
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
		latSign = parts[2].match(/N/i)?1:-1;
		lonSign = parts[4].match(/E/i)?1:-1;
		position = {};
		position.latitude = parse(parts[1], "lat") * latSign;
		position.longitude = parse(parts[3], "lon") * lonSign;
		return position;

		function parse(parseText, which){
			if (trace) print("parse ", which, " with ", parseText, "\n");
			phase = which;
			if (which = "lat") maxDeg = 90;
			else maxDeg = 180;
			// squeeze out all spaces
			parseText = parseText.replace(/\s/g, ''); // squeeze out all spaces

			// degrees minutes & seconds ?
			dmsPattern = /([0-9]{1,3})\xB0([0-9]{1,2})'([0-9][0-9]?.?[0-9]?)"/;
			dms = parseText.match(dmsPattern);
			if (dms != null){ // is degrees, minutes & seconds
				if (trace) print("DMS\t", dms, "\n");
				deg = check(dms[1]*1, maxDeg);
				min = check(dms[2]*1, 60);
				sec = check(dms[3]*1, 60);
				return(deg + min/60 + sec/3600);
				}

			//	degrees & minutes ?
			// sometmes ' used as decimal point in minutes.  Fix if needed.
			if (trace) print("Before fix: ", parseText, "\n");
			if (parseText.slice(-1) == "'"){	// remove any trailing '
				parseText = parseText.slice(0, -1);
				}
			parseText = parseText.replace("'", ".");	// and replace ' if any
			dmPattern = /([0-9]{1,3})\xB0([0-9][0-9]?([\.\']?[0-9][0-9]?)?)'?/;
			dm = parseText.match(dmPattern);
			if (dm != null){ // is degrees, minutes
				if (trace) print(parseText, "\tDM\t", dm, "\n");
				deg = check(dm[1]*1, maxDeg);
				min = check(dm[2]*1, 60);
				return(deg + min/60);
				}

			// must be degree only
			deg = parseText.slice(0, -1);
			return(check(deg*1, maxDeg));
			}
/*
		function count(text, ch){	// returns count of ch in text
			c = 0;
			for (i = 0; i < text.length; i++){
				if (text[i] == ch) c++;
				}
			return c;
			}
*/
		function check(value, max){	// sanity check 
			if (isNaN(value)) bail("not a number");
			if (value < 0 || value > max) bail("out of range");
			return value;
			};

		function bail(message){
			throw(title + " " + phase + " " + message);
			};
		}
    };
