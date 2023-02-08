shouldBe = "This is line one\nThis is line two\nThis is line three\nThis is line four\nThis is line five\nThis is line six\nThis is line seven\nThis is line eight\n";
text = readTextFile("textFile.txt");
if (text == shouldBe) scriptResult("Read local text file matched");
else scriptResult("Read local text file failed");
