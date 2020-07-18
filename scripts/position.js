function position(lat, lon){
	this.value = {latitude:lat, longitude: lon};
        
    Object.defineProperty(this, "formatted",{  // format for human eye
	enumerable: false,
	configurable: false,
	get: function () {
        latAbs = Math.abs(this.value.latitude);
        lonAbs = Math.abs(this.value.longitude);
        return (
            parseInt(latAbs) + "º " +
            (latAbs % parseInt(latAbs) * 60).toFixed(3) + "'" +
            ((this.value.latitude < 0) ? "S " : "N ") +
            ("00" + parseInt(lonAbs)).slice(-3) + "º " +
            (lonAbs % parseInt(lonAbs) * 60).toFixed(3) + "'" +
            ((this.value.longitude < 0) ? "W " : "E ")
	        )}
	    });
		
	Object.defineProperty(this, "NMEA",{   // format as in NMEA sentence
	enumerable: false,
	configurable: false,
	get: function () {
		latAbs = Math.abs(this.value.latitude);  wholeLatAbs =  Math.floor(latAbs);
        lonAbs = Math.abs(this.value.longitude);  wholeLonAbs =  Math.floor(lonAbs);
         return (
        	((wholeLatAbs*100) + ((latAbs%1)*60)).toFixed(3) + "," + ((this.value.latitude < 0) ? "S," : "N,") +
        	("00" + (((wholeLonAbs*100) + ((lonAbs%1)*60)).toFixed(3))).slice(-9) + "," + ((this.value.longitude < 0) ? "W" : "E")
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
		 this.value.latitude = whole + rest/60;
		 whole = parseInt(poses[1]/100);  // Longitude
		 rest = poses[1] - whole*100;
		 this.value.longitude = whole + rest/60;
		 poses = decoded[n].match(/[NSEW]/g);  // hemispheres
		 if (poses[0] == "S") this.value.latitude *= -1;
		 if (poses[1] == "W") this.value.longitude *= -1;
		 return(true);
		 }        
    };