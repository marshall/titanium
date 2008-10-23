package org.appcelerator.titanium.idlgen.rubyjs;

import java.util.ArrayList;
import java.util.HashMap;
import java.util.List;

import org.appcelerator.titanium.idlgen.FreemarkerGenerator;
import org.appcelerator.titanium.idlgen.GeneratedArtifact;
import org.appcelerator.titanium.idlgen.IDLInterface;

public class RubyJsGenerator extends FreemarkerGenerator {
	public static final String FILENAME = "app_command";
	
	@Override
	protected HashMap<String, Object> setupRoot(Object object) {
		HashMap<String, Object> map = super.setupRoot(object);
		map.put("interfaces", object);
		
		return map;
	}
	
	public StringBuffer generateHeader (List<IDLInterface> ifaces)
	{
		return processTemplate("rubyjs.h.fm", ifaces);
	}
	
	public StringBuffer generateImpl (List<IDLInterface> ifaces)
	{
		return processTemplate("rubyjs.cc.fm", ifaces);
	}
	
	public List<GeneratedArtifact> generateArtifacts(List<IDLInterface> ifaces) {
		ArrayList<GeneratedArtifact> artifacts = new ArrayList<GeneratedArtifact>();
		artifacts.add(new GeneratedArtifact(FILENAME + ".h", generateHeader(ifaces)));
		artifacts.add(new GeneratedArtifact(FILENAME + ".cc", generateImpl(ifaces)));
		
		return artifacts;
	}

}
