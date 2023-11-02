// report on bit combinations

typesToReport = ["Latitude","Longitude"];


trace = true;
decriptors = require("./pgnDescriptors.js")();
print(descriptors.length, " descriptors\n");

for (t = 0; t < typesToReport.length; t++){
	typeToReport = typesToReport[t];
	BitOffsets = [];
	BitLengths= [];
	BitStarts = [];
	for (d = 0; d < descriptors.length; d++){
		descriptor = descriptors[d];
		fields = descriptor.Fields;
		if (fields != undefined){
			for (f = 0; f < fields.length; f++){
				field = fields[f];
				if (field.Type == typeToReport){
					BitOffset = field.BitOffset;
					if (BitOffsets.indexOf(BitOffset)< 0) BitOffsets.push(BitOffset);
					BitLength = fields[f].BitLength;
					if (BitLengths.indexOf(BitLength)< 0) BitLengths.push(BitLength);
					BitStart = fields[f].BitStart;
					if (BitStarts.indexOf(BitStart)< 0) BitStarts.push(BitStart);
					}
				}
			}
		}
	printUnderlined(typeToReport, "\n");
	print("BitOffsets:", BitOffsets, "\n");
	print("\t", modulo8(BitOffsets), "\n");
	print("BitLengths:", BitLengths, "\n");
	print("\t", modulo8(BitOffsets), "\n");
	print("BitStarts:", BitStarts, "\n");
	print("\t", modulo8(BitStarts), "\n");
	}

stopScript();function modulo8(what){
	output = "Modulo8 ";
	for (m = 0; m < what.length; m++){
		output += what[m]%8 + " ";
		}
	return output;
	}



function numeric(a,b) {return(a-b)};
BitOffsets.sort(numeric);
BitLengths.sort(numeric);
BitStarts.sort(numeric);
print("BitOffsets:", BitOffsets, "\n");
print("BitLengths:", BitLengths, "\n")
print("BitStarts:", BitStarts, "\n")

stopScript();
// display example of familiar types
familiar = [
	"Date",
	"Time"
	];

printUnderlined("Examples of unfamiliar types\n");
for (d = 0; d < descriptors.length; d++){
	descriptor = descriptors[d];
	fields = descriptor.Fields;
	if (fields != undefined){
		for (f = 0; f < fields.length; f++){
			valueType = fields[f].Type;
			if (valueType == undefined) continue;
			if (familiar.indexOf(valueType) < 0){
				// layout with usefull spacing
				offset = 20;
				length = valueType.length;
				if (length > offset){
					valueTypeDisplay = valueType + "\n";
					length = 0;
					paddingLength = offset;
					}
				else {
					valueTypeDisplay = valueType;
					paddingLength = offset-valueType.length;
					}
				padding = "                                                   ".slice(0, paddingLength);
				print(valueTypeDisplay, padding, descriptor.PGN, "\t", descriptor.Id,"\n");
				print(JSON.stringify(fields[f], null, "\t"), "\n\n");
				familiar.push(valueType);	// to stop repeating
				}
			if (types.indexOf(valueType)< 0) types.push(valueType);	// do not repeat this one
			}
		}
	}