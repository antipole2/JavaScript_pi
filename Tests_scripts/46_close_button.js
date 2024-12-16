scriptResult("Failed");

onCloseButton(closed);
alert("With the Command button down\n(on Windows use the control button),\nclick close button on console ", consoleName());

function closed(){
	onSeconds();	// cancel timeout
	alert(false);	// clear alert
	scriptResult("Succeeded");
	}