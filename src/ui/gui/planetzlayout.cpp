
#include <boost/bind.hpp>
#include <boost/filesystem.hpp>
#include <boost/regex.hpp>
#include <cmath>
#include <cstring>
#include <sstream>

#include "planetzlayout.h"

#include "util/logger.h"
#include "util/vector.h"
#include "util/timer/timer.h"

#include "cuda/math.h"

#include "constants.h"

#define BUFSIZE 128

#define GETWIN(x)(WindowManager::getSingleton().getWindow(x))
#define GETWINCAST(t,x) static_cast<t >(GETWIN(x))

#define ERRORWIN(x) \
	do { \
		GETWIN("stError")->setText(x); \
		GETWIN("ErrorWin")->setVisible(true); \
	} while( 0 ) 

#define SETEVENT(x,y,z) GETWIN(x)->subscribeEvent((y),Event::Subscriber((z),this));

#ifdef _WIN32
# define snprintf sprintf_s
#endif

using namespace CEGUI;

using namespace boost::filesystem;

using std::pow;

class MyListboxItem : public ListboxTextItem  {
	public:
		MyListboxItem( const CEGUI::String& text )
			: ListboxTextItem(text)
		{
			setSelectionBrushImage ("QuadraticLook", "White");
			setSelectionColours (CEGUI::colour (0.0, 0.118, 1.0, 0.412));
		}
};

double mass_pow( double x )
{
	return pow( 2 , x );
}

const boost::regex PlanetzLayout::save_file("[\\w ]+.sav");
const boost::regex PlanetzLayout::file_cont("[\\w ]+");
const std::string PlanetzLayout::qsave_name = "qsave.sav";

PlanetzLayout::PlanetzLayout( Config& cfg )
	: Layout("planetz.layout") //, sel_planet(NULL)
	, config(cfg)
{
	WindowManager::getSingleton().getWindow("btnAdd")
		->subscribeEvent(PushButton::EventClicked
				,Event::Subscriber(&PlanetzLayout::add_planet,this));

	SETEVENT("btnChange",PushButton::EventClicked,&PlanetzLayout::change_planet);

	WindowManager::getSingleton().getWindow("btnPause")
		->subscribeEvent(PushButton::EventClicked
				,Event::Subscriber(&PlanetzLayout::pause,this));

	WindowManager::getSingleton().getWindow("btnSave")
		->subscribeEvent(PushButton::EventClicked
				,Event::Subscriber(&PlanetzLayout::save,this));

	WindowManager::getSingleton().getWindow("btnShowLoad")
		->subscribeEvent(PushButton::EventClicked
				,Event::Subscriber(&PlanetzLayout::show_load_win,this));

	SETEVENT("btnShowSave",PushButton::EventClicked,&PlanetzLayout::show_save_win);

	SETEVENT("btnShowOpt" ,PushButton::EventClicked,&PlanetzLayout::show_opt_win);
	SETEVENT("btnOptNope" ,PushButton::EventClicked,&PlanetzLayout::hide_opt_win);
	SETEVENT("btnOptOk" ,PushButton::EventClicked,&PlanetzLayout::apply_options);

	WindowManager::getSingleton().getWindow("btnLoad")
		->subscribeEvent(PushButton::EventClicked
				,Event::Subscriber(&PlanetzLayout::load,this));

	WindowManager::getSingleton().getWindow("btnReset")
		->subscribeEvent(PushButton::EventClicked
				,Event::Subscriber(&PlanetzLayout::reset_anim,this));

	WindowManager::getSingleton().getWindow("btnClear")
		->subscribeEvent(PushButton::EventClicked
				,Event::Subscriber(&PlanetzLayout::clear_win,this));

	WindowManager::getSingleton().getWindow("btnDel")
		->subscribeEvent(PushButton::EventClicked
				,Event::Subscriber(&PlanetzLayout::del_planet,this));

	SETEVENT("ErrorWin",FrameWindow::EventCloseClicked,&PlanetzLayout::close_win);
	SETEVENT("SavesListWin",FrameWindow::EventCloseClicked,&PlanetzLayout::close_win);
	SETEVENT("winSave",FrameWindow::EventCloseClicked,&PlanetzLayout::close_win);

	WindowManager::getSingleton().getWindow("slCamSpeed")
		->subscribeEvent(Scrollbar::EventScrollPositionChanged
				,Event::Subscriber(&PlanetzLayout::set_cam_speed,this));

	WindowManager::getSingleton().getWindow("slAnimSpeed")
		->subscribeEvent(Scrollbar::EventScrollPositionChanged
				,Event::Subscriber(&PlanetzLayout::set_anim_speed,this));

	WindowManager::getSingleton().getWindow("slMass")
		->subscribeEvent(Scrollbar::EventScrollPositionChanged
				,Event::Subscriber(&PlanetzLayout::set_mass_val,this));

	WindowManager::getSingleton().getWindow("slRadius")
		->subscribeEvent(Scrollbar::EventScrollPositionChanged
				,Event::Subscriber(&PlanetzLayout::set_radius_val,this));

	SETEVENT("spPosX",Spinner::EventValueChanged,&PlanetzLayout::changed_planet);
	SETEVENT("slMass"  ,Scrollbar::EventScrollPositionChanged,&PlanetzLayout::changed_planet);
	SETEVENT("slRadius",Scrollbar::EventScrollPositionChanged,&PlanetzLayout::changed_planet);

	SETEVENT("btnQSave",PushButton::EventClicked,&PlanetzLayout::qsave);
	SETEVENT("btnQLoad",PushButton::EventClicked,&PlanetzLayout::qload);

//        timer.call( boost::bind(&PlanetzLayout::update_show_window,this) , 0.1 , true );
}

PlanetzLayout::~PlanetzLayout()
{
}

void PlanetzLayout::updateOptions( Config& cfg )
{
	cfg.set("deffered.textures",GETWINCAST(Checkbox*,"cbTextures")->isSelected());
	cfg.set("deffered.lights_range",GETWINCAST(Checkbox*,"cbLights")->isSelected());
	cfg.set("deffered.lighting",GETWINCAST(Checkbox*,"cbLighting")->isSelected());
	cfg.set("deffered.normals",GETWINCAST(Checkbox*,"cbNormals")->isSelected());
	cfg.set("deffered.brightness",GETWINCAST(Spinner*,"spBright")->getCurrentValue());

	cfg.set("trace.enable",GETWINCAST(Checkbox*,"cbTrace")->isSelected());
	cfg.set("trace.visible",GETWINCAST(Checkbox*,"cbTraceVis")->isSelected());
	cfg.set("trace.frequency",(double)GETWINCAST(Spinner*,"spTraceFreq")->getCurrentValue());
	cfg.set("trace.length",(unsigned)GETWINCAST(Spinner*,"spTraceLenght")->getCurrentValue());

	cfg.set("phx.clusters",GETWINCAST(Checkbox*,"cbClusters")->isSelected());
}

void PlanetzLayout::setOptions( const Config& cfg )
{
	GETWINCAST(Checkbox*,"cbTextures")->setSelected(cfg.get<bool>("deffered.textures"));
	GETWINCAST(Checkbox*,"cbLights")->setSelected(cfg.get<bool>("deffered.lights_range"));
	GETWINCAST(Checkbox*,"cbLighting")->setSelected(cfg.get<bool>("deffered.lighting"));
	GETWINCAST(Checkbox*,"cbNormals")->setSelected(cfg.get<bool>("deffered.normals"));
	GETWINCAST(Spinner*,"spBright")->setCurrentValue(cfg.get<float>("deffered.brightness"));

	GETWINCAST(Checkbox*,"cbTrace")->setSelected(cfg.get<bool>("trace.enable"));
	GETWINCAST(Checkbox*,"cbTraceVis")->setSelected(cfg.get<bool>("trace.visible"));
	GETWINCAST(Spinner*,"spTraceLenght")->setCurrentValue(cfg.get<unsigned>("trace.length"));
	GETWINCAST(Spinner*,"spTraceFreq")->setCurrentValue(cfg.get<double>("trace.frequency"));
}

/*void PlanetzLayout::add_selected_planet( Planet*p )
{
	sel_planet = p;
	update_show_window();
}*/

void PlanetzLayout::show_show_window( const MEM::MISC::PhxPlanet& pp )
{
	GETWIN("ShowWin")->setVisible(true);
	sel_planet = pp;
	update_show_window();
	if( tc_show.good() ) tc_show.die();
	tc_show = timer.call(boost::bind(&PlanetzLayout::update_show_window,this) , 1.0 , true );
}

void PlanetzLayout::hide_show_window()
{
	GETWIN("ShowWin")->setVisible(false);
	tc_show.die();
}

void PlanetzLayout::update_show_window()
{
	if( !sel_planet.isValid() )
	{
		hide_show_window();
		return;
	}
	char buff[BUFSIZE];

	float3 pos = sel_planet.getPosition();
	snprintf(buff,BUFSIZE,"(%4.2f,%4.2f,%4.2f)",pos.x,pos.y,pos.z);
	WindowManager::getSingleton().getWindow("lbShowPos")->setText(buff);

	snprintf(buff,BUFSIZE,"%4.2f",sel_planet.getRadius() );
	WindowManager::getSingleton().getWindow("lbShowRadius")->setText(buff);

	float3 vel = sel_planet.getVelocity();
	snprintf(buff,BUFSIZE,"(%4.2f,%4.2f,%4.2f)",vel.x,vel.y,vel.z);
	WindowManager::getSingleton().getWindow("lbShowSpeedVector")->setText(buff);

	snprintf(buff,BUFSIZE,"%4.2f",length(vel));
	WindowManager::getSingleton().getWindow("lbShowSpeedScalar")->setText(buff);

	snprintf(buff,BUFSIZE,"%4.2f",sel_planet.getMass() );
	WindowManager::getSingleton().getWindow("lbShowMass")->setText(buff);

	snprintf(buff,BUFSIZE,"Aktualna planeta (%d)",sel_planet.getId());
	GETWIN("ShowWin")->setText(buff);
}

void PlanetzLayout::update_fps( int fps )
{
	std::stringstream ss;
	ss << "FPS: " << fps;
	WindowManager::getSingleton().getWindow("stFps")->setText(ss.str());
}

bool PlanetzLayout::del_planet( const CEGUI::EventArgs& e )
{
	log_printf(DBG,"[GUI] Removin planet!\n");
	on_planet_delete(sel_planet.getId());
	hide_show_window();
	return true;
}

bool PlanetzLayout::add_planet( const CEGUI::EventArgs& e )
{
	//        Listbox*lb = static_cast<Listbox*>(WindowManager::getSingleton().getWindow("lbox1"));
	//        lb->addItem( new ListboxTextItem("Item") );
	//        return true;
	
	MEM::MISC::PlanetParams pp = get_pp_from_add_win();

	log_printf(DBG,"[GUI] Adding planet at (%f,%f,%f) with speed (%f,%f,%f), mass %f and radius %f\n"
			,pp.pos.x,pp.pos.y,pp.pos.z
			,pp.vel.x,pp.vel.y,pp.vel.z
			,pp.mass,pp.radius );
	TODO( "Wybieranie modelu" );
	on_planet_add( pp );

	return true;
}

MEM::MISC::PlanetParams PlanetzLayout::get_pp_from_add_win()
{
	MEM::MISC::PlanetParams pp;
	pp.pos.x=static_cast<Spinner*>(WindowManager::getSingleton().getWindow("spPosX"))
		->getCurrentValue();
	pp.pos.y=static_cast<Spinner*>(WindowManager::getSingleton().getWindow("spPosY"))
		->getCurrentValue();
	pp.pos.z=static_cast<Spinner*>(WindowManager::getSingleton().getWindow("spPosZ"))
		->getCurrentValue();

	pp.vel.x=static_cast<Spinner*>(WindowManager::getSingleton().getWindow("spSpeedX"))
		->getCurrentValue();
	pp.vel.y=static_cast<Spinner*>(WindowManager::getSingleton().getWindow("spSpeedY"))
		->getCurrentValue();
	pp.vel.z=static_cast<Spinner*>(WindowManager::getSingleton().getWindow("spSpeedZ"))
		->getCurrentValue();

	pp.mass=static_cast<Scrollbar*>(WindowManager::getSingleton().getWindow("slMass"))
		->getScrollPosition();
	pp.mass = mass_pow(pp.mass);

	pp.radius=static_cast<Scrollbar*>(WindowManager::getSingleton().getWindow("slRadius"))
		->getScrollPosition();
	pp.radius++; // minimalny promien to 1

	pp.model = GETWINCAST(Spinner*,"spModel")->getCurrentValue();

	return pp;
}

bool PlanetzLayout::change_planet( const CEGUI::EventArgs& e )
{
	on_planet_change( get_pp_from_add_win() );
	return true;
}

bool PlanetzLayout::changed_planet( const CEGUI::EventArgs& e )
{
	on_planet_changed( get_pp_from_add_win() );
	return true;
}

bool PlanetzLayout::update_add_win( const MEM::MISC::PlanetParams& pp )
{
	GETWINCAST(Spinner*,"spPosX")->setCurrentValue(pp.pos.x);
	GETWINCAST(Spinner*,"spPosY")->setCurrentValue(pp.pos.y);
	GETWINCAST(Spinner*,"spPosZ")->setCurrentValue(pp.pos.z);
	GETWINCAST(Spinner*,"spSpeedX")->setCurrentValue(pp.vel.x);
	GETWINCAST(Spinner*,"spSpeedY")->setCurrentValue(pp.vel.y);
	GETWINCAST(Spinner*,"spSpeedZ")->setCurrentValue(pp.vel.z);
	GETWINCAST(Scrollbar*,"slRadius")->setScrollPosition( pp.radius  - 1 );
	return true;
}

bool PlanetzLayout::set_cam_speed( const CEGUI::EventArgs& e )
{
	on_cam_speed_changed( 
			pow(static_cast<Scrollbar*>(WindowManager::getSingleton().getWindow("slCamSpeed"))
			->getScrollPosition(),2) );
	return true;
}

bool PlanetzLayout::clear_win( const CEGUI::EventArgs& e )
{
	static_cast<Spinner*>(WindowManager::getSingleton().getWindow("spPosX"))
		->setCurrentValue( 0 );
	static_cast<Spinner*>(WindowManager::getSingleton().getWindow("spPosY"))
		->setCurrentValue( 0 );
	static_cast<Spinner*>(WindowManager::getSingleton().getWindow("spPosZ"))
		->setCurrentValue( 0 );

	static_cast<Spinner*>(WindowManager::getSingleton().getWindow("spSpeedX"))
		->setCurrentValue(0);
	static_cast<Spinner*>(WindowManager::getSingleton().getWindow("spSpeedY"))
		->setCurrentValue(0);
	static_cast<Spinner*>(WindowManager::getSingleton().getWindow("spSpeedZ"))
		->setCurrentValue( 0 );

	static_cast<Scrollbar*>(WindowManager::getSingleton().getWindow("slMass"))
		->setScrollPosition( 0 );

	static_cast<Scrollbar*>(WindowManager::getSingleton().getWindow("slRadius"))
		->setScrollPosition( 0 );

	return true;
}

bool PlanetzLayout::set_mass_val( const CEGUI::EventArgs& e )
{
	double mass=static_cast<Scrollbar*>(WindowManager::getSingleton().getWindow("slMass"))
		->getScrollPosition();
	mass = mass_pow(mass);
	char buff[BUFSIZE];
	snprintf(buff,BUFSIZE,"%4.2f",mass);
	WindowManager::getSingleton().getWindow("stMassCount")
		->setText(buff);
	return true;
}

bool PlanetzLayout::set_radius_val( const CEGUI::EventArgs& e )
{
	double radius=static_cast<Scrollbar*>(WindowManager::getSingleton().getWindow("slRadius"))
		->getScrollPosition();
	radius++;
	char buff[BUFSIZE];
	snprintf(buff,BUFSIZE,"%4.2f",radius);
	WindowManager::getSingleton().getWindow("stRadiusCount")
		->setText(buff);
	return true;
}

bool PlanetzLayout::pause( const CEGUI::EventArgs& e )
{
	CEGUI::Window* btn = GETWIN("btnPause");
	if( btn->getText() == "Start" )
		btn->setText("Pauza");
	else	btn->setText("Start");
	on_pause_click();
	return true;
}

bool PlanetzLayout::show_load_win( const CEGUI::EventArgs& e )
{
	Listbox*lb = static_cast<Listbox*>(WindowManager::getSingleton().getWindow("lstSaves"));
	lb->resetList();
	if( !exists( SAVES("") ) )  {
		ERRORWIN("Brak folderu z zapisanymi układami");
		return true;
	}

	
	directory_iterator end_itr; // default construction yields past-the-end
	for ( directory_iterator itr( SAVES("") ); itr != end_itr; ++itr )
	{
		if( !is_directory( itr->status() )
		 && boost::regex_match(itr->path().filename().c_str(),save_file) )
			lb->addItem(new MyListboxItem(itr->path().filename().c_str()));
	}

	GETWIN("SavesListWin")->setVisible(true);

	return true;
}

bool PlanetzLayout::load( const CEGUI::EventArgs& e )
{
	GETWIN("SavesListWin")->setVisible(false);

	ListboxItem*lbi = GETWINCAST(Listbox*,"lstSaves")->getFirstSelectedItem();
	
	if( lbi ) {
		GETWIN("btnPause")->setText("Start");
		on_load(SAVES(lbi->getText().c_str()));
	}

	return true;
}

bool PlanetzLayout::qload( const CEGUI::EventArgs& e )
{
	GETWIN("btnPause")->setText("Start");
	on_load( SAVES(qsave_name) );
	return true;
}

bool PlanetzLayout::show_save_win( const CEGUI::EventArgs& e )
{
	GETWIN("winSave")->setVisible(true);
	return true;
}

bool PlanetzLayout::show_opt_win( const CEGUI::EventArgs& e )
{
	setOptions( config );
	GETWIN("winOpt")->setVisible(!GETWIN("winOpt")->isVisible());
	return true;
}

bool PlanetzLayout::hide_opt_win( const CEGUI::EventArgs& e )
{
	GETWIN("winOpt")->setVisible(false);
	return true;
}

bool PlanetzLayout::apply_options( const CEGUI::EventArgs& e )
{
	GETWIN("winOpt")->setVisible(false);
	updateOptions( config );
	on_config_changed( config );
	return true;
}

bool PlanetzLayout::save( const CEGUI::EventArgs& e )
{
	std::string str = GETWIN("ebSave")->getText().c_str();
	GETWIN("winSave")->setVisible(false);

	if( !regex_match(str,save_file) ) {
		if( regex_match(str,file_cont) )
			str+=".sav";
		else { 
			GETWIN("ebSave")->setText("");
			ERRORWIN("Niepoprawna nazwa pliku");
			return true;
		}
	}

	log_printf(DBG,"Saving %s\n", str.c_str() );
	on_save( SAVES(str) );
	return true;
}

bool PlanetzLayout::qsave( const CEGUI::EventArgs& e )
{
	on_save( SAVES(qsave_name) );
	return true;
}

bool PlanetzLayout::reset_anim( const CEGUI::EventArgs& e )
{
	GETWIN("btnPause")->setText("Start");
	on_reset_click();
	return true;
}

bool PlanetzLayout::set_anim_speed( const CEGUI::EventArgs& e )
{
	on_sim_speed_changed(
		GETWINCAST(Scrollbar*,"slAnimSpeed")
			->getScrollPosition() + 1.0 ); // at least one frame
	return true;
}

bool PlanetzLayout::close_win( const CEGUI::EventArgs& e )
{
	static_cast<const WindowEventArgs&>(e).window->setVisible(false);
	return true;
}

