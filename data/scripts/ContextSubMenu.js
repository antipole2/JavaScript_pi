function contextSubMenu(){
	this.subMenu = [];
	this.add = function(call, menuName){
		if (typeof call !== 'function') throw("contextSubMenu first paramter must be callable"); 
		for ( var i = 0; i < this.subMenu.length; i++){
			if (menuName == this.subMenu[i].menuName) throw("contextSubMenu duplicate name " + menuName);
			}
		addition = {call:call, menuName:menuName};
		this.subMenu.push(addition);
		}
	return this;
	};