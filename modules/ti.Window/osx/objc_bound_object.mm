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
		return [NSString stringWithCString:value->ToString().c_str()];
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
		BoundList *list = value->ToList();
		for (int c=0;c<list->Size();c++)
		{
			Value *v = list->At(c);
			id arg = [ObjcBoundObject ValueToID:v key:[NSString stringWithFormat:@"%@[%d]",key,c] context:context];
			[result addObject:arg];
			[arg release];
		}
		return result;
	}
	return nil;
}

+(Value*)IDToValue:(id)arg context:(JSContextRef)context
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
	  return Value::Null();
	}
	if (arg == [WebUndefined undefined])
	{
	  return Value::Undefined();
	}
	if ([arg isKindOfClass:[NSNumber class]])
	{
	  NSNumber *num = (NSNumber*)arg;
	  NSString *type = [NSString stringWithFormat:@"%s",[num objCType]];
	  if ([type isEqualToString:@"c"])
	  {
	    return new Value((bool)[num boolValue]);
	  }
	  else if ([type isEqualToString:@"d"] || [type isEqualToString:@"f"])
	  {
	    return new Value((double)[num doubleValue]);
	  }
	  else
	  {
	    return new Value((int)[num intValue]);
	  }
	}  
	if ([arg isKindOfClass:[NSString class]])
	{
	  return new Value([(NSString*)arg UTF8String]);
	}
	if ([arg isKindOfClass:[NSArray class]])
	{
		BoundList *list = new StaticBoundList;
		NSArray* args = (NSArray*)arg;
		for (int c=0;c<(int)[args count];c++)
		{
			id value = [args objectAtIndex:c];
			Value *v = [ObjcBoundObject IDToValue:value context:context];
			list->Append(v);
			KR_DECREF(v);
		}
		Value *result = new Value(list);
		KR_DECREF(list);
		return result;
	}
	if ([arg isKindOfClass:[ObjcBoundObject class]])
	{
		ObjcBoundObject *b = (ObjcBoundObject*)arg;
		BoundObject *bound_object = [b boundObject];
		// attempt to up cast to these other types first
		BoundObject *bound_method = dynamic_cast<BoundMethod*>(bound_object);
		if (bound_method!=NULL)
		{
			return new Value(bound_method);
		}
		BoundList *bound_list = dynamic_cast<BoundList*>(bound_object);
		if (bound_list!=NULL)
		{
			return new Value(bound_list);
		}
		return new Value(bound_object);
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
			KJSBoundMethod *method = KJSUtil::ToBoundMethod(context,js);
		  	ScopedDereferencer r(method);
		  	return new Value(method);
		}
		KJSBoundObject *object = KJSUtil::ToBoundObject(context,js);
	  	ScopedDereferencer r(object);
	  	return new Value(object);
	}  
	return Value::Undefined();
}
-(id)initWithObject:(BoundObject*)obj key:(NSString*)k context:(JSContextRef)ctx
{
	self = [super init];
	if (self!=nil)
	{
		object = obj;
		key = k;
		context = ctx;
		[key retain];
		KR_ADDREF(object);
	}
	return self;
}
-(void)dealloc
{
	KR_DECREF(object);
	[key release];
	context = nil;
	[super dealloc];
}
-(BOOL)isWrappedBoundObject
{
	return YES;
}
-(BoundObject*)boundObject
{
	return object;
}
-(NSString*)description
{
	Value *toString = object->Get("toString");
	if (toString && toString->IsMethod())
	{
		ValueList args;
		Value *result = toString->ToMethod()->Call(args);
		ScopedDereferencer r(result);
		return [NSString stringWithCString:result->ToString().c_str()];
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
	KR_DECREF(object);
}
- (void)setValue:(id)value forUndefinedKey:(NSString *)k
{
	Value* v = [ObjcBoundObject IDToValue:value context:context];
	object->Set([k UTF8String],v);
	KR_DECREF(v);
}
- (id)valueForUndefinedKey:(NSString *)k
{
	Value *result = object->Get([k UTF8String]);
	ScopedDereferencer r(result);
	return [ObjcBoundObject ValueToID:result key:k context:context];
}
-(id)invokeDefaultMethodWithArguments:(NSArray*)args
{
	BoundMethod *method = dynamic_cast<BoundMethod*>(object);
	if (method)
	{
		ValueList a;
		for (int c=0;c<(int)[args count];c++)
		{
			id value = [args objectAtIndex:c];
			Value *arg = [ObjcBoundObject IDToValue:value context:context];
			a.push_back(arg);
		}
		Value *result = method->Call(a);
		ScopedDereferencer r(result);
		return [ObjcBoundObject ValueToID:result key:@"" context:context];
	}
	return [WebUndefined undefined];
}
-(id)invokeUndefinedMethodFromWebScript:(NSString *)name withArguments:(NSArray*)args
{
	Value *value = object->Get([name UTF8String]);
	if (value->IsMethod())
	{
		ValueList a;
		for (int c=0;c<(int)[args count];c++)
		{
			id value = [args objectAtIndex:c];
			Value *arg = [ObjcBoundObject IDToValue:value context:context];
			a.push_back(arg);
		}
		BoundMethod *method = value->ToMethod();
		Value *result = method->Call(a);
		ScopedDereferencer r(result);
		return [ObjcBoundObject ValueToID:result key:name context:context];
	}
	return [WebUndefined undefined];
}

@end
