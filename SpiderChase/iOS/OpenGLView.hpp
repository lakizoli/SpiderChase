//
//  OpenGLView.h
//  SpiderChase
//
//  Created by Laki, Zoltan on 2018. 06. 12..
//  Copyright © 2018. Immortal Games. All rights reserved.
//

#import <UIKit/UIKit.h>
#import <QuartzCore/QuartzCore.h>
#include <OpenGLES/ES2/gl.h>
#include <OpenGLES/ES2/glext.h>

@interface OpenGLView : UIView {
	CAEAGLLayer* _eaglLayer;
	EAGLContext* _context;
	GLuint _colorRenderBuffer;
}

@end
