#ifndef QtVTKRenderWindows_H
#define QtVTKRenderWindows_H

#include "vtkSmartPointer.h"
#include "vtkResliceImageViewer.h"
#include "vtkImagePlaneWidget.h"
#include "vtkDistanceWidget.h"
#include "vtkResliceImageViewerMeasurements.h"
#include <QMainWindow>
#include <QElapsedTimer>
#include <QVector3D>

#include "LeapInteraction.h"

class MyListener : public QObject, public Leap::Listener {
	Q_OBJECT

signals:
	void UpdateRectangle(QVector3D origin, QVector3D point1, QVector3D point2);
	void translate2(float v);

public:
	MyListener()
	{
		//timer = new QTimer(this);
		timer = new QElapsedTimer();
		timer->start();
	}

	virtual void onFrame(const Leap::Controller & ctl);

private:
	QElapsedTimer *timer;


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
		//virtual void UpdateSeedPlane(Leap::Vector origin, Leap::Vector point1, Leap::Vector point2);
		virtual void UpdateSeedPlane(QVector3D origin, QVector3D point1, QVector3D point2);
		virtual void UpdateSlicePlane(QVector3D origin, QVector3D point1, QVector3D point2);
		
protected:
	vtkSmartPointer< vtkResliceImageViewer > riw[3];
	vtkSmartPointer< vtkImagePlaneWidget > planeWidget[3];
	vtkSmartPointer< vtkDistanceWidget > DistanceWidget[3];
	vtkSmartPointer< vtkResliceImageViewerMeasurements > ResliceMeasurements;

	MyListener listener;
	Leap::Controller controller;

	vtkSmartPointer<vtkPlaneSource> seeds;
	protected slots:

private:

	// Designer form
	Ui_QtVTKRenderWindows *ui;
	int imageDims[3];
	double imageSpacing[3];
};

#endif // QtVTKRenderWindows_H
