function File(fileString, mode){
	this.id = _wxFile(0, this, fileString);
	this.length = _wxFile(-1, this.id);
	this.nextIndex = 0;
	this.eof = false;
	this.block = "";

	this.getAllText = function(){
		return(_wxFile(1, this.id));
		}
	this.getText = function(start, number){
		return(_wxFile(2, this.id, start, number));
		}
	this.getBytes = function(start, number){
		return(_wxFile(3, this.id, start, number));
		}
	this.getTextLine = function(){
		if (this.eof) return -1;
		blockSize = 5000;
		if (this.block.length == 0){
			if (this.nextIndex < this.length){
				this.block = this.getText(this.nextIndex, blockSize);
				this.nextIndex += this.block.length;
				}
			}
		thisLine = nextLine(this);
		if (thisLine == -1){
			if (this.nextIndex < this.length - 1){
				extension = this.getText(this.nextIndex, blockSize);
				this.block += extension;
				this.nextIndex += extension.length;
				}

			if (this.nextIndex > this.length) this.nextIndex = this.length-1;
			thisLine = nextLine(this);
			if (thisLine == -1){	// no more newlines
				line = this.block;
				this.block = "";
				this.eof = true;
				return line;
				}
			}
		return thisLine;
		}

	this.rewind = function(){
		this.nextIndex = 0;
		this.eof = false;
		this.block = "";
		}

	return this;	// end of constructor ----------------------------

	function nextLine(us){
		nlPos = us.block.indexOf("\n");
		if (nlPos >= 0){
			thisLine = us.block.slice(0, nlPos);
			us.block = us.block.slice(nlPos+1);
			return thisLine;
			}
		else return(-1);
		}
	}
