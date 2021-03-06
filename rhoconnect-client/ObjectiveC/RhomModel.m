//
//  RhomModel.m
//  SyncClientTest
//
//  Created by evgeny vovchenko on 8/23/10.
//  Copyright 2010 RhoMobile. All rights reserved.
//

#import "RhomModel.h"
#import "RhoConnectClient/RhoConnectClient.h"
#include "sync/SyncThread.h"
#import "RhoConnectClient.h"

@implementation RhomModel

@synthesize name;
@synthesize sync_type;
@synthesize model_type;
@synthesize associations;

- (id) init
{
	self = [super init];
	sync_type = RST_INCREMENTAL;
	model_type = RMT_PROPERTY_BAG;
    associations = NULL;
    
	return self;
}

- (RhoConnectNotify*) sync
{
    char* res = (char*)rho_sync_doSyncSourceByName([name cStringUsingEncoding:[NSString defaultCStringEncoding]]);
	
    RHO_CONNECT_NOTIFY oNotify = {0};
    rho_connectclient_parsenotify(res, &oNotify);
	rho_sync_free_string(res);
	
	return [[RhoConnectNotify alloc] init: &oNotify];
}

- (void) sync: (SEL) callback target:(id)target
{
	[RhoConnectClient setNotification:callback target:target];

	rho_sync_doSyncSourceByName([name cStringUsingEncoding:[NSString defaultCStringEncoding]]);	 
}

- (void) create: (NSMutableDictionary *) data
{
    unsigned long item = rho_connectclient_hash_create();
	
	for (NSString* key in data) 
	{
		rho_connectclient_hash_put(item, 
			[key cStringUsingEncoding:[NSString defaultCStringEncoding]], 
			[[data objectForKey:key] cStringUsingEncoding:[NSString defaultCStringEncoding]]
		);
	}	
	
    rho_connectclient_create_object([name cStringUsingEncoding:[NSString defaultCStringEncoding]], 
								 item);
    const char* szValue = rho_connectclient_hash_get(item, "object");
	if ( szValue )
		[data setValue: [NSString stringWithUTF8String: szValue] forKey:@"object"];
	else
		[data setValue: @"" forKey:@"object"];
	
    szValue = rho_connectclient_hash_get(item, "source_id");
	if ( szValue )
		[data setValue: [NSString stringWithUTF8String: szValue] forKey:@"source_id"];
	else 
		[data setValue: @"" forKey:@"source_id"];
	
	rho_connectclient_hash_delete(item);
}

int enum_func(const char* szKey, const char* szValue, void* pThis)
{
	NSMutableDictionary* data = (NSMutableDictionary*)pThis;
	[data setValue : [NSString stringWithUTF8String:szValue] forKey : [NSString stringWithUTF8String:szKey]];							 
	return 1;
}

- (NSMutableDictionary *) find: (NSString*)object_id
{
    unsigned long item = rho_connectclient_find(
		[name cStringUsingEncoding:[NSString defaultCStringEncoding]], 
        [object_id cStringUsingEncoding:[NSString defaultCStringEncoding]] );
	
	if ( item == 0 )
		return NULL;
	NSMutableDictionary* data = [[NSMutableDictionary alloc] init];
	rho_connectclient_hash_enumerate(item, &enum_func, data );

	return data;
}

- (NSMutableDictionary *) find_first: (NSDictionary *)cond
{
    unsigned long condhash = rho_connectclient_hash_create();
	for (NSString* key in cond) 
	{
		rho_connectclient_hash_put(condhash, 
								[key cStringUsingEncoding:[NSString defaultCStringEncoding]], 
								[[cond objectForKey:key] cStringUsingEncoding:[NSString defaultCStringEncoding]]
								);
	}	
	
    unsigned long item = rho_connectclient_find_first([name cStringUsingEncoding:[NSString defaultCStringEncoding]], condhash );
    rho_connectclient_hash_delete(condhash);
	
	if ( item == 0 )
		return NULL;
	NSMutableDictionary* data = [[NSMutableDictionary alloc] init];
	rho_connectclient_hash_enumerate(item, &enum_func, data );
	
	return data;
}

- (NSMutableArray *) find_all: (NSDictionary *)cond
{
    unsigned long condhash = rho_connectclient_hash_create();
	if ( cond )
	{
		for (NSString* key in cond) 
		{
			rho_connectclient_hash_put(condhash, 
								[key cStringUsingEncoding:[NSString defaultCStringEncoding]], 
								[[cond objectForKey:key] cStringUsingEncoding:[NSString defaultCStringEncoding]]
								);
		}	
	}
	
    unsigned long items = rho_connectclient_find_all([name cStringUsingEncoding:[NSString defaultCStringEncoding]], condhash );
    rho_connectclient_hash_delete(condhash);
	
	if ( items == 0 )
		return NULL;
	
	int nSize = rho_connectclient_strhasharray_size(items);
	NSMutableArray* arRes = [[NSMutableArray alloc] initWithCapacity:nSize];
    for ( int i = 0; i < nSize; i++ )
    {
		NSMutableDictionary* data = [[NSMutableDictionary alloc] init];
		rho_connectclient_hash_enumerate(rho_connectclient_strhasharray_get(items, i), &enum_func, data );
			
		[arRes addObject:data];
    }
	
	return arRes;
}

- (void) save: (NSDictionary *)data
{
    unsigned long item = rho_connectclient_hash_create();
	
	for (NSString* key in data) 
	{
		rho_connectclient_hash_put(item, 
								[key cStringUsingEncoding:[NSString defaultCStringEncoding]], 
								[[data objectForKey:key] cStringUsingEncoding:[NSString defaultCStringEncoding]]
								);
	}	
	
    rho_connectclient_save( [name cStringUsingEncoding:[NSString defaultCStringEncoding]], item );	
	
    rho_connectclient_hash_delete(item);	
}

- (void) destroy: (NSDictionary *)data
{
    unsigned long item = rho_connectclient_hash_create();
	
	for (NSString* key in data) 
	{
		rho_connectclient_hash_put(item, 
								[key cStringUsingEncoding:[NSString defaultCStringEncoding]], 
								[[data objectForKey:key] cStringUsingEncoding:[NSString defaultCStringEncoding]]
								);
	}	
	
	rho_connectclient_itemdestroy( [name cStringUsingEncoding:[NSString defaultCStringEncoding]], 
		item );
	
    rho_connectclient_hash_delete(item);		
}

- (void) startBulkUpdate
{
	rho_connectclient_start_bulkupdate([name cStringUsingEncoding:[NSString defaultCStringEncoding]]);
}

- (void) stopBulkUpdate
{
	rho_connectclient_stop_bulkupdate([name cStringUsingEncoding:[NSString defaultCStringEncoding]]);
}

@end
