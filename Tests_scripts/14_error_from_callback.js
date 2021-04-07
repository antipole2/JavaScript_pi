// throw error from callback
onSeconds(doit, 1);

function doit(){
	// next statement is deliberately wrong
	onSeconds(1, doit);
	throw("Thrown from call-back");
	}
