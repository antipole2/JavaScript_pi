socketChunker = require("socketChunker");

chunker = new socketChunker(handleEvent, 1234);

message = "This is a long multi-line message\nThis is a long multi-line message\nThis is a long multi-line message\nThis is a long multi-line message\nThis is a long multi-line message\nThis is a long multi-line message\nThis is a long multi-line message\nThis is a long multi-line message\nThis is a long multi-line message\nThis is a long multi-line message\nThis is a long multi-line message\nThis is a long multi-line message\nThis is a long multi-line message\nThis is a long multi-line message\nThis is a long multi-line message\nThis is a long multi-line message\nThis is a long multi-line message";
chunker.socketSend(message , 1234);
print("Chunker:\t", chunker, "\n");

function handleEvent(error, data){
	if (error) printRed("handleEvent got error ", error, ": ", data, "\n");
	else print("handleEvent called with data:\n", data, "\n");
	onSeconds(timerId);	// canacl the timer
	onSocketEvent(chunker.socketId);	// closes the socket
	if (data == message) stopScript("socketChunker OK");
	else stopScript("socketChunker failed");
	}

timerId = onSeconds(function(){onSocketEvent(chunker.socketId);}, 20);
scriptResult("socketChunker timed out");

