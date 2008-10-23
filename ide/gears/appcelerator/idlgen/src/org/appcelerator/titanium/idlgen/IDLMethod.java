package org.appcelerator.titanium.idlgen;

import java.util.ArrayList;

public class IDLMethod {
	protected String name, returnType;
	protected ArrayList<Argument> arguments = new ArrayList<Argument>();
	
	public static class Argument extends IDLAttribute {
		boolean in = false, out = false;
		
		public boolean isIn() { return this.in; }
		public boolean isOut() { return this.out; }
		public void setIn(boolean in) { this.in = in; }
		public void setOut(boolean out) { this.out = out; }
		
	}
	
	public String getName() {
		return name;
	}

	public void setName(String name) {
		this.name = name;
	}

	public String getReturnType() {
		return returnType;
	}

	public void setReturnType(String returnType) {
		this.returnType = returnType;
	}

	public ArrayList<Argument> getArguments() {
		return arguments;
	}
}
