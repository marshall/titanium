package org.appcelerator.titanium.idlgen;

import java.io.IOException;
import java.io.StringWriter;
import java.util.HashMap;

import freemarker.template.Configuration;
import freemarker.template.DefaultObjectWrapper;
import freemarker.template.Template;
import freemarker.template.TemplateException;

public abstract class FreemarkerGenerator implements Generator {
	
	protected Configuration configuration;
	
	public FreemarkerGenerator ()
	{
		configuration = new Configuration();
		configuration.setClassForTemplateLoading(getClass(), "");
		configuration.setObjectWrapper(new DefaultObjectWrapper());
	}
	
	protected HashMap<String,Object> setupRoot (Object object)
	{
		HashMap<String,Object> map = new HashMap<String,Object>();
		//map.put("interface", iface);
		
		return map;
	}
	
	protected StringBuffer processTemplate (String path, Object object)
	{
		try {
			Template template = configuration.getTemplate(path);
			StringWriter writer = new StringWriter();
			
			template.process(setupRoot(object), writer);
			
			return writer.getBuffer();
		} catch (IOException e) {
			// TODO Auto-generated catch block
			e.printStackTrace();
		} catch (TemplateException e) {
			// TODO Auto-generated catch block
			e.printStackTrace();
		}
		
		return null;
	}

}
