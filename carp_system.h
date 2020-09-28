#ifndef CARP_SYSTEM_INCLUDED
#define CARP_SYSTEM_INCLUDED (1)

#ifdef __APPLE__
#include <TargetConditionals.h>
#endif
#include <string.h>
#include <stdio.h>

#ifdef _WIN32
#include <Windows.h>
#include <Iphlpapi.h>
#pragma comment(lib, "Iphlpapi.lib")
#elif __ANDROID__
#include <sys/system_properties.h>
#endif

#include "sokol/sokol_app.h"

/* Set up for C function definitions, even when using C++ */
#ifdef __cplusplus
extern "C" {
#endif

extern void CARP_GetPlatform(char* buffer, size_t len);
extern int CARP_GetScreenWidth();
extern int CARP_GetScreenHeight();
extern void CARP_Alert(const char* message);
extern void CARP_InstallProgram(const char* file_path);

#ifdef __cplusplus
}
#endif
#endif

#ifndef CARP_SYSTEM_IMPL
#ifndef CARP_SYSTEM_IMPL_INCLUDE
#define CARP_SYSTEM_IMPL_INCLUDE

wchar_t* _carp_system_UTF82Unicode(const char* utf8)
{
	int len = MultiByteToWideChar(CP_UTF8, 0, utf8, -1, NULL, 0);
	wchar_t* result = 0;
	if (len >= 1) result = (wchar_t*)malloc(len * sizeof(wchar_t));
	MultiByteToWideChar(CP_UTF8, 0, utf8, -1, result, len);
	return result;
}

char* _carp_system_Unicode2UTF8(const wchar_t* unicode)
{
	int len = WideCharToMultiByte(CP_UTF8, 0, unicode, -1, NULL, 0, NULL, NULL);
	char* result = 0;
	if (len >= 1) result = (char*)malloc(len);
	WideCharToMultiByte(CP_UTF8, 0, unicode, -1, result, len, NULL, NULL);
	return result;
}

void CARP_GetPlatform(char* buffer, size_t len)
{
#ifdef _WIN32
	strcpy_s(buffer, len, "Windows");
#elif __ANDROID__
	strcpy(buffer, "Andoird");
#elif __APPLE__
#ifdef TARGET_OS_IPHONE
	strcpy(buffer, "iOS");
#else
	strcpy(buffer, "Mac");
#endif
#elif __EMSCRIPTEN__
	strcpy(buffer, "Emscripten");
#else
	strcpy(buffer, "");
#endif
}


#ifdef __APPLE__
#import <UIKit/UIKit.h>

/*
	The KeychainItemWrapper class is an abstraction layer for the iPhone Keychain communication. It is merely a
	simple wrapper to provide a distinct barrier between all the idiosyncracies involved with the Keychain
	CF/NS container objects.
*/
@interface KeychainItemWrapper : NSObject
{
	NSMutableDictionary* keychainItemData;		// The actual keychain item data backing store.
	NSMutableDictionary* genericPasswordQuery;	// A placeholder for the generic keychain item query used to locate the item.
}

@property(nonatomic, retain) NSMutableDictionary* keychainItemData;
@property(nonatomic, retain) NSMutableDictionary* genericPasswordQuery;

// Designated initializer.
-(id)initWithIdentifier: (NSString*)identifier accessGroup : (NSString*)accessGroup;
-(void)setObject:(id)inObject forKey : (id)key;
-(id)objectForKey:(id)key;

// Initializes and resets the default generic keychain item data.
-(void)resetKeychainItem;

@end


@interface KeychainItemWrapper(PrivateMethods)
/*
The decision behind the following two methods (secItemFormatToDictionary and dictionaryToSecItemFormat) was
to encapsulate the transition between what the detail view controller was expecting (NSString *) and what the
Keychain API expects as a validly constructed container class.
*/
-(NSMutableDictionary*)secItemFormatToDictionary:(NSDictionary*)dictionaryToConvert;
-(NSMutableDictionary*)dictionaryToSecItemFormat:(NSDictionary*)dictionaryToConvert;

// Updates the item in the keychain, or adds it if it doesn't exist.
-(void)writeToKeychain;

@end

@implementation KeychainItemWrapper

@synthesize keychainItemData, genericPasswordQuery;

-(id)initWithIdentifier: (NSString*)identifier accessGroup : (NSString*)accessGroup;
{
	if (self = [super init])
	{
		// Begin Keychain search setup. The genericPasswordQuery leverages the special user
		// defined attribute kSecAttrGeneric to distinguish itself between other generic Keychain
		// items which may be included by the same application.
		genericPasswordQuery = [[NSMutableDictionary alloc]init];

		[genericPasswordQuery setObject : (id)kSecClassGenericPassword forKey : (id)kSecClass] ;
		[genericPasswordQuery setObject : identifier forKey : (id)kSecAttrGeneric] ;

		// The keychain access group attribute determines if this item can be shared
		// amongst multiple apps whose code signing entitlements contain the same keychain access group.
		if (accessGroup != nil)
		{
#if TARGET_IPHONE_SIMULATOR
			// Ignore the access group if running on the iPhone simulator.
			// 
			// Apps that are built for the simulator aren't signed, so there's no keychain access group
			// for the simulator to check. This means that all apps can see all keychain items when run
			// on the simulator.
			//
			// If a SecItem contains an access group attribute, SecItemAdd and SecItemUpdate on the
			// simulator will return -25243 (errSecNoAccessForItem).
#else			
			[genericPasswordQuery setObject : accessGroup forKey : (id)kSecAttrAccessGroup];
#endif
		}

		// Use the proper search constants, return only the attributes of the first match.
		[genericPasswordQuery setObject : (id)kSecMatchLimitOne forKey : (id)kSecMatchLimit];
		[genericPasswordQuery setObject : (id)kCFBooleanTrue forKey : (id)kSecReturnAttributes] ;

		NSDictionary* tempQuery = [NSDictionary dictionaryWithDictionary : genericPasswordQuery];

		NSMutableDictionary* outDictionary = nil;

		if (!SecItemCopyMatching((CFDictionaryRef)tempQuery, (CFTypeRef*)&outDictionary) == noErr)
		{
			// Stick these default values into keychain item if nothing found.
			[self resetKeychainItem] ;

			// Add the generic attribute and the keychain access group.
			[keychainItemData setObject : identifier forKey : (id)kSecAttrGeneric] ;
			if (accessGroup != nil)
			{
#if TARGET_IPHONE_SIMULATOR
				// Ignore the access group if running on the iPhone simulator.
				// 
				// Apps that are built for the simulator aren't signed, so there's no keychain access group
				// for the simulator to check. This means that all apps can see all keychain items when run
				// on the simulator.
				//
				// If a SecItem contains an access group attribute, SecItemAdd and SecItemUpdate on the
				// simulator will return -25243 (errSecNoAccessForItem).
#else			
				[keychainItemData setObject : accessGroup forKey : (id)kSecAttrAccessGroup];
#endif
			}
		}
		else
		{
			// load the saved data from Keychain.
			self.keychainItemData = [self secItemFormatToDictionary : outDictionary];
		}

		[outDictionary release];
	}

	return self;
}

- (void)dealloc
{
	[keychainItemData release] ;
	[genericPasswordQuery release] ;

	[super dealloc] ;
}

- (void)setObject:(id)inObject forKey : (id)key
{
	if (inObject == nil) return;
	id currentObject = [keychainItemData objectForKey : key];
	if (![currentObject isEqual : inObject])
	{
		[keychainItemData setObject : inObject forKey : key] ;
		[self writeToKeychain] ;
	}
}

- (id)objectForKey:(id)key
{
	return[keychainItemData objectForKey : key];
}

- (void)resetKeychainItem
{
	OSStatus junk = noErr;
	if (!keychainItemData)
	{
		self.keychainItemData = [[NSMutableDictionary alloc]init];
	}
	else if (keychainItemData)
	{
		NSMutableDictionary* tempDictionary = [self dictionaryToSecItemFormat : keychainItemData];
		junk = SecItemDelete((CFDictionaryRef)tempDictionary);
		NSAssert(junk == noErr || junk == errSecItemNotFound, @"Problem deleting current dictionary.");
	}

	// Default attributes for keychain item.
	[keychainItemData setObject : @"" forKey:(id)kSecAttrAccount] ;
	[keychainItemData setObject : @"" forKey:(id)kSecAttrLabel] ;
	[keychainItemData setObject : @"" forKey:(id)kSecAttrDescription] ;

	// Default data for keychain item.
	[keychainItemData setObject : @"" forKey:(id)kSecValueData] ;
}

- (NSMutableDictionary*)dictionaryToSecItemFormat:(NSDictionary*)dictionaryToConvert
{
	// The assumption is that this method will be called with a properly populated dictionary
	// containing all the right key/value pairs for a SecItem.

	// Create a dictionary to return populated with the attributes and data.
	NSMutableDictionary* returnDictionary = [NSMutableDictionary dictionaryWithDictionary : dictionaryToConvert];

	// Add the Generic Password keychain item class attribute.
	[returnDictionary setObject : (id)kSecClassGenericPassword forKey : (id)kSecClass] ;

	// Convert the NSString to NSData to meet the requirements for the value type kSecValueData.
	// This is where to store sensitive data that should be encrypted.
	NSString* passwordString = [dictionaryToConvert objectForKey : (id)kSecValueData];
	[returnDictionary setObject : [passwordString dataUsingEncoding : NSUTF8StringEncoding] forKey : (id)kSecValueData] ;

	return returnDictionary;
}

- (NSMutableDictionary*)secItemFormatToDictionary:(NSDictionary*)dictionaryToConvert
{
	// The assumption is that this method will be called with a properly populated dictionary
	// containing all the right key/value pairs for the UI element.

	// Create a dictionary to return populated with the attributes and data.
	NSMutableDictionary* returnDictionary = [NSMutableDictionary dictionaryWithDictionary : dictionaryToConvert];

	// Add the proper search key and class attribute.
	[returnDictionary setObject : (id)kCFBooleanTrue forKey : (id)kSecReturnData] ;
	[returnDictionary setObject : (id)kSecClassGenericPassword forKey : (id)kSecClass] ;

	// Acquire the password data from the attributes.
	NSData* passwordData = NULL;
	if (SecItemCopyMatching((CFDictionaryRef)returnDictionary, (CFTypeRef*)&passwordData) == noErr)
	{
		// Remove the search, class, and identifier key/value, we don't need them anymore.
		[returnDictionary removeObjectForKey : (id)kSecReturnData] ;

		// Add the password to the dictionary, converting from NSData to NSString.
		NSString* password = [[[NSString alloc]initWithBytes:[passwordData bytes] length : [passwordData length]
			encoding : NSUTF8StringEncoding] autorelease];
		[returnDictionary setObject : password forKey : (id)kSecValueData] ;
	}
	else
	{
		// Don't do anything if nothing is found.
		NSAssert(NO, @"Serious error, no matching item found in the keychain.\n");
	}

	[passwordData release] ;

	return returnDictionary;
}

- (void)writeToKeychain
{
	NSDictionary* attributes = NULL;
	NSMutableDictionary* updateItem = NULL;
	OSStatus result;

	if (SecItemCopyMatching((CFDictionaryRef)genericPasswordQuery, (CFTypeRef*)&attributes) == noErr)
	{
		// First we need the attributes from the Keychain.
		updateItem = [NSMutableDictionary dictionaryWithDictionary : attributes];
		// Second we need to add the appropriate search key/values.
		[updateItem setObject : [genericPasswordQuery objectForKey : (id)kSecClass] forKey : (id)kSecClass] ;

		// Lastly, we need to set up the updated attribute list being careful to remove the class.
		NSMutableDictionary* tempCheck = [self dictionaryToSecItemFormat : keychainItemData];
		[tempCheck removeObjectForKey : (id)kSecClass] ;

#if TARGET_IPHONE_SIMULATOR
		// Remove the access group if running on the iPhone simulator.
		// 
		// Apps that are built for the simulator aren't signed, so there's no keychain access group
		// for the simulator to check. This means that all apps can see all keychain items when run
		// on the simulator.
		//
		// If a SecItem contains an access group attribute, SecItemAdd and SecItemUpdate on the
		// simulator will return -25243 (errSecNoAccessForItem).
		//
		// The access group attribute will be included in items returned by SecItemCopyMatching,
		// which is why we need to remove it before updating the item.
		[tempCheck removeObjectForKey : (id)kSecAttrAccessGroup];
#endif

		// An implicit assumption is that you can only update a single item at a time.

		result = SecItemUpdate((CFDictionaryRef)updateItem, (CFDictionaryRef)tempCheck);
		NSAssert(result == noErr, @"Couldn't update the Keychain Item.");
	}
	else
	{
		// No previous item found; add the new one.
		result = SecItemAdd((CFDictionaryRef)[self dictionaryToSecItemFormat : keychainItemData], NULL);
		NSAssert(result == noErr, @"Couldn't add the Keychain Item.");
	}
}

@end

#endif

void CARP_GetDeviceID(char* buffer, size_t len)
{
#ifdef _WIN32	
	strcpy_s(buffer, len, "");
	PIP_ADAPTER_INFO pAdapterInfo;
	PIP_ADAPTER_INFO pAdapter = NULL;
	DWORD dwRetVal = 0;

	// alloc memory
	ULONG ulOutBufLen = sizeof(IP_ADAPTER_INFO);
	pAdapterInfo = (IP_ADAPTER_INFO*)HeapAlloc(GetProcessHeap(), 0, sizeof(IP_ADAPTER_INFO));
	if (pAdapterInfo == NULL) return;

	// get info, alloc memory
	if (GetAdaptersInfo(pAdapterInfo, &ulOutBufLen) == ERROR_BUFFER_OVERFLOW) {
		HeapFree(GetProcessHeap(), 0, pAdapterInfo);
		pAdapterInfo = (IP_ADAPTER_INFO*)HeapAlloc(GetProcessHeap(), 0, ulOutBufLen);
		if (pAdapterInfo == NULL) return;
	}
	// get info
	if (GetAdaptersInfo(pAdapterInfo, &ulOutBufLen) == NO_ERROR) {
		pAdapter = pAdapterInfo;
		while (pAdapter) {
			if (pAdapter->Type == MIB_IF_TYPE_ETHERNET) {
				for (UINT i = 0; i < pAdapter->AddressLength; ++i) {
					char text[32] = { 0 };
					sprintf_s(text, "%.2X", (int)pAdapter->Address[i]);
					strcat_s(buffer, len, text);
				}
				break;
			}
			pAdapter = pAdapter->Next;
		}
	}
	// release
	if (pAdapterInfo)
		HeapFree(GetProcessHeap(), 0, pAdapterInfo);
#elif __ANDROID__
	strcpy(buffer, "");
	char info[256] = { 0 };
	__system_property_get("ro.product.model", info);
	strcat(buffer, text);
	__system_property_get("ro.serialno", info);
	strcat(buffer, text);
#elif __APPLE_
	strcpy(buffer, "");
	KeychainItemWrapper* keyChainWrapper = [[KeychainItemWrapper alloc]initWithIdentifier:@"ALittle" accessGroup:nil];
	NSString* UUID = [keyChainWrapper objectForKey : (id)kSecValueData];

	if (UUID == nil || UUID.length == 0) {
		UUID = [[[UIDevice currentDevice]identifierForVendor] UUIDString];
		[keyChainWrapper setObject : UUID forKey : (id)kSecAttrAccount] ;
		[keyChainWrapper setObject : UUID forKey : (id)kSecValueData] ;
	}
	[keyChainWrapper release];

	info = [UUID UTF8String];
	strcpy(buffer, [UUID UTF8String]);	
#endif
}

void CARP_InstallProgram(const char* file_path)
{
#ifdef _WIN32
	STARTUPINFO si;
	PROCESS_INFORMATION pi;
	ZeroMemory(&si, sizeof(si));
	si.cb = sizeof(si);
	ZeroMemory(&pi, sizeof(pi));
	CreateProcess(NULL, (LPTSTR)file_path, NULL, NULL, false, 0, NULL, NULL, &si, &pi);
#elif __ANDROID_
	
#endif
}

int CARP_GetScreenWidth()
{
#ifdef _WIN32
	return GetSystemMetrics(SM_CXSCREEN);
#elif __ANDROID__
	return sapp_width();
#elif __APPLE__
#ifdef TARGET_OS_IPHONE
	return sapp_width();
#else
	return [[UIScreen mainScreen]bounds].size.width * [UIScreen mainScreen].scale;
#endif
#else
	return sapp_width();
#endif
}

int CARP_GetScreenHeight()
{
#ifdef _WIN32
	return GetSystemMetrics(SM_CYSCREEN);
#elif __ANDROID__
	return sapp_height();
#elif __APPLE__
#ifdef TARGET_OS_IPHONE
	return sapp_height();
#else
	return [[UIScreen mainScreen]bounds].size.height * [UIScreen mainScreen].scale;
#endif
#else
	return sapp_height();
#endif
}

void CARP_Alert(const char* message)
{
#ifdef _WIN32
	wchar_t* msg = _carp_system_UTF82Unicode(message);
	MessageBoxW(NULL, msg, L"Alert", MB_OK);
	free(msg);
#endif
}

#endif
#endif