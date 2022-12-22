#pragma once

const char* hitboxes[] = 
{
	crypt_arr("Head"), 
	crypt_arr("Neck"), 
	crypt_arr("Chest"), 
	crypt_arr("Stomach"), 
	crypt_arr("Arms"),
	crypt_arr("Legs"),
	crypt_arr("Feet") 
};

const char* multipoint_hitboxes[] = 
{ 
	crypt_arr("Head"), 
	crypt_arr("Chest"), 
	crypt_arr("Stomach"),
	crypt_arr("Legs"),
	crypt_arr("Feet") 
};

const char* prioritized_hitbox[] = 
{
	crypt_arr("None"), 
	crypt_arr("Head"),
	crypt_arr("Body"),
	crypt_arr("Stomach") 
};

const char* selection[] = 
{ 
	crypt_arr("Nearest distance"),
	crypt_arr("Near crosshair"),
	crypt_arr("Highest damage"), 
	crypt_arr("Lowest health"), 
	crypt_arr("Least lag"),
	crypt_arr("Lowest height") 
};

const char* autostop_modifiers[] = 
{ 
	crypt_arr("Between shot"),
	crypt_arr("Hitchance fail"),
	crypt_arr("Lethal"),
	crypt_arr("Center"),
	crypt_arr("Early") 
};

const char* prefer_bodyaim_options[] = 
{
	crypt_arr("Always"),
	crypt_arr("Lethal"),
	crypt_arr("In air") 
};

const char* force_bodyaim_options[] = 
{
	crypt_arr("Always"),
	crypt_arr("HP below X number"),
	crypt_arr("In air")
};

const char* pitch[] = 
{ 
	crypt_arr("None"),
	crypt_arr("Down"),
	crypt_arr("Up"),
	crypt_arr("Zero"),
	crypt_arr("Random"), 
	crypt_arr("Jitter"), 
	crypt_arr("Fake down"), 
	crypt_arr("Fake up"),
	crypt_arr("Fake jitter") 
};

const char* yaw[] = 
{ 
	crypt_arr("Static"),
	crypt_arr("Jitter"), 
	crypt_arr("Spin") 
};

const char* baseangle[] =
{ 
	crypt_arr("Local view"),
	crypt_arr("At target") 
};

const char* fakelags[] = 
{ 
	crypt_arr("Static"), 
	crypt_arr("Random"), 
	crypt_arr("Dynamic"),
	crypt_arr("Fluctuative") 
};

const char* lagstrigger[] = 
{ 
	crypt_arr("In air"),
	crypt_arr("On land"), 
	crypt_arr("On peek"),
	crypt_arr("On shot"),
	crypt_arr("On reload"),
	crypt_arr("On velocity change")
};

const char* desync[] = 
{ 
	crypt_arr("None"), 
	crypt_arr("Static"),
	crypt_arr("Jitter") 
};

const char* lby_type[] = 
{
	crypt_arr("Normal"), 
	crypt_arr("Opposite"), 
	crypt_arr("Sway")
};

const char* legit_hitbox[] = 
{ 
	crypt_arr("Near crosshair"),
	crypt_arr("Head"), 
	crypt_arr("Body") 
};

const char* rcs_type[] = 
{
	crypt_arr("Always on"), 
	crypt_arr("On target") 
};

const char* legit_smooth[] = 
{
	crypt_arr("Static"),
	crypt_arr("Humanized")
};

const char* legit_fov[] = 
{ 
	crypt_arr("Static"), 
	crypt_arr("Dynamic") 
};

const char* slowwalk_type[] =
{
	crypt_arr("Accuracy"),
	crypt_arr("Custom")
};

const char* proj_combo[] =
{
	crypt_arr("Icon"),
	crypt_arr("Text"),
	crypt_arr("Box"),
	crypt_arr("Glow")
};

const char* weaponplayer[] =
{
	crypt_arr("Icon"),
	crypt_arr("Text")
};

const char* hitmarkers[] =
{
	crypt_arr("Crosshair"),
	crypt_arr("World")
};

const char* glowtype[] =
{
	crypt_arr("Standard"),
	crypt_arr("Pulse"),
	crypt_arr("Inner")
};

const char* local_chams_type[] =
{
	crypt_arr("Real"),
	crypt_arr("Desync")
};

const char* radiustype[] =
{
	crypt_arr("Dynamic"),
	crypt_arr("Static")
};

const char* reticletype[] =
{
	crypt_arr("3D"),
	crypt_arr("2D")
};

const char* chamsvisact[] =
{
	crypt_arr("Visible"),
	crypt_arr("Invisible")
};

const char* chamsvis[] =
{
	crypt_arr("Visible")
};

const char* chamstype[] =
{
	crypt_arr("Regular"),
	crypt_arr("Metallic"),
	crypt_arr("Flat"),
	crypt_arr("Pulse"),
	crypt_arr("Crystal"),
	crypt_arr("Glass"),
	crypt_arr("Circuit"),
	crypt_arr("Golden"),
	crypt_arr("Glow")
};

const char* flags[] =
{
	crypt_arr("Money"),
	crypt_arr("Armor"),
	crypt_arr("Defuse kit"),
	crypt_arr("Scoped"),
	crypt_arr("Fakeducking"),
	crypt_arr("Vulnerable"),
	crypt_arr("Delay"),
	crypt_arr("Bomb carrier")
};

const char* removals[] =
{
	crypt_arr("Scope"),
	crypt_arr("Zoom"),
	crypt_arr("Smoke"),
	crypt_arr("Flash"),
	crypt_arr("Recoil"),
	crypt_arr("Landing bob"),
	crypt_arr("Postprocessing"),
	crypt_arr("Fog"),
	crypt_arr("Shadows")
};

const char* indicators[] =
{
	crypt_arr("Fake"),
	crypt_arr("Desync side"),
	crypt_arr("Choke"),
	crypt_arr("Damage override"),
	crypt_arr("Safe points"),
	crypt_arr("Body aim"),
	crypt_arr("Double tap"),
	crypt_arr("Hide shots"),
	crypt_arr("Resolver override")
};

const char* proximity_tracers_mode[] =
{
	crypt_arr("None"),
	crypt_arr("Line"),
	crypt_arr("Beam")
};

const char* skybox[] =
{
	crypt_arr("None"),
	crypt_arr("Tibet"),
	crypt_arr("Baggage"),
	crypt_arr("Italy"),
	crypt_arr("Aztec"),
	crypt_arr("Vertigo"),
	crypt_arr("Daylight"),
	crypt_arr("Daylight 2"),
	crypt_arr("Clouds"),
	crypt_arr("Clouds 2"),
	crypt_arr("Gray"),
	crypt_arr("Clear"),
	crypt_arr("Canals"),
	crypt_arr("Cobblestone"),
	crypt_arr("Assault"),
	crypt_arr("Clouds dark"),
	crypt_arr("Night"),
	crypt_arr("Night 2"),
	crypt_arr("Night flat"),
	crypt_arr("Dusty"),
	crypt_arr("Rainy"),
	crypt_arr("Custom")
};

const char* mainwep[] =
{
	crypt_arr("None"),
	crypt_arr("Auto"),
	crypt_arr("AWP"),
	crypt_arr("SSG 08")
};

const char* secwep[] =
{
	crypt_arr("None"),
	crypt_arr("Dual Berettas"),
	crypt_arr("Deagle / Revolver")
};

const char* strafes[] =
{
	crypt_arr("None"),
	crypt_arr("Legit"),
	crypt_arr("Directional"),
	crypt_arr("Rage")
};

const char* leg_movement[] =
{
	crypt_arr("Default"),
	crypt_arr("Avoid slide"),
	crypt_arr("Force slide")
};
const char* events_output[] =
{
	crypt_arr("Console"),
	crypt_arr("Chat")
};

const char* events[] =
{
	crypt_arr("Player hits"),
	crypt_arr("Items"),
	crypt_arr("Bomb")
};

const char* dpi_scaler[] =
{
	"0.5",
	"0.75",
	"1",
	"1.25",
	"1.5"
};

const char* grenades[] =
{
	crypt_arr("Grenades"),
	crypt_arr("Armor"),
	crypt_arr("Taser"),
	crypt_arr("Defuser")
};

const char* sounds[] =
{
	crypt_arr("None"),
	crypt_arr("Metallic"),
	crypt_arr("Cod"),
	crypt_arr("Bubble"),
	crypt_arr("Stapler"),
	crypt_arr("Bell"),
	crypt_arr("Flick"),
	crypt_arr("Custom")
};

const char* player_model_t[] =
{
	crypt_arr("None"),
	crypt_arr("Getaway Sally | The Professionals"),
	crypt_arr("Number K | The Professionals"),
	crypt_arr("Little Kev | The Professionals"),
	crypt_arr("Safecracker Voltzmann | The Professionals"),
	crypt_arr("Bloody Darryl The Strapped | The Professionals"),
	crypt_arr("Sir Bloody Loudmouth Darryl | The Professionals"),
	crypt_arr("Sir Bloody Darryl Royale | The Professionals"),
	crypt_arr("Sir Bloody Skullhead Darryl | The Professionals"),
	crypt_arr("Sir Bloody Silent Darryl | The Professionals"),
	crypt_arr("Sir Bloody Miami Darryl | The Professionals"),
	crypt_arr("Street Soldier | Phoenix"),
	crypt_arr("Soldier | Phoenix"),
	crypt_arr("Slingshot | Phoenix"),
	crypt_arr("Enforcer | Phoenix"),
	crypt_arr("Mr. Muhlik | Elite Crew"),
	crypt_arr("Prof. Shahmat | Elite Crew"),
	crypt_arr("Osiris | Elite Crew"),
	crypt_arr("Ground Rebel | Elite Crew"),
	crypt_arr("The Elite Mr. Muhlik | Elite Crew"),
	crypt_arr("Trapper | Guerrilla Warfare"),
	crypt_arr("Trapper Aggressor | Guerrilla Warfare"),
	crypt_arr("Vypa Sista of the Revolution | Guerrilla Warfare"),
	crypt_arr("Col. Mangos Dabisi | Guerrilla Warfare"),
	crypt_arr("Arno The Overgrown | Guerrilla Warfare"),
	crypt_arr("Medium Rare' Crasswater | Guerrilla Warfare"),
	crypt_arr("Crasswater The Forgotten | Guerrilla Warfare"),
	crypt_arr("Elite Trapper Solman | Guerrilla Warfare"),
	crypt_arr("The Doctor' Romanov | Sabre"),
	crypt_arr("Blackwolf | Sabre"),
	crypt_arr("Maximus | Sabre"),
	crypt_arr("Dragomir | Sabre"),
	crypt_arr("Rezan The Ready | Sabre"),
	crypt_arr("Rezan the Redshirt | Sabre"),
	crypt_arr("Dragomir | Sabre Footsoldier")
};

const char* dpi_scale_type[] =
{
	crypt_arr("100%"),
	crypt_arr("110%"),
	crypt_arr("120%"),
	crypt_arr("130%"),
	crypt_arr("140%"),
	crypt_arr("150%"),
	crypt_arr("160%"),
	crypt_arr("170%"),
	crypt_arr("180%"),
	crypt_arr("190%"),
	crypt_arr("200%")
};

const char* player_model_ct[] =
{
	crypt_arr("None"),
	crypt_arr("Cmdr. Davida 'Goggles' Fernandez | SEAL Frogman"),
	crypt_arr("Cmdr. Frank 'Wet Sox' Baroud | SEAL Frogman"),
	crypt_arr("Lieutenant Rex Krikey | SEAL Frogman"),
	crypt_arr("Michael Syfers | FBI Sniper"),
	crypt_arr("Operator | FBI SWAT"),
	crypt_arr("Special Agent Ava | FBI"),
	crypt_arr("Markus Delrow | FBI HRT"),
	crypt_arr("Sous-Lieutenant Medic | Gendarmerie Nationale"),
	crypt_arr("Chem-Haz Capitaine | Gendarmerie Nationale"),
	crypt_arr("Chef d'Escadron Rouchard | Gendarmerie Nationale"),
	crypt_arr("Aspirant | Gendarmerie Nationale"),
	crypt_arr("Officer Jacques Beltram | Gendarmerie Nationale"),
	crypt_arr("D Squadron Officer | NZSAS"),
	crypt_arr("B Squadron Officer | SAS"),
	crypt_arr("Seal Team 6 Soldier | NSWC SEAL"),
	crypt_arr("Buckshot | NSWC SEAL"),
	crypt_arr("Lt. Commander Ricksaw | NSWC SEAL"),
	crypt_arr("Blueberries' Buckshot | NSWC SEAL"),
	crypt_arr("3rd Commando Company | KSK"),
	crypt_arr("Two Times' McCoy | TACP Cavalry"),
	crypt_arr("Two Times' McCoy | USAF TACP"),
	crypt_arr("Primeiro Tenente | Brazilian 1st Battalion"),
	crypt_arr("Cmdr. Mae 'Dead Cold' Jamison | SWAT"),
	crypt_arr("1st Lieutenant Farlow | SWAT"),
	crypt_arr("John 'Van Healen' Kask | SWAT"),
	crypt_arr("Bio-Haz Specialist | SWAT"),
	crypt_arr("Sergeant Bombson | SWAT"),
	crypt_arr("Chem-Haz Specialist | SWAT"),
	crypt_arr("Lieutenant 'Tree Hugger' Farlow | SWAT")
};