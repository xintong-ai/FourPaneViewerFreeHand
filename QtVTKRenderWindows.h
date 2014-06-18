#ifndef QtVTKRenderWindows_H
#define QtVTKRenderWindows_H

#include "vtkSmartPointer.h"
#include "vtkResliceImageViewer.h"
#include "vtkImagePlaneWidget.h"
#include "vtkDistanceWidget.h"
#include "vtkResliceImageViewerMeasurements.h"
#include <QMainWindow>
#include <QElapsedTimer>

#include "LeapInteraction.h"

class MyListener : public QObject, public Leap::Listener {
	Q_OBJECT


public:
	MyListener()
	{
		//timer = new QTimer(this);
		timer = new QElapsedTimer();
		timer->start();
	}

	virtual void onFrame(const Leap::Controller & ctl) {

		if(timer->elapsed() > 100)
		{
			Leap::Frame f = ctl.frame();
			// This is a hack so that we avoid having to declare a signal and
			// use moc generated code.
			setObjectName(QString::number(f.id()));
			// emits objectNameChanged(QString)
			emit translate(SimpleTranslate(f));
			timer->restart();
		}

	}

private:
	QElapsedTimer *timer;

signals:
	void translate(float v);
};

// Forward Qt class declarations
class Ui_QtVTKRenderWindows;


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
