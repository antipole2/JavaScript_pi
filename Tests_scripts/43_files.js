File = require("/Users/Tony/OpenCPN_project/JavaScript-project/JavaScript_pi/data/scripts/File.js");
myFile = new File("00_main.js");
print (myFile, "\n");

data = myFile.getAllText();;

builtLines = "";
while (!myFile.eof) builtLines += myFile.getTextLine() + "\n";
builtLines = builtLines.slice(0, -1);	// drop extra \n

if (data == builtLines) scriptResult("Files OK");
else {
	printBlue("'", data, "'\n");
	printGreen("'", builtLines, "'\n");
	scriptResult("Files not OK");
	}