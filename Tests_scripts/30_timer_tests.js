// Excrcise an test timers
print("Repeated onSeconds calls will lose time compared with five second timer\n");

var tickCounter = 0
var tickPeriod = 1;

var times = ["Zero", "One", "Two", "Three", "Four", "Five", "Six", "Seven", "Eight", "Nine", "Ten"];
alert("This timer test takes about 20 seconds");
onSeconds(tick, tickPeriod, 0);
onSeconds(five, 5, "Five seconds up");
onSeconds(finish, 20);

function tick(which){
	if (tickCounter > times.length-1) {print(which, " ");}
	else print(times[which], "\n");
	tickCounter++;
	if (tickCounter > 25) throw("Finish timer failed to finish");
	onSeconds(tick, tickPeriod, tickCounter);
	}

function two(a){
	print(a, "\n");
	}

function five(why){
	print(why, "\n");
	}

function finish(){
	print("\n");
	alert();
	stopScript("Timer tests ended OK");
	}
