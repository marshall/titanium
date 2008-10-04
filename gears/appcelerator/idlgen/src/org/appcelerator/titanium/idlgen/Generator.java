package org.appcelerator.titanium.idlgen;

import java.util.List;

public interface Generator {

	public List<GeneratedArtifact> generateArtifacts(IDLInterface iface);
}
