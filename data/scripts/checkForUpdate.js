function checkForScriptUpdates(scriptName, scriptVersion, checkDays, versionCheckURL, scriptURL){
	var trace = false;
	function _compareVersions(first, second){
		// compare two version strings, e.g. 1.2.34
		// if second > first return 1, == 0,  < -1
//		if (trace) print("compareVersions first: ", first, "\tsecond: ", second, "\n");
		a = first.toString().split(".");
		b = second.toString().split(".");
		while (a.length < b.length) a.push(0);
		while (b.length < a.length) b.push(0);
//		if (trace) print("compareVersions a:", a, "\tb:", b, "\n");
		for (var i = 0; i < a.length; i++){
			if (b[i] > a[i]) return 1;
			if (b[i] < a[i]) return -1;
			}
		return 0;
		}
	if (!OCPNisOnline()) return;
	now = new Date().getTime();
	if (trace) print("Now: ", now, "\n");
	if (_remember == undefined)_remember = {};
	if (_remember.versionControl == undefined){
		if (trace) print("No existing _remember.versionControl\n");
		_remember.versionControl = {};
		_remember.versionControl.lastCheck = 0;
		}
	else if (_remember.versionControl.scriptName != scriptName){
		_remember.versionControl.scriptName = scriptName;
		_remember.versionControl.lastCheck = 0;
		}
	lastCheck = _remember.versionControl.lastCheck;
	nextCheck = lastCheck + checkDays*24*60*60*1000;
	if (trace) print("versionControl.lastCheck was ", lastCheck, "\tnext due in ", nextCheck-lastCheck, "\n");
	if (now < nextCheck){
		if (trace) print("No version check due\n");
		return;
		}
	_remember.versionControl.lastCheck = now;
	if (trace) print("versionControl.lastCheck updated to ", now, "\n");
	details = JSON.parse(readTextFile(versionCheckURL));
	if (trace) print("Version control:\n", JSON.stringify(details, null, "\t"), "\n");
	if (details.name != scriptName) throw("checkForUpdates checking wrong JSON updates control file");
	if (details.pluginVersion != undefined){
		// only proceed if plugin version sufficient
		plugin = OCPNgetPluginConfig();
		major = plugin.PluginVersionMajor;
		minor = plugin.PluginVersionMinor;
		patch = plugin.patch;
		pluginCombined = major + "." + minor + "." + patch;
		if (_compareVersions(details.pluginVersion, pluginCombined) < 0) return;
		}

	if (_compareVersions(scriptVersion, details.version) > 0){
		if (details.new != undefined) news = "\nNew: " + details.new;
		else news = "";
		message = "You have script version " + scriptVersion
			+ "\nUpdate to version " + details.version + " available."
			+ "\nDate: " + details.date
			+ news
			+ "\n \nUpdating will lose any local changes you have made"
			+ "\nHave you local changes to save?"
			+ "\n'Cancel' to ignore and continue with this script run"
			+ "\n'Yes' to save your own updates"
			+ "\n'No' to proceed to update";
		response = messageBox(message, "YesNo");
		if (response == 3){
			// if no script (3rd) argument in call, use scipt location from JSON
			if (arguments.length < 5) scriptURL = details.script;
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
			stopScript("Bring your updates to the updated script once it is loaded");
			}
		else _remember.versionControl.lastCheck = now;
		}
	else if (trace) print("Version already up to date\n");
	}
