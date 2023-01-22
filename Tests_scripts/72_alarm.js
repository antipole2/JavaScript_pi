OK = OCPNsoundAlarm();
result = messageBox("Did alarm sound?", "YesNo", "Alarm Test");
switch (result){
	case 0: scriptResult("Alarm test cancelled");
		break;
	case 1:
	case 2: scriptResult("Alarm OK");
		break;
	case 3: scriptResult("Alarm test reported failed");
		break;
	}