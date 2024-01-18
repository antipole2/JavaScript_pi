function File(fileString, mode){
	if ((mode < 0) || (mode > 5)) throw("File - invalid mode");
	this.mode = mode;
	this.id = _wxFile(0, this, fileString, mode);
	this.length = _wxFile(-1, this.id);
	this.lbl = false;	// true if reading line by line
	this.lblEof = false;	// special eof flag when reading line by line
	this.block = "";
	if (this.mode == void 0) this.mode = 0;	// default to read

	this.length = function(){
		return(_wxFile(-1, this.id));
		}
	this.seek = function(offset){
		if (arguments.length != 1) throw("File.seek no offset");
		this.lbl = false;
		return(_wxFile(1, this.id, offset));
		}
	this.tell = function(){
		return(_wxFile(2, this.id));
		}
	this.eof = function(){
		if (this.lbl) return this.lblEof;
		else return(_wxFile(3, this.id));
		}
	this.getAllText = function(){
		this.lbl = false;
		_wxFile(1, this.id, 0);	// rewind
		return(_wxFile(4, this.id));
		}
	this.getText = function(number){
		if (arguments.length != 1) throw("File.getText no count");
		this.lbl = false;
		return(_wxFile(5, this.id, number));
		}
	this.getBytes = function(number){
		if (arguments.length != 1) throw("File.getBytes no count");
		this.lbl = false;
		return(_wxFile(6, this.id, number));
		}
	this.getTextLine = function(){
		if (!this.lbl){	// just starting line by line
			this.length = _wxFile(-1, this.id);
			this.lblEof = false;
			this.lbl = true;
			}
		if (this.lblEof || (this.length == 0)) return -1;
		blockSize =  5000;
		if (this.block.length == 0){
			this.block = _wxFile(5, this.id, blockSize);
			}
		thisLine = nextLine(this);
		if (thisLine != -1) return thisLine;
		extension = _wxFile(5, this.id, blockSize);
		this.block += extension;
		thisLine = nextLine(this);
		if (thisLine == -1){	// no more newlines in this block
			line = this.block;
			this.block = "";
			this.lblEof = true;
			return line;
			}
		return thisLine;
		}
	this.writeText= function(data){
		OK = _wxFile(7, this.id, data);
		return OK;
		}
	this.writeBytes= function(data){
		OK = _wxFile(8, this.id, data);
		return OK;
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
