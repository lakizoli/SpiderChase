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

@implementation OpenGLView {
	CAEAGLLayer* _eaglLayer;
	EAGLContext* _context;
	GLuint _colorRenderBuffer;
	GLuint _depthRenderBuffer;
	GLuint _frameBuffer;

	GLint _framebufferWidth;
	GLint _framebufferHeight;
}

#pragma mark - Setup functions

+ (Class) layerClass {
	return [CAEAGLLayer class];
}

- (void) createContext {
	//Setup layer
	_eaglLayer = (CAEAGLLayer*) self.layer;
	_eaglLayer.opaque = YES;
	
	//Choose API (OpenGL ES 2.0)
	EAGLRenderingAPI api = kEAGLRenderingAPIOpenGLES2;
	_context = [[EAGLContext alloc] initWithAPI:api];
	if (!_context) {
		NSLog (@"Failed to initialize OpenGLES 2.0 context");
		exit (1);
	}
	
	if (![EAGLContext setCurrentContext:_context]) {
		NSLog (@"Failed to set current OpenGL context");
		exit (1);
	}
}

- (void) destroy {
	if (_frameBuffer > 0) {
		glDeleteFramebuffers (1, &_frameBuffer);
		_frameBuffer = 0;
	}
	
	if (_colorRenderBuffer > 0) {
		glDeleteRenderbuffers (1, &_colorRenderBuffer);
		_colorRenderBuffer = 0;
	}
	
	if (_depthRenderBuffer > 0) {
		glDeleteRenderbuffers (1, &_depthRenderBuffer);
		_depthRenderBuffer = 0;
	}
}

- (void) setup {
	//Clear state
	_colorRenderBuffer = 0;
	_depthRenderBuffer = 0;
	_frameBuffer = 0;
	_framebufferHeight = 0;
	_framebufferWidth = 0;
	
	//Render buffer
	glGenRenderbuffers (1, &_colorRenderBuffer);
	glBindRenderbuffer (GL_RENDERBUFFER, _colorRenderBuffer);

	//Frame buffer
	glGenFramebuffers (1, &_frameBuffer);
	glBindFramebuffer (GL_FRAMEBUFFER, _frameBuffer);

	//Setup context
	[_context renderbufferStorage:GL_RENDERBUFFER fromDrawable:_eaglLayer];
	glFramebufferRenderbuffer (GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER, _colorRenderBuffer);
	
	glGetRenderbufferParameteriv (GL_RENDERBUFFER, GL_RENDERBUFFER_WIDTH, &_framebufferWidth);
	glGetRenderbufferParameteriv (GL_RENDERBUFFER, GL_RENDERBUFFER_HEIGHT, &_framebufferHeight);

	//Depth buffer
//	GLuint depthBuffer;
//	glGenRenderbuffers (1, &depthBuffer);
//	glBindRenderbuffer (GL_RENDERBUFFER, depthBuffer);
//	glRenderbufferStorage (GL_RENDERBUFFER, GL_DEPTH_COMPONENT24_OES, _framebufferWidth, _framebufferHeight);
//	glFramebufferRenderbuffer (GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, depthBuffer);

	//Check frame buffer status
	if (glCheckFramebufferStatus (GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
		NSLog(@"failed to make complete framebuffer object %x", glCheckFramebufferStatus(GL_FRAMEBUFFER));
		exit (1); //Exit
	}
	
	//Viewport
	glViewport (0, 0, _framebufferWidth, _framebufferHeight);
	
	//Setup display link
	CADisplayLink* displayLink = [CADisplayLink displayLinkWithTarget:self selector:@selector(render:)];
	[displayLink addToRunLoop:[NSRunLoop currentRunLoop] forMode:NSDefaultRunLoopMode];
}

#pragma mark - Render functions

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
	
	//Run the game
	Game& game = Game::Get ();

	double timeInSec = [self getTimeInMillisec] / 1000.0;
	if (!game.Step (timeInSec)) {
		exit (0); //Exit game
	}
	
	[_context presentRenderbuffer:GL_RENDERBUFFER];
}

#pragma mark - View functions

- (instancetype)initWithCoder:(NSCoder *)aDecoder {
	self = [super initWithCoder:aDecoder];
	if (self) {
		[self createContext];
		[self setup];
	}
	return self;
}

- (id)initWithFrame:(CGRect)frame
{
	self = [super initWithFrame:frame];
	if (self) {
		[self createContext];
		[self setup];
	}
	return self;
}

- (void)layoutSubviews {
	NSLog (@"layoutSubviews");
	
	[EAGLContext setCurrentContext:_context];
	[self destroy];
	[self setup];
}

- (void)dealloc {
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
