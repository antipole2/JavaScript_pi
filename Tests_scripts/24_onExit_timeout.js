onExit(finishing);
scriptResult("Done main");

function finishing(){
	timeAlloc(5);	// set short timeout
	longTime(1000000);
	scriptResult("finishing finished!");
	}
function longTime(t){
	for (i = 0; i < t; i++){
		a = 1234*i;
		}
	}