tests = [
	"20°30.4'N  05° 30.5'W",
	"2°3'S  05° 30' 30\"e",
	"20°30.4'N  05° 30' 30\"W",
	"20.5°N 5.0W"
	];

expected = [
	"20° 30.400'N 005° 30.500'W",
	"02° 03.000'S 005° 30.500'E",
	"20° 30.400'N 005° 30.500'W",
	"20° 30.000'N 005° 0.000'W"
	];


Position = require("/Users/Tony/OpenCPN_project/JavaScript-project/JavaScript_pi/data/scripts/Position.js");

errors = 0;
for (t = 0; t < tests.length; t++){
	test = tests[t];
	position = new Position(test);
	print(t, "\t", position, "\t", position.formatted,"\n");
	if (position.formatted != expected[t]){
		print("Test: ", test, "\nexpected: ", expected[t], "\nresult:   ", position.formatted, "\n\n");
		errors++;
		}
	}
printOrange(errors, " error(s)\n");

throwTests = [
	"20°30.4'A  05° 30.5'W",
	"200°3'S  05° 30' 30\"e",
	"20°30.4'N  05x° 30' 30\"W",
	"-20.5°N 5.0W"
	];

throwsMissed = 0;
for (t = 0; t < throwTests.length; t++){
	test = throwTests[t];
	try	{
		position = new Position(test);
		}
	catch(err){
		print("throwTest ", t, " threw ", err, "\n");
		continue;
		}
	printOrange("throwTest ", t, " failed to throw\n");
	throwsMissed++;
	}
printOrange(throwsMissed, " missed throw error(s)\n");

scriptResult(((errors + throwsMissed) == 0)?"Success":"Errors");