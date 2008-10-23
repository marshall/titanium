package org.appcelerator.titanium.idlgen;

import java.util.ArrayList;
import java.util.Set;
import java.util.TreeSet;

import org.appcelerator.titanium.idlgen.npapi.NPAPIGenerator;

public class IDLInterface {
	protected String name;
	
	protected ArrayList<IDLAttribute> attributes = new ArrayList<IDLAttribute>();
	protected ArrayList<IDLMethod> methods = new ArrayList<IDLMethod>();
	protected ArrayList<IDLAttribute.Constant> constants = new ArrayList<IDLAttribute.Constant>();
	
	public String getName() {
		return name;
	}
	public ArrayList<IDLAttribute.Constant> getConstants() {
		return constants;
	}
	public void setName(String name) {
		this.name = name;
	}
	public ArrayList<IDLAttribute> getAttributes() {
		return attributes;
	}
	public ArrayList<IDLMethod> getMethods() {
		return methods;
	}
	
	
}
