#ifndef QtVTKRenderWindows_H
#define QtVTKRenderWindows_H

#include "vtkSmartPointer.h"
#include "vtkResliceImageViewer.h"
#include "vtkImagePlaneWidget.h"
#include "vtkDistanceWidget.h"
#include "vtkResliceImageViewerMeasurements.h"
#include <QMainWindow>
#include "leap.h"

// Forward Qt class declarations
class Ui_QtVTKRenderWindows;
using namespace Leap;

inline float SimpleTranslate(Frame frame)
{
	const GestureList gestures = frame.gestures();
  for (int g = 0; g < gestures.count(); ++g) {
    Gesture gesture = gestures[g];

    switch (gesture.type()) {
      case Gesture::TYPE_CIRCLE:
      {
        CircleGesture circle = gesture;
        std::string clockwiseness;

        if (circle.pointable().direction().angleTo(circle.normal()) <= PI/4) {
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

class MyListener : public QObject, public Leap::Listener {
	Q_OBJECT
public:
	virtual void onFrame(const Leap::Controller & ctl) {
		Leap::Frame f = ctl.frame();
		// This is a hack so that we avoid having to declare a signal and
		// use moc generated code.
		setObjectName(QString::number(f.id()));
		// emits objectNameChanged(QString)
		emit translate(SimpleTranslate(f));
	}

private:
	

signals:
	void translate(float v);
};


class QtVTKRenderWindows : public QMainWindow
{
	Q_OBJECT
public:

	// Constructor/Destructor
	QtVTKRenderWindows(int argc, char *argv[]);
	~QtVTKRenderWindows() {}

	public slots:

		virtual void slotExit();
		virtual void resliceMode(int);
		virtual void thickMode(int);
		virtual void SetBlendModeToMaxIP();
		virtual void SetBlendModeToMinIP();
		virtual void SetBlendModeToMeanIP();
		virtual void SetBlendMode(int);
		virtual void ResetViews();
		virtual void Render();
		virtual void AddDistanceMeasurementToView1();
		virtual void AddDistanceMeasurementToView( int );
		virtual void SimpleTranslate(float v);

protected:
	vtkSmartPointer< vtkResliceImageViewer > riw[3];
	vtkSmartPointer< vtkImagePlaneWidget > planeWidget[3];
	vtkSmartPointer< vtkDistanceWidget > DistanceWidget[3];
	vtkSmartPointer< vtkResliceImageViewerMeasurements > ResliceMeasurements;

	MyListener listener;
	Leap::Controller controller;

	protected slots:

private:

	// Designer form
	Ui_QtVTKRenderWindows *ui;
};

#endif // QtVTKRenderWindows_H
