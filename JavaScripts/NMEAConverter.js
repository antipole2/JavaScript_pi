// insert magnetic variation into RMC sentence
var vardegs = "";
var varEW = "";

OCPNonNMEAsentence(processNMEA);

function processNMEA(sentence){
	if (sentence.slice(3,6) == "HDG")
		{
		splut = sentence.split(",");
		vardegs = splut[4];	varEW = splut[5];
		}
	else if (sentence.slice(3,6) == "RMC")
		{
		splut = sentence.split(",");
		if ((splut[10] == "") && (vardegs != ""))
			{   // only if no existing variation and we have var to insert
			splut[10] = vardegs; splut[11] = varEW;
			splut[0] = "$JSRMC";
			result = splut.join(",");
			OCPNpushNMEA(result);
			}
		}
	OCPNonNMEAsentence(processNMEA);
	};



