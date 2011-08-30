#include "MacUrlHandler.h"
#include "Cosmographia.h"
#include <QDebug>

#import <Foundation/NSAppleEventManager.h>
#import <Foundation/NSNotification.h>
#import <AppKit/NSApplication.h>

@interface EventHandler : NSObject

{
@private
MacUrlHandler* m_handler;
}

@end


@implementation EventHandler

- (id) initWithHandler: (MacUrlHandler*) handler
{
    self = [super init];

    if (self)
    {
        NSLog(@"eventHandler::init");

        NSNotificationCenter* defaultCenter = [NSNotificationCenter defaultCenter];
        [defaultCenter addObserver:self
                       selector:@selector(applicationWillFinishLaunching:)
                       name:nil//NSApplicationDidFinishLaunchingNotification
                       object:nil];
        m_handler = handler;
    }

    return self;
}


- (void) applicationWillFinishLaunching:(NSNotification *) notification
{
    //NSLog(@"Setting up event handler: %@", [notification name]);
    if ([[notification name] isEqualToString: NSApplicationDidFinishLaunchingNotification])
    {
        qDebug() << "Setting up event handler";
        NSAppleEventManager *appleEventManager = [NSAppleEventManager sharedAppleEventManager];
        NSLog(@"Event manager: %@", appleEventManager);
        [appleEventManager setEventHandler:self andSelector:@selector(handleGetURLEvent:withReplyEvent:) forEventClass:kInternetEventClass andEventID:kAEGetURL];
    }

    if ([[notification name] isEqualToString: NSApplicationWillFinishLaunchingNotification] ||
        [[notification name] isEqualToString: NSApplicationDidFinishLaunchingNotification])
    {
        NSAppleEventManager *appleEventManager = [NSAppleEventManager sharedAppleEventManager];
        NSLog(@"Checking apple event manager: %@", appleEventManager);
    }
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

@end


class MacUrlHandler::Private
{
public:
    EventHandler* m_handler;
};


MacUrlHandler::MacUrlHandler() :
    m_privateData(NULL)
{
    EventHandler* eventHandler = [[EventHandler alloc] initWithHandler:this];
    NSLog(@"Initialized event handler: %@", eventHandler);

    m_privateData = new Private();
    m_privateData->m_handler = eventHandler;
}


MacUrlHandler::~MacUrlHandler()
{
    [m_privateData->m_handler release];
}


MacUrlHandler*
MacUrlHandler::Create()
{
    return new MacUrlHandler();
}


void
MacUrlHandler::handleUrl(const QString& url)
{
    m_lastUrl = url;
    emit urlRequested(url);
}
