#include "testApp.h"


//--------------------------------------------------------------
void testApp::setup()
{
	ofSetWindowTitle( "particle example" );
	ofBackground( 0, 0, 0 );
	
	if ( !m_emitter.loadFromXml( "circles.pex" ) )
	{
		ofLog( OF_LOG_ERROR, "testApp::setup() - failed to load emitter config" );
	}
}

//--------------------------------------------------------------
void testApp::exit()
{
	// TODO
}

//--------------------------------------------------------------
void testApp::update()
{
	m_emitter.update();
}

//--------------------------------------------------------------
void testApp::draw()
{
	m_emitter.draw( m_mousePos.x, m_mousePos.y );
	
	ofSetColor( 255, 255, 255 );
	ofDrawBitmapString( "fps: " + ofToString( ofGetFrameRate(), 2 ), 20, 20 );
}


//--------------------------------------------------------------
void testApp::keyPressed  (int key)
{
	// nothing
}

//--------------------------------------------------------------
void testApp::keyReleased  (int key)
{
	// nothing
}

//--------------------------------------------------------------
void testApp::mouseMoved(int x, int y )
{
	// nothing
}

//--------------------------------------------------------------
void testApp::mouseDragged(int x, int y, int button)
{
	// nothing
	m_mousePos.x = x;
	m_mousePos.y = y;
}

//--------------------------------------------------------------
void testApp::mousePressed(int x, int y, int button)
{
	// nothing
}


//--------------------------------------------------------------
void testApp::mouseReleased(int x, int y, int button)
{
	// nothing
}

//--------------------------------------------------------------
void testApp::windowResized(int w, int h)
{
	// nothing
}
