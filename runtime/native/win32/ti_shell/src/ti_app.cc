/*
* Copyright 2006-2008 Appcelerator, Inc.
*
* Licensed under the Apache License, Version 2.0 (the "License");
* you may not use this file except in compliance with the License.
* You may obtain a copy of the License at
*
* http://www.apache.org/licenses/LICENSE-2.0
*
* Unless required by applicable law or agreed to in writing, software
* distributed under the License is distributed on an "AS IS" BASIS,
* WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
* See the License for the specific language governing permissions and
* limitations under the License.
*/
#include <windows.h>
#include <atlbase.h>
#include <commctrl.h>
#include <commdlg.h>

#include "ti_app.h"

#define nodeValue(n) (n->nodeType == MSXML2::NODE_ATTRIBUTE ? _bstr_t(n->nodeValue) : _bstr_t(n->firstChild->nodeValue))

void TiApp::readElement(IXMLDOMElementPtr element)
{
	if (strcmp(element->nodeName, "name") == 0) {
		appName = nodeValue(element);
	}
	else if (strcmp(element->nodeName, "window") == 0) {
		IXMLDOMAttributePtr widthAttr = element->getAttributeNode("width");
		IXMLDOMAttributePtr heightAttr = element->getAttributeNode("height");
		width = atoi(nodeValue(widthAttr));
		height = atoi(nodeValue(heightAttr));

		IXMLDOMNodePtr child = element->firstChild;
		while (child != NULL) 
		{
			switch (child->nodeType) {
				case MSXML2::NODE_ELEMENT: {
					IXMLDOMElementPtr element = (IXMLDOMElementPtr) child;
					if (strcmp(element->nodeName, "title") == 0) {
						title = nodeValue(child);
					}
					else if (strcmp(element->nodeName, "start") == 0) {
						startPath = nodeValue(child);
					}
				} break;
			}
			child = child->nextSibling;
		}
	}
}

TiApp::TiApp(std::wstring& xmlfile)
{
	::CoInitialize(NULL);

	IXMLDOMDocument2Ptr doc;
	HRESULT result = doc.CreateInstance(__uuidof(DOMDocument40));

	doc->async = VARIANT_FALSE;

	if (doc->load(xmlfile.c_str())) {
		IXMLDOMElementPtr root = doc->documentElement;
		IXMLDOMNodePtr child = root->firstChild;
		while (child != NULL) 
		{
			switch (child->nodeType) {
				case MSXML2::NODE_ELEMENT:
					IXMLDOMElementPtr element = (IXMLDOMElementPtr) child;
					readElement(element);
					break;
			}
			child = child->nextSibling;
		}
	}
}

TiApp::~TiApp()
{
	::CoUninitialize();
}