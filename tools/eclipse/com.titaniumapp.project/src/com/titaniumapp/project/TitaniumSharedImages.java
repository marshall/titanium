package com.titaniumapp.project;

import java.net.URL;
import java.util.HashMap;

import org.eclipse.core.runtime.FileLocator;
import org.eclipse.core.runtime.Path;
import org.eclipse.jface.resource.ImageDescriptor;
import org.eclipse.swt.graphics.Image;

public class TitaniumSharedImages {

	public static final String TITANIUM_128 = "titanium.png";
	public static final String TITANIUM_64 = "titanium_64.png";
	public static final String TITANIUM_16 = "titanium_16.png";
	
	protected static HashMap<String, Image> images = new HashMap<String,Image>();
	protected static HashMap<String, ImageDescriptor> descriptors = new HashMap<String,ImageDescriptor>();
	
	protected static void initImage(String id)
	{
		if (!descriptors.containsKey(id)) {
			URL location =
				FileLocator.find(Activator.getDefault().getBundle(), new Path(id), null);
			
			descriptors.put(id, ImageDescriptor.createFromURL(location));
			images.put(id, descriptors.get(id).createImage());
		}
	}
	
	public static Image getImage(String id)
	{
		initImage(id);
		return images.get(id);
	}
	
	public static ImageDescriptor getImageDescriptor(String id)
	{
		initImage(id);
		return descriptors.get(id);
	}
	
	public static void dispose()
	{
		for (Image img : images.values()) {
			img.dispose();
		}

		images.clear();
		descriptors.clear();
	}
}
