function makeBits(offset, length, start){
	return({"BitOffset":offset, "BitLength":length,"BitStart":start});
	}

function matchBits(aBits, bBits){
	if (aBits.BitOffset != bBits.BitOffset) return aBits.BitOffset-bBits.BitOffset;
	if (aBits.BitLength != bBits.BitLength) return aBits.BitLength-bBits.BitLength;
	return aBits.BitStart-bBits.BitStart;
	}

function getBitArray	// return one bit array from buffer
	(buff,	// the data buffer
	offset,	// offset in bits to start of what we need
	bitLength,// number of bits in data
	start	// offset from start
	){
	output = [];	// binary array to return
	byteOffset = Math.floor(offset/8);
	data = buff[byteOffset];
	bits = 8-start;	// bits in first byte
	data = data << start;
printOrange(data, "\n");
	for (b = bits; b >= 0; b--){
		output.push((data & 0x8) >> 7);
		data << 1;
		}
	return output;
	}

buffer = [0x1E];
print(getBitArray(buffer, 0, 4, 0), "\n");





	