#include "ui_QtVTKRenderWindows.h"
#include "QtVTKRenderWindows.h"

#include <vtkRenderer.h>
#include <vtkRenderWindow.h>
#include "vtkResliceImageViewer.h"
#include "vtkResliceCursorLineRepresentation.h"
#include "vtkResliceCursorThickLineRepresentation.h"
#include "vtkResliceCursorWidget.h"
#include "vtkResliceCursorActor.h"
#include "vtkResliceCursorPolyDataAlgorithm.h"
#include "vtkResliceCursor.h"
#include "vtkDICOMImageReader.h"
#include "vtkCellPicker.h"
#include "vtkProperty.h"
#include "vtkPlane.h"
#include "vtkImageData.h"
#include "vtkCommand.h"
#include "vtkPlaneSource.h"
#include "vtkLookupTable.h"
#include "vtkImageMapToWindowLevelColors.h"
#include "vtkInteractorStyleImage.h"
#include "vtkImageSlabReslice.h"
#include "vtkBoundedPlanePointPlacer.h"
#include "vtkDistanceWidget.h"
#include "vtkDistanceRepresentation.h"
#include "vtkHandleRepresentation.h"
#include "vtkResliceImageViewerMeasurements.h"
#include "vtkDistanceRepresentation2D.h"
#include "vtkPointHandleRepresentation3D.h"
#include "vtkPointHandleRepresentation2D.h"
#include "vtkTransform.h"
#include "vtkColorTransferFunction.h"
#include "vtkPiecewiseFunction.h"
#include "vtkVolumeProperty.h"
#include "vtkSmartVolumeMapper.h"
#include "vtkImageResample.h"
#include "vtkNew.h"

#include "vtkMultiBlockPLOT3DReader.h"
#include "vtkStreamLine.h"
#include "vtkPolyDataMapper.h"
#include "vtkStructuredGridOutlineFilter.h"
#include <vtkMultiBlockPLOT3DReader.h>
#include <vtkMultiBlockDataSet.h>

#include "qlabel.h"



//----------------------------------------------------------------------------
class vtkResliceCursorCallback : public vtkCommand
{
public:
	static vtkResliceCursorCallback *New()
	{ return new vtkResliceCursorCallback; }

	void Execute( vtkObject *caller, unsigned long ev,
		void *callData )
	{

		if (ev == vtkResliceCursorWidget::WindowLevelEvent ||
			ev == vtkCommand::WindowLevelEvent ||
			ev == vtkResliceCursorWidget::ResliceThicknessChangedEvent)
		{
			// Render everything
			for (int i = 0; i < 3; i++)
			{
				this->RCW[i]->Render();
			}
			this->IPW[0]->GetInteractor()->GetRenderWindow()->Render();
			return;
		}

		vtkImagePlaneWidget* ipw =
			dynamic_cast< vtkImagePlaneWidget* >( caller );
		if (ipw)
		{
			double* wl = static_cast<double*>( callData );

			if ( ipw == this->IPW[0] )
			{
				this->IPW[1]->SetWindowLevel(wl[0],wl[1],1);
				this->IPW[2]->SetWindowLevel(wl[0],wl[1],1);
			}
			else if( ipw == this->IPW[1] )
			{
				this->IPW[0]->SetWindowLevel(wl[0],wl[1],1);
				this->IPW[2]->SetWindowLevel(wl[0],wl[1],1);
			}
			else if (ipw == this->IPW[2])
			{
				this->IPW[0]->SetWindowLevel(wl[0],wl[1],1);
				this->IPW[1]->SetWindowLevel(wl[0],wl[1],1);
			}
		}

		vtkResliceCursorWidget *rcw = dynamic_cast<
			vtkResliceCursorWidget * >(caller);
		if (rcw)
		{
			vtkResliceCursorLineRepresentation *rep = dynamic_cast<
				vtkResliceCursorLineRepresentation * >(rcw->GetRepresentation());
			// Although the return value is not used, we keep the get calls
			// in case they had side-effects
			rep->GetResliceCursorActor()->GetCursorAlgorithm()->GetResliceCursor();
			for (int i = 0; i < 3; i++)
			{
				vtkPlaneSource *ps = static_cast< vtkPlaneSource * >(
					this->IPW[i]->GetPolyDataAlgorithm());
				ps->SetOrigin(this->RCW[i]->GetResliceCursorRepresentation()->
					GetPlaneSource()->GetOrigin());
				ps->SetPoint1(this->RCW[i]->GetResliceCursorRepresentation()->
					GetPlaneSource()->GetPoint1());
				ps->SetPoint2(this->RCW[i]->GetResliceCursorRepresentation()->
					GetPlaneSource()->GetPoint2());

				// If the reslice plane has modified, update it on the 3D widget
				this->IPW[i]->UpdatePlacement();
			}
		}

		// Render everything
		for (int i = 0; i < 3; i++)
		{
			this->RCW[i]->Render();
		}
		this->IPW[0]->GetInteractor()->GetRenderWindow()->Render();
	}

	vtkResliceCursorCallback() {}
	vtkImagePlaneWidget* IPW[3];
	vtkResliceCursorWidget *RCW[3];
};

vtkSmartPointer<vtkImageAlgorithm> ReadImageData(int argc, char *argv[])
{
	int count = 1;
	char *dirname = NULL;
	char *fileName=0;
	int fileType=0;
	vtkSmartPointer<vtkImageAlgorithm> readerRet;
	//while ( count < argc )
	//{
		if ( !strcmp( argv[count], "-DICOM" ) )
		{
			dirname = new char[strlen(argv[count+1])+1];
			sprintf( dirname, "%s", argv[count+1] );
			count += 2;
			vtkSmartPointer<vtkDICOMImageReader> reader =
				vtkSmartPointer<vtkDICOMImageReader>::New();
			reader->SetDirectoryName(dirname);
			readerRet = reader;
		}
		else if ( !strcmp( argv[count], "-PLOT3D" ) )
		{
			//fileName = new char[strlen(argv[count+1])+1];
			////fileType = VTI_FILETYPE;
			//sprintf( fileName, "%s", argv[count+1] );

			std::string xyzFile = argv[count+1]; // "combxyz.bin";
			std::string qFile = argv[count+2]; // "combq.bin";
			count += 3;

			vtkSmartPointer<vtkMultiBlockPLOT3DReader> pl3d =
				vtkSmartPointer<vtkMultiBlockPLOT3DReader>::New();
			pl3d->SetXYZFileName(xyzFile.c_str());
			pl3d->SetQFileName(qFile.c_str());
			pl3d->SetScalarFunctionNumber(100);
			pl3d->SetVectorFunctionNumber(202);
			//readerRet = pl3d;
			//pl3d->Update();

		}
	//}


	readerRet->Update();

	return readerRet;
	//readerRet = reader;
	//	reader->GetOutput()->GetDimensions(imageDims);
}


QtVTKRenderWindows::QtVTKRenderWindows( int argc, char *argv[])
{
	this->ui = new Ui_QtVTKRenderWindows;
	this->ui->setupUi(this);

	vtkImageData *input=0;
	vtkSmartPointer<vtkImageAlgorithm> reader = ReadImageData(argc, argv);
	input = reader->GetOutput();
	int imageDims[3];
	input->GetDimensions(imageDims);


	for (int i = 0; i < 3; i++)
	{
		riw[i] = vtkSmartPointer< vtkResliceImageViewer >::New();
	}

	this->ui->view1->SetRenderWindow(riw[0]->GetRenderWindow());
	riw[0]->SetupInteractor(
		this->ui->view1->GetRenderWindow()->GetInteractor());

	this->ui->view2->SetRenderWindow(riw[1]->GetRenderWindow());
	riw[1]->SetupInteractor(
		this->ui->view2->GetRenderWindow()->GetInteractor());

	this->ui->view3->SetRenderWindow(riw[2]->GetRenderWindow());
	riw[2]->SetupInteractor(
		this->ui->view3->GetRenderWindow()->GetInteractor());

	for (int i = 0; i < 3; i++)
	{
		// make them all share the same reslice cursor object.
		vtkResliceCursorLineRepresentation *rep =
			vtkResliceCursorLineRepresentation::SafeDownCast(
			riw[i]->GetResliceCursorWidget()->GetRepresentation());
		riw[i]->SetResliceCursor(riw[0]->GetResliceCursor());

		rep->GetResliceCursorActor()->
			GetCursorAlgorithm()->SetReslicePlaneNormal(i);

		riw[i]->SetInputData(input);
		riw[i]->SetSliceOrientation(i);
		riw[i]->SetResliceModeToAxisAligned();
	}

	vtkSmartPointer<vtkCellPicker> picker =
		vtkSmartPointer<vtkCellPicker>::New();
	picker->SetTolerance(0.005);

	vtkSmartPointer<vtkProperty> ipwProp =
		vtkSmartPointer<vtkProperty>::New();

	vtkSmartPointer< vtkRenderer > ren =
		vtkSmartPointer< vtkRenderer >::New();

	this->ui->view4->GetRenderWindow()->AddRenderer(ren);
	vtkRenderWindowInteractor *iren = this->ui->view4->GetInteractor();

	for (int i = 0; i < 3; i++)
	{
		planeWidget[i] = vtkSmartPointer<vtkImagePlaneWidget>::New();
		planeWidget[i]->SetInteractor( iren );
		planeWidget[i]->SetPicker(picker);
		planeWidget[i]->RestrictPlaneToVolumeOn();
		double color[3] = {0, 0, 0};
		color[i] = 1;
		planeWidget[i]->GetPlaneProperty()->SetColor(color);

		color[0] /= 4.0;
		color[1] /= 4.0;
		color[2] /= 4.0;
		riw[i]->GetRenderer()->SetBackground( color );

		planeWidget[i]->SetTexturePlaneProperty(ipwProp);
		planeWidget[i]->TextureInterpolateOff();
		planeWidget[i]->SetResliceInterpolateToLinear();
		planeWidget[i]->SetInputConnection(reader->GetOutputPort());
		planeWidget[i]->SetPlaneOrientation(i);
		planeWidget[i]->SetSliceIndex(imageDims[i]/2);
		planeWidget[i]->DisplayTextOn();
		planeWidget[i]->SetDefaultRenderer(ren);
		planeWidget[i]->SetWindowLevel(1358, -27);
		planeWidget[i]->On();
		planeWidget[i]->InteractionOn();
	}

	//////////////////volume rendering
	vtkSmartPointer< vtkRenderer > renVol =
		vtkSmartPointer< vtkRenderer >::New();
	this->ui->view5->GetRenderWindow()->AddRenderer(renVol);
	vtkRenderWindowInteractor *irenVol = this->ui->view4->GetInteractor();



	vtkImageResample *resample = vtkImageResample::New();

	// Create our volume and mapper
	vtkVolume *volume = vtkVolume::New();
	vtkSmartVolumeMapper *mapper = vtkSmartVolumeMapper::New();

	mapper->SetInputConnection( reader->GetOutputPort() );


	// Create our transfer function
	vtkColorTransferFunction *colorFun = vtkColorTransferFunction::New();
	vtkPiecewiseFunction *opacityFun = vtkPiecewiseFunction::New();

	// Create the property and attach the transfer functions
	vtkVolumeProperty *property = vtkVolumeProperty::New();
	property->SetIndependentComponents(true);
	property->SetColor( colorFun );
	property->SetScalarOpacity( opacityFun );
	property->SetInterpolationTypeToLinear();

	// connect up the volume to the property and the mapper
	volume->SetProperty( property );
	volume->SetMapper( mapper );

	// Depending on the blend type selected as a command line option,
	// adjustthe transfer function
	colorFun->AddRGBPoint( -3024, 0, 0, 0, 0.5, 0.0 );
	colorFun->AddRGBPoint( -16, 0.73, 0.25, 0.30, 0.49, .61 );
	colorFun->AddRGBPoint( 641, .90, .82, .56, .5, 0.0 );
	colorFun->AddRGBPoint( 3071, 1, 1, 1, .5, 0.0 );

	opacityFun->AddPoint(-3024, 0, 0.5, 0.0 );
	opacityFun->AddPoint(-16, 0, .49, .61 );
	opacityFun->AddPoint(641, .72, .5, 0.0 );
	opacityFun->AddPoint(3071, .71, 0.5, 0.0);

	mapper->SetBlendModeToComposite();
	property->ShadeOn();
	property->SetAmbient(0.1);
	property->SetDiffuse(0.9);
	property->SetSpecular(0.2);
	property->SetSpecularPower(10.0);
	property->SetScalarOpacityUnitDistance(0.8919);

	// Add the volume to the scene
	renVol->AddVolume( volume );
	////////////////////////////

	//////////////////
	vtkSmartPointer< vtkRenderer > renSL =
		vtkSmartPointer< vtkRenderer >::New();
	this->ui->view6->GetRenderWindow()->AddRenderer(renSL);
	vtkRenderWindowInteractor *irenSL = this->ui->view4->GetInteractor();

	std::string xyzFile = argv[3]; // "combxyz.bin";
	std::string qFile = argv[4]; // "combq.bin";

  vtkSmartPointer<vtkMultiBlockPLOT3DReader> pl3d =
    vtkSmartPointer<vtkMultiBlockPLOT3DReader>::New();

	pl3d->SetXYZFileName(xyzFile.c_str());
	pl3d->SetQFileName(qFile.c_str());
	pl3d->SetScalarFunctionNumber(100);
	pl3d->SetVectorFunctionNumber(202);
	pl3d->Update();
	//	int imageDims[3];
	//pl3d->GetOutput()->GetBlock(0)->get ->GetDimensions(imageDims);

	// Source of the streamlines
	vtkSmartPointer<vtkPlaneSource> seeds = 
		vtkSmartPointer<vtkPlaneSource>::New();
	seeds->SetXResolution(4);
	seeds->SetYResolution(4);
	seeds->SetOrigin(2,-2,26);
	seeds->SetPoint1(2,2,26);
	seeds->SetPoint2(2,-2,32);
#if 1

	// Streamline itself
	vtkSmartPointer<vtkStreamLine> streamLine = 
		vtkSmartPointer<vtkStreamLine>::New();
#if VTK_MAJOR_VERSION <= 5
	streamLine->SetInputConnection(pl3d->GetOutputPort());
	streamLine->SetSource(seeds->GetOutput());
#else
	pl3d->Update();
	streamLine->SetInputData(pl3d->GetOutput()->GetBlock(0));
	streamLine->SetSourceConnection(seeds->GetOutputPort());
#endif
	//streamLine->SetStartPosition(2,-2,30);
	// as alternative to the SetSource(), which can handle multiple
	// streamlines, you can set a SINGLE streamline from
	// SetStartPosition()
	streamLine->SetMaximumPropagationTime(200);
	streamLine->SetIntegrationStepLength(.2);
	streamLine->SetStepLength(.001);
	streamLine->SetNumberOfThreads(1);
	streamLine->SetIntegrationDirectionToForward();
	streamLine->VorticityOn();

	vtkSmartPointer<vtkPolyDataMapper> streamLineMapper = 
		vtkSmartPointer<vtkPolyDataMapper>::New();
	streamLineMapper->SetInputConnection(streamLine->GetOutputPort());

	vtkSmartPointer<vtkActor> streamLineActor = 
		vtkSmartPointer<vtkActor>::New();
	streamLineActor->SetMapper(streamLineMapper);
	streamLineActor->VisibilityOn();

	// Outline-Filter for the grid
	vtkSmartPointer<vtkStructuredGridOutlineFilter> outline = 
		vtkSmartPointer<vtkStructuredGridOutlineFilter>::New();
#if VTK_MAJOR_VERSION <= 5
	outline->SetInputConnection(pl3d->GetOutputPort());
#else
	outline->SetInputData(pl3d->GetOutput()->GetBlock(0));
#endif
	vtkSmartPointer<vtkPolyDataMapper> outlineMapper = 
		vtkSmartPointer<vtkPolyDataMapper>::New();
	outlineMapper->SetInputConnection(outline->GetOutputPort());

	vtkSmartPointer<vtkActor> outlineActor = 
		vtkSmartPointer<vtkActor>::New();
	outlineActor->SetMapper(outlineMapper);
	outlineActor->GetProperty()->SetColor(1, 1, 1);

	// Create the RenderWindow, Renderer and Actors
	//vtkSmartPointer<vtkRenderer> renderer = 
	//	vtkSmartPointer<vtkRenderer>::New();
	/*vtkSmartPointer<vtkRenderWindow> renderWindow = 
		vtkSmartPointer<vtkRenderWindow>::New();
	renderWindow->AddRenderer(renderer);*/

	//vtkSmartPointer<vtkRenderWindowInteractor> interactor = 
	//	vtkSmartPointer<vtkRenderWindowInteractor>::New();
	//interactor->SetRenderWindow(renderWindow);

	vtkSmartPointer<vtkInteractorStyleTrackballCamera> style = 
		vtkSmartPointer<vtkInteractorStyleTrackballCamera>::New();
	irenSL->SetInteractorStyle(style);

	renSL->AddActor(streamLineActor);
	renSL->AddActor(outlineActor);

	// Add the actors to the renderer, set the background and size
	renSL->SetBackground(0.1, 0.2, 0.4);
	//renderWindow->SetSize(300, 300);
	irenSL->Initialize();
	//renderWindow->Render();
	////////////////////
#endif


	vtkSmartPointer<vtkResliceCursorCallback> cbk =
		vtkSmartPointer<vtkResliceCursorCallback>::New();

	for (int i = 0; i < 3; i++)
	{
		cbk->IPW[i] = planeWidget[i];
		cbk->RCW[i] = riw[i]->GetResliceCursorWidget();
		riw[i]->GetResliceCursorWidget()->AddObserver(
			vtkResliceCursorWidget::ResliceAxesChangedEvent, cbk );
		riw[i]->GetResliceCursorWidget()->AddObserver(
			vtkResliceCursorWidget::WindowLevelEvent, cbk );
		riw[i]->GetResliceCursorWidget()->AddObserver(
			vtkResliceCursorWidget::ResliceThicknessChangedEvent, cbk );
		riw[i]->GetResliceCursorWidget()->AddObserver(
			vtkResliceCursorWidget::ResetCursorEvent, cbk );
		riw[i]->GetInteractorStyle()->AddObserver(
			vtkCommand::WindowLevelEvent, cbk );

		// Make them all share the same color map.
		riw[i]->SetLookupTable(riw[0]->GetLookupTable());
		planeWidget[i]->GetColorMap()->SetLookupTable(riw[0]->GetLookupTable());
		//planeWidget[i]->GetColorMap()->SetInput(riw[i]->GetResliceCursorWidget()->GetResliceCursorRepresentation()->GetColorMap()->GetInput());
		planeWidget[i]->SetColorMap(riw[i]->GetResliceCursorWidget()->GetResliceCursorRepresentation()->GetColorMap());

	}

	this->ui->view1->show();
	this->ui->view2->show();
	this->ui->view3->show();

	// Set up action signals and slots
	connect(this->ui->actionExit, SIGNAL(triggered()), this, SLOT(slotExit()));
	connect(this->ui->resliceModeCheckBox, SIGNAL(stateChanged(int)), this, SLOT(resliceMode(int)));
	connect(this->ui->thickModeCheckBox, SIGNAL(stateChanged(int)), this, SLOT(thickMode(int)));
	this->ui->thickModeCheckBox->setEnabled(0);

	connect(this->ui->radioButton_Max, SIGNAL(pressed()), this, SLOT(SetBlendModeToMaxIP()));
	connect(this->ui->radioButton_Min, SIGNAL(pressed()), this, SLOT(SetBlendModeToMinIP()));
	connect(this->ui->radioButton_Mean, SIGNAL(pressed()), this, SLOT(SetBlendModeToMeanIP()));
	this->ui->blendModeGroupBox->setEnabled(0);

	connect(this->ui->resetButton, SIGNAL(pressed()), this, SLOT(ResetViews()));
	connect(this->ui->AddDistance1Button, SIGNAL(pressed()), this, SLOT(AddDistanceMeasurementToView1()));

	controller.addListener(listener);
	controller.enableGesture(Leap::Gesture::TYPE_CIRCLE);
	//QLabel frameLabel;
	//frameLabel.setMinimumSize(200, 50);
	//frameLabel.show();
	this->ui->label_observer->connect(&listener, SIGNAL(objectNameChanged(QString)),
		SLOT(setText(QString)));
	connect(&listener, SIGNAL(translate(float)), this, SLOT(SimpleTranslate(float)));

	//reader->Delete();
};

void QtVTKRenderWindows::slotExit()
{
	qApp->exit();
}

void QtVTKRenderWindows::resliceMode(int mode)
{
	this->ui->thickModeCheckBox->setEnabled(mode ? 1 : 0);
	this->ui->blendModeGroupBox->setEnabled(mode ? 1 : 0);

	for (int i = 0; i < 3; i++)
	{
		riw[i]->SetResliceMode(mode ? 1 : 0);
		riw[i]->GetRenderer()->ResetCamera();
		riw[i]->Render();
	}
}

void QtVTKRenderWindows::thickMode(int mode)
{
	for (int i = 0; i < 3; i++)
	{
		riw[i]->SetThickMode(mode ? 1 : 0);
		riw[i]->Render();
	}
}

void QtVTKRenderWindows::SetBlendMode(int m)
{
	for (int i = 0; i < 3; i++)
	{
		vtkImageSlabReslice *thickSlabReslice = vtkImageSlabReslice::SafeDownCast(
			vtkResliceCursorThickLineRepresentation::SafeDownCast(
			riw[i]->GetResliceCursorWidget()->GetRepresentation())->GetReslice());
		thickSlabReslice->SetBlendMode(m);
		riw[i]->Render();
	}
}

void QtVTKRenderWindows::SetBlendModeToMaxIP()
{
	this->SetBlendMode(VTK_IMAGE_SLAB_MAX);
}

void QtVTKRenderWindows::SetBlendModeToMinIP()
{
	this->SetBlendMode(VTK_IMAGE_SLAB_MIN);
}

void QtVTKRenderWindows::SetBlendModeToMeanIP()
{
	this->SetBlendMode(VTK_IMAGE_SLAB_MEAN);
}

void QtVTKRenderWindows::ResetViews()
{
	// Reset the reslice image views
	for (int i = 0; i < 3; i++)
	{
		riw[i]->Reset();
	}

	// Also sync the Image plane widget on the 3D top right view with any
	// changes to the reslice cursor.
	for (int i = 0; i < 3; i++)
	{
		vtkPlaneSource *ps = static_cast< vtkPlaneSource * >(
			planeWidget[i]->GetPolyDataAlgorithm());
		ps->SetNormal(riw[0]->GetResliceCursor()->GetPlane(i)->GetNormal());
		ps->SetCenter(riw[0]->GetResliceCursor()->GetPlane(i)->GetOrigin());

		// If the reslice plane has modified, update it on the 3D widget
		this->planeWidget[i]->UpdatePlacement();
	}

	// Render in response to changes.
	this->Render();
}

void QtVTKRenderWindows::Render()
{
	for (int i = 0; i < 3; i++)
	{
		riw[i]->Render();
	}
	this->ui->view3->GetRenderWindow()->Render();
}

void QtVTKRenderWindows::AddDistanceMeasurementToView1()
{
	this->AddDistanceMeasurementToView(1);
}

void QtVTKRenderWindows::AddDistanceMeasurementToView(int i)
{
	// remove existing widgets.
	if (this->DistanceWidget[i])
	{
		this->DistanceWidget[i]->SetEnabled(0);
		this->DistanceWidget[i] = NULL;
	}

	// add new widget
	this->DistanceWidget[i] = vtkSmartPointer< vtkDistanceWidget >::New();
	this->DistanceWidget[i]->SetInteractor(
		this->riw[i]->GetResliceCursorWidget()->GetInteractor());

	// Set a priority higher than our reslice cursor widget
	this->DistanceWidget[i]->SetPriority(
		this->riw[i]->GetResliceCursorWidget()->GetPriority() + 0.01);

	vtkSmartPointer< vtkPointHandleRepresentation2D > handleRep =
		vtkSmartPointer< vtkPointHandleRepresentation2D >::New();
	vtkSmartPointer< vtkDistanceRepresentation2D > distanceRep =
		vtkSmartPointer< vtkDistanceRepresentation2D >::New();
	distanceRep->SetHandleRepresentation(handleRep);
	this->DistanceWidget[i]->SetRepresentation(distanceRep);
	distanceRep->InstantiateHandleRepresentation();
	distanceRep->GetPoint1Representation()->SetPointPlacer(riw[i]->GetPointPlacer());
	distanceRep->GetPoint2Representation()->SetPointPlacer(riw[i]->GetPointPlacer());

	// Add the distance to the list of widgets whose visibility is managed based
	// on the reslice plane by the ResliceImageViewerMeasurements class
	this->riw[i]->GetMeasurements()->AddItem(this->DistanceWidget[i]);

	this->DistanceWidget[i]->CreateDefaultRepresentation();
	this->DistanceWidget[i]->EnabledOn();
}

void QtVTKRenderWindows::SimpleTranslate(float v)
{
	double a[3], b[3];
	planeWidget[0]->GetOrigin(a);
	a[0] += v;
	planeWidget[0]->SetOrigin(a);
	planeWidget[0]->UpdatePlacement();
	ui->view4->repaint();
}
