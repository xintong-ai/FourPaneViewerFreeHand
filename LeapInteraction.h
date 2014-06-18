#include "leap.h"

//using namespace Leap;

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
