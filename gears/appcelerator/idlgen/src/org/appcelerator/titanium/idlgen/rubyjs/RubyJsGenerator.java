package org.appcelerator.titanium.idlgen.rubyjs;

import java.util.ArrayList;
import java.util.List;

import org.appcelerator.titanium.idlgen.FreemarkerGenerator;
import org.appcelerator.titanium.idlgen.GeneratedArtifact;
import org.appcelerator.titanium.idlgen.IDLInterface;

public class RubyJsGenerator extends FreemarkerGenerator {
	public StringBuffer generateHeader (IDLInterface iface)
	{
		return processTemplate("rubyjs.h.fm", iface);
	}
	
	public StringBuffer generateImpl (IDLInterface iface)
	{
		return processTemplate("rubyjs.cc.fm", iface);
	}
	
	public List<GeneratedArtifact> generateArtifacts(IDLInterface iface) {
		ArrayList<GeneratedArtifact> artifacts = new ArrayList<GeneratedArtifact>();
		artifacts.add(new GeneratedArtifact(iface.getName() + ".h", generateHeader(iface)));
		artifacts.add(new GeneratedArtifact(iface.getName() + ".cc", generateImpl(iface)));
		
		return artifacts;
	}

}
