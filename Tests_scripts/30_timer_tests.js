// timer and callback tests

result = "";
var methodId;

clock = {count:0, tick: function(){
			clock.count++;
			if (clock.count == 9){
				onSeconds(methodId);
				}
			}
		};

methodId = onAllSeconds(clock.tick, 0.1);

onExit(report);
onSeconds(stage2, 3);
onSeconds(function(){result += "A";}, 2)

for (i = 1; i < 10; i++){
	id = onSeconds(note, 1, i);
	if (i ==5) idToCancel = id
	}

onSeconds(idToCancel);	// test cancelling valid timer
try { onSeconds(1);}
catch(err){
	print("Cancelling timer with invalid id caught with '", err.message, "'\n");
	}

function note(what){
	result += what;
	};

function report(){
	if (clock.count = 9) result += "M";
	scriptResult(result);
	}

function stage2(){
	for (i = 1; i < 10; i++){
		id = onSeconds(note, 1, i*10);
		if (i ==5) idToCancel = id
		}
	onSeconds();	// cancel all timers
	};
