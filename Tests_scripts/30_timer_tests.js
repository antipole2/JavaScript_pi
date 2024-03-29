// Excrcise and test timers

var tickCounter = 0
var tickPeriod = 0.5;

var times = ["Zero", "One", "Two", "Three", "Four", "Five", "Six", "Seven", "Eight", "Nine", "Ten"];
alert("This timer test takes upto 10 seconds");
tickID = onAllSeconds(tick, tickPeriod, "X");
onSeconds(five, 3, "\nThree seconds up\n");
onSeconds(finish, 10);

function tick(which){
	if (tickCounter > 5 /*times.length-1*/) {print(tickCounter, " ");}
	else print(times[tickCounter], "\n");
	tickCounter++;
	if (tickCounter > 12) onSeconds(tickID);	// cancel this timer
	if (tickCounter > 15) throw("\nFinish timer failed to finish");
	}
/*
function two(a){
	print(a, "\n");
	}
*/

function five(why){
	print(why, "\n");
	}

function finish(){
	print("\n");
	alert();
	stopScript((tickCounter ==13)?"Timer tests ended OK":"Timer tests failed");
	}
