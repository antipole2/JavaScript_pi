requireList = [
	"canboatAnalyzer",
	"checkForUpdate",
	"ContextSubMenu",
	"fibonacci",
	"File",
	"NMEA2000",
	"pgnDescriptors",
	"pluginVersion",
	"Position",
	"Route",
	"shell",
	"socketChunker",
	"Track",
	"Waypoint"
	];
	
errors = 0;
for (t in requireList){
	try {
		loaded = require(requireList[t]);
		print(requireList[t], " loaded OK\n");
		}
	catch (err){
		printRed(requireList[t], " failed  to load with ", err.toString(), "\n");
		errors++
		}
	}

scriptResult("require tests " + ((errors > 0)?"had errors":"OK"));

