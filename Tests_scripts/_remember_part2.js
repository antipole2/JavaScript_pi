testObject = {what: "Remember what?", day:5, of: "Novemeber", why: "Gunpowder treason and plot!"};
print("Recovered: ", _remember, "\n");
if (JSON.stringify(_remember) == JSON.stringify(testObject)) stopScript("_remember OK");
else stopScript("_remember failed");
