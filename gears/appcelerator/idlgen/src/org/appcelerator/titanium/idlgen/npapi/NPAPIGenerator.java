package org.appcelerator.titanium.idlgen.npapi;

import java.util.ArrayList;
import java.util.HashMap;
import java.util.List;
import java.util.Set;
import java.util.TreeSet;

import org.appcelerator.titanium.idlgen.FreemarkerGenerator;
import org.appcelerator.titanium.idlgen.GeneratedArtifact;
import org.appcelerator.titanium.idlgen.IDLAttribute;
import org.appcelerator.titanium.idlgen.IDLInterface;
import org.appcelerator.titanium.idlgen.IDLMethod;

import freemarker.template.TemplateMethodModel;
import freemarker.template.TemplateModelException;

public class NPAPIGenerator extends FreemarkerGenerator {
	@Override
	protected HashMap<String, Object> setupRoot(Object object) {
		HashMap<String, Object> map = super.setupRoot(object);
		map.put("interface", object);
		
		return map;
	}
	
	protected String generateToNPVariant (IDLAttribute attr) {
		return generateToNPVariant(attr.getType(), "instance->_" + attr.getName(), "variant");
	}
	
	protected static final String NPOBJECT_CTYPE = "NPObject *";
	
	public class ToCTypeMethod implements TemplateMethodModel {
		public Object exec(List list) throws TemplateModelException {
			return toCType((String)list.get(0));
		}
	}
	
	protected static String toCType (String type) {
		if (type.equals("string") || type.equals("wstring") || type.equals("DOMString")) {
			return "std::string";
		}
		else if (type.equals("wchar")) {
			return "char";
		}
		else if (type.equals("long") || type.equals("PRInt32")) {
			return "int32";
		}
		else if (type.equals("PRUint16")) {
			return "uint16";
		}
		else if (type.equals("PRUint32") || type.equals("unsigned")) {
			return "uint32";
		}
		else if (type.equals("PRUint64")) {
			return "uint32";
		}
		else if (type.equals("boolean")) {
			return "bool";
		}
		else if (type.equals("void")) {
			return "void";
		}
		else if (type.equals("octet")) {
			return "uint8_t";
		}
		else {
			// Assume an NPObject
			return NPOBJECT_CTYPE;
		}
	}
	
	public class HasNativeImplMethod implements TemplateMethodModel
	{
		public Object exec(List list) throws TemplateModelException {
			return hasNativeImpl((String)list.get(0));
		}
	}
	
	protected static boolean hasNativeImpl (String type) {
		if (type.equals("nsIDOMEvent")) return false;
		else if (type.equals("nsISupports")) return false;
		
		return true;
	}
	
	protected static boolean isReferencedType(String type) {
		return toCType(type).equals(NPOBJECT_CTYPE)
			&& hasNativeImpl(type);
	}
	
	public class GetReferencedInterfacesMethod implements TemplateMethodModel
	{
		public Object exec(List list) throws TemplateModelException {
			return getReferencedInterfaces((IDLInterface)list.get(0));
		}
	}
	
	public static Set<String> getReferencedInterfaces (IDLInterface iface)
	{
		TreeSet<String> ifaces = new TreeSet<String>();
		for (IDLAttribute attr : iface.getAttributes()) {
			if (isReferencedType(attr.getType())) {
				ifaces.add(attr.getType());
			}
		}
		for (IDLMethod method : iface.getMethods()) {
			for (IDLMethod.Argument arg : method.getArguments()) {
				if (isReferencedType(arg.getType())) {
					ifaces.add(arg.getType());
				}
			}
			if (isReferencedType(method.getReturnType())) {
				ifaces.add(method.getReturnType());
			}
		}
		return ifaces;
	}
	
	public class GenerateToNPVariantMethod implements TemplateMethodModel
	{
		public Object exec(List list) throws TemplateModelException {
			return generateToNPVariant((String)list.get(0), (String)list.get(1), (String)list.get(2));
		}
	}
	
	protected String generateToNPVariant (String type, String value, String variant)
	{
		if (type.equals("string") || type.equals("wstring") || type.equals("DOMString")) {
			return "STRINGZ_TO_NPVARIANT("+value+".c_str(), *"+variant+")";
		}
		else if (type.equals("long") || type.startsWith("PRInt") || type.equals("wchar") ||
				type.equals("char") || type.startsWith("PRUint") || type.equals("unsigned") || type.equals("octet")) {
			return "INT32_TO_NPVARIANT("+value+", *"+variant+")";
		}
		else if (type.equals("boolean")) {
			return "BOOLEAN_TO_NPVARIANT("+value+", *"+variant+")";
		}
		else if (type.equals("void")) {
			return "VOID_TO_NPVARIANT(*"+variant+")";
		}
		else {
			// Assume an NPObject
			return "OBJECT_TO_NPVARIANT("+value+", *"+variant+")";
		}
	}
	
	public class GenerateToClassTypeMethod implements TemplateMethodModel
	{
		public Object exec(List list) throws TemplateModelException {
			return generateToClassType((String)list.get(0), (String)list.get(1));
		}
	}
	
	protected String generateToClassType (String type, String value)
	{
		if (type.equals("string") || type.equals("wstring") || type.equals("DOMString")) {
			return "NPStringToString(NPVARIANT_TO_STRING("+value+"))";
		}
		else if (type.equals("long") || type.startsWith("PRInt") || type.startsWith("PRUint") ||
				type.equals("wchar") || type.equals("char") || type.equals("unsigned") || type.equals("octet")) {
			return "NPVARIANT_TO_INT32("+value+")";
		}
		else if (type.equals("boolean")) {
			return "NPVARIANT_TO_BOOLEAN("+value+")";
		}
		else if (type.equals("double")) {
			return "NPVARIANT_TO_DOUBLE("+value+")";
		}
		else {
			// Assume an NPObject
			if (hasNativeImpl(type)) {
				return "(" + type + "*)" + "NPVARIANT_TO_OBJECT("+value+")";
			} else {
				return "NPVARIANT_TO_OBJECT("+value+")";
			}
		}
	}
	
	protected HashMap<String,Object> setupRoot (IDLInterface iface)
	{
		HashMap<String,Object> map = super.setupRoot(iface);
		
		map.put("generateToNPVariant", new GenerateToNPVariantMethod());
		map.put("toCType", new ToCTypeMethod());
		map.put("generateToClassType", new GenerateToClassTypeMethod());
		map.put("hasNativeImpl", new HasNativeImplMethod());
		map.put("getReferencedInterfaces", new GetReferencedInterfacesMethod());
		
		return map;
	}
	
	public StringBuffer generateProxyHeader (IDLInterface iface)
	{
		return processTemplate("npheader.h.fm", iface);
	}
	
	public StringBuffer generateProxy (IDLInterface iface)
	{
		return processTemplate("npimpl.c.fm", iface);
	}
	
	public StringBuffer generateClassHeader (IDLInterface iface)
	{
		return processTemplate("classheader.h.fm", iface);
	}
	
	public StringBuffer generateConstants (IDLInterface iface)
	{
		return processTemplate("constants.js.fm", iface);
	}
	
	public List<GeneratedArtifact> generateArtifacts (List<IDLInterface> ifaces)
	{
		ArrayList<GeneratedArtifact> artifacts = new ArrayList<GeneratedArtifact>();
		
		for (IDLInterface iface : ifaces)
		{
			artifacts.add(new GeneratedArtifact(iface.getName() + "_np.h", generateProxyHeader(iface)));
			artifacts.add(new GeneratedArtifact(iface.getName() + "_np.cpp", generateProxy(iface)));
			artifacts.add(new GeneratedArtifact(iface.getName() + ".h", generateClassHeader(iface)));
			artifacts.add(new GeneratedArtifact(iface.getName() + "_constants.js", generateConstants(iface)));
		}
		
		return artifacts;
	}
}
