scriptResult("Failed");

onCloseButton(closed);
alert("Click close button on console ", consoleName());

function closed(){
	onSeconds();	// cancel timeout
	alert(false);	// clear alert
	scriptResult("Succeeded");
	}