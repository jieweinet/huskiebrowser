// Copyright 2018 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef IOS_WEB_VIEW_PUBLIC_CWV_AUTOFILL_DATA_MANAGER_H_
#define IOS_WEB_VIEW_PUBLIC_CWV_AUTOFILL_DATA_MANAGER_H_

#import <Foundation/Foundation.h>

#import "cwv_export.h"

NS_ASSUME_NONNULL_BEGIN

@class CWVAutofillProfile;
@class CWVCreditCard;
@class CWVPassword;
@protocol CWVAutofillDataManagerObserver;

// Exposes saved autofill data such as address profiles and credit cards.
CWV_EXPORT
@interface CWVAutofillDataManager : NSObject

- (instancetype)init NS_UNAVAILABLE;

// Adds |observer| for data changes.
- (void)addObserver:(__weak id<CWVAutofillDataManagerObserver>)observer;

// Removes |observer| that was previously added with |addObserver|.
- (void)removeObserver:(__weak id<CWVAutofillDataManagerObserver>)observer;

// Returns all saved profiles for address autofill in |completionHandler|.
- (void)fetchProfilesWithCompletionHandler:
    (void (^)(NSArray<CWVAutofillProfile*>* profiles))completionHandler;

// Updates the profile.
- (void)updateProfile:(CWVAutofillProfile*)profile;

// Deletes the profile.
- (void)deleteProfile:(CWVAutofillProfile*)profile;

// Returns all saved credit cards for payment autofill in |completionHandler|.
- (void)fetchCreditCardsWithCompletionHandler:
    (void (^)(NSArray<CWVCreditCard*>* creditCards))completionHandler;

// Returns all saved passwords for password autofill in |completionHandler|.
- (void)fetchPasswordsWithCompletionHandler:
    (void (^)(NSArray<CWVPassword*>* passwords))completionHandler;

// Deletes the password.
- (void)deletePassword:(CWVPassword*)password;

// Adds a new password created from the iOS credential provider extension.
// |username| The login username for this password.
// |serviceIdentifier| The service for which this password is for. This should
// be derived from a -[ASCredentialServiceIdentifier identifier].
// |keychainIdentifier| Used to retrieve the password value from the keychain.
// This should identify a password previously stored using the APIs in
// CWVCredentialProviderUtils.
- (void)addNewPasswordForUsername:(NSString*)username
                serviceIdentifier:(NSString*)serviceIdentifier
               keychainIdentifier:(NSString*)keychainIdentifier;

@end

NS_ASSUME_NONNULL_END

#endif  // IOS_WEB_VIEW_PUBLIC_CWV_AUTOFILL_DATA_MANAGER_H_
