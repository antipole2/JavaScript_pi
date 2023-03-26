shouldBe = "This is line one\nThis is line two\nThis is line three\nThis is line four\nThis is line five\nThis is line six\nThis is line seven\nThis is line eight";
text = readTextFile("textFile.txt");
printOrange(text);
if (text == shouldBe) scriptResult("Read local text file matched");
else scriptResult("Read local text file failed");

