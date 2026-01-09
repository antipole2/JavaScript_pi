Position = require("Position");

errors = 0;
position = new Position(Math.random() * 80,Math.random() * -180);
// print("Position: ", position, "\n");
thisMoment = new Date();
seconds = thisMoment.getTime()/1000;
sog = Math.floor(Math.random() * 10);
cog = Math.floor(Math.random() * 360);
// print("sog: ", sog, "\tcog: ", cog, "\n");
OCPNpushNMEA("$JSVTG," + cog + ",T,,M," + sog + ",N,,K,A");
OCPNpushNMEA("$JSGLL," + position.NMEA + "," + 123456 + ",A,A");
onSeconds(check, 1);

function check(){
	nav = OCPNgetNavigation();
//	print(nav, "\n");
	match("Fix time", nav.fixTime, seconds, 1);
	match("Latitude", nav.position.latitude, position.latitude, 0.000001);
	match("Longitude", nav.position.longitude, position.longitude, 0.000001);
	match("SOG", nav.SOG, sog, 0.000001);
	match("COG", nav.COG, cog, 0.000001);
	scriptResult(!errors ? "Navigation OK" : ("Navigation check errors: " +errors));
	}

function match(what, a, b, margin){
	// compare a & b within margin
	difference = Math.abs(a - b);
	if (difference > margin ){
		print(what ,"  ", a , " ", b,  "\tdiffers by ", difference, "\n");
		errors++
		}
	}
