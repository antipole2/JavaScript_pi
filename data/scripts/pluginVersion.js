function pluginVersion(needed){
	// check needed plugin version number v1.0

	// check argument and develop numeric array of it
	if (arguments.length != 1) throw("pluginVersion requires 1 argument");
	given = needed.toString().split(".");
	if (given.length > 3) throw("pluginVersion argument not valid version");
	for (var i = 0; i < given.length; i++){
		if (isNaN(given[i])) throw("pluginVersion argument includes non-number");
		given[i] = Number(given[i]);
		}
	if (given.length == 1) given[1]= given[2] = 0;
	if (given.length == 2) given[2] = 0;

	givenCombined = given[0]*10000 + given[1] * 100 + given[2];
	plugin = OCPNgetPluginConfig();
	major = plugin.PluginVersionMajor;
	minor = plugin.PluginVersionMinor;
	patch = plugin.patch;
	pluginCombined = major*10000 + minor*100 + patch;
	if (givenCombined > pluginCombined) throw("pluginVersion requires " + needed + " but is " + major + "." + minor + "." + patch);
	}
