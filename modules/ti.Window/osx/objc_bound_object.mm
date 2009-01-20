/**
 * Appcelerator Titanium - licensed under the Apache Public License 2
 * see LICENSE in the root folder for details on the license.
 * Copyright (c) 2008 Appcelerator, Inc. All Rights Reserved.
 */
#import "objc_bound_object.h"

@implementation ObjcBoundObject

+(id)ValueToID:(Value*)value key:(NSString*)key context:(JSContextRef)context
{
	if (value->IsUndefined())
	{
		return [WebUndefined undefined];
	}
	if (value->IsNull())
	{
		return nil;
	}
	if (value->IsObject())
	{
		return [[ObjcBoundObject alloc] initWithObject:value->ToObject() key:key context:context];
	}
	if (value->IsMethod())
	{
		return [[ObjcBoundObject alloc] initWithObject:value->ToMethod() key:key context:context];
	}
	if (value->IsString())
	{
		return [NSString stringWithCString:value->ToString()];
	}
	if (value->IsInt())
	{
		return [NSNumber numberWithInt:value->ToInt()];
	}
	if (value->IsDouble())
	{
		return [NSNumber numberWithDouble:value->ToDouble()];
	}
	if (value->IsBool())
	{
		return [NSNumber numberWithBool:value->ToBool()];
	}
	if (value->IsList())
	{
		NSMutableArray *result = [[NSMutableArray alloc] init];
		SharedBoundList list = value->ToList();
		for (int c=0;c<list->Size();c++)
		{
			SharedValue v = list->At(c);
			id arg = [ObjcBoundObject ValueToID:v key:[NSString stringWithFormat:@"%@[%d]",key,c] context:context];
			[result addObject:arg];
			[arg release];
		}
		return result;
	}
	return nil;
}

+(SharedValue)IDToValue:(id)arg context:(JSContextRef)context
{
	/**
	 *  Conversion from WebScriptObject
	 *
	 *  JavaScript              ObjC
	 *  ----------              ----------
	 *   null            =>      nil
	 *   undefined       =>      WebUndefined
	 *   number          =>      NSNumber
	 *   boolean         =>      CFBoolean
	 *   string          =>      NSString
	 *   object          =>      id
	 */
	if (arg == nil)
	{
	  return Value::Null;
	}
	if (arg == [WebUndefined undefined])
	{
	  return Value::Undefined;
	}
	if ([arg isKindOfClass:[NSNumber class]])
	{
	  NSNumber *num = (NSNumber*)arg;
	  NSString *type = [NSString stringWithFormat:@"%s",[num objCType]];
	  if ([type isEqualToString:@"c"])
	  {
	    return Value::NewBool((bool)[num boolValue]);
	  }
	  else if ([type isEqualToString:@"d"] || [type isEqualToString:@"f"])
	  {
	    return Value::NewDouble((double)[num doubleValue]);
	  }
	  else
	  {
	    return Value::NewInt((int)[num intValue]);
	  }
	}  
	if ([arg isKindOfClass:[NSString class]])
	{
	  return Value::NewString([(NSString*)arg UTF8String]);
	}
	if ([arg isKindOfClass:[NSArray class]])
	{
		SharedBoundList list = new StaticBoundList;
		NSArray* args = (NSArray*)arg;
		for (int c=0;c<(int)[args count];c++)
		{
			id value = [args objectAtIndex:c];
			SharedValue v = [ObjcBoundObject IDToValue:value context:context];
			list->Append(v);
		}
		return Value::NewList(list);
	}
	if ([arg isKindOfClass:[ObjcBoundObject class]])
	{
		ObjcBoundObject *b = (ObjcBoundObject*)arg;
		SharedBoundObject bound_object = [b boundObject];
		// attempt to up cast to these other types first
		BoundMethod *bound_method = dynamic_cast<BoundMethod*>(bound_object.get());
		if (bound_method!=NULL)
		{
			SharedBoundMethod sbm = bound_method;
			return Value::NewMethod(sbm);
		}
		BoundList *bound_list = dynamic_cast<BoundList*>(bound_object.get());
		if (bound_list!=NULL)
		{
			SharedBoundList sbl = bound_list;
			return Value::NewList(sbl);
		}
		return Value::NewObject(bound_object);
	}
	if ([arg isKindOfClass:[WebScriptObject class]])
	{
		WebScriptObject *script = (WebScriptObject*)arg;
		if ([script respondsToSelector:@selector(isWrappedBoundObject)])
		{
			NSLog(@"===> Wrapped object = %@",script);
			// // script can be a wrapped JS object or one of our 
			// // bound objects
			// BoundObject *bound_object = reinterpret_cast<BoundObject*>(script);
			// // attempt to up cast to these other types first
			// BoundObject *bound_method = dynamic_cast<BoundMethod*>(bound_object);
			// if (bound_method!=NULL)
			// {
			// 	return new Value(bound_method);
			// }
			// BoundList *bound_list = dynamic_cast<BoundList*>(bound_object);
			// if (bound_list!=NULL)
			// {
			// 	return new Value(bound_list);
			// }
			// return new Value(bound_object);
		}
		//FIXME - BoundList
		
		JSObjectRef js = [script JSObject];
		if (JSObjectIsFunction(context,js))
		{
			SharedBoundMethod method = KJSUtil::ToBoundMethod(context,js);
		  	return Value::NewMethod(method);
		}
		SharedBoundObject object = KJSUtil::ToBoundObject(context,js);
	  	return Value::NewObject(object);
	}  
	return Value::Undefined;
}
-(id)initWithObject:(SharedPtr<BoundObject>)obj key:(NSString*)k context:(JSContextRef)ctx
{
	self = [super init];
	if (self!=nil)
	{
		// in objective-c you can't alloc a C++ when an objective-c class 
		// is instantiated
		object = new SharedPtr<BoundObject>(obj);
		key = k;
		context = ctx;
		[key retain];
	}
	return self;
}
-(void)dealloc
{
	[key release];
	delete object;
	context = nil;
	[super dealloc];
}
-(BOOL)isWrappedBoundObject
{
	return YES;
}
-(SharedPtr<BoundObject>)boundObject
{
	SharedPtr<BoundObject> o = object->get();
	return o;
}
-(NSString*)description
{
	SharedValue toString = object->get()->Get("toString");
	if (toString && toString->IsMethod())
	{
		ValueList args;
		SharedValue result = toString->ToMethod()->Call(args);
		return [NSString stringWithCString:result->ToString()];
	}
	return [NSString stringWithFormat:@"[ObjcBoundObject:%@ native]",key];
}
+(BOOL)isKeyExcludedFromWebScript:(const char*)name
{
	return YES;
}
+(BOOL)isSelectorExcludedFromWebScript:(SEL)sel
{
	return YES;
}
+(NSString*)webScriptNameForKey:(const char*)name
{
	return nil;
}
+(NSString*)webScriptNameForSelector:(SEL)sel
{
	return nil;
}
-(void)finalizeForWebScript
{
}
- (void)setValue:(id)value forUndefinedKey:(NSString *)k
{
	SharedValue v = [ObjcBoundObject IDToValue:value context:context];
	object->get()->Set([k UTF8String],v);
}
- (id)valueForUndefinedKey:(NSString *)k
{
	SharedValue result = object->get()->Get([k UTF8String]);
	return [ObjcBoundObject ValueToID:result key:k context:context];
}
-(id)invokeDefaultMethodWithArguments:(NSArray*)args
{
	SharedBoundMethod method = dynamic_cast<BoundMethod*>(object->get());
	if (method)
	{
		ValueList a;
		for (int c=0;c<(int)[args count];c++)
		{
			id value = [args objectAtIndex:c];
			SharedValue arg = [ObjcBoundObject IDToValue:value context:context];
			a.push_back(arg);
		}
		SharedValue result = method->Call(a);
		return [ObjcBoundObject ValueToID:result key:@"" context:context];
	}
	return [WebUndefined undefined];
}
-(id)invokeUndefinedMethodFromWebScript:(NSString *)name withArguments:(NSArray*)args
{
	SharedValue value = object->get()->Get([name UTF8String]);
	if (value->IsMethod())
	{
		ValueList a;
		for (int c=0;c<(int)[args count];c++)
		{
			id value = [args objectAtIndex:c];
			SharedValue arg = [ObjcBoundObject IDToValue:value context:context];
			a.push_back(arg);
		}
		SharedBoundMethod method = value->ToMethod();
		SharedValue result = method->Call(a);
		return [ObjcBoundObject ValueToID:result key:name context:context];
	}
	return [WebUndefined undefined];
}

@end
