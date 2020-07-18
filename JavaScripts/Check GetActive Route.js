	OCPNonMessageName(handleAR, “OCPN_ACTIVE_ROUTELEG_RESPONSE”);
	OCPNsendMessage(“OCPN_ACTIVE_ROUTELEG_REQUEST”);
	print(“Finished in  Listenout\n”);
}

function handleAR(activeRouteJS){
        print(activeRouteJS, “\n”);
	activeRoute = JSON.parse(activeRouteJS);
	// NB JSON creates an array
	if (!activeRoute[0].error )
	{  // we have an active route
    	routeGUID = activeRoute[0].active_route_GUID;
    	// bug in OpenCPN - the GUID is retuned as the waypoint name.
    	activeWaypointName = routeGUID;
print (“routeGUID:”, routeGUIDE);	
	else print("No active route");

}