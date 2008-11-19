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

bool stringToBool (std::string str) {
	std::transform(str.begin(), str.end(), str.begin(), tolower);
	if (str == "yes" || str == "true" || str == "on") {
		return true;
	}
	return false;
}

#define boolValue(n) (stringToBool(std::string(nodeValue(n))))

void TiApp::readIconElement(IXMLDOMElementPtr element)
{
	if (strcmp(element->nodeName, "image16") == 0) {
		icon16 = nodeValue(element);
	}
	else if (strcmp(element->nodeName, "image32") == 0) {
		icon32 = nodeValue(element);
	}
	else if (strcmp(element->nodeName, "image48") == 0) {
		icon48 = nodeValue(element);
	}
}

void TiApp::readWindowElement(IXMLDOMElementPtr element)
{
	if (strcmp(element->nodeName, "title") == 0) {
		title = nodeValue(element);
	}
	else if (strcmp(element->nodeName, "start") == 0) {
		startPath = nodeValue(element);
	}
	else if (strcmp(element->nodeName, "maximizable") == 0) {
		maximizable = boolValue(element);
	}
	else if (strcmp(element->nodeName, "minimizable") == 0) {
		minimizable = boolValue(element);
	}
	else if (strcmp(element->nodeName, "closeable") == 0) {
		closeable = boolValue(element);
	}
	else if (strcmp(element->nodeName, "resizable") == 0) {
		resizable = boolValue(element);
	}
	else if (strcmp(element->nodeName, "chrome") == 0) {
		usingChrome = boolValue(element);
		IXMLDOMAttributePtr scrollbars = element->getAttributeNode("scrollbars");
		if (scrollbars != NULL) {
			usingScrollbars = boolValue(scrollbars);
		}
	}
	else if (strcmp(element->nodeName, "transparency") == 0) {
		transparency = (float)atof(nodeValue(element));
	}
}

void TiApp::readElement(IXMLDOMElementPtr element)
{
	if (strcmp(element->nodeName, "name") == 0) {
		appName = nodeValue(element);
	}
	else if (strcmp(element->nodeName, "description") == 0) {
		description = nodeValue(element);
	}
	else if (strcmp(element->nodeName, "copyright") == 0) {
		copyright = nodeValue(element);
	}
	else if (strcmp(element->nodeName, "homepage") == 0) {
		homepage = nodeValue(element);
	}
	else if (strcmp(element->nodeName, "version") == 0) {
		version = nodeValue(element);
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
					IXMLDOMElementPtr element2 = (IXMLDOMElementPtr) child;
					this->readWindowElement(element2);
				} break;
			}
			child = child->nextSibling;
		}
	}
	else if (strcmp(element->nodeName, "icon") == 0) {
		IXMLDOMNodePtr child = element->firstChild;
		while (child != NULL) 
		{
			switch (child->nodeType) {
				case MSXML2::NODE_ELEMENT: {
					IXMLDOMElementPtr element2 = (IXMLDOMElementPtr) child;
					this->readIconElement(element2);
				} break;
			}
			child = child->nextSibling;
		}
	}
}

TiApp::TiApp(std::wstring& xmlfile) :
	maximizable(true), minimizable(true),
	closeable(true), resizable(true),
	usingChrome(false), usingScrollbars(true), transparency(1.0),
	width(800), height(600)
{
	::CoInitialize(NULL);

	IXMLDOMDocument2Ptr doc;
	HRESULT result = doc.CreateInstance(__uuidof(DOMDocument60));

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