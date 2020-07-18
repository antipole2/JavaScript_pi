module.exports = function Boat(_name, _make, _length, _beam, _draft){
	this.name = _name;
	this.make = _make;
	this.length = _length;
	this.beam = _beam;
	this.draft = _draft;
	this.deckArea = function() {return(this.length * this.beam * 0.6)}	
	}
	
Boat.prototype.summary = function(){
	return(this.name + " is an " + this.make);
	}
