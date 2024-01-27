function File(fileString, mode){
	if (mode == void 0) mode = 0;	// default to READ
	else if ((mode < 0) || (mode > 5)) throw("File - invalid mode");
	// set up own properties with appropriate access and limit enumerability
	setup = _wxFile(0, this, fileString, mode);
	// we set up attributes in way that avoids them being enumerable and some not writeable
	Object.defineProperty(this, "id", {value: setup.id, enumerable:true,writable:false,});
	Object.defineProperty(this, "fileString", {value: setup.filePath,enumerable:true,writable: false,});
	Object.defineProperty(this, "lbl", {value: false,writable: true});
	Object.defineProperty(this, "lblEof", {value:false,writable: true});
	Object.defineProperty(this, "block", {value:"",writable: true});
/*
	// global variable for file ops if not yet defined
	if (typeof READ == "undefined"){
		Object.defineProperty(globalThis, "READ", {value:0,writable: false});
		Object.defineProperty(globalThis, "WRITE", {value:1,writable: false});
		Object.defineProperty(globalThis, "READ_WRITE", {value:2,writable: false});
		Object.defineProperty(globalThis, "APPEND", {value:3,writable: false});
		Object.defineProperty(globalThis, "WRITE_EXCL", {value:4,writable: false});
		}
*/

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
		if (arguments.length != 1) throw("File.getBytes has just 1 arg");
		this.lbl = false;
		return(_wxFile(6, this.id, number));
		}
	this.getTextLine = function(){
		trace = false;
		blockSize =  5000;
		if (!this.lbl){	// just starting line by line
			if (trace) printOrange("getTextLine first time\n");
			this.lblEof = false;
			this.lbl = true;
			_wxFile(1, this.id, 0);	// rewind
			this.block = _wxFile(5, this.id, blockSize);;			
			}
		if (this.lblEof || (this.length == 0)) return -1;
		thisLine = nextLine(this);
		if (thisLine != -1) return thisLine;
		// reached end of block
		if (this.tell() == this.length()){	// no more blocks
			if (trace) printOrange("No more on disk\n");
			this.lblEof = true;
			return this.block;	// last bit of block
			}
		extension = _wxFile(5, this.id, blockSize);
		if (trace) printOrange("extending - remaining:", this.block.length, "\ttell:", this.tell(), "\textension:", extension.length, "\n");
		this.block += extension;
		thisLine = nextLine(this);
		if (thisLine == -1){	// no more newlines in this block
			if (this.trace) printOrange("no more newlines in this block\n"); 
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
