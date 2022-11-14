shouldBe = "This is line one\nThis is line two\nThis is line three\nThis is line four\nThis is line five\nThis is line six\nThis is line seven\nThis is line eight";
text = readTextFile("https://raw.githubusercontent.com/antipole2/JavaScript_pi/master/Tests_scripts/textFile.txt");
if (text = shouldBe) scriptResult("Read remote text file matched");
else scriptResult("Read remote text file failed");
