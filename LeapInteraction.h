#ifndef LEAP_INTERACTION_H
#define LEAP_INTERACTION_H
#include "leap.h"

//using namespace Leap;

inline float Clamp(float v)
{
	v = v > 1.0f ? 1.0f : v;
	v = v < 0.0f ? 0.0f : v;
	return v;
}

inline Leap::Vector Clamp(Leap::Vector v)
{
	return Leap::Vector(Clamp(v.x), Clamp(v.y), Clamp(v.z));
}

inline float SimpleTranslate(Leap::Frame frame)
{
	const Leap::GestureList gestures = frame.gestures();
	for (int g = 0; g < gestures.count(); ++g) {
		Leap::Gesture gesture = gestures[g];

		switch (gesture.type()) {
		case Leap::Gesture::TYPE_CIRCLE:
			{
				Leap::CircleGesture circle = gesture;
				std::string clockwiseness;

				if (circle.pointable().direction().angleTo(circle.normal()) <= Leap::PI/4) {
					//clockwiseness = "clockwise";
					return 1;
				} else {
					//clockwiseness = "counterclockwise";
					return -1;
				}
				break;
			}
		default:
			std::cout << std::string(2, ' ')  << "Unknown gesture type." << std::endl;
			break;
		}
	}
}


inline Leap::Vector RelativePalm3DLoc(Leap::Frame frame, Leap::Vector p)
{
	Leap::Hand leftHand = frame.hands().leftmost();
	Leap::Hand rightHand = frame.hands().rightmost();
	Leap::Vector palmCenter = leftHand.stabilizedPalmPosition();
	float spaceSide = leftHand.palmWidth() * 2;
	Leap::Vector dir = leftHand.direction().normalized();
	Leap::Finger middleFinger = leftHand.fingers().fingerType(Leap::Finger::Type::TYPE_MIDDLE).frontmost();
	Leap::Vector middleFingerTip = middleFinger.stabilizedTipPosition();
	Leap::Vector palmNormal = leftHand.palmNormal();
	Leap::Vector yDir = palmNormal.cross(dir).normalized();
	Leap::Vector origin = palmCenter - dir * spaceSide * 0.3 - yDir * spaceSide * 0.5;

	//	Leap::Vector pointerTip = rightHand.fingers().fingerType(Leap::Finger::Type::TYPE_INDEX).frontmost().tipPosition();
	Leap::Vector vecPalmCenter2Tip = p - palmCenter;
	float dist2Palm = vecPalmCenter2Tip.dot(palmNormal.normalized());
	//http://stackoverflow.com/questions/9605556/how-to-project-a-3d-point-to-a-3d-plane
	Leap::Vector projTip = vecPalmCenter2Tip - dist2Palm * palmNormal + palmCenter;

	Leap::Vector vecOrigin2ProjTip = projTip - origin;
	Leap::Vector ret;
	ret.x = vecOrigin2ProjTip.dot(dir) / spaceSide;
	ret.y = vecOrigin2ProjTip.dot(yDir) / spaceSide;
	ret.z = (dist2Palm - 50) / spaceSide;


	return ret;
}


inline void GetRectangle(Leap::Frame frame, Leap::Vector &origin, Leap::Vector &point1, Leap::Vector &point2)
{
	//Leap::Hand leftHand = frame.hands().leftmost();
	Leap::Hand rightHand = frame.hands().rightmost();
	//Leap::Vector dirLeft = leftHand.direction().normalized();
	//Leap::Vector zDir = leftHand.palmNormal().normalized();
	//Leap::Vector yDir = zDir.cross(dirLeft).normalized();
	//Leap::Vector xDir = yDir.cross(zDir);
	//
	//Leap::Vector rightNormal = rightHand.palmNormal().normalized();
	//planeNormal.x = rightNormal.dot(dirLeft);
	//planeNormal.y = rightNormal.dot(yDir);
	//planeNormal.z = rightNormal.dot(palmNormalLeft);
	//planeNormal = planeNormal.normalized();

	origin = Clamp(RelativePalm3DLoc(frame, rightHand.stabilizedPalmPosition()));
	point1 = rightHand.fingers().fingerType(Leap::Finger::Type::TYPE_INDEX).frontmost().tipPosition();
	point2 = rightHand.fingers().fingerType(Leap::Finger::Type::TYPE_THUMB).frontmost().tipPosition();
	cout<<"**point1"<<point1.x<<","<<point1.y<<","<<point1.z<<endl;
	cout<<"**point2"<<point2.x<<","<<point2.y<<","<<point2.z<<endl;

	point1 = Clamp(RelativePalm3DLoc(frame, (point1 - origin).normalized() * 10 + origin));
	point2 = Clamp(RelativePalm3DLoc(frame, (point2 - origin).normalized() * 10 + origin));
}

inline Leap::Vector NormlizePoint(Leap::Vector p)
{
	return Clamp(Leap::Vector((p.x + 50) * 0.01 , (p.y - 150) * 0.01, (p.z + 50) * 0.01));
}

inline void GetAbsoluteRectangle(Leap::Frame frame, Leap::Vector &origin, Leap::Vector &point1, Leap::Vector &point2)
{
	//Leap::Hand leftHand = frame.hands().leftmost();
	Leap::Hand hand = frame.hands().leftmost();
	//Leap::Vector dirLeft = leftHand.direction().normalized();
	//Leap::Vector zDir = leftHand.palmNormal().normalized();
	//Leap::Vector yDir = zDir.cross(dirLeft).normalized();
	//Leap::Vector xDir = yDir.cross(zDir);
	//
	//Leap::Vector rightNormal = rightHand.palmNormal().normalized();
	//planeNormal.x = rightNormal.dot(dirLeft);
	//planeNormal.y = rightNormal.dot(yDir);
	//planeNormal.z = rightNormal.dot(palmNormalLeft);
	//planeNormal = planeNormal.normalized();

	Leap::Vector center = hand.palmPosition();
	point1 = hand.fingers().fingerType(Leap::Finger::Type::TYPE_MIDDLE).frontmost().tipPosition();
	//point2 = hand.fingers().fingerType(Leap::Finger::Type::TYPE_THUMB).frontmost().tipPosition();

	Leap::Vector normal = hand.palmNormal().normalized();
	Leap::Vector vec1 = point1 - center;
	Leap::Vector dir1 = vec1.normalized();
	Leap::Vector dir2 = normal.cross(vec1).normalized();
	origin = center - dir1 * 40 - dir2 * 40;
	point1 = center + dir1 * 40 - dir2 * 40;
	point2 = center - dir1 * 40 + dir2 * 40;


	//point1 = (point1 - origin).normalized() * 10 + origin;
	//point2 = (point2 - origin).normalized() * 10 + origin;
	origin = NormlizePoint(origin);
	point1 = NormlizePoint(point1);
	point2 = NormlizePoint(point2);

	//cout<<"**origin:"<<origin.x<<","<<origin.y<<","<<origin.z<<endl;
	//cout<<"**point1:"<<point1.x<<","<<point1.y<<","<<point1.z<<endl;
	//cout<<"**point2:"<<point2.x<<","<<point2.y<<","<<point2.z<<endl;


	//cout<<"**origin:"<<origin.x<<","<<origin.y<<","<<origin.z<<endl;
	//cout<<"**point1:"<<point1.x<<","<<point1.y<<","<<point1.z<<endl;
	//cout<<"**point2:"<<point2.x<<","<<point2.y<<","<<point2.z<<endl;
}

#endif