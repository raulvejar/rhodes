//
//  main.m
//  SyncClientTest
//
//  Created by evgeny vovchenko on 8/19/10.
//  Copyright RhoMobile 2010. All rights reserved.
//

#import <UIKit/UIKit.h>
#import "../../RhoConnectClient.h"

RhoConnectClient* sclient;
RhomModel* product;
RhomModel* perftest;
NSArray* models;

int ResetAndLogout()
{
	[sclient database_full_reset_and_logout];
	if ([sclient is_logged_in]) 
		return 0;

	return 1;
}

int shouldNotSyncWithoutLogin()
{
	return 1;
}

int shouldLogin()
{
	RhoConnectNotify* res = [sclient loginWithUser:@"" pwd:@""];
	int nErr = res.error_code;
	[res release];
	if ( nErr!= RHO_ERR_NONE || ![sclient is_logged_in]) {
		return 0;
	}
	
	return 1;
}

int shouldSyncProductByName()
{
	RhoConnectNotify* res = [product sync];
	int	nErr = res.error_code;
	[res release];
	if ( nErr!= RHO_ERR_NONE ) {
		return 0;
	}

	return 1;
}

int shouldSearchProduct()
{
	return 1;
}

@interface CObjectCallback : NSObject {
}
@property(readonly) RhoConnectObjectNotify* m_pNotify;

- (id) init;
- (void)objectNotifyCallback:(RhoConnectObjectNotify*) notify;
@end

@implementation CObjectCallback
@synthesize  m_pNotify;
- (id) init
{
    self = [super init];
	    
    [sclient setObjectNotification:@selector(objectNotifyCallback:) target:self];        
    return self;
}

- (void)objectNotifyCallback:(RhoConnectObjectNotify*) notify
{
    m_pNotify = notify;
}
@end

int shouldCreateNewProduct()
{
	NSMutableDictionary* item = [[NSMutableDictionary alloc] init];
	[item setValue:@"Test" forKey:@"name"];	
    [item setValue:@"123" forKey:@"sku"];	
	[product create:item];
	if ( [item objectForKey:@"object"] == NULL || [item objectForKey:@"source_id"] == NULL ) 
		return 0;

	NSDictionary* item2 = [product find:[item valueForKey:@"object"]];
	if ( ![item2 isEqualToDictionary: item])
		return 0;

    CObjectCallback* pObjectCallback = [[ CObjectCallback alloc] init];
    [sclient addObjectNotify: [[item objectForKey:@"source_id"] intValue] szObject:[item valueForKey:@"object"] ];
     
	RhoConnectNotify* res = [product sync];
	if ( res.error_code!= RHO_ERR_NONE )
		return 0;

	NSDictionary* item3 = [product find:[item valueForKey:@"object"]];
	if ( item3 )
		return 0;

	if ( !pObjectCallback.m_pNotify.created_objects )
        return 0;

	if ( [pObjectCallback.m_pNotify.created_objects count] <= 0 )
        return 0;
    
	if ( [[pObjectCallback.m_pNotify.created_objects objectAtIndex:0] compare: [item valueForKey:@"object"]] != 0 )
        return 0;
    
	[res release];

	[item3 release];
	[item2 release];
	[item release];
	[pObjectCallback release];
    
	return 1;
}

int shouldModifyProduct()
{
	NSMutableDictionary* cond = [[NSMutableDictionary alloc] init];
	[cond setValue:@"Test" forKey:@"name"];							 
	
	NSMutableDictionary* item = [product find_first:cond];	
	if ( !item )
		return 0;
	
	NSString* saved_object = [NSString stringWithString: [item valueForKey:@"object"]];
	NSMutableString* new_sku = [[NSMutableString alloc]init];
    if ( [item valueForKey:@"sku"] != nil)
    {
        [new_sku appendString:[item valueForKey:@"sku"]];
    }
    
	[new_sku appendString: @"_TEST"];
	
	[item setValue:new_sku forKey:@"sku"];
	[product save: item];
	
	RhoConnectNotify* res = [product sync];
	if ( res.error_code!= RHO_ERR_NONE )
		return 0;
	
	NSDictionary* item3 = [product find:saved_object];
	if ( !item3 )
		return 0;
	
	if ( ![[item3 valueForKey:@"sku"] isEqual: [item valueForKey:@"sku"]])
		return 0;
	
	[cond release];
	[item release];	
	[item3 release];	
	[res release];		
	[new_sku release];				
	
	return 1;
}

int shouldDeleteAllTestProduct()
{
	NSMutableDictionary* cond = [[NSMutableDictionary alloc] init];
	[cond setValue:@"Test" forKey:@"name"];							 
	
	NSMutableArray* items = [product find_all:cond];	
	if ( !items )
		return 0;
	
	for( NSDictionary* item in items)
	{
		[product destroy: item];
	}
	
	RhoConnectNotify* res = [product sync];
	if ( res.error_code!= RHO_ERR_NONE )
		return 0;

	NSMutableDictionary* item = [product find_first:cond];	
	if ( item )
		return 0;

	[items release];
	[res release];
	
	return 1;
}

int shouldPerfomanceTest_create(int nCount)
{
	[perftest startBulkUpdate];	
	for (int i = 0; i < nCount; i++) 
	{
		NSMutableDictionary* item = [[NSMutableDictionary alloc] init];
		[item setValue: [NSString stringWithFormat:@"Test%d", i] forKey:@"name"];							 
		[perftest create:item];
	}
	[perftest stopBulkUpdate];
	
	return 1;
}

int shouldPerfomanceTest_delete()
{
	NSMutableArray* items = [perftest find_all:NULL];	
	if ( !items )
		return 0;
	
	[perftest startBulkUpdate];
	for( NSDictionary* item in items)
	{
		[perftest destroy: item];
	}
	[perftest stopBulkUpdate];
	
	return 1;
}

int runObjCClientTest()
{
	RhomModel* customer = [[RhomModel alloc] init];
	customer.name = @"Customer";
	
	product = [[RhomModel alloc] init];
	product.name = @"Product";
    
    //product.name = @"Product_s";
    //product.model_type = RMT_PROPERTY_FIXEDSCHEMA;    
    //add to schema:
    /*CREATE TABLE "Product_s" ( 
                              "brand" varchar default null,
                              "created_at" varchar default null,
                              "name" varchar default null,
                              "price" varchar default null,
                              "quantity" varchar default null,
                              "sku" varchar default null,
                              "updated_at" varchar default null,
                              "object" varchar(255) PRIMARY KEY );
    
    */
	perftest = [[RhomModel alloc] init];
	perftest.name = @"Perftest";
	perftest.sync_type = RST_NONE;
	
	sclient = [[RhoConnectClient alloc] init];
	models = [NSArray arrayWithObjects:customer, product, perftest, nil];	
	[product release];
	[perftest release];
	[customer release];
	
	[sclient addModels:models];
	
	//[models release];
    //sclient.threaded_mode = FALSE;
	//sclient.poll_interval = 0;
	
	if ( !ResetAndLogout() )
		return 0;

    sclient.sync_server = @"http://rhodes-store-server.heroku.com/application";
 
	if ( !shouldNotSyncWithoutLogin() )
		return 0;

	if ( !shouldLogin() )
		return 0;
	
	if ( !shouldSyncProductByName() )
		return 0;

	if ( !shouldSearchProduct() )
		return 0;
	
	if ( !shouldCreateNewProduct() )
		return 0;
	
	if ( !shouldModifyProduct() )
		return 0;
	
	if ( !shouldDeleteAllTestProduct() )
		return 0;

	if ( !shouldPerfomanceTest_create(100) )
		return 0;
	if ( !shouldPerfomanceTest_delete() )
		return 0;
	
	return 1;
}

int runSyncClientTests();
int main(int argc, char *argv[]) {
    
    NSAutoreleasePool * pool = [[NSAutoreleasePool alloc] init];
	
	[RhoConnectClient initDatabase];
	
//    int retVal = UIApplicationMain(argc, argv, nil, nil);
	
    //int retVal = runSyncClientTests();
	int retVal = runObjCClientTest();
	
	[models release];
//	[product release];
//	[perftest release];
	[sclient release];
	if (retVal)
		NSLog(@"SUCCESS");
	else
		NSLog(@"FAILURE");
	
    [pool release];
    return retVal;
}
