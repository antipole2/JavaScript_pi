Position = require("Position");

position = new Position(Math.random() * 80,Math.random() * -180);
thisMoment = new Date();
seconds = thisMoment.getTime()/1000;
sog = Math.floor(Math.random() * 10);
cog = Math.floor(Math.random() * 360);
OCPNpushNMEA("$JSVTG," + cog + ",T,,M," + sog + ",N,,K,A");
OCPNpushNMEA("$JSGLL," + position.NMEA + "," + 123456 + ",A,A");
onSeconds(step, 1);

function step(){
	nav = OCPNgetNavigation();
print(nav, "\n");
	if (
		(Math.abs(nav.fixTime-seconds) <= 1)
		&& (Math.abs(nav.position.latitude - position.latitude) < 0.000001)
		&& (Math.abs(nav.position.longitude - position.longitude) < 0.000001)
		&& (Math.abs(nav.SOG - sog) < 0.001)
		&& (Math.abs(nav.COG - cog) < 1)
		)
		stopScript("Navigation OK");
	else stopScript("Navigation error");
	}

