sentence1 = "Test sentence 1";
sentence2 = "Test sentence 2";
port1= 12345;
port2 = 54321;
count = 0;
id1 = onSocketEvent(report1, port1);
id2 = onSocketEvent(report2, port2);

function report1(error, data){
	if (error) print("error code: ", error, "\n");
	else {
		if (typeof data != "string") throw("1. Non string sata returned");
		print("Report1 got ", data, "\n");
		if (data != sentence1) throw("Sentence1 wrong");
		socketSend(id1, sentence2);	// a reply
		}
	}

function report2(error, data){
	if (error) print("error code: ", error, "\n");
	else {
		if (typeof data != "string") throw("2. Non string sata returned");
		print("Report2 got ", data, "\n");
		if (data != sentence2) throw("Sentence2 wrong");
		stopScript("Sockets good");
		}
	}

onSeconds(function(){throw("Soockest test timed out");},1);
socketSend(id2, sentence1, port1);
