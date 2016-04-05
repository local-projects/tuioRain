//
//  ofApp.cpp
//  Putzer
//
//  Created by Eric Rieper on 3/31/16.
//
//

#include "ofApp.h"

//--------------------------------------------------------------
void ofApp::setup() {

  // OF Setup
  ofBackground(200, 200, 200);
  ofSetFrameRate(60);

  // Window Setup
  ofSetWindowTitle("Putzer!");


  // GUI Setup
  gui.setup();

	// General info labels
	gui.add(infoGroup.setup("Info"));
	infoGroup.add(putzerEnabledToggle.setup("Putzer Enabled", true));
  infoGroup.add(ipAddressLabel.setup("IP", "127.0.0.1"));
  infoGroup.add(portLabel.setup("PORT", "3333"));
	infoGroup.add(touchCountLabel.setup("CURRENT TOUCHES", "0"));
	
	// Touch properties
	gui.add(touchesGroup.setup("Touches"));
  touchesGroup.add(touchFrequency.setup("Frames Between Touches", 5, 1, 120));
	touchesGroup.add(numTouches.setup("Number of Touches",  2, 1, 10));
	touchesGroup.add(minTouchDir.setup("Min Touch Duration", 0.05, 0.01, 10));
	touchesGroup.add(maxTouchDir.setup("Max Touch Duration", 1.0, 0.01, 10));
	touchesGroup.add(maxSwipeLengthSlider.setup("Max Swipe Distance", 5, 0.0, 10.0));
	
	// Touch area motion settings
	gui.add(motionGroup.setup("Area Motion"));
  motionGroup.add(marchHorizontalToggle.setup("Sweep X", false));
	motionGroup.add(marchVerticalToggle.setup("Sweep Y", false));
	motionGroup.add(sineHorizontalToggle.setup("Oscillate X", false));
	motionGroup.add(sineVerticalToggle.setup("Oscillate Y", false));
	motionGroup.add(horizontalSweepSpeed.setup("X Motion Duration",1,0.1,20));
	motionGroup.add(verticalSweepSpeed.setup("Y Motion Duration",2,0.1,20));

  gui.loadFromFile("settings.xml");

  // TUIO Setup
  myTuioServer = new ofxTuioServer;

  string str = ipAddressLabel;
  char *cstr = &str[0u];

  myTuioServer->start(cstr, ofToInt(portLabel));
  myTuioServer->setVerbose(false);
	
	ofSetWindowShape((gui.getHeight() + (gui.getPosition().y*2)) * 2, gui.getHeight() + (gui.getPosition().y *2));
	
}

//--------------------------------------------------------------

void ofApp::update() {
	
	// REMOVE OLD TOUCHES ////////////////////////////////////
	for (auto touch : fakeTouches) {
		if (!touch->cursorIsActive) {
			if (touch) {
				int removeIt = 0;
				for (auto touchToRemove : fakeTouches){
					if (touchToRemove == touch){
						delete fakeTouches[removeIt];
						fakeTouches.erase(fakeTouches.begin()+removeIt);
					}
					removeIt++;
				}
			}
		}
	}

	// UPDATE EACH TOUCHES ////////////////////////////////////
	for (auto touch : fakeTouches) {
    touch->update();
  }
	
	// UPDATE THE TUIO SERVER //////////////////////////////
  myTuioServer->run();
	
	// UPDATE THE GUI //////////////////////////////////////
	if (ofGetFrameNum() % 15 == 0){
		if (fakeTouches.size() >= 256){
			touchCountLabel.setBackgroundColor(ofColor(100,10,40));
		}
		else {
			touchCountLabel.setBackgroundColor(gui.getBackgroundColor());
		}
		touchCountLabel = ofToString(fakeTouches.size());
	}
	
}

//--------------------------------------------------------------

void ofApp::draw() {

  ofBackgroundGradient(ofColor(170, 160, 160), ofColor(120, 120, 130), OF_GRADIENT_LINEAR);
	
	// CREATING THE TOUCHES ////////////////////////////
	if (putzerEnabledToggle && fakeTouches.size() < 256){ // Limits total number of touches to
		if (ofGetFrameNum()%touchFrequency == 0){			// prevent buffer out of memory error.
			
			int cappedNumTouches = numTouches;
			if ((marchHorizontalToggle && marchVerticalToggle) || (sineHorizontalToggle && marchVerticalToggle)){ cappedNumTouches = 1; } // Only make one touch because we have fixed X and Y position
			for (int i = 0; i < cappedNumTouches; i++){	// Create the touch touches!
				createtouch();
			}
		}
	}

	// DRAWING THE TOUCHES /////////////////////////////
  ofSetColor(ofColor::white);
  ofPushStyle();
	for (auto touch : fakeTouches) {
    if (touch->cursorIsActive) {
      ofDrawCircle(touch->cursorPosition.x, touch->cursorPosition.y, 4);
    }
  }
  ofPopStyle();

	// DRAW THE GUI ////////////////////////////////////////
  gui.draw();
}

//--------------------------------------------------------------

void ofApp::createtouch(){
	
	// CREATE POSITION, DURATION, AND SWIPE PROPERTIES /////
	ofPoint		newTouchPosition(ofRandom(0, ofGetWidth()),ofRandom(0, ofGetHeight()));
	float			newTouchDuration(ofRandom(minTouchDir, maxTouchDir));
	ofVec2f		newToucheswipeDistance(ofRandom(-maxSwipeLengthSlider,maxSwipeLengthSlider),ofRandom(-maxSwipeLengthSlider,maxSwipeLengthSlider));
	bool			newTouchDoesSwipe = random()%3;
	
	// MODIFY POSITION BASED ON SWEEPING OR OSCILLATING ////
	if (marchHorizontalToggle) {
		float hMillis = horizontalSweepSpeed * 1000;
		newTouchPosition.x = ofGetWidth() *  ((ofGetElapsedTimeMillis() % (int)hMillis / hMillis)) ;
	}
	if (marchVerticalToggle) {
		float vMillis = verticalSweepSpeed * 1000;
		newTouchPosition.y = ofGetHeight() *  ((ofGetElapsedTimeMillis() % (int)vMillis / vMillis)) ;
	}
	
	if (sineHorizontalToggle){
		newTouchPosition.x = ofGetWidth() *  ( 0.5+(sinf(ofGetElapsedTimef() / ((float)horizontalSweepSpeed*0.5))*0.5) );
	}
	if (sineVerticalToggle){
		newTouchPosition.y = ofGetHeight() *  ( 0.5+(cosf(ofGetElapsedTimef() / ((float)verticalSweepSpeed*0.5))*0.5) );
	}
	
	// CREATE AND PUSH touch INTO fakeTouches //////////////////
	fakeTouches.push_back(new FakeTouch(newTouchPosition,
																	newTouchDuration,
																	newToucheswipeDistance,
																	newTouchDoesSwipe,
																	myTuioServer));
}

//--------------------------------------------------------------
void ofApp::keyPressed(int key) {}

//--------------------------------------------------------------
void ofApp::keyReleased(int key) {}

//--------------------------------------------------------------
void ofApp::mouseMoved(int x, int y) {}

//--------------------------------------------------------------
void ofApp::mouseDragged(int x, int y, int button) {}

//--------------------------------------------------------------
void ofApp::mousePressed(int x, int y, int button) {}

//--------------------------------------------------------------
void ofApp::mouseReleased(int x, int y, int button) {}

//--------------------------------------------------------------
void ofApp::mouseEntered(int x, int y) {}

//--------------------------------------------------------------
void ofApp::mouseExited(int x, int y) {}

//--------------------------------------------------------------
void ofApp::windowResized(int w, int h) {}

//--------------------------------------------------------------
void ofApp::gotMessage(ofMessage msg) {}

//--------------------------------------------------------------
void ofApp::dragEvent(ofDragInfo dragInfo) {}