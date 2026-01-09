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
		if (typeof data == "string") throw("1. String sata returned");
		decoded = CBOR.decode(data);
		print("Report1 got ", decoded, "\n");
		if (decoded != sentence1) throw("Decoded 1 wrong");
		cbor2 = CBOR.encode(sentence2);
		socketSend(id1, cbor2);	// a reply
		}
	}

function report2(error, data){
	if (error) print("error code: ", error, "\n");
	else {
		if (typeof data == "string") throw("2. String data returned");
		decoded = CBOR.decode(data);
		print("Report2 got ", decoded, "\n");
		if (decoded != sentence2) throw("Sentence2 wrong");
		stopScript("Sockets CBOR test good");
		}
	}

onSeconds(function(){throw("Soockest test timed out");},1);
cbor1 = CBOR.encode(sentence1);
socketSend(id2, cbor1, port1);
