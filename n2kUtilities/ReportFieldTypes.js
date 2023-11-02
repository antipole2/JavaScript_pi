// report on all field types

trace = true;
// display example of familiar types
familiar = [
	"Date",
	"Time"
	];
decriptors = require("./pgnDescriptors.js")();
print(descriptors.length, " descriptors\n");

types = [];
for (d = 0; d < descriptors.length; d++){
	descriptor = descriptors[d];
	fields = descriptor.Fields;
	if (fields != undefined){
		for (f = 0; f < fields.length; f++){
			valueType = fields[f].Type;
			if (familiar.indexOf(valueType) >= 0) continue;	// skip familiar
			if (types.indexOf(valueType)< 0) types.push(valueType);
			}
		}
	}
types.sort();
print(JSON.stringify(types, null, "\t"), "\n");

stopScript();

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
