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
			// don't release!
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
		SharedBoundMethod method = bound_object.cast<BoundMethod>();
		if (!method.isNull())
		{
			return Value::NewMethod(method);
		}
		SharedBoundList list = bound_object.cast<BoundList>();
		if (!list.isNull())
		{
			return Value::NewList(list);
		}
		return Value::NewObject(bound_object);
	}
	if ([arg isKindOfClass:[WebScriptObject class]])
	{
		WebScriptObject *script = (WebScriptObject*)arg;
		JSObjectRef js = [script JSObject];
		if (JSObjectIsFunction(context,js))
		{
			SharedBoundMethod method = KJSUtil::ToBoundMethod(context,js,NULL);
		  	return Value::NewMethod(method);
		}
		else if (KJSUtil::IsArrayLike(js,context))
		{
			SharedBoundList list = KJSUtil::ToBoundList(context,js);
		  	return Value::NewList(list);
		}
		SharedBoundObject object = KJSUtil::ToBoundObject(context,js);
	  	return Value::NewObject(object);
	}  
	return Value::Undefined;
}
-(id)initWithObject:(SharedBoundObject)obj key:(NSString*)k context:(JSContextRef)ctx
{
	self = [super init];
	if (self!=nil)
	{
		// in objective-c you can't alloc a C++ when an objective-c class 
		// is instantiated
		object = new SharedBoundObject(obj);
		key = [[NSString alloc] initWithString:k];
		context = ctx;
	}
	return self;
}
-(void)dealloc
{
	KR_DUMP_LOCATION
	
	[key release];
	if (object!=nil)
	{
		delete object;
		object = nil;
	}
	context = nil;
	[super dealloc];
}
-(BOOL)isWrappedBoundObject
{
	return YES;
}
-(SharedPtr<BoundObject>)boundObject
{
	SharedPtr<BoundObject> o(*object);
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
	return [NSString stringWithFormat:@"[%@ native]",key];
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
	KR_DUMP_LOCATION
	NSLog(@"Finalize: %@, count=%d",key,[self retainCount]);
	[self release]; // we can now release the reference that the JS layer held
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
-(id)invoke:(const ValueList&)args name:(NSString*)name method:(SharedBoundMethod)method
{
	SharedValue m = (*object)->Get([name UTF8String]);
	if (!m->IsMethod())
	{
		NSString *err = [NSString stringWithFormat:@"no method named: %@",name];
		[WebScriptObject throwException:err];
		return [WebUndefined undefined];
	}
	Value exception;
	ti::BoundMethodDispatch bmd(m->ToMethod());
	SharedValue result = bmd.Call(args,&exception);
	if (exception.IsString())
	{
		NSString *err = [NSString stringWithCString:exception.ToString()];
		[WebScriptObject throwException:err];
	}
	else
	{
		return [ObjcBoundObject ValueToID:result key:name context:context];
	}
	return [WebUndefined undefined];
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
		return [self invoke:a name:@"" method:method];
	}
	return [WebUndefined undefined];
}
-(id)invokeUndefinedMethodFromWebScript:(NSString *)name withArguments:(NSArray*)args
{
	NSLog(@"invoking method %@ on %@",name,key);
	SharedValue value = (*object)->Get([name UTF8String]);
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
		return [self invoke:a name:name method:method];
	}
	return [WebUndefined undefined];
}

@end
