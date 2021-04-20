function abc(){
	OCPNaddSingleWaypoint(1,2,3);
	}

function outer(){
	abc();
	}

outer();