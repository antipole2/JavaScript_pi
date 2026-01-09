// Callback cancellation tests
id1 = onSeconds(dontCall, 10);
id2 = onSeconds(doCall, 1);
id3 = onSeconds(dontCall, 10);

ids = onSeconds("?");
print("Test 1 ids", ids, "\n");
if (!compare(ids, [id1,id2, id3])) stopScript("Step1 arrays fail to match");
onSeconds(id2);
ids = onSeconds("?");
print("Test 2 ids", ids, "\n");
if (!compare(ids, [id1,id3])) stopScript("Step2 arrays fail to match");
onSeconds();	// cancel all remaining
ids = onSeconds("?");
print("Test 3 ids", ids, "\n");
if (ids.length !=  0) stopScript("Step3 failed to cancel all");
scriptResult("Callback cancellations OK");

function dontCall(){
	stopScript("function dontCall was called");
	}

function doCall(){
	print("doCall was called\n");
	}

function compare(a1, a2){	// compare two numeric arrays
	if (a1.length != a2.length) return false;
	for (var a = 0; a < a1.length; a++){
		if (a1[a] != a2[a]) return false;
		}
	return true;
	}
