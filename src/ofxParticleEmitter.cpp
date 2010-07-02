//
// ofxParticleEmitter.cpp
//
// Copyright (c) 2010 71Squared, ported to Openframeworks by Shawn Roske
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
// THE SOFTWARE.

#include "ofxParticleEmitter.h"

// ------------------------------------------------------------------------
// Lifecycle
// ------------------------------------------------------------------------

ofxParticleEmitter::ofxParticleEmitter()
{
	settings = NULL;
	
	emitterType = kParticleTypeGravity;
	texture = NULL;
	sourcePosition.x = sourcePosition.y = 0.0f;
	sourcePositionVariance.x = sourcePositionVariance.y = 0.0f;
	angle = angleVariance = 0.0f;								
	speed = speedVariance = 0.0f;	
	radialAcceleration = tangentialAcceleration = 0.0f;
	radialAccelVariance = tangentialAccelVariance = 0.0f;
	gravity.x = gravity.y = 0.0f;
	particleLifespan = particleLifespanVariance = 0.0f;			
	startColor.red = startColor.green = startColor.blue = startColor.alpha = 1.0f;
	startColorVariance.red = startColorVariance.green = startColorVariance.blue = startColorVariance.alpha = 1.0f;
	finishColor.red = finishColor.green = finishColor.blue = finishColor.alpha = 1.0f;
	finishColorVariance.red = finishColorVariance.green = finishColorVariance.blue = finishColorVariance.alpha = 1.0f;
	startParticleSize = startParticleSizeVariance = 0.0f;
	finishParticleSize = finishParticleSizeVariance = 0.0f;
	maxParticles = 0.0f;
	particleCount = 0;
	emissionRate = 0.0f;
	emitCounter = 0.0f;	
	elapsedTime = 0.0f;
	duration = -1;
    
	blendFuncSource = blendFuncDestination = 0;

	maxRadius = maxRadiusVariance = radiusSpeed = minRadius = 0.0f;
	rotatePerSecond = rotatePerSecondVariance = 0.0f;
	
	active = useTexture = false;
	particleIndex = 0;

	verticesID = 0;
	particles = NULL;
	vertices = NULL;
}

ofxParticleEmitter::~ofxParticleEmitter()
{
	exit();
}

void ofxParticleEmitter::exit()
{	
	if ( texture != NULL )
		delete texture;
	texture = NULL;
	
	if ( particles != NULL )
		delete particles;
	particles = NULL;
	
	if ( vertices != NULL )
		delete vertices;
	vertices = NULL;
	
	glDeleteBuffers( 1, &verticesID );
}

bool ofxParticleEmitter::loadFromXml( const std::string& filename )
{
	bool ok = false;
	
	settings = new ofxXmlSettings();
	
	ok = settings->loadFile( filename );
	if ( ok )
	{
		parseParticleConfig();
		setupArrays();
		
		ok = active = true;
	}

	delete settings;
	settings = NULL;
	
	return ok;
}

void ofxParticleEmitter::parseParticleConfig()
{
	if ( settings == NULL )
	{
		ofLog( OF_LOG_ERROR, "ofxParticleEmitter::parseParticleConfig() - XML settings is invalid!" );
		return;
	}
	
	settings->pushTag( "particleEmitterConfig" );

	std::string imageFilename	= settings->getAttribute( "texture", "name", "" );
	std::string imageData		= settings->getAttribute( "texture", "data", "" );
	
	if ( imageFilename != "" )
	{
		ofLog( OF_LOG_WARNING, "ofxParticleEmitter::parseParticleConfig() - loading image file" );
		
		texture = new ofImage();
		texture->loadImage( imageFilename );
		texture->setUseTexture( true );
	}
	else if ( imageData != "" )
	{
		// TODO
		
		ofLog( OF_LOG_ERROR, "ofxParticleEmitter::parseParticleConfig() - image data found but not yet implemented!" );
		return;
	}

    emitterType					= settings->getAttribute( "emitterType", "value", emitterType );
	
	sourcePosition.x			= settings->getAttribute( "sourcePosition", "x", sourcePosition.x );
	sourcePosition.y			= settings->getAttribute( "sourcePosition", "y", sourcePosition.y );
	
	speed						= settings->getAttribute( "speed", "value", speed );
	speedVariance				= settings->getAttribute( "speedVariance", "value", speedVariance );
	particleLifespan			= settings->getAttribute( "particleLifespan", "value", particleLifespan );
	particleLifespanVariance	= settings->getAttribute( "particleLifespanVariance", "value", particleLifespanVariance );
	angle						= settings->getAttribute( "angle", "value", angle );
	angleVariance				= settings->getAttribute( "angleVariance", "value", angleVariance );
	
	gravity.x					= settings->getAttribute( "gravity", "x", gravity.x );
	gravity.y					= settings->getAttribute( "gravity", "y", gravity.y );
	
	radialAcceleration			= settings->getAttribute( "radialAcceleration", "value", radialAcceleration );
	tangentialAcceleration		= settings->getAttribute( "tangentialAcceleration", "value", tangentialAcceleration );
	
	startColor.red				= settings->getAttribute( "startColor", "red", startColor.red );
	startColor.green			= settings->getAttribute( "startColor", "green", startColor.green );
	startColor.blue				= settings->getAttribute( "startColor", "blue", startColor.blue );
	startColor.alpha			= settings->getAttribute( "startColor", "alpha", startColor.alpha );
	
	startColorVariance.red		= settings->getAttribute( "startColorVariance", "red", startColorVariance.red );
	startColorVariance.green	= settings->getAttribute( "startColorVariance", "green", startColorVariance.green );
	startColorVariance.blue		= settings->getAttribute( "startColorVariance", "blue", startColorVariance.blue );
	startColorVariance.alpha	= settings->getAttribute( "startColorVariance", "alpha", startColorVariance.alpha );
	
	finishColor.red				= settings->getAttribute( "finishColor", "red", finishColor.red );
	finishColor.green			= settings->getAttribute( "finishColor", "green", finishColor.green );
	finishColor.blue			= settings->getAttribute( "finishColor", "blue", finishColor.blue );
	finishColor.alpha			= settings->getAttribute( "finishColor", "alpha", finishColor.alpha );
	
	finishColorVariance.red		= settings->getAttribute( "finishColorVariance", "red", finishColorVariance.red );
	finishColorVariance.green	= settings->getAttribute( "finishColorVariance", "green", finishColorVariance.green );
	finishColorVariance.blue	= settings->getAttribute( "finishColorVariance", "blue", finishColorVariance.blue );
	finishColorVariance.alpha	= settings->getAttribute( "finishColorVariance", "alpha", finishColorVariance.alpha );
	
	maxParticles				= settings->getAttribute( "maxParticles", "value", maxParticles );
	startParticleSize			= settings->getAttribute( "startParticleSize", "value", startParticleSize );
	startParticleSizeVariance	= settings->getAttribute( "startParticleSizeVariance", "value", startParticleSizeVariance );
	finishParticleSize			= settings->getAttribute( "finishParticleSize", "value", finishParticleSize );
	finishParticleSizeVariance	= settings->getAttribute( "finishParticleSizeVariance", "value", finishParticleSizeVariance );
	duration					= settings->getAttribute( "duration", "value", duration );
	blendFuncSource				= settings->getAttribute( "blendFuncSource", "value", blendFuncSource );
	blendFuncDestination		= settings->getAttribute( "blendFuncDestination", "value", blendFuncDestination );
	
	maxRadius					= settings->getAttribute( "maxRadius", "value", maxRadius );
	maxRadiusVariance			= settings->getAttribute( "maxRadiusVariance", "value", maxRadiusVariance );
	radiusSpeed					= settings->getAttribute( "radiusSpeed", "value", radiusSpeed );
	minRadius					= settings->getAttribute( "minRadius", "value", minRadius );
	
	rotatePerSecond				= settings->getAttribute( "rotatePerSecond", "value", rotatePerSecond );
	rotatePerSecondVariance		= settings->getAttribute( "rotatePerSecondVariance", "value", rotatePerSecondVariance );
	
	// Calculate the emission rate
	emissionRate = maxParticles / particleLifespan;
}

void ofxParticleEmitter::setupArrays()
{
	// Allocate the memory necessary for the particle emitter arrays
	particles = (Particle*)malloc( sizeof( Particle ) * maxParticles );
	vertices = (PointSprite*)malloc( sizeof( PointSprite ) * maxParticles );
	
	// If one of the arrays cannot be allocated throw an assertion as this is bad
	assert( particles && vertices );
	
	// Generate the vertices VBO
	glGenBuffers( 1, &verticesID );
	
	// Set the particle count to zero
	particleCount = 0;
	
	// Reset the elapsed time
	elapsedTime = 0;
}

// ------------------------------------------------------------------------
// Update
// ------------------------------------------------------------------------

void ofxParticleEmitter::update()
{
	if ( !active ) return;
	
	// TODO
	
	// need delta
	
	/*
	
	// If the emitter is active and the emission rate is greater than zero then emit
	// particles
	if(active && emissionRate) {
		float rate = 1.0f/emissionRate;
		emitCounter += aDelta;
		while(particleCount < maxParticles && emitCounter > rate) {
			[self addParticle];
			emitCounter -= rate;
		}
		
		elapsedTime += aDelta;
		if(duration != -1 && duration < elapsedTime)
			[self stopParticleEmitter];
	}
	
	// Reset the particle index before updating the particles in this emitter
	particleIndex = 0;
	
	// Loop through all the particles updating their location and color
	while(particleIndex < particleCount) {
		
		// Get the particle for the current particle index
		Particle *currentParticle = &particles[particleIndex];
        
        // FIX 1
        // Reduce the life span of the particle
        currentParticle->timeToLive -= aDelta;
		
		// If the current particle is alive then update it
		if(currentParticle->timeToLive > 0) {
			
			// If maxRadius is greater than 0 then the particles are going to spin otherwise
			// they are effected by speed and gravity
			if (emitterType == kParticleTypeRadial) {
				
                // FIX 2
                // Update the angle of the particle from the sourcePosition and the radius.  This is only
				// done of the particles are rotating
				currentParticle->angle += currentParticle->degreesPerSecond * aDelta;
				currentParticle->radius -= currentParticle->radiusDelta;
                
				Vector2f tmp;
				tmp.x = sourcePosition.x - cosf(currentParticle->angle) * currentParticle->radius;
				tmp.y = sourcePosition.y - sinf(currentParticle->angle) * currentParticle->radius;
				currentParticle->position = tmp;
				
				if (currentParticle->radius < minRadius)
					currentParticle->timeToLive = 0;
			} else {
				Vector2f tmp, radial, tangential;
                
                radial = Vector2fZero;
                Vector2f diff = Vector2fSub(currentParticle->startPos, Vector2fZero);
                
                currentParticle->position = Vector2fSub(currentParticle->position, diff);
                
                if (currentParticle->position.x || currentParticle->position.y)
                    radial = Vector2fNormalize(currentParticle->position);
                
                tangential.x = radial.x;
                tangential.y = radial.y;
                radial = Vector2fMultiply(radial, currentParticle->radialAcceleration);
                
                GLfloat newy = tangential.x;
                tangential.x = -tangential.y;
                tangential.y = newy;
                tangential = Vector2fMultiply(tangential, currentParticle->tangentialAcceleration);
                
				tmp = Vector2fAdd( Vector2fAdd(radial, tangential), gravity);
                tmp = Vector2fMultiply(tmp, aDelta);
				currentParticle->direction = Vector2fAdd(currentParticle->direction, tmp);
				tmp = Vector2fMultiply(currentParticle->direction, aDelta);
				currentParticle->position = Vector2fAdd(currentParticle->position, tmp);
                currentParticle->position = Vector2fAdd(currentParticle->position, diff);
			}
			
			// Update the particles color
			currentParticle->color.red += currentParticle->deltaColor.red;
			currentParticle->color.green += currentParticle->deltaColor.green;
			currentParticle->color.blue += currentParticle->deltaColor.blue;
			currentParticle->color.alpha += currentParticle->deltaColor.alpha;
			
			// Place the position of the current particle into the vertices array
			vertices[particleIndex].x = currentParticle->position.x;
			vertices[particleIndex].y = currentParticle->position.y;
			
			// Place the size of the current particle in the size array
			currentParticle->particleSize += currentParticle->particleSizeDelta;
			vertices[particleIndex].size = MAX(0, currentParticle->particleSize);
			
			// Place the color of the current particle into the color array
			vertices[particleIndex].color = currentParticle->color;
			
			// Update the particle counter
			particleIndex++;
		} else {
			
			// As the particle is not alive anymore replace it with the last active particle 
			// in the array and reduce the count of particles by one.  This causes all active particles
			// to be packed together at the start of the array so that a particle which has run out of
			// life will only drop into this clause once
			if(particleIndex != particleCount - 1)
				particles[particleIndex] = particles[particleCount - 1];
			particleCount--;
		}
	}
	
	*/
}

// ------------------------------------------------------------------------
// Render
// ------------------------------------------------------------------------

void ofxParticleEmitter::draw(int x /* = 0 */, int y /* = 0 */)
{
	if ( !active ) return;
	
	ofSetColor( 255, 255, 255 );

	ofEnableAlphaBlending();
	texture->draw( x, y );
	ofDisableAlphaBlending();
	
	/*
	// Disable the texture coord array so that texture information is not copied over when rendering
	// the point sprites.
	glDisableClientState(GL_TEXTURE_COORD_ARRAY);
	
	// Bind to the verticesID VBO and popuate it with the necessary vertex & color informaiton
	glBindBuffer(GL_ARRAY_BUFFER, verticesID);
	glBufferData(GL_ARRAY_BUFFER, sizeof(PointSprite) * maxParticles, vertices, GL_DYNAMIC_DRAW);
	
	// Configure the vertex pointer which will use the currently bound VBO for its data
	glVertexPointer(2, GL_FLOAT, sizeof(PointSprite), 0);
	glColorPointer(4,GL_FLOAT,sizeof(PointSprite),(GLvoid*) (sizeof(GLfloat)*3));
	
	// Bind to the particles texture
	glBindTexture(GL_TEXTURE_2D, texture.name);
	
	// Enable the point size array
	glEnableClientState(GL_POINT_SIZE_ARRAY_OES);
	
	// Configure the point size pointer which will use the currently bound VBO.  PointSprite contains
	// both the location of the point as well as its size, so the config below tells the point size
	// pointer where in the currently bound VBO it can find the size for each point
	glPointSizePointerOES(GL_FLOAT,sizeof(PointSprite),(GLvoid*) (sizeof(GL_FLOAT)*2));
	
	// Change the blend function used if blendAdditive has been set
	
    // Set the blend function based on the configuration
    glBlendFunc(blendFuncSource, blendFuncDestination);
	
	// Enable and configure point sprites which we are going to use for our particles
	glEnable(GL_POINT_SPRITE_OES);
	glTexEnvi( GL_POINT_SPRITE_OES, GL_COORD_REPLACE_OES, GL_TRUE );
	
	// Now that all of the VBOs have been used to configure the vertices, pointer size and color
	// use glDrawArrays to draw the points
	glDrawArrays(GL_POINTS, 0, particleIndex);
	
	// Unbind the current VBO
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	
	// Disable the client states which have been used incase the next draw function does 
	// not need or use them
	glDisableClientState(GL_POINT_SIZE_ARRAY_OES);
	glDisable(GL_POINT_SPRITE_OES);
	
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	
	// Re-enable the texture coordinates as we use them elsewhere in the game and it is expected that
	// its on
	glEnableClientState(GL_TEXTURE_COORD_ARRAY);
	 
	 */
}



