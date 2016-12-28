//
//  LFDevice.h
//  Flipper
//
//  Created by George Morgan on 12/27/16.
//  Copyright © 2016 Flipper. All rights reserved.
//

#import <Foundation/Foundation.h>
#import "LED.h"

@interface LFDevice : NSObject

@property (nonatomic) LFLED *led;

- (id) initWithName:(NSString *)name;

@end
