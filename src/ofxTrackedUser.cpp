/*
 * ofxTrackedUser.cpp
 *
 * Copyright 2011 (c) Matthew Gingold http://gingold.com.au
 * Originally forked from a project by roxlu http://www.roxlu.com/ 
 *
 * OSC implementation by Martin Froehlich http://maybites.ch
 *
 * Permission is hereby granted, free of charge, to any person
 * obtaining a copy of this software and associated documentation
 * files (the "Software"), to deal in the Software without
 * restriction, including without limitation the rights to use,
 * copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following
 * conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
 * OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
 * HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
 * WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
 * OTHER DEALINGS IN THE SOFTWARE.
 *
 */

#include "ofxTrackedUser.h"

ofxTrackedUser::ofxTrackedUser(ofxOpenNIContext* pContext) 
:neck(XN_SKEL_HEAD, XN_SKEL_NECK)

// left arm + shoulder
,left_shoulder(XN_SKEL_NECK, XN_SKEL_LEFT_SHOULDER)
,left_upper_arm(XN_SKEL_LEFT_SHOULDER, XN_SKEL_LEFT_ELBOW)
,left_lower_arm(XN_SKEL_LEFT_ELBOW, XN_SKEL_LEFT_HAND)

// right arm + shoulder
,right_shoulder(XN_SKEL_NECK, XN_SKEL_RIGHT_SHOULDER)
,right_upper_arm(XN_SKEL_RIGHT_SHOULDER, XN_SKEL_RIGHT_ELBOW)
,right_lower_arm(XN_SKEL_RIGHT_ELBOW, XN_SKEL_RIGHT_HAND)

// upper torso
,left_upper_torso(XN_SKEL_LEFT_SHOULDER, XN_SKEL_TORSO)
,right_upper_torso(XN_SKEL_RIGHT_SHOULDER, XN_SKEL_TORSO)

// left lower torso + leg
,left_lower_torso(XN_SKEL_TORSO, XN_SKEL_LEFT_HIP)
,left_upper_leg(XN_SKEL_LEFT_HIP, XN_SKEL_LEFT_KNEE)
,left_lower_leg(XN_SKEL_LEFT_KNEE, XN_SKEL_LEFT_FOOT)

// right lower torso + leg
,right_lower_torso(XN_SKEL_TORSO, XN_SKEL_RIGHT_HIP)
,right_upper_leg(XN_SKEL_RIGHT_HIP, XN_SKEL_RIGHT_KNEE)
,right_lower_leg(XN_SKEL_RIGHT_KNEE, XN_SKEL_RIGHT_FOOT)

,hip(XN_SKEL_LEFT_HIP, XN_SKEL_RIGHT_HIP)
{
	context = pContext;
	context->getDepthGenerator(&depth_generator);
	context->getUserGenerator(&user_generator);
    
    // open an outgoing connection to HOST:PORT
	sender.setup( HOST, PORT );

}

void ofxTrackedUser::updateBonePositions() {
	
	updateLimb(neck);
	
	// left arm + shoulder
	updateLimb(left_shoulder);
	updateLimb(left_upper_arm);
	updateLimb(left_lower_arm);
	
	// right arm + shoulder
	updateLimb(right_shoulder);
	updateLimb(right_upper_arm);
	updateLimb(right_lower_arm);
	
	// upper torso
	updateLimb(left_upper_torso);
	updateLimb(right_upper_torso);
	
	// left lower torso + leg
	updateLimb(left_lower_torso);
	updateLimb(left_upper_leg);
	updateLimb(left_lower_leg);
	
	// right lower torso + leg
	updateLimb(right_lower_torso);
	updateLimb(right_upper_leg);
	updateLimb(right_lower_leg);

	updateLimb(hip);	
}

void ofxTrackedUser::updateLimb(ofxLimb& rLimb) {
	
	if(!user_generator.GetSkeletonCap().IsTracking(id)) {
		//printf("Not tracking this user: %d\n", id);
		return;
	}
	
	XnSkeletonJointPosition a,b;
	user_generator.GetSkeletonCap().GetSkeletonJointPosition(id, rLimb.start_joint, a);
	user_generator.GetSkeletonCap().GetSkeletonJointPosition(id, rLimb.end_joint, b);
        
    if(a.fConfidence < 0.3f || b.fConfidence < 0.3f) {
		rLimb.found = false; 
		return;
	}
	
	rLimb.found = true;
	rLimb.position[0] = a.position;
	rLimb.position[1] = b.position;
	
    //OSC
	rLimb.position3D[0] = a.position;
	rLimb.position3D[1] = b.position;
	rLimb.confidence0 = a.fConfidence;
	rLimb.confidence1 = b.fConfidence;

	depth_generator.ConvertRealWorldToProjective(2, rLimb.position, rLimb.position);
	
}

void ofxTrackedUser::debugDraw(const float wScale, const float hScale) {
	
	glPushMatrix();

    glScalef(wScale, hScale, 1);
  
    neck.debugDraw();

	// left arm + shoulder
	left_shoulder.debugDraw();
	left_upper_arm.debugDraw();
	left_lower_arm.debugDraw();
	
	// right arm + shoulder
	right_shoulder.debugDraw();
	right_upper_arm.debugDraw();
	right_lower_arm.debugDraw();
	
	// upper torso
	left_upper_torso.debugDraw();
	right_upper_torso.debugDraw();
	
	// left lower torso + leg
	left_lower_torso.debugDraw();
	left_upper_leg.debugDraw();
	left_lower_leg.debugDraw();

	// right lower torso + leg
	right_lower_torso.debugDraw();
	right_upper_leg.debugDraw();
	right_lower_leg.debugDraw();
	
	hip.debugDraw();
    
    ofDrawBitmapString(ofToString((int)id), neck.position[0].X + 10, neck.position[0].Y);

    glPopMatrix();
}

//--------------------------------------------------------------
//OSC....
//--------------------------------------------------------------
void ofxTrackedUser::sendRaw() {
	
	ofxOscMessage message;
	message.setAddress("/skeleton/data");
	message.addIntArg( (int) id );
    
	//add head
	message.addFloatArg( neck.confidence0 );
	message.addFloatArg( neck.position3D[0].X );
	message.addFloatArg( neck.position3D[0].Z );
	message.addFloatArg( neck.position3D[0].Y    );
	
	
	//neck
	addLimbData(message, neck);
	
	// left arm + shoulder
	addLimbData(message, left_shoulder);
	addLimbData(message, left_upper_arm);
	addLimbData(message, left_lower_arm);
	
	// right arm + shoulder
	addLimbData(message, right_shoulder);
	addLimbData(message, right_upper_arm);
	addLimbData(message, right_lower_arm);
	
	// torso
	addLimbData(message, left_upper_torso);
	
	// left hip + leg
	addLimbData(message, left_lower_torso);
	addLimbData(message, left_upper_leg);
	addLimbData(message, left_lower_leg);
	
	// right lower torso + leg
	addLimbData(message, right_lower_torso);
	addLimbData(message, right_upper_leg);
	addLimbData(message, right_lower_leg);
	
	
	sender.sendMessage( message );
	
}

void ofxTrackedUser::addLimbData(ofxOscMessage& m, ofxLimb& rLimb){
	//ofVec3f center(rLimb.position3D[1].X + 210.0f, rLimb.position3D[1].Z + 2330.0f, rLimb.position3D[1].Y - 862.0f);
	//center.rotateRad(-1.3f, 0.0f, 0.0f);
	m.addFloatArg( rLimb.confidence1 );
	m.addFloatArg( rLimb.position3D[1].X );
	m.addFloatArg( rLimb.position3D[1].Z );
	m.addFloatArg( rLimb.position3D[1].Y );
}

void ofxTrackedUser::sendDetail() {
	
	sendOscSimpleMessage("/skeleton/start");
	
	//neck.debugDraw();
	sendOscMessage("/skeleton/neck", neck);
	
	// left arm + shoulder
    sendOscMessage("/skeleton/left/shoulder", left_shoulder);
    sendOscMessage("/skeleton/left/arm/upper", left_upper_arm);
    sendOscMessage("/skeleton/left/arm/lower", left_lower_arm);
	
	// right arm + shoulder
    sendOscMessage("/skeleton/right/shoulder", right_shoulder);
    sendOscMessage("/skeleton/right/arm/upper", right_upper_arm);
    sendOscMessage("/skeleton/right/arm/lower", right_lower_arm);
	
	// upper torso
    sendOscMessage("/skeleton/left/torso/uppper", left_upper_torso);
    sendOscMessage("/skeleton/right/torso/upper", right_upper_torso);
	
	// left lower torso + leg
    sendOscMessage("/skeleton/left/torso/lower", left_lower_torso);
    sendOscMessage("/skeleton/left/leg/upper", left_upper_leg);
    sendOscMessage("/skeleton/left/leg/lower", left_lower_leg);
	
	// right lower torso + leg
	sendOscMessage("/skeleton/right/torso/lower", right_lower_torso);
	sendOscMessage("/skeleton/right/leg/upper", right_upper_leg);
	sendOscMessage("/skeleton/right/leg/lower", right_lower_leg);
	
	sendOscMessage("/skeleton/hip", hip);
	
	sendOscSimpleMessage("/skeleton/end");
	
}

void ofxTrackedUser::sendOscSimpleMessage  (const string& address){
	ofxOscMessage m;
	m.setAddress( address );
	m.addIntArg( (int) id );
	sender.sendMessage( m );
}

void ofxTrackedUser::sendOscMessage  (const string& address, ofxLimb& rLimb){
	if(rLimb.found == true){
		ofxOscMessage m;
		m.setAddress( address );
		m.addIntArg( (int) id );
		m.addFloatArg( rLimb.position3D[0].X );
		m.addFloatArg( rLimb.position3D[0].Y );
		m.addFloatArg( rLimb.position3D[0].Z );
		m.addFloatArg( rLimb.position3D[1].X );
		m.addFloatArg( rLimb.position3D[1].Y );
		m.addFloatArg( rLimb.position3D[1].Z );
		//m.addStringArg( "hello" );
		sender.sendMessage( m );
	}
}


