function checkVersion(version, checkDays,
		scriptURL, versionCheckURL){
	function compareVersions(first, second){
		// compare two version strings, e.g. 1.2.34
		// if second > first return 1, == 0,  < -1
		a = first.toString().split(".");
		b = second.toString().split(".");
		while (a.length < b.length) a.push(0);
		while (b.length < a.length) b.push(0);
		for (var i = 0; i < a.length; i++){
			if (b[i] > a[i]) return 1;
			if (b[i] < a[i]) return -1;
			}
	return 0;
	}
	var trace = false;
	var conf = OCPNgetPluginConfig();
	var pluginVersion = Number(conf.PluginVersionMajor + "." + conf.PluginVersionMinor);
	if (trace) print("Plugin version: ", pluginVersion, "\n");
	if (pluginVersion < 3.0) throw(scriptName + " requires plugin v3.0 or later.");
	if (!OCPNisOnline()) return;
	if (_remember == undefined) _remember = {};
	now = Date.now();
	if (trace) print("Now: ", now, "\n");
	if (_remember.hasOwnProperty("versionControl")){
		lastCheck = _remember.versionControl.lastCheck;
		nextCheck = lastCheck + checkDays*24*60*60*1000;
		if (trace) print("versionControl.lastCheck was ", lastCheck, "\tnext due ", nextCheck-lastCheck, "\n");
		if (now < nextCheck){
			if (trace) print("No version check due\n");
			return;
			}
		_remember.versionControl.lastCheck = now;
		}
	else _remember.versionControl = {"lastCheck":0};
	if (trace) print("versionControl.lastCheck updated to ", now, "\n");
	details = JSON.parse(readTextFile(versionCheckURL));
	if (trace) print("Version control:\n", JSON.stringify(details, null, "\t"), "\n");
	if (compareVersions(pluginVersion, details.pluginMinVersion) > 0){
//	if (details.pluginMinVersion > pluginVersion){
		message = "You have script version " + scriptVersion
			+ "\nUpdate to version " + details.version + " available."
			+ "\nDate: " + details.date + "\nNew: " + details.new
			+ "\n \nRequires plugin minimum version " + details.pluginMinVersion
			+ "\nYou have version " + pluginVersion
			+ "\nChoose 'Yes' to check again on next run"
			+ "\n'No' to check again in " + checkDays +  " days"
			+ "\nTo supress all checks, disable the call to checkForUpdates";
		response = messageBox(message, "YesNo");
		if (response == 2) _remember.versionControl.lastCheck = 0;
		else if (response == 1) _remember.versionControl.lastCheck += now + checkDays*24*60*60*1000;
		}
//	else if (scriptVersion < details.version){
	else if (compareVersions(scriptVersion, details.version) > 0){
		message = "You have script version " + scriptVersion
			+ "\nUpdate to version " + details.version + " available."
			+ "\nDate: " + details.date + "\nNew: " + details.new
			+ "\n \nUpdating will lose any local changes you have made"
			+ "\nHave you local changes to save?"
			+ "\n'Cancel' to ignore and continue with this script run"
			+ "\n \nTo supress update prompts, disable the call to checkVersion";
		response = messageBox(message, "YesNo");
		if (response == 3){
			require("Consoles");
			consoleLoad(consoleName(), scriptURL);
			message = "Script updated.\nYou need to save it locally if you want to run it off-line"
				+ "\nYou can now run the updated script.";
			messageBox(message);
			stopScript("Script updated");
			}
		else if (response == 2){
			message = "Use or create a spare console and copy your script into it"
				+ "\nYou may want to save that somewhere"
				+ "\nThen start this script again and choose 'No' in previous step"
				+ "\nto obtain updated script";
			messageBox(message);
			stopScript("Bring your updates to the updated script");
			}
		else _remember.versionControl.lastCheck = now;
		}
	else if (trace) print("Version already up to date\n");
	}
