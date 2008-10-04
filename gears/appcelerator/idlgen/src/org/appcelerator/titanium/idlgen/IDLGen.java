package org.appcelerator.titanium.idlgen;
import java.io.File;
import java.io.FileInputStream;
import java.io.FileOutputStream;
import java.io.InputStream;

import org.appcelerator.titanium.idlgen.gen.IDLLexer;
import org.appcelerator.titanium.idlgen.gen.IDLParser;
import org.appcelerator.titanium.idlgen.gen.IDLTokenTypes;
import org.appcelerator.titanium.idlgen.npapi.NPAPIGenerator;
import org.appcelerator.titanium.idlgen.rubyjs.RubyJsGenerator;

import antlr.collections.AST;

public class IDLGen {

	protected Generator generator;
	
	public IDLInterface parseAST (AST ast)
	{
		IDLInterface iface = new IDLInterface();
		
		for (AST child = ast.getFirstChild(); child != null; child = child.getNextSibling())
		{
			if (child.getType() == IDLTokenTypes.IDENT) {
				if (iface.getName() == null) {
					// It's the interface name
					iface.setName(child.getText());
					continue;
				}
				else {
					// It's a method decl
					AST methodChild = child.getFirstChild();
					if (methodChild != null) {
						IDLMethod method = new IDLMethod();
						String methodName = child.getText();
						String returnType = methodChild.getText();
						method.setName(methodName);
						method.setReturnType(returnType);
						
						for (methodChild = methodChild.getNextSibling(); methodChild != null; methodChild = methodChild.getNextSibling())
						{
							IDLMethod.Argument arg = new IDLMethod.Argument();
							
							AST argChild = methodChild.getFirstChild();
							String inout = methodChild.getText();
							String argType = argChild.getText();
							String argName = argChild.getNextSibling().getText();
							
							if (inout.equals("in")) arg.setIn(true);
							else if (inout.equals("out")) arg.setOut(true);
							arg.setName(argName);
							arg.setType(argType);
							method.getArguments().add(arg);
						}
						
						iface.getMethods().add(method);
						continue;
					}
					
					
				}
			}
			else if (child.getType() == IDLTokenTypes.LITERAL_const) {
				IDLAttribute.Constant constant = new IDLAttribute.Constant();
				AST type = child.getFirstChild();
				constant.setType(type.getText());
				constant.setName(type.getNextSibling().getText());
				
				AST valueAST = type.getNextSibling().getNextSibling();
				String value = valueAST.getText();
				if (value.equals("-")) {
					value += valueAST.getFirstChild().getText();
				}
				
				constant.setValue(value);
				
				iface.getConstants().add(constant);
			}
			else if (child.getType() == IDLTokenTypes.LITERAL_attribute) {
				IDLAttribute attribute = new IDLAttribute();
				
				AST firstChild = child.getFirstChild();
				if (firstChild.getText().equals("readonly")) {
					attribute.setReadOnly(true);
					attribute.setType(firstChild.getNextSibling().getText());
					attribute.setName(firstChild.getNextSibling().getNextSibling().getText());
				} else {
					attribute.setType(firstChild.getText());
					attribute.setName(firstChild.getNextSibling().getText());
				}
				
				iface.getAttributes().add(attribute);
				continue;
			}
		}
		
		return iface;
	}
	
	public static void main(String[] args) {
		try {
			InputStream stream = null;
			Generator generator = null;
			
			if (args.length > 1) {
				if (args[0].equalsIgnoreCase("npapi"))
					generator = new NPAPIGenerator();
				else if (args[0].equalsIgnoreCase("rubyjs"))
					generator = new RubyJsGenerator();
				
				stream = new FileInputStream(args[1]);
			}

			if (generator != null && stream != null) {
				IDLLexer lexer = new IDLLexer(stream);
				IDLParser parser = new IDLParser(lexer);
				parser.specification();
				
				IDLGen idlGen = new IDLGen();
				
				IDLInterface iface = idlGen.parseAST(parser.getAST());
				
				File root = new File(args[2]);
				
				for (GeneratedArtifact artifact : generator.generateArtifacts(iface))
				{
					FileOutputStream out = new FileOutputStream(new File(root, artifact.getFilename()));
					out.write(artifact.getContents().toString().getBytes());
					out.close();
				}
				
			}
		} catch (Exception e) {
			System.err.println("exception: " + e);
		}
	}

}
