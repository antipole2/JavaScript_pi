function checkForScriptUpdates(scriptName, scriptVersion, checkDays, versionCheckURL, scriptURL){
	var trace = false;

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
	if (trace) print("VersionControl:\n", JSON.stringify(details, null, "\t"), "\n");
	if (details.name != scriptName) throw("checkForUpdates checking wrong JSON updates control file");
	if (details.pluginVersion != undefined){
		// only proceed if plugin version sufficient
		plugin = OCPNgetPluginConfig();
		major = plugin.PluginVersionMajor;
		minor = plugin.PluginVersionMinor;
		patch = plugin.patch;
		pluginCombined = major + "." + minor + "." + patch;
		if (_compareVersions(details.pluginVersion, pluginCombined) < 0){
			printOrange("JavaScript plugin needs to be updated before you can update the ", scriptName, " script\n");
			return;
			}
		}

	if (_compareVersions(scriptVersion, details.version) > 0){
		// we can update this script if desired
		if (details.new != undefined) news = "\nNew: " + details.new;
		else news = "";
		message = "You have script version " + scriptVersion
			+ "\nUpdate to version " + details.version + " available."
			+ "\nDate: " + details.date
			+ news
			+ "\nGet updated script?";
		try {
			response = messageBox(message, "YesNo");
			}
		catch (err){	// treat Cancel as "No"
			response = 3;
			}
		if (response == 3) return;
		if (response != 2) throw("checkForUpdates logic error");
		// go ahead with update
		require("Consoles");
		newVersionLength = details.version.toString().length;
		newConsoleName = (scriptName.substr(0, 13-newVersionLength) + "_" + details.version).replace(/\./g, "_");
		if (consoleExists(newConsoleName)){
			alert("Console for updated script ", newConsoleName, " already exists\nClose it before getting update");
			return;
			}
		consoleAdd(newConsoleName);
		if (scriptURL == undefined) scriptURL = details.script;
		consoleLoad(newConsoleName, scriptURL);
		alert("Console ", newConsoleName, " contains new version ", details.version, " of script ", scriptName, "\n\nIf you have modified your script, you need to incorporate these changes into the new version\nIf you want to run this script off-line, you need to save it to a local file");
		}
	else if (trace) print("Version already up to date\n");
	_remember.versionControl.lastCheck = now;

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
	}
