errors = 0;
count = 0;
// Clear out any existing notifications
notes = OCPNgetNotifications();
for (var i = 0; i < notes.length; i++) OCPNackNotification(notes[i].GUID);

id1 = OCPNonAllNotificationAction(handle1);
id2 = OCPNonNotificationAction(handle2);
guid1 = OCPNraiseNotification(0, "Test notification severity 0", 14);
guid2 = OCPNraiseNotification(1, "Test notification severity 1");
guid3 = OCPNraiseNotification(2, "Test notification severity 2", 12);
guid3 = OCPNraiseNotification(2, "Test notification severity 2");
print("Initial notifications\n");
notifications = OCPNgetNotifications();
if (notifications.length != 4) errors++;
else count++;
display(notifications);
ok = OCPNackNotification(guid2);
print("\nAfter ack notification result was:", ok, "\n");
notifications = OCPNgetNotifications();
if (notifications.length != 3) errors++;
else count++;
display(notifications);

onSeconds(again1, 2);
onSeconds(again2, 4);
onExit(summarise);

function again1(){
	print("\nAfter delay 1\n");
	display(OCPNgetNotifications());
	OCPNonNotificationAction(id1);
	}

function again2(){
	print("\nAfter delay 2\n");
	display(OCPNgetNotifications());
	}

function display(notifications){
	print("Notification count: ", notifications.length, "\n");
//	for (var i = 0; i < notifications.length; i++) print(notifications[i], "\n");
	}

function handle1(note){
	printBlue("Heard about ", note.GUID, "\t", note.action, "\n");
	count++;
	}
function handle2(note){
	printGreen("Heard about ", note.GUID, "\t", note.action, "\n");
	count++;
	}

function summarise(){
	onSeconds();
	OCPNonNotificationAction();
	notifications = OCPNgetNotifications();
	for (var i = 0; i < notifications.length; i++)
		OCPNackNotification(notifications[i].GUID);
	print("Wrapping up\n");
	if ((errors == 0) && (count == 8)) scriptResult("Notes OK");
	else scriptResult("Errors:" + errors + " count:" + count);
	}




