#include <irrlicht.h>

using namespace irr;
using namespace gui;
using namespace video;
using namespace core;

#ifdef _MSC_VER
#pragma comment(lib, "Irrlicht.lib")
#endif

// To DO: Finish Y Rotation
IrrlichtDevice *Device = 0;
core::stringc ScalingXML; 
core::stringc YRotXML;
core::stringc OffX;
core::stringc OffY;
core::stringc OffZ;
core::stringc StartUpModelFile;
core::stringw MessageText;
core::stringw Caption;
scene::ISceneNode* Model = 0;
float ModelXRotation = 0;
float ModelYRotation = 0;
float ModelZRotation = 0;
float OffsetX = 0;
float OffsetY = 0;
float OffsetZ = 0;
bool Octree=false;
bool doSave = false;
bool UseLight=false;
bool doScale = false;
bool doOffset = false;
bool doRotation = false;

scene::ICameraSceneNode* Camera[2] = {0, 0};

// Values used to identify individual GUI elements
enum
{
	GUI_ID_DIALOG_ROOT_WINDOW  = 0x10000,

	GUI_ID_X_SCALE,
	GUI_ID_Y_SCALE,
	GUI_ID_Z_SCALE,
	GUI_ID_Y_ROT,

	GUI_ID_X_OFF,
	GUI_ID_Y_OFF,
	GUI_ID_Z_OFF,
	GUI_ID_OFFSET,

	GUI_ID_OPEN_MODEL,
	GUI_ID_SET_MODEL_ARCHIVE,
	GUI_ID_LOAD_AS_OCTREE,

	GUI_ID_TOGGLE_DEBUG_INFO,

	GUI_ID_DEBUG_OFF,
	GUI_ID_DEBUG_BOUNDING_BOX,
	GUI_ID_DEBUG_NORMALS,
	GUI_ID_DEBUG_SKELETON,
	GUI_ID_DEBUG_WIRE_OVERLAY,
	GUI_ID_DEBUG_HALF_TRANSPARENT,
	GUI_ID_DEBUG_BUFFERS_BOUNDING_BOXES,
	GUI_ID_DEBUG_ALL,

	GUI_ID_MODEL_MATERIAL_SOLID,
	GUI_ID_MODEL_MATERIAL_TRANSPARENT,
	GUI_ID_MODEL_MATERIAL_REFLECTION,

	GUI_ID_CAMERA_MAYA,
	GUI_ID_CAMERA_FIRST_PERSON,

	GUI_ID_POSITION_TEXT,

	GUI_ID_QUIT_DISCARD,
	GUI_ID_QUIT_SAVE,

	GUI_ID_SKIN_TRANSPARENCY,
	GUI_ID_SKIN_ANIMATION_FPS,

	GUI_ID_BUTTON_SET_SCALE,
	GUI_ID_BUTTON_SET_Y_ROT,
	GUI_ID_BUTTON_SCALE_MUL10,
	GUI_ID_BUTTON_SCALE_DIV10,
	GUI_ID_BUTTON_SELECT_ARCHIVE,

	GUI_ID_ANIMATION_INFO,

	// And some magic numbers
	MAX_FRAMERATE = 50,
	DEFAULT_FRAMERATE = 30,
};


/*
Toggle between various cameras
*/
void setActiveCamera(scene::ICameraSceneNode* newActive)
{
	if (0 == Device)
		return;

	scene::ICameraSceneNode * active = Device->getSceneManager()->getActiveCamera();
	active->setInputReceiverEnabled(false);

	float PosX = active->getPosition().X;
    float PosY = active->getPosition().Y;
    float PosZ = active->getPosition().Z;
	float TgtX = active->getTarget().X;
    float TgtY = active->getTarget().Y;
    float TgtZ = active->getTarget().Z;

	newActive->setInputReceiverEnabled(true);
	Device->getSceneManager()->setActiveCamera(newActive);
	
	newActive->setPosition(core::vector3df(PosX, PosY, PosZ));
	//newActive->setTarget(core::vector3df(TgtX, TgtY, TgtZ));
}

/*
	Set the skin transparency by changing the alpha values of all skin-colors
*/
void setSkinTransparency(s32 alpha, irr::gui::IGUISkin * skin)
{
	for (s32 i=0; i<irr::gui::EGDC_COUNT ; ++i)
	{
		video::SColor col = skin->getColor((EGUI_DEFAULT_COLOR)i);
		col.setAlpha(alpha);
		skin->setColor((EGUI_DEFAULT_COLOR)i, col);
	}
}

/*
  Update the display of the model scaling
*/
/*

*/

void updateScaleInfo(scene::ISceneNode* model)
{
    IGUIEnvironment* env = Device->getGUIEnvironment();
	gui::IGUIElement* root = env->getRootGUIElement();
	core::vector3df scale = model->getScale();
	//core::stringw teste;
	char teste[200];
	sprintf(teste, "%0.2f", scale.X);
	root->getElementFromId(GUI_ID_X_SCALE, true)->setText( core::stringw(teste).c_str() );
}

void updateRotYInfo(scene::ISceneNode* model)
{
	IGUIEnvironment* env = Device->getGUIEnvironment();
	gui::IGUIElement* root = env->getRootGUIElement();
	core::vector3df rot = model->getRotation();
	ModelYRotation = (int)(3600 + (int)rot.Y)%360;
	root->getElementFromId(GUI_ID_Y_ROT, true)->setText( core::stringw((int)rot.Y).c_str() );

}

void updateOffsetInfo(scene::ISceneNode* model)
{
    IGUIEnvironment* env = Device->getGUIEnvironment();
	gui::IGUIElement* root = env->getRootGUIElement();
	core::vector3df offset = model->getPosition();
	root->getElementFromId(GUI_ID_X_OFF, true)->setText( core::stringw((int)offset.X).c_str() );
	root->getElementFromId(GUI_ID_Y_OFF, true)->setText( core::stringw((int)offset.Y).c_str() );
	root->getElementFromId(GUI_ID_Z_OFF, true)->setText( core::stringw((int)offset.Z).c_str() );
}

/*

/*
The second function loadModel() loads a model and displays it using an
addAnimatedMeshSceneNode and the scene manager. Nothing difficult. It also
displays a short message box, if the model could not be loaded.
*/
void loadModel(const c8* fn)
{
	// modify the name if it a .pk3 file

	io::path filename(fn);

	io::path extension;
	core::getFileNameExtension(extension, filename);
	extension.make_lower();

	// if a texture is loaded apply it to the current model..
	if (extension == ".jpg" || extension == ".pcx" ||
		extension == ".png" || extension == ".ppm" ||
		extension == ".pgm" || extension == ".pbm" ||
		extension == ".psd" || extension == ".tga" ||
		extension == ".bmp" || extension == ".wal" ||
		extension == ".rgb" || extension == ".rgba")
	{
		video::ITexture * texture =
			Device->getVideoDriver()->getTexture( filename );
		if ( texture && Model )
		{
			// always reload texture
			Device->getVideoDriver()->removeTexture(texture);
			texture = Device->getVideoDriver()->getTexture( filename );

			Model->setMaterialTexture(0, texture);
		}
		return;
	}
	// if a archive is loaded add it to the FileArchive..
	else if (extension == ".pk3" || extension == ".zip" || extension == ".pak" || extension == ".npk")
	{
		Device->getFileSystem()->addFileArchive(filename.c_str());
		return;
	}

	// load a model into the engine

	if (Model)
		Model->remove();

	Model = 0;

	scene::IAnimatedMesh* m = Device->getSceneManager()->getMesh( filename.c_str() );

	if (!m)
	{
		// model could not be loaded

		if (StartUpModelFile != filename)
			Device->getGUIEnvironment()->addMessageBox(
			Caption.c_str(), L"The model could not be loaded. " \
			L"Maybe it is not a supported file format.");
		return;
	}

	// set default material properties

	if (Octree)
		Model = Device->getSceneManager()->addOctreeSceneNode(m->getMesh(0));
	else
	{
		scene::IAnimatedMeshSceneNode* animModel = Device->getSceneManager()->addAnimatedMeshSceneNode(m);
		animModel->setAnimationSpeed(30);
		Model = animModel;
	}
	Model->setMaterialFlag(video::EMF_LIGHTING, UseLight);
	Model->setMaterialFlag(video::EMF_NORMALIZE_NORMALS, UseLight);
  	Model->setMaterialFlag(video::EMF_BACK_FACE_CULLING, true);
	Model->setDebugDataVisible(scene::EDS_OFF);

	// we need to uncheck the menu entries. would be cool to fake a menu event, but
	// that's not so simple. so we do it brute force
	gui::IGUIContextMenu* menu = (gui::IGUIContextMenu*)Device->getGUIEnvironment()->getRootGUIElement()->getElementFromId(GUI_ID_TOGGLE_DEBUG_INFO, true);
	if (menu)
		for(int item = 1; item < 6; ++item)
			menu->setItemChecked(item, false);
	updateScaleInfo(Model);
}



class MyEventReceiver : public IEventReceiver
{
public:
	virtual bool OnEvent(const SEvent& event)
	{
		
		if (event.EventType == EET_KEY_INPUT_EVENT)  // && event.KeyInput.PressedDown == false)
		{
			if ( OnKeyUp(event.KeyInput.Key) )
				return true;
		}

		if (event.EventType == EET_GUI_EVENT)
		{
			s32 id = event.GUIEvent.Caller->getID();
			IGUIEnvironment* env = Device->getGUIEnvironment();

			switch(event.GUIEvent.EventType)
			{
			case EGET_MENU_ITEM_SELECTED:
					// a menu item was clicked
					OnMenuItemSelected( (IGUIContextMenu*)event.GUIEvent.Caller );
				break;

			case EGET_FILE_SELECTED:
				{
					// load the model file, selected in the file open dialog
					IGUIFileOpenDialog* dialog =
						(IGUIFileOpenDialog*)event.GUIEvent.Caller;
					loadModel(core::stringc(dialog->getFileName()).c_str());
				}
				break;

			case EGET_SCROLL_BAR_CHANGED:

				// control skin transparency
				if (id == GUI_ID_SKIN_TRANSPARENCY)
				{
					const s32 pos = ((IGUIScrollBar*)event.GUIEvent.Caller)->getPos();
					setSkinTransparency(pos, env->getSkin());
				}
				// control animation speed
				else if (id == GUI_ID_SKIN_ANIMATION_FPS)
				{
					const s32 pos = ((IGUIScrollBar*)event.GUIEvent.Caller)->getPos();
					if (scene::ESNT_ANIMATED_MESH == Model->getType())
						((scene::IAnimatedMeshSceneNode*)Model)->setAnimationSpeed((f32)pos);
				}
				break;

			case EGET_BUTTON_CLICKED:

				switch(id)
				{
				case GUI_ID_BUTTON_SET_SCALE:
					{
						// set scale
						gui::IGUIElement* root = env->getRootGUIElement();
						core::vector3df scale;
						core::stringc s;

						s = root->getElementFromId(GUI_ID_X_SCALE, true)->getText();
						scale.X = (f32)atof(s.c_str());
						scale.Y = scale.X;
						scale.Z = scale.X;

						if (Model) Model->setScale(scale);
						updateScaleInfo(Model);
					}
					break;
				case GUI_ID_BUTTON_SET_Y_ROT:
					{
						gui::IGUIElement* root = env->getRootGUIElement();
						core::stringc s;
						s = root->getElementFromId(GUI_ID_Y_ROT, true)->getText();
						float YRot = (f32)atof(s.c_str());
						if (Model) Model->setRotation( core::vector3df(0, YRot,0) );
						updateRotYInfo(Model);
					}
					break;
				case GUI_ID_OFFSET:
					{
						gui::IGUIElement* root = env->getRootGUIElement();
						core::stringc x;
						core::stringc y;
						core::stringc z;
						x = root->getElementFromId(GUI_ID_X_OFF, true)->getText();
						y = root->getElementFromId(GUI_ID_Y_OFF, true)->getText();
						z = root->getElementFromId(GUI_ID_Z_OFF, true)->getText();
						float X = (f32)atof(x.c_str());
						float Y = (f32)atof(y.c_str());
						float Z = (f32)atof(z.c_str());
						Model->setPosition( core::vector3df(X,Y,Z));
					}
					break;
				case GUI_ID_BUTTON_SCALE_MUL10:
					if (Model)
						Model->setScale(Model->getScale()*10.f);
					updateScaleInfo(Model);
					break;
				case GUI_ID_BUTTON_SCALE_DIV10:
					if (Model)
						Model->setScale(Model->getScale()*0.1f);
					updateScaleInfo(Model);
					break;
				case GUI_ID_BUTTON_SELECT_ARCHIVE:
					env->addFileOpenDialog(L"Please select your game archive/directory");
					break;
				}

				break;
			default:
				break;
			}
		}

		return false;
	}


	/*
		Handle key-up events
	*/
	bool OnKeyUp(irr::EKEY_CODE keyCode)
	{
		if (keyCode == irr::KEY_ESCAPE)
		{
			if (Device)
			{
				scene::ICameraSceneNode * camera = Device->getSceneManager()->getActiveCamera();

				if (camera)
				{
					camera->setInputReceiverEnabled( !camera->isInputReceiverEnabled() );
				}
				setActiveCamera(Camera[0]);
				return true;
			}
		}
		if (keyCode == irr::KEY_KEY_E)	
		{
			ModelYRotation += 1;
			if (Model) Model->setRotation( core::vector3df(0, ModelYRotation,0) );
			updateRotYInfo(Model);
		}
		if (keyCode == irr::KEY_KEY_Q)	
		{
			ModelYRotation -= 1;
			if (Model) Model->setRotation( core::vector3df(0, ModelYRotation,0) );
			updateRotYInfo(Model);
		}
		//if (keyCode == irr::KEY_KEY_A)	ModelXRotation += 90;
		//if (keyCode == irr::KEY_KEY_D)	ModelXRotation -= 90;
//		if (keyCode == irr::KEY_KEY_W)	ModelZRotation += 90;
		//if (keyCode == irr::KEY_KEY_S)	ModelZRotation -= 90;

 		if (keyCode == irr::KEY_KEY_A)	OffsetX += 100;
		if (keyCode == irr::KEY_KEY_D)	OffsetX -= 100;
		if (keyCode == irr::KEY_KEY_W)	OffsetZ -= 100;
		if (keyCode == irr::KEY_KEY_S)	OffsetZ += 100;
		
		return false;
	}


	/*
		Handle "menu item clicked" events.
	*/
	void OnMenuItemSelected( IGUIContextMenu* menu )
	{
		s32 id = menu->getItemCommandId(menu->getSelectedItem());
		IGUIEnvironment* env = Device->getGUIEnvironment();

		switch(id)
		{
		case GUI_ID_OPEN_MODEL: // FilOnButtonSetScalinge -> Open Model
			env->addFileOpenDialog(L"Please select a model file to open" );
			break;
		case GUI_ID_SET_MODEL_ARCHIVE: // File -> Set Model Archive
			env->addFileOpenDialog(L"Please select your game archive/directory");
			break;
		case GUI_ID_LOAD_AS_OCTREE: // File -> LoadAsOctree
			Octree = !Octree;
			menu->setItemChecked(menu->getSelectedItem(), Octree);
			break;
		case GUI_ID_QUIT_DISCARD: // File -> Quit
			Device->closeDevice();
			doSave = false;
			break;
		case GUI_ID_QUIT_SAVE: // File -> Quit
			Device->closeDevice();			
			doSave = true;
			break;

		//case GUI_ID_SKY_BOX_VISIBLE: // View -> Skybox
			//menu->setItemChecked(menu->getSelectedItem(), !menu->isItemChecked(menu->getSelectedItem()));
//			SkyBox->setVisible(!SkyBox->isVisible());
			break;
		case GUI_ID_DEBUG_OFF: // View -> Debug Information
			menu->setItemChecked(menu->getSelectedItem()+1, false);
			menu->setItemChecked(menu->getSelectedItem()+2, false);
			menu->setItemChecked(menu->getSelectedItem()+3, false);
			menu->setItemChecked(menu->getSelectedItem()+4, false);
			menu->setItemChecked(menu->getSelectedItem()+5, false);
			menu->setItemChecked(menu->getSelectedItem()+6, false);
			if (Model)
				Model->setDebugDataVisible(scene::EDS_OFF);
			break;
		case GUI_ID_DEBUG_BOUNDING_BOX: // View -> Debug Information
			menu->setItemChecked(menu->getSelectedItem(), !menu->isItemChecked(menu->getSelectedItem()));
			if (Model)
				Model->setDebugDataVisible((scene::E_DEBUG_SCENE_TYPE)(Model->isDebugDataVisible()^scene::EDS_BBOX));
			break;
		case GUI_ID_DEBUG_NORMALS: // View -> Debug Information
			menu->setItemChecked(menu->getSelectedItem(), !menu->isItemChecked(menu->getSelectedItem()));
			if (Model)
				Model->setDebugDataVisible((scene::E_DEBUG_SCENE_TYPE)(Model->isDebugDataVisible()^scene::EDS_NORMALS));
			break;
		case GUI_ID_DEBUG_SKELETON: // View -> Debug Information
			menu->setItemChecked(menu->getSelectedItem(), !menu->isItemChecked(menu->getSelectedItem()));
			if (Model)
				Model->setDebugDataVisible((scene::E_DEBUG_SCENE_TYPE)(Model->isDebugDataVisible()^scene::EDS_SKELETON));
			break;
		case GUI_ID_DEBUG_WIRE_OVERLAY: // View -> Debug Information
			menu->setItemChecked(menu->getSelectedItem(), !menu->isItemChecked(menu->getSelectedItem()));
			if (Model)
				Model->setDebugDataVisible((scene::E_DEBUG_SCENE_TYPE)(Model->isDebugDataVisible()^scene::EDS_MESH_WIRE_OVERLAY));
			break;
		case GUI_ID_DEBUG_HALF_TRANSPARENT: // View -> Debug Information
			menu->setItemChecked(menu->getSelectedItem(), !menu->isItemChecked(menu->getSelectedItem()));
			if (Model)
				Model->setDebugDataVisible((scene::E_DEBUG_SCENE_TYPE)(Model->isDebugDataVisible()^scene::EDS_HALF_TRANSPARENCY));
			break;
		case GUI_ID_DEBUG_BUFFERS_BOUNDING_BOXES: // View -> Debug Information
			menu->setItemChecked(menu->getSelectedItem(), !menu->isItemChecked(menu->getSelectedItem()));
			if (Model)
				Model->setDebugDataVisible((scene::E_DEBUG_SCENE_TYPE)(Model->isDebugDataVisible()^scene::EDS_BBOX_BUFFERS));
			break;
		case GUI_ID_DEBUG_ALL: // View -> Debug Information
			menu->setItemChecked(menu->getSelectedItem()-1, true);
			menu->setItemChecked(menu->getSelectedItem()-2, true);
			menu->setItemChecked(menu->getSelectedItem()-3, true);
			menu->setItemChecked(menu->getSelectedItem()-4, true);
			menu->setItemChecked(menu->getSelectedItem()-5, true);
			menu->setItemChecked(menu->getSelectedItem()-6, true);
			if (Model)
				Model->setDebugDataVisible(scene::EDS_FULL);
			break;
		case GUI_ID_MODEL_MATERIAL_SOLID: // View -> Material -> Solid
			if (Model)
				Model->setMaterialType(video::EMT_SOLID);
			break;
		case GUI_ID_MODEL_MATERIAL_TRANSPARENT: // View -> Material -> Transparent
			if (Model)
				Model->setMaterialType(video::EMT_TRANSPARENT_ADD_COLOR);
			break;
		case GUI_ID_MODEL_MATERIAL_REFLECTION: // View -> Material -> Reflection
			if (Model)
				Model->setMaterialType(video::EMT_SPHERE_MAP);
			break;

		case GUI_ID_CAMERA_MAYA:
			setActiveCamera(Camera[0]);
			break;
		case GUI_ID_CAMERA_FIRST_PERSON:
			setActiveCamera(Camera[1]);
			break;
		}
	}

};

int main(int argc, char* argv[])
{
	// ask user for driver 
	// To DO: arguments to get different videotype
	video::E_DRIVER_TYPE driverType=video::EDT_OPENGL;

	//video::E_DRIVER_TYPE driverType=video::EDT_DIRECT3D9;

	// create device and exit if creation failed
	MyEventReceiver receiver;
	Device = createDevice(driverType, core::dimension2d<u32>(800, 600),
		16, false, false, false, &receiver);
	if (Device == 0)
		return 1; // could not create selected driver.

	Device->setResizable(true);

	Device->setWindowCaption(L"Irrlicht Engine - Loading...");
	printf("%s%\n", argv[0]);
	printf("%s%\n", argv[1]);

	video::IVideoDriver* driver = Device->getVideoDriver();
	IGUIEnvironment* env = Device->getGUIEnvironment();
	scene::ISceneManager* smgr = Device->getSceneManager();
	smgr->getParameters()->setAttribute(scene::COLLADA_CREATE_SCENE_INSTANCES, true);

	driver->setTextureCreationFlag(video::ETCF_ALWAYS_32_BIT, true);

	smgr->addLightSceneNode(0, core::vector3df(200,200,200),
		video::SColorf(1.0f,1.0f,1.0f),2000);
	smgr->setAmbientLight(video::SColorf(0.3f,0.3f,0.3f));
	// add our media directory as "search path"
	Device->getFileSystem()->addFolderFileArchive("./");

	/*
	The next step is to read the configuration file. It is stored in the xml
	format and looks a little bit like this:

	@verbatim
	<?xml version="1.0"?>
	<config>
		<startUpModel file="some filename" />
		<messageText caption="Irrlicht Engine Mesh Viewer">
			Hello!
		</messageText>
	</config>
	@endverbatim

	We need the data stored in there to be written into the global variables
	StartUpModelFile, MessageText and Caption. This is now done using the
	Irrlicht Engine integrated XML parser:
	*/

	// read configuration from xml file

	io::IXMLReader* xml = Device->getFileSystem()->createXMLReader( L"previn");

	while(xml && xml->read())
	{
		switch(xml->getNodeType())
		{
		case io::EXN_TEXT:
			// in this xml file, the only text which occurs is the
			// messageText
			MessageText = xml->getNodeData();
			break;
		case io::EXN_ELEMENT:
			{

				if (core::stringw("startUpModel") == xml->getNodeName())
					StartUpModelFile = xml->getAttributeValue(L"file");
				else if (core::stringw("messageText") == xml->getNodeName())
					Caption = xml->getAttributeValue(L"caption");
				else if (core::stringc("scaling") == xml->getNodeName())
				{
					ScalingXML = xml->getAttributeValue(L"value");
					doScale = true;										
				}
				else if (core::stringc("rotation") == xml->getNodeName())
				{
					YRotXML = xml->getAttributeValue(L"yvalue");
					doRotation = true;										
				}
				else if (core::stringc("offset") == xml->getNodeName())
				{
					OffX = xml->getAttributeValue(L"xvalue");
					OffY = xml->getAttributeValue(L"yvalue");
					OffZ = xml->getAttributeValue(L"zvalue");
					gui::IGUIElement* root = env->getRootGUIElement();
					doOffset = true;
				}
			}
			break;
		default:
			break;
		}
	}	

	if (xml)
		xml->drop(); // don't forget to delete the xml reader

	if (argc > 1)
		StartUpModelFile = argv[1];

	/*
	That wasn't difficult. Now we'll set a nicer font and create the Menu.
	It is possible to create submenus for every menu item. The call
	menu->addItem(L"File", -1, true, true); for example adds a new menu
	Item with the name "File" and the id -1. The following parameter says
	that the menu item should be enabled, and the last one says, that there
	should be a submenu. The submenu can now be accessed with
	menu->getSubMenu(0), because the "File" entry is the menu item with
	index 0.
	*/

	// set a nicer font

	IGUISkin* skin = env->getSkin();
	IGUIFont* font = env->getFont("fonthaettenschweiler.bmp");
	if (font)
		skin->setFont(font);

	// create menu
	gui::IGUIContextMenu* menu = env->addMenu();
	menu->addItem(L"File", -1, true, true);
	menu->addItem(L"View", -1, true, true);
	menu->addItem(L"Camera", -1, true, true);

	gui::IGUIContextMenu* submenu;
	submenu = menu->getSubMenu(0);
	//submenu->addItem(L"Open Model File...", GUI_ID_OPEN_MODEL);
	//submenu->addItem(L"Set Model Archive...", GUI_ID_SET_MODEL_ARCHIVE);
	//submenu->addItem(L"Load as Octree", GUI_ID_LOAD_AS_OCTREE);
	//submenu->addSeparator();
	submenu->addItem(L"Quit and Discard Changes", GUI_ID_QUIT_DISCARD);
	submenu->addItem(L"Quit and Save Changes", GUI_ID_QUIT_SAVE);

	submenu = menu->getSubMenu(1);
	//submenu->addItem(L"sky box visible", GUI_ID_SKY_BOX_VISIBLE, true, false, true);
	submenu->addItem(L"toggle model debug information", GUI_ID_TOGGLE_DEBUG_INFO, true, true);
	submenu->addItem(L"model material", -1, true, true );

	submenu = submenu->getSubMenu(0);
	submenu->addItem(L"Off", GUI_ID_DEBUG_OFF);
	submenu->addItem(L"Bounding Box", GUI_ID_DEBUG_BOUNDING_BOX);
	submenu->addItem(L"Normals", GUI_ID_DEBUG_NORMALS);
	submenu->addItem(L"Skeleton", GUI_ID_DEBUG_SKELETON);
	submenu->addItem(L"Wire overlay", GUI_ID_DEBUG_WIRE_OVERLAY);
	submenu->addItem(L"Half-Transparent", GUI_ID_DEBUG_HALF_TRANSPARENT);
	submenu->addItem(L"Buffers bounding boxes", GUI_ID_DEBUG_BUFFERS_BOUNDING_BOXES);
	submenu->addItem(L"All", GUI_ID_DEBUG_ALL);

	submenu = menu->getSubMenu(1)->getSubMenu(1);
	submenu->addItem(L"Solid", GUI_ID_MODEL_MATERIAL_SOLID);
	submenu->addItem(L"Transparent", GUI_ID_MODEL_MATERIAL_TRANSPARENT);
	submenu->addItem(L"Reflection", GUI_ID_MODEL_MATERIAL_REFLECTION);

	submenu = menu->getSubMenu(2);
	submenu->addItem(L"Maya Style", GUI_ID_CAMERA_MAYA);
	submenu->addItem(L"First Person", GUI_ID_CAMERA_FIRST_PERSON);

	gui::IGUIToolBar* bar = env->addToolBar();

	for (s32 i=0; i<gui::EGDC_COUNT ; ++i)
	{
		video::SColor col = env->getSkin()->getColor((gui::EGUI_DEFAULT_COLOR)i);
		col.setAlpha(255);
		env->getSkin()->setColor((gui::EGUI_DEFAULT_COLOR)i, col);
	}

	IGUIStaticText* fpstext = env->addStaticText(L"",
			core::rect<s32>(720,4,795,23), true, false, bar);

	IGUIStaticText* postext = env->addStaticText(L"",
			core::rect<s32>(10,50,470,80),false, false, 0, GUI_ID_POSITION_TEXT);
	postext->setVisible(false);

	env->addStaticText(L"Scaling: ", core::rect<s32>(10,6,220,23), false, false, bar);
	env->addEditBox(L"1", core::rect<s32>(57,4,126,23), true, bar, GUI_ID_X_SCALE);
	env->addButton(core::rect<s32>(130,4,160,23), bar, GUI_ID_BUTTON_SET_SCALE, L"Set");
	env->addButton(core::rect<s32>(170,4,200,23), bar, GUI_ID_BUTTON_SCALE_MUL10, L"* 10");
	env->addButton(core::rect<s32>(210,4,240,23), bar, GUI_ID_BUTTON_SCALE_DIV10, L"* 0.1");

	env->addStaticText(L"Y Rotation: ", core::rect<s32>(255,6,390,23), false, false, bar); 
	env->addEditBox(L"0	", core::rect<s32>(320,4,350,23), true, bar, GUI_ID_Y_ROT);
	env->addButton(core::rect<s32>(354,4,384,23), bar, GUI_ID_BUTTON_SET_Y_ROT, L"Set");

	env->addStaticText(L"Position offset: X                 Y                 Z", core::rect<s32>(400,6, 750,23), false, false, bar); 
	env->addEditBox(L"0	", core::rect<s32>(496,4,536,23), true, bar, GUI_ID_X_OFF);
	env->addEditBox(L"0	", core::rect<s32>(554,4,594,23), true, bar, GUI_ID_Y_OFF);
	env->addEditBox(L"0	", core::rect<s32>(612,4,652,23), true, bar, GUI_ID_Z_OFF);
	env->addButton(core::rect<s32>(660,4,690,23), bar, GUI_ID_OFFSET, L"Set");

	// set window caption

	Caption += " - [";
	Caption += driver->getName();
	Caption += "]";
	Device->setWindowCaption(Caption.c_str());	

	/*
	That's nearly the whole application. We simply show the about message
	box at start up, and load the first model. To make everything look
	better, a skybox is created and a user controled camera, to make the
	application a little bit more interactive. Finally, everything is drawn
	in a standard drawing loop.
	*/

	// show about message box and load default model
	loadModel(StartUpModelFile.c_str());

	// add a camera scene node
	Camera[0] = smgr->addCameraSceneNodeMaya();
	Camera[0]->setFarValue(50000.f);
	// Maya cameras reposition themselves relative to their target, so target the location
	// where the mesh scene node is placed.
	Camera[0]->setTarget(core::vector3df(7000,3000,7000));
	Camera[0]->setPosition(core::vector3df(0,0,-70));

	//Camera[0]->setScale(core::vector3df(100,100,100));

	

	Camera[1] = smgr->addCameraSceneNodeFPS();
	Camera[1]->setFarValue(50000.f);	

	Camera[1]->setPosition(core::vector3df(7000,3000,7000));
	Camera[1]->setPosition(core::vector3df(7000,3000,7000));
	
	setActiveCamera(Camera[0]);	

	// draw everything

	scene::ISceneNode* cube = smgr->addCubeSceneNode(160);
	if (cube)
	{
		    cube->setPosition(core::vector3df(0, 0, 0));
            cube->setMaterialTexture(0, driver->getTexture("wall.bmp"));
            cube->setMaterialFlag(video::EMF_LIGHTING, false);
	}

	if (doScale)
	{
		core::vector3df scale;
		scale.X = (f32)atof(ScalingXML.c_str());
		scale.Y = (f32)atof(ScalingXML.c_str());
		scale.Z = (f32)atof(ScalingXML.c_str());
		if (Model) Model->setScale(scale);
		if (Model) updateScaleInfo(Model);
		doScale = false;
	}

	if (doOffset)
	{
	    core::vector3df offset;
		offset.X = (f32)atof(OffX.c_str());
		offset.Y = (f32)atof(OffY.c_str());
		offset.Z = (f32)atof(OffZ.c_str());
		if (Model) Model->setPosition(offset);
		if (Model) updateOffsetInfo(Model);
		doOffset = false;

	}

	if (doRotation)
	{
		if (Model) Model->setRotation( core::vector3df(0, (f32)atof(YRotXML.c_str()),0) );
		updateRotYInfo(Model);

	}

	while(Device->run() && driver)
	{
		if (Device->isWindowActive())
		{
			//driver->beginScene(true, true, video::SColor(255,100,101,140));
			//driver->beginScene(true, true, video::SColor(255,72,204,204));
			driver->beginScene(true, true, video::SColor(255,0,200,210));

			/* draw axis*/
			//core::vector3df pos = scene::ISceneNode::getPosition(); 
			driver->draw3DLine( core::vector3df(0,0,0), core::vector3df( 3000, 0, 0 ), video::SColor( 255, 255, 0, 0 ) ); 
			driver->draw3DLine( core::vector3df(0,0,0), core::vector3df( 0, 3000, 0 ), video::SColor( 255, 0, 255, 0 ) ); 
			driver->draw3DLine( core::vector3df(0,0,0), core::vector3df( 0, 0, 3000 ), video::SColor( 255, 0, 0, 255 ) );

			smgr->drawAll();
			env->drawAll();			

			/* bounding box */
			video::SMaterial matl; 
			matl.Lighting = false; 
			core::aabbox3df box0( core::vector3df(8000, 0, 8000), core::vector3df(-8000, 0, -8000) );
			core::aabbox3df box1( core::vector3df(8001, 0, 8001), core::vector3df(-8001, 0, -8001) );
			core::aabbox3df box2( core::vector3df(8001, 0, 8002), core::vector3df(-8001, 0, -8002) );
			core::aabbox3df box3( core::vector3df(8001, 0, 8003), core::vector3df(-8001, 0, -8003) );
			core::aabbox3df box4( core::vector3df(8001, 0, 8004), core::vector3df(-8001, 0, -8004) );
			core::aabbox3df box5( core::vector3df(8001, 0, 8005), core::vector3df(-8001, 0, -8005) );
			core::aabbox3df box6( core::vector3df(8001, 0, 8006), core::vector3df(-8001, 0, -8006) );
			core::aabbox3df box7( core::vector3df(8001, 0, 8007), core::vector3df(-8001, 0, -8007) );
			core::aabbox3df box8( core::vector3df(8001, 0, 8008), core::vector3df(-8001, 0, -8008) );
			driver->setTransform(video::ETS_WORLD, core::IdentityMatrix); 
			driver->setMaterial(matl);
			driver->draw3DBox(box0, video::SColor(255, 255, 0, 0)); 
			driver->draw3DBox(box1, video::SColor(255, 255, 0, 0)); 
			driver->draw3DBox(box2, video::SColor(255, 255, 0, 0)); 
			driver->draw3DBox(box3, video::SColor(255, 255, 0, 0)); 
			driver->draw3DBox(box4, video::SColor(255, 255, 0, 0)); 
			driver->draw3DBox(box5, video::SColor(255, 255, 0, 0)); 
			driver->draw3DBox(box6, video::SColor(255, 255, 0, 0)); 
			driver->draw3DBox(box7, video::SColor(255, 255, 0, 0)); 
			driver->draw3DBox(box8, video::SColor(255, 255, 0, 0)); 

			Model->setRotation( core::vector3df(0, ModelYRotation,0) );
			//Model->setRotation( core::vector3df(270, 180, 0) );

			//Model->setPosition( core::vector3df(OffsetX, OffsetY, OffsetZ));			
			driver->endScene();

			// update information about current frame-rate
			core::stringw str(L"Faces: ");
			//str.append(core::stringw(driver->getFPS()));
			//str += L"Face Count: ";
			str.append(core::stringw(driver->getPrimitiveCountDrawn()));

			//str += L" RotY:"; 
			//str.append(core::stringc(ScalingXML));
			//str.append(core::stringc(YRotXML));
			//str += L" RotX:"; 
			//str.append(core::stringw((int)ModelXRotation%360));
			//str += L" RotZ:"; 
			//str.append(core::stringw((int)ModelZRotation%360));


			// update information about the active camera
			scene::ICameraSceneNode* cam = Device->getSceneManager()->getActiveCamera();
			//str += L" Camera Pos: ";

			//str.append(core::stringw((int)cam->getPosition().X));
			//str += L" ";
			//str.append(core::stringw((int)cam->getPosition().Y));
			//str += L" ";
			//str.append(core::stringw((int)cam->getPosition().Z));
			//str += L" Tgt: ";
			//str.append(core::stringw((int)cam->getTarget().X));
			//str += L" ";
			//str.append(core::stringw((int)cam->getTarget().Y));
			//str += L" ";
			//str.append(core::stringw((int)cam->getTarget().Z));

			fpstext->setText(str.c_str());

			postext->setText(str.c_str());

			//updateToolBox();
		}
		else
			Device->yield();
	}


    env = Device->getGUIEnvironment();
	gui::IGUIElement* root = env->getRootGUIElement();
    core::vector3df offset = Model->getPosition();
    core::vector3df scale = Model->getScale();
	core::vector3df rot = Model->getRotation();
	ModelYRotation = (int)(3600 + (int)rot.Y)%360;

	Device->drop();

	if (doSave) 
	{
		FILE *fp = fopen("prevout", "w+b");
		char TempString[2048];
		sprintf(TempString, "o %.0f %.0f %.0f", offset.X, offset.Y, offset.Z);
		fputs(TempString, fp);
   	    sprintf(TempString, "\ns %0.2f", scale.X);
		fputs(TempString, fp);
   	    sprintf(TempString, "\ny %.0f", ModelYRotation);
		fputs(TempString, fp);
		fclose(fp);

		return 1;
	}
	else return 0;
}
