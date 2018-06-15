//
//  OpenGLView.m
//  SpiderChase
//
//  Created by Laki, Zoltan on 2018. 06. 12..
//  Copyright © 2018. Immortal Games. All rights reserved.
//

#include "stdafx.h"
#import "OpenGLView.hpp"
#include "game.hpp"

@implementation OpenGLView

GLuint programObject;

+ (Class)layerClass {
	return [CAEAGLLayer class];
}

- (void)setupLayer {
	_eaglLayer = (CAEAGLLayer*) self.layer;
	_eaglLayer.opaque = YES;
}

- (void)setupContext {
	EAGLRenderingAPI api = kEAGLRenderingAPIOpenGLES2;
	_context = [[EAGLContext alloc] initWithAPI:api];
	if (!_context) {
		NSLog(@"Failed to initialize OpenGLES 2.0 context");
		exit(1);
	}
	
	if (![EAGLContext setCurrentContext:_context]) {
		NSLog(@"Failed to set current OpenGL context");
		exit(1);
	}
}

- (void)setupRenderBuffer {
	glGenRenderbuffers(1, &_colorRenderBuffer);
	glBindRenderbuffer(GL_RENDERBUFFER, _colorRenderBuffer);
	[_context renderbufferStorage:GL_RENDERBUFFER fromDrawable:_eaglLayer];
}

- (void)setupFrameBuffer {
	GLuint framebuffer;
	glGenFramebuffers(1, &framebuffer);
	glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER, _colorRenderBuffer);
}

- (void)setupDepthBuffer {
	GLint framebufferWidth, framebufferHeight;
	glGetRenderbufferParameteriv(GL_RENDERBUFFER, GL_RENDERBUFFER_WIDTH, &framebufferWidth);
	glGetRenderbufferParameteriv(GL_RENDERBUFFER, GL_RENDERBUFFER_HEIGHT, &framebufferHeight);

	GLuint depthBuffer;
	glGenRenderbuffers(1, &depthBuffer);
	glBindRenderbuffer(GL_RENDERBUFFER, depthBuffer);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24_OES, framebufferWidth, framebufferHeight);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, depthBuffer);
}

- (void)setupDisplayLink {
	CADisplayLink* displayLink = [CADisplayLink displayLinkWithTarget:self selector:@selector(render:)];
	[displayLink addToRunLoop:[NSRunLoop currentRunLoop] forMode:NSDefaultRunLoopMode];
}

- (double)getTimeInMillisec {
	static std::chrono::high_resolution_clock::time_point start = std::chrono::high_resolution_clock::now ();
	static bool firstCall = true;
	
	if (firstCall) {
		firstCall = false;
		return 0;
	}
	
	//after first frame
	std::chrono::high_resolution_clock::time_point curTime = std::chrono::high_resolution_clock::now ();
	std::chrono::duration<double, std::milli> diff = curTime - start;
	return diff.count ();
}

- (void)render:(CADisplayLink*)displayLink {
	
	static double add = 0;
	add += 0.1;
	glClearColor(0, (104.0 + add)/255.0, 55.0/255.0, 1.0);
	glClear(GL_COLOR_BUFFER_BIT);
	
	glDisable (GL_DEPTH_TEST);
	
	glViewport (0, 0, 320, 480);
	
	//Run the game
	Game& game = Game::Get ();

	double timeInSec = [self getTimeInMillisec] / 1000.0;
	if (!game.Step (timeInSec)) {
		//return pvr::Result::ExitRenderFrame;
		return; //Exit game
	}
	
	[_context presentRenderbuffer:GL_RENDERBUFFER];
}

- (instancetype)initWithCoder:(NSCoder *)aDecoder {
	self = [super initWithCoder:aDecoder];
	if (self) {
		[self setupLayer];
		[self setupContext];
		[self setupRenderBuffer];
		[self setupFrameBuffer];
		//[self setupDepthBuffer];
		
		if(glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
			NSLog(@"failed to make complete framebuffer object %x", glCheckFramebufferStatus(GL_FRAMEBUFFER));
			return nil;
		}
		
		[self setupDisplayLink];
	}
	return self;
}

- (id)initWithFrame:(CGRect)frame
{
	self = [super initWithFrame:frame];
	if (self) {
		[self setupLayer];
		[self setupContext];
		[self setupRenderBuffer];
		[self setupFrameBuffer];
		//[self setupDepthBuffer];
		
		if(glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
			NSLog(@"failed to make complete framebuffer object %x", glCheckFramebufferStatus(GL_FRAMEBUFFER));
			return nil;
		}

		[self setupDisplayLink];
	}
	return self;
}

- (void)dealloc
{
	_context = nil;
}


/*
// Only override drawRect: if you perform custom drawing.
// An empty implementation adversely affects performance during animation.
- (void)drawRect:(CGRect)rect {
    // Drawing code
}
*/

@end
