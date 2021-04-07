// causes run-time error from Duktape
// onSeconds parameters wrong way round
onSeconds(1, doit);

function doit(){
	throw("Thrown from call-back");
	}