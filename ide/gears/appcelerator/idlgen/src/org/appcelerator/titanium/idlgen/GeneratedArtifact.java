package org.appcelerator.titanium.idlgen;

public class GeneratedArtifact {

	protected String filename;
	protected StringBuffer contents;
	
	public GeneratedArtifact (String filename, StringBuffer contents) {
		this.filename = filename;
		this.contents = contents;
	}
	
	public String getFilename() {
		return filename;
	}
	public void setFilename(String filename) {
		this.filename = filename;
	}
	public StringBuffer getContents() {
		return contents;
	}
	public void setContents(StringBuffer contents) {
		this.contents = contents;
	}
	
}
