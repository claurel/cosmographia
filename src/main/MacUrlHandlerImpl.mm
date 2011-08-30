#include "MacUrlHandler.h"
#include "Cosmographia.h"

#import <Foundation/NSAppleEventManager.h>
#import <Foundation/NSNotification.h>
#import <AppKit/NSApplication.h>

@interface EventHandler : NSObject
{
@private
MacUrlHandler* m_handler;
}

-(void) setUrlHandler: (MacUrlHandler*) handler;
@end

@implementation EventHandler

- (id)init
{
    self = [super init];

    if (self)
    {
        NSLog(@"eventHandler::init");

        NSNotificationCenter* defaultCenter = [NSNotificationCenter defaultCenter];
        [defaultCenter addObserver:self
                       selector:@selector(applicationDidFinishLaunching:)
                       name:NSApplicationDidFinishLaunchingNotification
                       object:nil];
        m_handler = 0;
    }

    return self;
}


- (void) applicationDidFinishLaunching:(NSNotification *) notification
{
    NSAppleEventManager *appleEventManager = [NSAppleEventManager sharedAppleEventManager];
    [appleEventManager setEventHandler:self andSelector:@selector(handleGetURLEvent:withReplyEvent:) forEventClass:kInternetEventClass andEventID:kAEGetURL];
}


- (void) handleGetURLEvent:(NSAppleEventDescriptor *)event withReplyEvent:(NSAppleEventDescriptor*) replyEvent
{
    NSString* url = [[event paramDescriptorForKeyword:keyDirectObject] stringValue];
    NSLog(@"handleGetURLEvent: %@", url);
    if (m_handler && url)
    {
        const char* utf8Data = [url UTF8String];
        m_handler->handleUrl(QString::fromUtf8(utf8Data));
    }
}

- (void) setUrlHandler: (MacUrlHandler*) handler
{
    m_handler = handler;
}

@end


MacUrlHandler::MacUrlHandler() :
    m_privateData(NULL)
{
    EventHandler* eventHandler = [[EventHandler alloc] init];
    NSLog(@"Initialized event handler: %@", eventHandler);

    m_privateData = eventHandler;
    [eventHandler setUrlHandler:this];
}


MacUrlHandler::~MacUrlHandler()
{
    [(EventHandler*) m_privateData dealloc];
}


MacUrlHandler*
MacUrlHandler::Create()
{
    return new MacUrlHandler();
}


void
MacUrlHandler::handleUrl(const QString& url)
{
    emit urlRequested(url);
}
