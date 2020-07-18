// listens for OpenCPN sentences and NMEA messages over configured time and reports
// sorts NMEA messages by count and then alphabetically

var timeSeconds = 30; // time period over which to count
var progressSeconds = 5;  // count down this often
var secondsLeft = timeSeconds;
var log = [];   // will be array of entries

OCPNonSeconds(progress, 1); // count down every progressSeconds secs, but first almost immediately
OCPNonSeconds(report, timeSeconds);
OCPNonNMEAsentence(logit);

function progress(){
	if (secondsLeft == timeSeconds){
		// this is first time in
		print("Seconds remaining");
		}
	if (secondsLeft >= progressSeconds){
		print(" ", secondsLeft);
		if (secondsLeft > progressSeconds) OCPNonSeconds(progress, progressSeconds);
		secondsLeft -= progressSeconds;
		}
	}

function logit(returned){
	var entry = {type: "", count: 0};
	if (secondsLeft > 0) OCPNonNMEAsentence(logit);
	if (returned.OK){
		thisType = returned.value.slice(1, 6);
		count = log.length
		if (count > 0){
			var i = 0;	// declared outside for loop to keep scope available
			for ( ; i < count; i++){
				if (log[i].type == thisType){
					log[i].count += 1;
					return;
					}
				}
			}
		// no match or first entry - create new one
		entry.type = thisType;
		entry.count = 1;
		log.push(entry);
		}
	}
	

function report(){
	secondsLeft = 0;
	print("\n\nMessages seen:\n", OCPNgetMessageNames());
	entryCount = log.length;
	if (entryCount > 0){
		log.sort(function(a,b){ // sort log first by count then by type alphabetically
			if (a.count != b.count) return (b.count - a.count);
			else return ((a.type < b.type) ? -1 : 1);
			});
		print("\nNMEA sentences seen:\n");
		for (i in log){
			print(log[i].type, " ", log[i].count, "\n");
			}
		}
	else print("\nNo NMEA to report\n");
	OCPNcancelAll();
	}
