File = require("File");
READ = 0; WRITE = 1; READ_WRITE = 2; APPEND = 3; WRITE_EXCL = 4;	// permissions for tests
myFile = new File("00_main.js");	//default is READ
print (myFile, "\n");
errorCount = 0;
data = myFile.getAllText();

print("Seek to start gives: ", myFile.seek(0),"\n");
print("Seek to midway gives: ", myFile.seek(5123),"\n");
print("Eof at midway gives: ", myFile.eof(),"\n");
print("Tell gives: ", myFile.tell(), "\n");;
print("Seek to end gives: ", myFile.seek(-1),"\n");
print("Eof at end gives: ", myFile.eof(),"\n");

builtLines = "";
myFile.seek(0);
while (!myFile.eof()) {
	builtLines += myFile.getTextLine() + "\n";
	}
builtLines = builtLines.slice(0, -1);	// drop extra \n

if (data == builtLines) scriptResult("Files OK");
else {
	printBlue("'", data, "'\n");
	printGreen("'", builtLines, "'\n");
	scriptResult("Files not OK");
	errorCount++;
	}

print("Write test\n");
inFile = new File("textFile.txt");
testLines = inFile.getAllText();
outFile = new File("testWriteData.txt", WRITE);
inFile.seek(0);
outFile.seek(0);
while (!inFile.eof()) {
	outFile.writeText(inFile.getTextLine() +"\n");
	}
outFile = new File("testWriteData.txt", READ);
result = outFile.getAllText();
result = result.trim();// clear extra \n we added at end
if (result != testLines){
	printOrange("writeText failed\n");
	printBlue("'", testLines, "'\n");
	printGreen("'", result, "'\n");
	errorCount++;
	}
print("Append test\n");
outFile = new File("testWriteData.txt", APPEND );
outFile.seek(outFile.length());
inFile.seek(0);
while (!inFile.eof()) {
	outFile.writeText(inFile.getTextLine() +"\n");
	}
outFile = new File("testWriteData.txt", READ );
resultDouble = outFile.getAllText();
resultDouble = resultDouble.trim();// clear extra \n we added at end
endoubled = testLines + "\n"+ testLines;
if (resultDouble != endoubled){
	printOrange("Append failed\n");
	printBlue("'", endoubled, "'\n");
	printGreen("'", resultDouble, "'\n");
	errorCount++;
	}

print("Update test\n");
outFile = new File("testWriteData.txt", WRITE );	// overwrite with test data
outFile.writeText(testLines);
outFile = new File("testWriteData.txt", READ_WRITE );
toReplace = "This is line four";
replacementLine = "THIS IS LINE FOUR";
startPos = testLines.indexOf(toReplace);
outFile.seek(startPos);
outFile.writeText(replacementLine);
updatedTestLines = testLines.replace(toReplace, replacementLine);

updated = outFile.getAllText();
if (updated != updatedTestLines){
	printOrange("Update failed\n");
	printBlue("'", updatedTestLines, "'\n");
	printGreen("'", updated, "'\n");
	errorCount++;
	}

print("Byte test\n");  // will convert back to lower case usiung byte arithmatic
phraseLength = toReplace.length;
outFile.seek(startPos);
bytes = outFile.getBytes(phraseLength);
for (i = 1; i < bytes.length; i++) if (bytes[i] >= 65) bytes[i] += 32;	// to lower case leavingg initial unchanged
outFile.seek(startPos);
outFile.writeBytes(bytes);
updated = outFile.getAllText();
if (updated != result){
	printOrange("Byte test failed\n");
	printBlue("'", result, "'\n");
	printGreen("'", updated, "'\n");
	errorCount++;
	}

scriptResult(errorCount > 0 ? "Files had errors" : "Files OK");