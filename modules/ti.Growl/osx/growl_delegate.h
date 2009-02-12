#import "GrowlApplicationBridge.h"
#import <Foundation/Foundation.h>
#import <Cocoa/Cocoa.h>
#include <kroll/kroll.h>

@interface TiGrowlDelegate : NSObject <GrowlApplicationBridgeDelegate> {
	BOOL growlReady;
};

-(id)init;
-(BOOL) growlReady;
@end

@interface MethodWrapper : NSObject {
	SharedBoundMethod *method;
};

- (id) initWithMethod:(SharedBoundMethod*)m;
- (SharedBoundMethod*) method;
- (void) dealloc;
@end
