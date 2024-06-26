// uimenu.cpp
//
// MiniDexed - Dexed FM synthesizer for bare metal Raspberry Pi
// Copyright (C) 2022  The MiniDexed Team
//
// Original author of this class:
//	R. Stange <rsta2@o2online.de>
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.
//
#include "uimenu.h"
#include "minidexed.h"
#include "mididevice.h"
#include "userinterface.h"
#include "sysexfileloader.h"
#include "config.h"
#include <cmath>
#include <circle/sysconfig.h>
#include <assert.h>

using namespace std;
LOGMODULE ("uimenu");

const CUIMenu::TMenuItem CUIMenu::s_MenuRoot[] =
{
	{"MDX816", MainMenuHandler, s_MainMenu},
	{0}
};

// inserting menu items before "TG1" affect TGShortcutHandler()
const CUIMenu::TMenuItem CUIMenu::s_MainMenu[] =
{
	{"Performance",	MenuHandler,s_PerformanceMenu}, 
	{"Effects",	MenuHandler,	s_EffectsMenu},
	{"TG1",		MenuHandler,	s_TGMenu, 0},
#ifdef ARM_ALLOW_MULTI_CORE
	{"TG2",		MenuHandler,	s_TGMenu, 1},
	{"TG3",		MenuHandler,	s_TGMenu, 2},
	{"TG4",		MenuHandler,	s_TGMenu, 3},
	{"TG5",		MenuHandler,	s_TGMenu, 4},
	{"TG6",		MenuHandler,	s_TGMenu, 5},
	{"TG7",		MenuHandler,	s_TGMenu, 6},
	{"TG8",		MenuHandler,	s_TGMenu, 7},
#endif
	{0}
};

const CUIMenu::TMenuItem CUIMenu::s_TGMenu[] =
{
	{"Voice",		SelectVoice},
	{"Bank",		SelectVoiceBank},
	{"Volume",		EditTGParameter,	0,	CMiniDexed::TGParameterVolume},
#ifdef ARM_ALLOW_MULTI_CORE
	{"Pan",		EditTGParameter,		0,	CMiniDexed::TGParameterPan},
#endif
	{"Reverb-Send",	EditTGParameter,	0,	CMiniDexed::TGParameterReverbSend},
	{"Detune",		EditTGParameter,	0,	CMiniDexed::TGParameterMasterTune},
	{"Cutoff",		EditTGParameter,	0,	CMiniDexed::TGParameterCutoff},
	{"Resonance",	EditTGParameter,	0,	CMiniDexed::TGParameterResonance},
	{"Pitch Bend",	MenuHandler,		s_EditPitchBendMenu},
	{"Portamento",	MenuHandler,		s_EditPortamentoMenu},
	{"Poly/Mono",	EditTGParameter,	0,	CMiniDexed::TGParameterMonoMode}, 
	{"Modulation",	MenuHandler,		s_ModulationMenu},
	{"Channel",		EditTGParameter,	0,	CMiniDexed::TGParameterMIDIChannel},
	{"Edit Voice",	MenuHandler,		s_EditVoiceMenu},
	{0}
};

const CUIMenu::TMenuItem CUIMenu::s_EffectsMenu[] =
{
	{"Compress",	EditGlobalParameter,0,	CMiniDexed::ParameterCompressorEnable},
#ifdef ARM_ALLOW_MULTI_CORE
	{"Reverb",		MenuHandler,		s_ReverbMenu},
#endif
	{0}
};

const CUIMenu::TMenuItem CUIMenu::s_EditPitchBendMenu[] =
{
	{"Bend Range",	EditTGParameter2,	0,	CMiniDexed::TGParameterPitchBendRange},
	{"Bend Step",	EditTGParameter2,	0,	CMiniDexed::TGParameterPitchBendStep},
	{0}
};

const CUIMenu::TMenuItem CUIMenu::s_EditPortamentoMenu[] =
{
	{"Mode",		EditTGParameter2,	0,	CMiniDexed::TGParameterPortamentoMode},
	{"Glissando",	EditTGParameter2,	0,	CMiniDexed::TGParameterPortamentoGlissando},
	{"Time",		EditTGParameter2,	0,	CMiniDexed::TGParameterPortamentoTime},
	{0}
};

const CUIMenu::TMenuItem CUIMenu::s_ModulationMenu[] =
{
	{"Mod. Wheel",		MenuHandler,	s_ModulationMenuParameters,	CMiniDexed::TGParameterMWRange},
	{"Foot Control",	MenuHandler,	s_ModulationMenuParameters,	CMiniDexed::TGParameterFCRange},
	{"Breath Control",	MenuHandler,	s_ModulationMenuParameters,	CMiniDexed::TGParameterBCRange},
	{"Aftertouch",		MenuHandler,	s_ModulationMenuParameters,	CMiniDexed::TGParameterATRange},
	{0}
};

const CUIMenu::TMenuItem CUIMenu::s_ModulationMenuParameters[] =
{
	{"Range",		EditTGParameterModulation,	0, 0},
	{"Pitch",		EditTGParameterModulation,	0, 1},
	{"Amplitude",	EditTGParameterModulation,	0, 2},
	{"EG Bias",		EditTGParameterModulation,	0, 3},
	{0}
};

#ifdef ARM_ALLOW_MULTI_CORE

const CUIMenu::TMenuItem CUIMenu::s_ReverbMenu[] =
{
	{"Enable",		EditGlobalParameter,	0,	CMiniDexed::ParameterReverbEnable},
	{"Size",		EditGlobalParameter,	0,	CMiniDexed::ParameterReverbSize},
	{"High damp",	EditGlobalParameter,	0,	CMiniDexed::ParameterReverbHighDamp},
	{"Low damp",	EditGlobalParameter,	0,	CMiniDexed::ParameterReverbLowDamp},
	{"Low pass",	EditGlobalParameter,	0,	CMiniDexed::ParameterReverbLowPass},
	{"Diffusion",	EditGlobalParameter,	0,	CMiniDexed::ParameterReverbDiffusion},
	{"Level",		EditGlobalParameter,	0,	CMiniDexed::ParameterReverbLevel},
	{0}
};

#endif

// inserting menu items before "OP1" affect OPShortcutHandler()
const CUIMenu::TMenuItem CUIMenu::s_EditVoiceMenu[] =
{
	{"OP1",			MenuHandler,		s_OperatorMenu, 0},
	{"OP2",			MenuHandler,		s_OperatorMenu, 1},
	{"OP3",			MenuHandler,		s_OperatorMenu, 2},
	{"OP4",			MenuHandler,		s_OperatorMenu, 3},
	{"OP5",			MenuHandler,		s_OperatorMenu, 4},
	{"OP6",			MenuHandler,		s_OperatorMenu, 5},
	{"Algorithm",	EditVoiceParameter,	0,		DEXED_ALGORITHM},
	{"Feedback",	EditVoiceParameter,	0,		DEXED_FEEDBACK},
	{"P EG Rate 1",	EditVoiceParameter,	0,		DEXED_PITCH_EG_R1},
	{"P EG Rate 2",	EditVoiceParameter,	0,		DEXED_PITCH_EG_R2},
	{"P EG Rate 3",	EditVoiceParameter,	0,		DEXED_PITCH_EG_R3},
	{"P EG Rate 4",	EditVoiceParameter,	0,		DEXED_PITCH_EG_R4},
	{"P EG Level 1",EditVoiceParameter,	0,		DEXED_PITCH_EG_L1},
	{"P EG Level 2",EditVoiceParameter,	0,		DEXED_PITCH_EG_L2},
	{"P EG Level 3",EditVoiceParameter,	0,		DEXED_PITCH_EG_L3},
	{"P EG Level 4",EditVoiceParameter,	0,		DEXED_PITCH_EG_L4},
	{"Osc Key Sync",EditVoiceParameter,	0,		DEXED_OSC_KEY_SYNC},
	{"LFO Speed",	EditVoiceParameter,	0,		DEXED_LFO_SPEED},
	{"LFO Delay",	EditVoiceParameter,	0,		DEXED_LFO_DELAY},
	{"LFO PMD",		EditVoiceParameter,	0,		DEXED_LFO_PITCH_MOD_DEP},
	{"LFO AMD",		EditVoiceParameter,	0,		DEXED_LFO_AMP_MOD_DEP},
	{"LFO Sync",	EditVoiceParameter,	0,		DEXED_LFO_SYNC},
	{"LFO Wave",	EditVoiceParameter,	0,		DEXED_LFO_WAVE},
	{"P Mod Sens.",	EditVoiceParameter,	0,		DEXED_LFO_PITCH_MOD_SENS},
	{"Transpose",	EditVoiceParameter,	0,		DEXED_TRANSPOSE},
	{"Name",		InputTxt,0 , 3}, 
	{0}
};

const CUIMenu::TMenuItem CUIMenu::s_OperatorMenu[] =
{
	{"Output Level",EditOPParameter,	0,	DEXED_OP_OUTPUT_LEV},
	{"Freq Coarse",	EditOPParameter,	0,	DEXED_OP_FREQ_COARSE},
	{"Freq Fine",	EditOPParameter,	0,	DEXED_OP_FREQ_FINE},
	{"Osc Detune",	EditOPParameter,	0,	DEXED_OP_OSC_DETUNE},
	{"Osc Mode",	EditOPParameter,	0,	DEXED_OP_OSC_MODE},
	{"EG Rate 1",	EditOPParameter,	0,	DEXED_OP_EG_R1},
	{"EG Rate 2",	EditOPParameter,	0,	DEXED_OP_EG_R2},
	{"EG Rate 3",	EditOPParameter,	0,	DEXED_OP_EG_R3},
	{"EG Rate 4",	EditOPParameter,	0,	DEXED_OP_EG_R4},
	{"EG Level 1",	EditOPParameter,	0,	DEXED_OP_EG_L1},
	{"EG Level 2",	EditOPParameter,	0,	DEXED_OP_EG_L2},
	{"EG Level 3",	EditOPParameter,	0,	DEXED_OP_EG_L3},
	{"EG Level 4",	EditOPParameter,	0,	DEXED_OP_EG_L4},
	{"Break Point",	EditOPParameter,	0,	DEXED_OP_LEV_SCL_BRK_PT},
	{"L Key Depth",	EditOPParameter,	0,	DEXED_OP_SCL_LEFT_DEPTH},
	{"R Key Depth",	EditOPParameter,	0,	DEXED_OP_SCL_RGHT_DEPTH},
	{"L Key Scale",	EditOPParameter,	0,	DEXED_OP_SCL_LEFT_CURVE},
	{"R Key Scale",	EditOPParameter,	0,	DEXED_OP_SCL_RGHT_CURVE},
	{"Rate Scaling",EditOPParameter,	0,	DEXED_OP_OSC_RATE_SCALE},
	{"A Mod Sens.",	EditOPParameter,	0,	DEXED_OP_AMP_MOD_SENS},
	{"K Vel. Sens.",EditOPParameter,	0,	DEXED_OP_KEY_VEL_SENS},
	{"Enable", 		EditOPParameter, 	0, DEXED_OP_ENABLE},
	{0}
};

const CUIMenu::TMenuItem CUIMenu::s_SaveMenu[] =
{
	{"Overwrite",		SavePerformance, 0, 0}, 
	{"New",				InputTxt, 0 , 1}, 
	{"Save as default",	SavePerformance, 0, 1}, 
	{0}
};

// must match CMiniDexed::TParameter
const CUIMenu::TParameter CUIMenu::s_GlobalParameter[CMiniDexed::ParameterUnknown] =
{
	{0,	1,	1,	ToOnOff},									// ParameterCompessorEnable
	{0,	1,	1,	ToOnOff},									// ParameterReverbEnable
	{0,	99,	1},												// ParameterReverbSize
	{0,	99,	1},												// ParameterReverbHighDamp
	{0,	99,	1},												// ParameterReverbLowDamp
	{0,	99,	1},												// ParameterReverbLowPass
	{0,	99,	1},												// ParameterReverbDiffusion
	{0,	99,	1},												// ParameterReverbLevel
	{0,	CMIDIDevice::ChannelUnknown-1,	1, ToMIDIChannel}, 	// ParameterPerformanceSelectChannel
	{0, NUM_PERFORMANCE_BANKS, 1}							// ParameterPerformanceBank
};

// must match CMiniDexed::TTGParameter
const CUIMenu::TParameter CUIMenu::s_TGParameter[CMiniDexed::TGParameterUnknown] =
{
	{0,	CSysExFileLoader::MaxVoiceBankID,	1},				// TGParameterVoiceBank
	{0, 0, 0},												// TGParameterVoiceBankMSB (not used in menus)
	{0, 0, 0},												// TGParameterVoiceBankLSB (not used in menus)
	{0,	CSysExFileLoader::VoicesPerBank-1,	1},				// TGParameterVoice
	{0,	127,	8, ToVolume},								// TGParameterVolume
	{0,	127,	8, ToPan},									// TGParameterPan
	{-99,	99,	1},											// TGParameterMasterTune
	{0,	99,	1},												// TGParameterCutoff
	{0,	99,	1},												// TGParameterResonance
	{0,	CMIDIDevice::ChannelUnknown-1,	1, ToMIDIChannel}, 	// TGParameterMIDIChannel
	{0, 99, 1},												// TGParameterReverbSend
	{0,	12,	1},												// TGParameterPitchBendRange
	{0,	12,	1},												// TGParameterPitchBendStep
	{0,	1,	1, ToPortaMode},								// TGParameterPortamentoMode
	{0,	1,	1, ToPortaGlissando},							// TGParameterPortamentoGlissando
	{0,	99,	1},												// TGParameterPortamentoTime
	{0,	1,	1, ToPolyMono}, 								// TGParameterMonoMode 
	{0, 99, 1}, 											//MW Range
	{0, 1, 1, ToOnOff}, 									//MW Pitch
	{0, 1, 1, ToOnOff}, 									//MW Amp
	{0, 1, 1, ToOnOff}, 									//MW EGBias
	{0, 99, 1}, 											//FC Range
	{0, 1, 1, ToOnOff}, 									//FC Pitch
	{0, 1, 1, ToOnOff}, 									//FC Amp
	{0, 1, 1, ToOnOff}, 									//FC EGBias
	{0, 99, 1}, 											//BC Range
	{0, 1, 1, ToOnOff}, 									//BC Pitch
	{0, 1, 1, ToOnOff}, 									//BC Amp
	{0, 1, 1, ToOnOff}, 									//BC EGBias
	{0, 99, 1}, 											//AT Range
	{0, 1, 1, ToOnOff}, 									//AT Pitch
	{0, 1, 1, ToOnOff}, 									//AT Amp
	{0, 1, 1, ToOnOff}										//AT EGBias
};

// must match DexedVoiceParameters in Synth_Dexed
const CUIMenu::TParameter CUIMenu::s_VoiceParameter[] =
{
	{0,	99,	1},						// DEXED_PITCH_EG_R1
	{0,	99,	1},						// DEXED_PITCH_EG_R2
	{0,	99,	1},						// DEXED_PITCH_EG_R3
	{0,	99,	1},						// DEXED_PITCH_EG_R4
	{0,	99,	1},						// DEXED_PITCH_EG_L1
	{0,	99,	1},						// DEXED_PITCH_EG_L2
	{0,	99,	1},						// DEXED_PITCH_EG_L3
	{0,	99,	1},						// DEXED_PITCH_EG_L4
	{0,	31,	1,	ToAlgorithm},		// DEXED_ALGORITHM
	{0,	7,	1},						// DEXED_FEEDBACK
	{0,	1,	1,	ToOnOff},			// DEXED_OSC_KEY_SYNC
	{0,	99,	1},						// DEXED_LFO_SPEED
	{0,	99,	1},						// DEXED_LFO_DELAY
	{0,	99,	1},						// DEXED_LFO_PITCH_MOD_DEP
	{0,	99,	1},						// DEXED_LFO_AMP_MOD_DEP
	{0,	1,	1,	ToOnOff},			// DEXED_LFO_SYNC
	{0,	5,	1,	ToLFOWaveform},		// DEXED_LFO_WAVE
	{0,	7,	1},						// DEXED_LFO_PITCH_MOD_SENS
	{0,	48,	1,	ToTransposeNote},	// DEXED_TRANSPOSE
	{0,	1,	1}						// Voice Name - Dummy parameters for in case new item would be added in future 
};

// must match DexedVoiceOPParameters in Synth_Dexed
const CUIMenu::TParameter CUIMenu::s_OPParameter[] =
{
	{0,	99,	1},						// DEXED_OP_EG_R1
	{0,	99,	1},						// DEXED_OP_EG_R2
	{0,	99,	1},						// DEXED_OP_EG_R3
	{0,	99,	1},						// DEXED_OP_EG_R4
	{0,	99,	1},						// DEXED_OP_EG_L1
	{0,	99,	1},						// DEXED_OP_EG_L2
	{0,	99,	1},						// DEXED_OP_EG_L3
	{0,	99,	1},						// DEXED_OP_EG_L4
	{0,	99,	1,	ToBreakpointNote},	// DEXED_OP_LEV_SCL_BRK_PT
	{0,	99,	1},						// DEXED_OP_SCL_LEFT_DEPTH
	{0,	99,	1},						// DEXED_OP_SCL_RGHT_DEPTH
	{0,	3,	1,	ToKeyboardCurve},	// DEXED_OP_SCL_LEFT_CURVE
	{0,	3,	1,	ToKeyboardCurve},	// DEXED_OP_SCL_RGHT_CURVE
	{0,	7,	1},						// DEXED_OP_OSC_RATE_SCALE
	{0,	3,	1},						// DEXED_OP_AMP_MOD_SENS
	{0,	7,	1},						// DEXED_OP_KEY_VEL_SENS
	{0,	99,	1},						// DEXED_OP_OUTPUT_LEV
	{0,	1,	1,	ToOscillatorMode},	// DEXED_OP_OSC_MODE
	{0,	31,	1},						// DEXED_OP_FREQ_COARSE
	{0,	99,	1},						// DEXED_OP_FREQ_FINE
	{0,	14,	1,	ToOscillatorDetune},// DEXED_OP_OSC_DETUNE
	{0, 1, 1, ToOnOff}				// DEXED_OP_ENABLE
};

const char CUIMenu::s_NoteName[100][5] =
{
"A-1", "A#-1", "B-1", "C0", "C#0", "D0", "D#0", "E0", "F0", "F#0", "G0", "G#0",
"A0", "A#0", "B0", "C1", "C#1", "D1", "D#1", "E1", "F1", "F#1", "G1", "G#1",
"A1", "A#1", "B1", "C2", "C#2", "D2", "D#2", "E2", "F2", "F#2", "G2", "G#2",
"A2", "A#2", "B2", "C3", "C#3", "D3", "D#3", "E3", "F3", "F#3", "G3", "G#3",
"A3", "A#3", "B3", "C4", "C#4", "D4", "D#4", "E4", "F4", "F#4", "G4", "G#4",
"A4", "A#4", "B4", "C5", "C#5", "D5", "D#5", "E5", "F5", "F#5", "G5", "G#5",
"A5", "A#5", "B5", "C6", "C#6", "D6", "D#6", "E6", "F6", "F#6", "G6", "G#6",
"A6", "A#6", "B6", "C7", "C#7", "D7", "D#7", "E7", "F7", "F#7", "G7", "G#7",
"A7", "A#7", "B7", "C8"
};

static const unsigned NoteC3 = 39;

const CUIMenu::TMenuItem CUIMenu::s_PerformanceMenu[] =
{
	{"Perf Load",	SelectPerformance, 0, 0}, 
	{"Perf Bank",	SelectPerformanceBank, 0, 0},	
	{"Perf Save",	MenuHandler, s_SaveMenu},
	{"Perf Delete",	SelectPerformance, 0, 1},
	{"PCCH",	EditGlobalParameter, 0,	CMiniDexed::ParameterPerformanceSelectChannel},
	{0}
};


CUIMenu::CUIMenu (CUserInterface *pUI, CMiniDexed *pMiniDexed)
:	m_pUI (pUI),
	m_pMiniDexed (pMiniDexed),
	m_pParentMenu (s_MenuRoot),
	m_pCurrentMenu (s_MainMenu),
	m_nCurrentMenuItem (0),
	m_nCurrentSelection (0),
	m_nCurrentParameter (0),
	m_nCurrentMenuDepth (0)
{
/*
#ifndef ARM_ALLOW_MULTI_CORE
	// If there is just one core, then there is only a single
	// tone generator so start on the TG1 menu...
	m_pParentMenu = s_MainMenu;
	m_pCurrentMenu = s_TGMenu;
	m_nCurrentMenuItem = 0;
	m_nCurrentSelection = 0;
	m_nCurrentParameter = 0;
	m_nCurrentMenuDepth = 1;

	// Place the "root" menu at the top of the stack
	m_MenuStackParent[0] = s_MenuRoot;
	m_MenuStackMenu[0] = s_MainMenu;
	m_nMenuStackItem[0]	= 0;
	m_nMenuStackSelection[0] = 0;
	m_nMenuStackParameter[0] = 0;
#endif
*/	
}

void CUIMenu::EventHandler (TMenuEvent Event)
{
	switch (Event)
	{
		//back to previous menu
		case MenuEventBack:				
			if (m_nCurrentMenuDepth)
			{
				m_nCurrentMenuDepth--;

				m_pParentMenu = m_MenuStackParent[m_nCurrentMenuDepth];
				m_pCurrentMenu = m_MenuStackMenu[m_nCurrentMenuDepth];
				m_nCurrentMenuItem = m_nMenuStackItem[m_nCurrentMenuDepth];
				m_nCurrentSelection = m_nMenuStackSelection[m_nCurrentMenuDepth];
				m_nCurrentParameter = m_nMenuStackParameter[m_nCurrentMenuDepth];

				(*m_pParentMenu[m_nCurrentMenuItem].Handler) (this, Event);
				EventHandler(MenuEventUpdate);
			}
			break;

		case MenuEventHome:
//	#ifdef ARM_ALLOW_MULTI_CORE
			m_pParentMenu = s_MenuRoot;
			m_pCurrentMenu = s_MainMenu;
			m_nCurrentMenuItem = 0;
			m_nCurrentSelection = 0;
			m_nCurrentParameter = 0;
			m_nCurrentMenuDepth = 0;
/*			
	#else
			// "Home" is the TG0 menu if only one TG active
			m_pParentMenu = s_MainMenu;
			m_pCurrentMenu = s_TGMenu;
			m_nCurrentMenuItem = 0;
			m_nCurrentSelection = 0;
			m_nCurrentParameter = 0;
			m_nCurrentMenuDepth = 1;
			// Place the "root" menu at the top of the stack
			m_MenuStackParent[0] = s_MenuRoot;
			m_MenuStackMenu[0] = s_MainMenu;
			m_nMenuStackItem[0] = 0;
			m_nMenuStackSelection[0] = 0;
			m_nMenuStackParameter[0] = 0;
	#endif
*/	
			(*m_pParentMenu[m_nCurrentMenuItem].Handler) (this, Event);
			EventHandler(MenuEventUpdate);
			break;

		case MenuEventPgmUp:
		case MenuEventPgmDown:
			PgmUpDownHandler(Event);
			break;

		case MenuEventTGUp:
		case MenuEventTGDown:
			TGUpDownHandler(Event);
			break;

		default:
			(*m_pParentMenu[m_nCurrentMenuItem].Handler) (this, Event);
			break;
	}
}

// special handler for main men
// it handles also the MasterVolume
void CUIMenu::MainMenuHandler (CUIMenu *pUIMenu, TMenuEvent Event)
{

	//unsigned nTG = pUIMenu->m_nMenuStackParameter[pUIMenu->m_nCurrentMenuDepth-2];

	int nValue = pUIMenu->m_pMiniDexed->GetMasterVolume();

	switch (Event)
	{
		case MenuEventUpdate:
			break;

		case MenuEventBack:
			if (pUIMenu->m_hMainMenuTimer != 0) 
			{
				CTimer::Get ()->CancelKernelTimer (pUIMenu->m_hMainMenuTimer);
				pUIMenu->m_hMainMenuTimer = 0;
			}	
			pUIMenu->m_bSetMainVolume = false;				
			pUIMenu->m_bShowSetMainVolume = false;
			pUIMenu->m_hMainMenuTimer = CTimer::Get ()->StartKernelTimer (MSEC2HZ (MENU_CANCEL_DELAY), TimerHandlerCancelTimed, 0, pUIMenu);
			break;

		case MenuEventHome:
			if (pUIMenu->m_hMainMenuTimer != 0) 
			{
				CTimer::Get ()->CancelKernelTimer (pUIMenu->m_hMainMenuTimer);
				pUIMenu->m_hMainMenuTimer = 0;
			}	
			pUIMenu->m_bSetMainVolume=true;
			pUIMenu->m_bShowSetMainVolume=false;
			pUIMenu->m_nCurrentMenuDepth=0;
			break;

		case MenuEventSelect:	
			if (pUIMenu->m_hMainMenuTimer != 0) 
			{
				CTimer::Get ()->CancelKernelTimer (pUIMenu->m_hMainMenuTimer);
				pUIMenu->m_hMainMenuTimer = 0;
			}	
			if (pUIMenu->m_bSetMainVolume) 
			{
				pUIMenu->m_bSetMainVolume = false;				
				pUIMenu->m_bShowSetMainVolume = false;
				pUIMenu->m_hMainMenuTimer = CTimer::Get ()->StartKernelTimer (MSEC2HZ (MENU_CANCEL_DELAY), TimerHandlerCancelTimed, 0, pUIMenu);				
			}
			else
			{
				assert (pUIMenu->m_nCurrentMenuDepth < MaxMenuDepth);
				// push current menu onto stack
				pUIMenu->m_MenuStackParent[pUIMenu->m_nCurrentMenuDepth] = pUIMenu->m_pParentMenu;
				pUIMenu->m_MenuStackMenu[pUIMenu->m_nCurrentMenuDepth] = pUIMenu->m_pCurrentMenu;
				pUIMenu->m_nMenuStackItem[pUIMenu->m_nCurrentMenuDepth] = pUIMenu->m_nCurrentMenuItem;
				pUIMenu->m_nMenuStackSelection[pUIMenu->m_nCurrentMenuDepth] = pUIMenu->m_nCurrentSelection;
				pUIMenu->m_nMenuStackParameter[pUIMenu->m_nCurrentMenuDepth] = pUIMenu->m_nCurrentParameter;
				// increase depth
				pUIMenu->m_nCurrentMenuDepth++;
				// assign new menu data
				pUIMenu->m_pParentMenu = pUIMenu->m_pCurrentMenu;
				pUIMenu->m_nCurrentParameter = pUIMenu->m_pCurrentMenu[pUIMenu->m_nCurrentSelection].Parameter;
				pUIMenu->m_pCurrentMenu = pUIMenu->m_pCurrentMenu[pUIMenu->m_nCurrentSelection].MenuItem;
				pUIMenu->m_nCurrentMenuItem = pUIMenu->m_nCurrentSelection;
				pUIMenu->m_nCurrentSelection = 0;
			}
			break;

		case MenuEventStepDown:
			if (pUIMenu->m_bSetMainVolume) 
			{
				nValue -= STEP_MASTER_VOLUME;
				if (nValue < 0)
				{
					nValue = 0;
				}
				pUIMenu->m_pMiniDexed->SetMasterVolume((unsigned) nValue);
				pUIMenu->m_bShowSetMainVolume = true;
			}
			else
			{
				if (pUIMenu->m_nCurrentSelection > 0)
				{
					pUIMenu->m_nCurrentSelection--;
				}
			}
			if (pUIMenu->m_hMainMenuTimer != 0) 
			{
					CTimer::Get ()->CancelKernelTimer (pUIMenu->m_hMainMenuTimer);
			}
			pUIMenu->m_hMainMenuTimer = CTimer::Get ()->StartKernelTimer (MSEC2HZ (MENU_CANCEL_DELAY), TimerHandlerCancelTimed, 0, pUIMenu);
			break;

		case MenuEventStepUp:
			if (pUIMenu->m_bSetMainVolume) 
			{
				nValue += STEP_MASTER_VOLUME;
				if (nValue > MAX_MASTER_VOLUME)
				{
					nValue = MAX_MASTER_VOLUME;
				}
				pUIMenu->m_pMiniDexed->SetMasterVolume(nValue);
				pUIMenu->m_bShowSetMainVolume = true;
			}
			else
			{
				++pUIMenu->m_nCurrentSelection;
				if (!pUIMenu->m_pCurrentMenu[pUIMenu->m_nCurrentSelection].Name)  // more entries?
				{
					pUIMenu->m_nCurrentSelection--;
				}
			}
			if (pUIMenu->m_hMainMenuTimer != 0) 
			{
					CTimer::Get ()->CancelKernelTimer (pUIMenu->m_hMainMenuTimer);
			}
			pUIMenu->m_hMainMenuTimer = CTimer::Get ()->StartKernelTimer (MSEC2HZ (MENU_CANCEL_DELAY), TimerHandlerCancelTimed, 0, pUIMenu);
			break;
		case MenuEventCancelTimedFeature:
			if (pUIMenu->m_bShowSetMainVolume)			
			{
				pUIMenu->m_bShowSetMainVolume = false;
			}
			if (!pUIMenu->m_bSetMainVolume) 
			{
				pUIMenu->m_bSetMainVolume = true;
			}
			break;

		case MenuEventModeMidi:
			{
				CMiniDexed::TGMidiMode midiMode = pUIMenu->m_pMiniDexed->GetGlobalMidiMode();
				switch (midiMode)
				{
					case CMiniDexed::TGMidiMode::MidiModeNormal:
						pUIMenu->m_pMiniDexed->SetGlobalMidiMode(CMiniDexed::TGMidiMode::MidiMode1Only);
						break;
					case CMiniDexed::TGMidiMode::MidiMode1Only:
						pUIMenu->m_pMiniDexed->SetGlobalMidiMode(CMiniDexed::TGMidiMode::MidiMode1All);
						break;
					case CMiniDexed::TGMidiMode::MidiMode1All:
						pUIMenu->m_pMiniDexed->SetGlobalMidiMode(CMiniDexed::TGMidiMode::MidiModeOmni);
						break;
					case CMiniDexed::TGMidiMode::MidiModeOmni:
						pUIMenu->m_pMiniDexed->SetGlobalMidiMode(CMiniDexed::TGMidiMode::MidiModeNormal);
						break;
					default:
						break;
				}
			}
			break;			
		default:
			return;
	}

	// show things...
	if (pUIMenu->m_bSetMainVolume)
	{
		if(pUIMenu->m_bShowSetMainVolume)
		{
			pUIMenu->m_pUI->DisplayWrite ("Master",
				      "Volume",
				      ToVolume(nValue).c_str (),
				      nValue > 0, nValue < MAX_MASTER_VOLUME);
		}
		else
		{
			pUIMenu->m_pUI->DisplayWrite (
				pUIMenu->m_pParentMenu[pUIMenu->m_nCurrentMenuItem].Name,
				pUIMenu->m_pMiniDexed->GetGlobalMidiModeString().c_str(),
				pUIMenu->m_pMiniDexed->GetPerformanceName(pUIMenu->m_pMiniDexed->GetPerformanceID()).c_str(),
				false,
				false);

		}
	}
	else
	{
		if (pUIMenu->m_pCurrentMenu)				// if this is another menu?
		{
			pUIMenu->m_pUI->DisplayWrite (
				pUIMenu->m_pParentMenu[pUIMenu->m_nCurrentMenuItem].Name,
				pUIMenu->m_pMiniDexed->GetGlobalMidiModeString().c_str(),
				pUIMenu->m_pCurrentMenu[pUIMenu->m_nCurrentSelection].Name,
				pUIMenu->m_nCurrentSelection > 0,
				!!pUIMenu->m_pCurrentMenu[pUIMenu->m_nCurrentSelection+1].Name);
		}
		else
		{
			pUIMenu->EventHandler (MenuEventUpdate);	// no, update parameter display
		}
	}
}


void CUIMenu::MenuHandler (CUIMenu *pUIMenu, TMenuEvent Event)
{
	switch (Event)
	{
		case MenuEventUpdate:
			break;

		case MenuEventSelect:				
			assert (pUIMenu->m_nCurrentMenuDepth < MaxMenuDepth);
			// push current menu onto stack
			pUIMenu->m_MenuStackParent[pUIMenu->m_nCurrentMenuDepth] = pUIMenu->m_pParentMenu;
			pUIMenu->m_MenuStackMenu[pUIMenu->m_nCurrentMenuDepth] = pUIMenu->m_pCurrentMenu;
			pUIMenu->m_nMenuStackItem[pUIMenu->m_nCurrentMenuDepth] = pUIMenu->m_nCurrentMenuItem;
			pUIMenu->m_nMenuStackSelection[pUIMenu->m_nCurrentMenuDepth] = pUIMenu->m_nCurrentSelection;
			pUIMenu->m_nMenuStackParameter[pUIMenu->m_nCurrentMenuDepth] = pUIMenu->m_nCurrentParameter;
			// increase depth
			pUIMenu->m_nCurrentMenuDepth++;
			// assign new menu data
			pUIMenu->m_pParentMenu = pUIMenu->m_pCurrentMenu;
			pUIMenu->m_nCurrentParameter = pUIMenu->m_pCurrentMenu[pUIMenu->m_nCurrentSelection].Parameter;
			pUIMenu->m_pCurrentMenu = pUIMenu->m_pCurrentMenu[pUIMenu->m_nCurrentSelection].MenuItem;
			pUIMenu->m_nCurrentMenuItem = pUIMenu->m_nCurrentSelection;
			pUIMenu->m_nCurrentSelection = 0;
			break;

		case MenuEventStepDown:
			if (pUIMenu->m_nCurrentSelection > 0)
			{
				pUIMenu->m_nCurrentSelection--;
			}
			break;

		case MenuEventStepUp:
			++pUIMenu->m_nCurrentSelection;
			if (!pUIMenu->m_pCurrentMenu[pUIMenu->m_nCurrentSelection].Name)  // more entries?
			{
				pUIMenu->m_nCurrentSelection--;
			}
			break;

		default:
			return;
	}

	if (pUIMenu->m_pCurrentMenu)				// if this is another menu?
	{
		pUIMenu->m_pUI->DisplayWrite (
			pUIMenu->m_pParentMenu[pUIMenu->m_nCurrentMenuItem].Name,
			"",
			pUIMenu->m_pCurrentMenu[pUIMenu->m_nCurrentSelection].Name,
			pUIMenu->m_nCurrentSelection > 0,
			!!pUIMenu->m_pCurrentMenu[pUIMenu->m_nCurrentSelection+1].Name);
	}
	else
	{
		pUIMenu->EventHandler (MenuEventUpdate);	// no, update parameter display
	}
}

void CUIMenu::EditGlobalParameter (CUIMenu *pUIMenu, TMenuEvent Event)
{
	CMiniDexed::TParameter Param = (CMiniDexed::TParameter) pUIMenu->m_nCurrentParameter;
	const TParameter &rParam = s_GlobalParameter[Param];

	int nValue = pUIMenu->m_pMiniDexed->GetParameter (Param);

	switch (Event)
	{
		case MenuEventUpdate:
			break;

		case MenuEventStepDown:
			nValue -= rParam.Increment;
			if (nValue < rParam.Minimum)
			{
				nValue = rParam.Minimum;
			}
			pUIMenu->m_pMiniDexed->SetParameter (Param, nValue);
			break;

		case MenuEventStepUp:
			nValue += rParam.Increment;
			if (nValue > rParam.Maximum)
			{
				nValue = rParam.Maximum;
			}
			pUIMenu->m_pMiniDexed->SetParameter (Param, nValue);
			break;

		default:
			return;
	}

	const char *pMenuName =
		pUIMenu->m_MenuStackParent[pUIMenu->m_nCurrentMenuDepth-1]
			[pUIMenu->m_nMenuStackItem[pUIMenu->m_nCurrentMenuDepth-1]].Name;

	string Value = GetGlobalValueString (Param, pUIMenu->m_pMiniDexed->GetParameter (Param));

	pUIMenu->m_pUI->DisplayWrite (pMenuName,
				      pUIMenu->m_pParentMenu[pUIMenu->m_nCurrentMenuItem].Name,
				      Value.c_str (),
				      nValue > rParam.Minimum, nValue < rParam.Maximum);
}

void CUIMenu::SelectVoiceBank (CUIMenu *pUIMenu, TMenuEvent Event)
{
	unsigned nTG = pUIMenu->m_nMenuStackParameter[pUIMenu->m_nCurrentMenuDepth-1];
    int nVoice = pUIMenu->m_pMiniDexed->GetTGParameter (CMiniDexed::TGParameterVoice, nTG);
	int nVB = pUIMenu->m_pMiniDexed->GetTGParameter (CMiniDexed::TGParameterVoiceBank, nTG);

	switch (Event)
	{
		case MenuEventUpdate:
			break;

		case MenuEventStepDown:
			nVB = pUIMenu->m_pMiniDexed->GetSysExFileLoader ()->GetNextBankDown(nVB);
			pUIMenu->m_pMiniDexed->SetTGParameter (CMiniDexed::TGParameterVoiceBank, nVB, nTG);
			// update voice to new bank
			pUIMenu->m_pMiniDexed->SetTGParameter (CMiniDexed::TGParameterVoice, nVoice, nTG);
			break;

		case MenuEventStepUp:
			nVB = pUIMenu->m_pMiniDexed->GetSysExFileLoader ()->GetNextBankUp(nVB);
			pUIMenu->m_pMiniDexed->SetTGParameter (CMiniDexed::TGParameterVoiceBank, nVB, nTG);
			// update voice to new bank
			pUIMenu->m_pMiniDexed->SetTGParameter (CMiniDexed::TGParameterVoice, nVoice, nTG);
			break;

		case MenuEventPressAndStepDown:
		case MenuEventPressAndStepUp:
			pUIMenu->TGShortcutHandler (Event);
			return;

		default:
			return;
	}

	string TG ("TG");
	TG += to_string (nTG+1);

	string Value = to_string (nVB+1) + "="
		       + pUIMenu->m_pMiniDexed->GetSysExFileLoader ()->GetBankName (nVB);

	pUIMenu->m_pUI->DisplayWrite (TG.c_str (),
				      pUIMenu->m_pParentMenu[pUIMenu->m_nCurrentMenuItem].Name,
				      Value.c_str (),
				      nVB > 0, nVB < (int) CSysExFileLoader::MaxVoiceBankID);
}

void CUIMenu::SelectVoice (CUIMenu *pUIMenu, TMenuEvent Event)
{
	unsigned nTG = pUIMenu->m_nMenuStackParameter[pUIMenu->m_nCurrentMenuDepth-1];

	int nVoice = pUIMenu->m_pMiniDexed->GetTGParameter (CMiniDexed::TGParameterVoice, nTG);
	int nVB = pUIMenu->m_pMiniDexed->GetTGParameter(CMiniDexed::TGParameterVoiceBank, nTG);

	switch (Event)
	{
		case MenuEventUpdate:
			break;

		case MenuEventStepDown:
			if (--nVoice < 0)
			{
				// Switch down a voice bank and set to the last voice 
				nVoice = CSysExFileLoader::VoicesPerBank-1;
				nVB = pUIMenu->m_pMiniDexed->GetSysExFileLoader ()->GetNextBankDown(nVB);
				pUIMenu->m_pMiniDexed->SetTGParameter (CMiniDexed::TGParameterVoiceBank, nVB, nTG);
			}
			pUIMenu->m_pMiniDexed->SetTGParameter (CMiniDexed::TGParameterVoice, nVoice, nTG);
			break;

		case MenuEventStepUp:
			if (++nVoice > (int) CSysExFileLoader::VoicesPerBank-1)
			{
				// Switch up a voice bank and reset to  0
				nVoice = 0;
				nVB = pUIMenu->m_pMiniDexed->GetSysExFileLoader ()->GetNextBankUp(nVB);
				pUIMenu->m_pMiniDexed->SetTGParameter (CMiniDexed::TGParameterVoiceBank, nVB, nTG);
			}
			pUIMenu->m_pMiniDexed->SetTGParameter (CMiniDexed::TGParameterVoice, nVoice, nTG);
			break;

		case MenuEventPressAndStepDown:
		case MenuEventPressAndStepUp:
			pUIMenu->TGShortcutHandler (Event);
			return;

		default:
			return;
	}

	// Skip empty voices.
	// Use same criteria in PgmUpDownHandler() too.
	string voiceName = pUIMenu->m_pMiniDexed->GetVoiceName (nTG);

	if (voiceName == "EMPTY     "
	    || voiceName == "          "
	    || voiceName == "----------"
	    || voiceName == "~~~~~~~~~~" 
		|| voiceName == "")
	{
		if (Event == MenuEventStepUp) {
			CUIMenu::SelectVoice (pUIMenu, MenuEventStepUp);
		}
		else if (Event == MenuEventStepDown) {
			CUIMenu::SelectVoice (pUIMenu, MenuEventStepDown);
		}
	} else {
		string TG ("TG");
		TG += to_string (nTG+1);

		string Value = to_string (nVoice+1) + "=" + voiceName;
		string mnuName = pUIMenu->m_pParentMenu[pUIMenu->m_nCurrentMenuItem].Name;
		pUIMenu->m_pUI->DisplayWrite (TG.c_str (),
					      (mnuName + "  Bnk=" + to_string(nVB+1)).c_str(),
					      Value.c_str (),
					      nVoice > 0, nVoice < (int) CSysExFileLoader::VoicesPerBank-1);
	}
}

void CUIMenu::EditTGParameter (CUIMenu *pUIMenu, TMenuEvent Event) // second menu level. Redundant code but in order to not modified original code
{

	unsigned nTG = pUIMenu->m_nMenuStackParameter[pUIMenu->m_nCurrentMenuDepth-1]; 

	CMiniDexed::TTGParameter Param = (CMiniDexed::TTGParameter) pUIMenu->m_nCurrentParameter;
	const TParameter &rParam = s_TGParameter[Param];

	int nValue = pUIMenu->m_pMiniDexed->GetTGParameter (Param, nTG);

	switch (Event)
	{
		case MenuEventUpdate:
			break;

		case MenuEventStepDown:
			nValue -= rParam.Increment;
			if (nValue < rParam.Minimum)
			{
				nValue = rParam.Minimum;
			}
			pUIMenu->m_pMiniDexed->SetTGParameter (Param, nValue, nTG);
			break;

		case MenuEventStepUp:
			nValue += rParam.Increment;
			if (nValue > rParam.Maximum)
			{
				nValue = rParam.Maximum;
			}
			pUIMenu->m_pMiniDexed->SetTGParameter (Param, nValue, nTG);
			break;

		case MenuEventPressAndStepDown:
		case MenuEventPressAndStepUp:
			pUIMenu->TGShortcutHandler (Event);
			return;

		default:
			return;
	}

	string TG ("TG");
	TG += to_string (nTG+1);

	string Value = GetTGValueString (Param, pUIMenu->m_pMiniDexed->GetTGParameter (Param, nTG));

	pUIMenu->m_pUI->DisplayWrite (TG.c_str (),
				      pUIMenu->m_pParentMenu[pUIMenu->m_nCurrentMenuItem].Name,
				      Value.c_str (),
				      nValue > rParam.Minimum, nValue < rParam.Maximum);
				   
}


void CUIMenu::EditTGParameter2 (CUIMenu *pUIMenu, TMenuEvent Event) // second menu level. Redundant code but in order to not modified original code
{

	unsigned nTG = pUIMenu->m_nMenuStackParameter[pUIMenu->m_nCurrentMenuDepth-2]; 

	CMiniDexed::TTGParameter Param = (CMiniDexed::TTGParameter) pUIMenu->m_nCurrentParameter;
	const TParameter &rParam = s_TGParameter[Param];

	int nValue = pUIMenu->m_pMiniDexed->GetTGParameter (Param, nTG);

	switch (Event)
	{
		case MenuEventUpdate:
			break;

		case MenuEventStepDown:
			nValue -= rParam.Increment;
			if (nValue < rParam.Minimum)
			{
				nValue = rParam.Minimum;
			}
			pUIMenu->m_pMiniDexed->SetTGParameter (Param, nValue, nTG);
			break;

		case MenuEventStepUp:
			nValue += rParam.Increment;
			if (nValue > rParam.Maximum)
			{
				nValue = rParam.Maximum;
			}
			pUIMenu->m_pMiniDexed->SetTGParameter (Param, nValue, nTG);
			break;

		case MenuEventPressAndStepDown:
		case MenuEventPressAndStepUp:
			pUIMenu->TGShortcutHandler (Event);
			return;

		default:
			return;
	}

	string TG ("TG");
	TG += to_string (nTG+1);

	string Value = GetTGValueString (Param, pUIMenu->m_pMiniDexed->GetTGParameter (Param, nTG));

	pUIMenu->m_pUI->DisplayWrite (TG.c_str (),
				      pUIMenu->m_pParentMenu[pUIMenu->m_nCurrentMenuItem].Name,
				      Value.c_str (),
				      nValue > rParam.Minimum, nValue < rParam.Maximum);
				   
}

void CUIMenu::EditVoiceParameter (CUIMenu *pUIMenu, TMenuEvent Event)
{
	unsigned nTG = pUIMenu->m_nMenuStackParameter[pUIMenu->m_nCurrentMenuDepth-2];

	unsigned nParam = pUIMenu->m_nCurrentParameter;
	const TParameter &rParam = s_VoiceParameter[nParam];

	int nValue = pUIMenu->m_pMiniDexed->GetVoiceParameter (nParam, CMiniDexed::NoOP, nTG);

	switch (Event)
	{
		case MenuEventUpdate:
			break;

		case MenuEventStepDown:
			nValue -= rParam.Increment;
			if (nValue < rParam.Minimum)
			{
				nValue = rParam.Minimum;
			}
			pUIMenu->m_pMiniDexed->SetVoiceParameter (nParam, nValue, CMiniDexed::NoOP, nTG);
			break;

		case MenuEventStepUp:
			nValue += rParam.Increment;
			if (nValue > rParam.Maximum)
			{
				nValue = rParam.Maximum;
			}
			pUIMenu->m_pMiniDexed->SetVoiceParameter (nParam, nValue, CMiniDexed::NoOP, nTG);
			break;

		case MenuEventPressAndStepDown:
		case MenuEventPressAndStepUp:
			pUIMenu->TGShortcutHandler (Event);
			return;

		default:
			return;
	}

	string TG ("TG");
	TG += to_string (nTG+1);

	string Value = GetVoiceValueString (nParam, nValue);

	pUIMenu->m_pUI->DisplayWrite (TG.c_str (),
				      pUIMenu->m_pParentMenu[pUIMenu->m_nCurrentMenuItem].Name,
				      Value.c_str (),
				      nValue > rParam.Minimum, nValue < rParam.Maximum);
}

void CUIMenu::EditOPParameter (CUIMenu *pUIMenu, TMenuEvent Event)
{
	unsigned nTG = pUIMenu->m_nMenuStackParameter[pUIMenu->m_nCurrentMenuDepth-3];
	unsigned nOP = pUIMenu->m_nMenuStackParameter[pUIMenu->m_nCurrentMenuDepth-1];

	unsigned nParam = pUIMenu->m_nCurrentParameter;
	const TParameter &rParam = s_OPParameter[nParam];

	int nValue = pUIMenu->m_pMiniDexed->GetVoiceParameter (nParam, nOP, nTG);

	switch (Event)
	{
		case MenuEventUpdate:
			break;

		case MenuEventStepDown:
			nValue -= rParam.Increment;
			if (nValue < rParam.Minimum)
			{
				nValue = rParam.Minimum;
			}
			pUIMenu->m_pMiniDexed->SetVoiceParameter (nParam, nValue, nOP, nTG);
			break;

		case MenuEventStepUp:
			nValue += rParam.Increment;
			if (nValue > rParam.Maximum)
			{
				nValue = rParam.Maximum;
			}
			pUIMenu->m_pMiniDexed->SetVoiceParameter (nParam, nValue, nOP, nTG);
			break;

		case MenuEventPressAndStepDown:
		case MenuEventPressAndStepUp:
			pUIMenu->OPShortcutHandler (Event);
			return;

		default:
			return;
	}

	string OP ("OP");
	OP += to_string (nOP+1);

	string Value;

	static const int FixedMultiplier[4] = {1, 10, 100, 1000};
	if (nParam == DEXED_OP_FREQ_COARSE)
	{
		if (!pUIMenu->m_pMiniDexed->GetVoiceParameter (DEXED_OP_OSC_MODE, nOP, nTG))
		{
			// Ratio
			if (!nValue)
			{
				Value = "0.50";
			}
			else
			{
				Value = to_string (nValue);
				Value += ".00";
			}
		}
		else
		{
			// Fixed
			Value = to_string (FixedMultiplier[nValue % 4]);
		}
	}
	else if (nParam == DEXED_OP_FREQ_FINE)
	{
		int nCoarse = pUIMenu->m_pMiniDexed->GetVoiceParameter (
							DEXED_OP_FREQ_COARSE, nOP, nTG);

		char Buffer[20];
		if (!pUIMenu->m_pMiniDexed->GetVoiceParameter (DEXED_OP_OSC_MODE, nOP, nTG))
		{
			// Ratio
			float fValue = 1.0f + nValue / 100.0f;
			fValue *= !nCoarse ? 0.5f : (float) nCoarse;
			sprintf (Buffer, "%.2f", (double) fValue);
		}
		else
		{
			// Fixed
			float fValue = powf (1.023293f, (float) nValue);
			fValue *= (float) FixedMultiplier[nCoarse % 4];
			sprintf (Buffer, "%.3fHz", (double) fValue);
		}

		Value = Buffer;
	}
	else
	{
		Value = GetOPValueString (nParam, nValue);
	}

	pUIMenu->m_pUI->DisplayWrite (OP.c_str (),
				      pUIMenu->m_pParentMenu[pUIMenu->m_nCurrentMenuItem].Name,
				      Value.c_str (),
				      nValue > rParam.Minimum, nValue < rParam.Maximum);
}

void CUIMenu::SavePerformance (CUIMenu *pUIMenu, TMenuEvent Event)
{
	if (Event != MenuEventUpdate)
	{
		return;
	}

	bool bOK = pUIMenu->m_pMiniDexed->SavePerformance (pUIMenu->m_nCurrentParameter == 1);

	const char *pMenuName =
		pUIMenu->m_MenuStackParent[pUIMenu->m_nCurrentMenuDepth-1]
			[pUIMenu->m_nMenuStackItem[pUIMenu->m_nCurrentMenuDepth-1]].Name;

	pUIMenu->m_pUI->DisplayWrite (pMenuName,
				      pUIMenu->m_pParentMenu[pUIMenu->m_nCurrentMenuItem].Name,
				      bOK ? "Completed" : "Error",
				      false, false);

	CTimer::Get ()->StartKernelTimer (MSEC2HZ (1500), TimerHandler, 0, pUIMenu);
}

string CUIMenu::GetGlobalValueString (unsigned nParameter, int nValue)
{
	string Result;

	assert (nParameter < sizeof CUIMenu::s_GlobalParameter / sizeof CUIMenu::s_GlobalParameter[0]);

	CUIMenu::TToString *pToString = CUIMenu::s_GlobalParameter[nParameter].ToString;
	if (pToString)
	{
		Result = (*pToString) (nValue);
	}
	else
	{
		Result = to_string (nValue);
	}

	return Result;
}

string CUIMenu::GetTGValueString (unsigned nTGParameter, int nValue)
{
	string Result;

	assert (nTGParameter < sizeof CUIMenu::s_TGParameter / sizeof CUIMenu::s_TGParameter[0]);

	CUIMenu::TToString *pToString = CUIMenu::s_TGParameter[nTGParameter].ToString;
	if (pToString)
	{
		Result = (*pToString) (nValue);
	}
	else
	{
		Result = to_string (nValue);
	}

	return Result;
}

string CUIMenu::GetVoiceValueString (unsigned nVoiceParameter, int nValue)
{
	string Result;

	assert (nVoiceParameter < sizeof CUIMenu::s_VoiceParameter / sizeof CUIMenu::s_VoiceParameter[0]);

	CUIMenu::TToString *pToString = CUIMenu::s_VoiceParameter[nVoiceParameter].ToString;
	if (pToString)
	{
		Result = (*pToString) (nValue);
	}
	else
	{
		Result = to_string (nValue);
	}

	return Result;
}

string CUIMenu::GetOPValueString (unsigned nOPParameter, int nValue)
{
	string Result;

	assert (nOPParameter < sizeof CUIMenu::s_OPParameter / sizeof CUIMenu::s_OPParameter[0]);

	CUIMenu::TToString *pToString = CUIMenu::s_OPParameter[nOPParameter].ToString;
	if (pToString)
	{
		Result = (*pToString) (nValue);
	}
	else
	{
		Result = to_string (nValue);
	} 

	return Result;
}

string CUIMenu::ToVolume (int nValue)
{
	unsigned nIndex = nValue * (CConfig::LCDColumns-2) / 127;
	string VolumeBar(nIndex, '\xFF');
	return VolumeBar;
}

string CUIMenu::ToPan (int nValue)
{
	assert(CConfig::LCDColumns > 7);
	static unsigned MaxChars = CConfig::LCDColumns-3;
	unsigned nIndex = nValue * MaxChars / 127;	
	if (nIndex == MaxChars)
	{
		nIndex--;
	}
	string PanMarker((CConfig::LCDColumns-4)/2, '.');
	PanMarker += ':';
	PanMarker += string((CConfig::LCDColumns-4)/2, '.');
	PanMarker[nIndex] = '\xFF';			// 0xFF is the block character
	return PanMarker;
}

string CUIMenu::ToMIDIChannel (int nValue)
{
	switch (nValue)
	{
		case CMIDIDevice::OmniMode:	return "Omni";
		case CMIDIDevice::Disabled:	return "Off";
		default: return to_string (nValue+1);
	}
}

string CUIMenu::ToAlgorithm (int nValue)
{
	return to_string (nValue + 1);
}

string CUIMenu::ToOnOff (int nValue)
{
	static const char *OnOff[] = {"Off", "On"};

	assert ((unsigned) nValue < sizeof OnOff / sizeof OnOff[0]);

	return OnOff[nValue];
}

string CUIMenu::ToLFOWaveform (int nValue)
{
	static const char *Waveform[] = {"Triangle", "Saw down", "Saw up",
					 "Square", "Sine", "Sample/Hold"};

	assert ((unsigned) nValue < sizeof Waveform / sizeof Waveform[0]);

	return Waveform[nValue];
}

string CUIMenu::ToTransposeNote (int nValue)
{
	nValue += NoteC3 - 24;

	assert ((unsigned) nValue < sizeof s_NoteName / sizeof s_NoteName[0]);

	return s_NoteName[nValue];
}

string CUIMenu::ToBreakpointNote (int nValue)
{
	assert ((unsigned) nValue < sizeof s_NoteName / sizeof s_NoteName[0]);

	return s_NoteName[nValue];
}

string CUIMenu::ToKeyboardCurve (int nValue)
{
	static const char *Curve[] = {"-Lin", "-Exp", "+Exp", "+Lin"};

	assert ((unsigned) nValue < sizeof Curve / sizeof Curve[0]);

	return Curve[nValue];
}

string CUIMenu::ToOscillatorMode (int nValue)
{
	static const char *Mode[] = {"Ratio", "Fixed"};

	assert ((unsigned) nValue < sizeof Mode / sizeof Mode[0]);

	return Mode[nValue];
}

string CUIMenu::ToOscillatorDetune (int nValue)
{
	string Result;

	nValue -= 7;

	if (nValue > 0)
	{
		Result = "+" + to_string (nValue);
	}
	else
	{
		Result = to_string (nValue);
	}

	return Result;
}

string CUIMenu::ToPortaMode (int nValue)
{
	switch (nValue)
	{
	case 0:		return "Fingered";
	case 1:		return "Full time";
	default:	return to_string (nValue);
	}
};

string CUIMenu::ToPortaGlissando (int nValue)
{
	switch (nValue)
	{
	case 0:		return "Off";
	case 1:		return "On";
	default:	return to_string (nValue);
	}
};

string CUIMenu::ToPolyMono (int nValue)
{
	switch (nValue)
	{
	case 0:		return "Poly";
	case 1:		return "Mono";
	default:	return to_string (nValue);
	}
}

void CUIMenu::TGShortcutHandler (TMenuEvent Event)
{
	assert (m_nCurrentMenuDepth >= 2);
	assert (m_MenuStackMenu[0] = s_MainMenu);
	unsigned nTG = m_nMenuStackSelection[0];
	assert (nTG < CConfig::ToneGenerators);
	assert (m_nMenuStackItem[1] == nTG);
	assert (m_nMenuStackParameter[1] == nTG);

	assert (   Event == MenuEventPressAndStepDown
		 	|| Event == MenuEventPressAndStepUp);
	if (Event == MenuEventPressAndStepDown)
	{
		nTG--;
	}
	else
	{
		nTG++;
	}

	if (nTG < CConfig::ToneGenerators)
	{
		m_nMenuStackSelection[0] = nTG;
		m_nMenuStackItem[1] = nTG;
		m_nMenuStackParameter[1] = nTG;

		EventHandler (MenuEventUpdate);
	}
}

void CUIMenu::OPShortcutHandler (TMenuEvent Event)
{
	assert (m_nCurrentMenuDepth >= 3);
	assert (m_MenuStackMenu[m_nCurrentMenuDepth-2] = s_EditVoiceMenu);
	unsigned nOP = m_nMenuStackSelection[m_nCurrentMenuDepth-2];
	assert (nOP < 6);
	assert (m_nMenuStackItem[m_nCurrentMenuDepth-1] == nOP);
	assert (m_nMenuStackParameter[m_nCurrentMenuDepth-1] == nOP);

	assert (   Event == MenuEventPressAndStepDown
			|| Event == MenuEventPressAndStepUp);
	if (Event == MenuEventPressAndStepDown)
	{
		nOP--;
	}
	else
	{
		nOP++;
	}

	if (nOP < 6)
	{
		m_nMenuStackSelection[m_nCurrentMenuDepth-2] = nOP;
		m_nMenuStackItem[m_nCurrentMenuDepth-1] = nOP;
		m_nMenuStackParameter[m_nCurrentMenuDepth-1] = nOP;

		EventHandler (MenuEventUpdate);
	}
}

void CUIMenu::PgmUpDownHandler (TMenuEvent Event)
{
	if (m_pMiniDexed->GetParameter (CMiniDexed::ParameterPerformanceSelectChannel) != CMIDIDevice::Disabled)
	{
		// Program Up/Down acts on performances
		unsigned nLastPerformance = m_pMiniDexed->GetLastPerformanceID();
		unsigned nPerformance = m_pMiniDexed->GetPerformanceID();
		unsigned nStart = nPerformance;
		//LOGNOTE("Performance actual=%d, last=%d", nPerformance, nLastPerformance);
		if (Event == MenuEventPgmDown)
		{
			do
			{
				if (nPerformance == 0)
				{
					// Wrap around
					nPerformance = nLastPerformance;
				}
				else if (nPerformance > 0)
				{
					--nPerformance;
				}
			} while ((m_pMiniDexed->IsValidPerformance(nPerformance) != true) && (nPerformance != nStart));
		}
		else // MenuEventPgmUp
		{
			do
			{
				if (nPerformance == nLastPerformance)
				{
					// Wrap around
					nPerformance = 0;
				}
				else if (nPerformance < nLastPerformance)
				{
					++nPerformance;
				}
			} while ((m_pMiniDexed->IsValidPerformance(nPerformance) != true) && (nPerformance != nStart));
		}
		m_nSelectedPerformanceID = nPerformance;
		m_pMiniDexed->LoadPerformance(m_nSelectedPerformanceID);
		//LOGNOTE("Performance new=%d, last=%d", m_nSelectedPerformanceID, nLastPerformance);
	}
	else
	{
		// Program Up/Down acts on voices within a TG.
	
		// If we're not in the root menu, then see if we are already in a TG menu,
		// then find the current TG number. Otherwise assume TG1 (nTG=0).
		unsigned nTG = 0;
		if (m_MenuStackMenu[0] == s_MainMenu && (m_pCurrentMenu == s_TGMenu) || (m_MenuStackMenu[1] == s_TGMenu)) 
		{
			nTG = m_nMenuStackSelection[0];
		}
		assert (nTG < CConfig::ToneGenerators);

		int nPgm = m_pMiniDexed->GetTGParameter (CMiniDexed::TGParameterVoice, nTG);

		assert (Event == MenuEventPgmDown || Event == MenuEventPgmUp);
		if (Event == MenuEventPgmDown)
		{
			//LOGNOTE("PgmDown");
			if (--nPgm < 0)
			{
				// Switch down a voice bank and set to the last voice
				nPgm = CSysExFileLoader::VoicesPerBank-1;
				int nVB = m_pMiniDexed->GetTGParameter(CMiniDexed::TGParameterVoiceBank, nTG);
				nVB = m_pMiniDexed->GetSysExFileLoader ()->GetNextBankDown(nVB);
				m_pMiniDexed->SetTGParameter (CMiniDexed::TGParameterVoiceBank, nVB, nTG);
			}
		}
		else
		{
			//LOGNOTE("PgmUp");
			if (++nPgm > (int) CSysExFileLoader::VoicesPerBank-1)
			{
				// Switch up a voice bank and reset to voice 0
				nPgm = 0;
				int nVB = m_pMiniDexed->GetTGParameter(CMiniDexed::TGParameterVoiceBank, nTG);
				nVB = m_pMiniDexed->GetSysExFileLoader ()->GetNextBankUp(nVB);
				m_pMiniDexed->SetTGParameter (CMiniDexed::TGParameterVoiceBank, nVB, nTG);
			}
		}
		m_pMiniDexed->SetTGParameter (CMiniDexed::TGParameterVoice, nPgm, nTG);

		// Skip empty voices.
		// Use same criteria in SelectVoiceNumber () too.
		string voiceName = m_pMiniDexed->GetVoiceName (nTG);
		if (voiceName == "EMPTY     "
			|| voiceName == "          "
			|| voiceName == "----------"
			|| voiceName == "~~~~~~~~~~" 
			|| voiceName == "")
		{
			if (Event == MenuEventPgmUp) {
				PgmUpDownHandler (MenuEventPgmUp);
			}
			if (Event == MenuEventPgmDown) {
				PgmUpDownHandler (MenuEventPgmDown);
			}
		}
	}
}

void CUIMenu::TGUpDownHandler (TMenuEvent Event)
{
	// This will update the menus to position it for the next TG up or down
	unsigned nTG = 0;
	
	if (CConfig::ToneGenerators <= 1) 
	{
		// Nothing to do if only a single TG
		return;
	}

	// If we're not in the root menu, then see if we are already in a TG menu,
	// then find the current TG number. Otherwise assume TG1 (nTG=0).
	if (m_MenuStackMenu[0] == s_MainMenu && (m_pCurrentMenu == s_TGMenu) || (m_MenuStackMenu[1] == s_TGMenu)) 
	{
		nTG = m_nMenuStackSelection[0] - 2;
	}

	assert (nTG < CConfig::ToneGenerators);
	assert (Event == MenuEventTGDown || Event == MenuEventTGUp);
	if (Event == MenuEventTGDown)
	{
		//LOGNOTE("TGDown");
		if (nTG > 0) 
		{
			nTG--;
		}
	}
	else
	{
		//LOGNOTE("TGUp");
		if (nTG < CConfig::ToneGenerators - 1) 
		{
			nTG++;
		}
	}

	// Set menu to the appropriate TG menu as follows:
	//  Top = Root
	//  Menu [0] = Main
	//  Menu [1] = TG Menu
	m_pParentMenu = s_MainMenu;
	m_pCurrentMenu = s_TGMenu;
	m_nCurrentMenuItem = nTG + 2;
	m_nCurrentSelection = 0;
	m_nCurrentParameter = nTG;
	m_nCurrentMenuDepth = 1;

	// Place the main menu on the stack with Root as the parent
	m_MenuStackParent[0] = s_MenuRoot;
	m_MenuStackMenu[0] = s_MainMenu;
	m_nMenuStackItem[0] = 0;
	m_nMenuStackSelection[0] = nTG + 2;
	m_nMenuStackParameter[0] = 0;

	EventHandler (MenuEventUpdate);
}

void CUIMenu::TimerHandler (TKernelTimerHandle hTimer, void *pParam, void *pContext)
{
	CUIMenu *pThis = static_cast<CUIMenu *> (pContext);
	assert (pThis);

	pThis->EventHandler (MenuEventBack);
}

void CUIMenu::TimerHandlerNoBack (TKernelTimerHandle hTimer, void *pParam, void *pContext)
{
	CUIMenu *pThis = static_cast<CUIMenu *> (pContext);
	assert (pThis);
	
	pThis->m_bSplashShow = false;
	
	pThis->EventHandler (MenuEventUpdate);
}

void CUIMenu::TimerHandlerCancelTimed (TKernelTimerHandle hTimer, void *pParam, void *pContext)
{
	CUIMenu *pThis = static_cast<CUIMenu *> (pContext);
	assert (pThis);
	pThis->m_hMainMenuTimer = 0;
	pThis->EventHandler (MenuEventCancelTimedFeature);
}

void CUIMenu::SelectPerformance (CUIMenu *pUIMenu, TMenuEvent Event)
{
	bool bPerformanceSelectToLoad = pUIMenu->m_pMiniDexed->GetPerformanceSelectToLoad();
	unsigned nLastPerformance = pUIMenu->m_pMiniDexed->GetLastPerformanceID();
	unsigned nValue = pUIMenu->m_nSelectedPerformanceID;
	if (nValue == 9999 || !pUIMenu->m_pMiniDexed->IsValidPerformance(nValue))
	{
		nValue = pUIMenu->m_nSelectedPerformanceID = pUIMenu->m_pMiniDexed->GetPerformanceID();
	}
	unsigned nStart = nValue;
	std::string Value;
		
	if (Event == MenuEventUpdate)
	{
		pUIMenu->m_bPerformanceDeleteMode=false;
	}
	
	if (pUIMenu->m_bSplashShow)
	{
		return;
	}		
	
	// select performance mode
	if(!pUIMenu->m_bPerformanceDeleteMode)
	{
		switch (Event)
		{
		case MenuEventUpdate:
			break;

		case MenuEventStepDown:
			do
			{
				if (nValue == 0)
				{
					// Wrap around
					nValue = nLastPerformance;
				}
				else if (nValue > 0)
				{
					--nValue;
				}
			} while ((pUIMenu->m_pMiniDexed->IsValidPerformance(nValue) != true) && (nValue != nStart));
			pUIMenu->m_nSelectedPerformanceID = nValue;
			if (!bPerformanceSelectToLoad && pUIMenu->m_nCurrentParameter==0)
			{
				pUIMenu->m_pMiniDexed->LoadPerformance(nValue);
			}
			break;

		case MenuEventStepUp:
			do
			{
				if (nValue == nLastPerformance)
				{
					// Wrap around
					nValue = 0;
				}
				else if (nValue < nLastPerformance)
				{
					++nValue;
				}
			} while ((pUIMenu->m_pMiniDexed->IsValidPerformance(nValue) != true) && (nValue != nStart));
			pUIMenu->m_nSelectedPerformanceID = nValue;
			if (!bPerformanceSelectToLoad && pUIMenu->m_nCurrentParameter==0)
			{
				pUIMenu->m_pMiniDexed->LoadPerformance(nValue);
			}
			break;

		case MenuEventSelect:	
			switch (pUIMenu->m_nCurrentParameter)
			{
				case 0:	//load
					if (bPerformanceSelectToLoad)
					{
						pUIMenu->m_pMiniDexed->LoadPerformance(nValue);
					}
					break;
				case 1:	//delete ???
					if (pUIMenu->m_pMiniDexed->IsValidPerformance(pUIMenu->m_nSelectedPerformanceID))
					{
						pUIMenu->m_bPerformanceDeleteMode=true;
						pUIMenu->m_bConfirmDeletePerformance=false;
					}
					break;
				default:
					break;
			}
			break;
		default:
			return;
		}
	}
	else	// ask confirm delete
	{
		switch (Event)
		{
			case MenuEventUpdate:
				break;

			case MenuEventStepDown:
				pUIMenu->m_bConfirmDeletePerformance=false;
				break;

			case MenuEventStepUp:
				pUIMenu->m_bConfirmDeletePerformance=true;
				break;

			case MenuEventSelect:	
				pUIMenu->m_bPerformanceDeleteMode=false;
				if (pUIMenu->m_bConfirmDeletePerformance)
				{
					pUIMenu->m_nSelectedPerformanceID = 0;
					pUIMenu->m_bConfirmDeletePerformance=false;
					pUIMenu->m_pUI->DisplayWrite ("", "Delete", pUIMenu->m_pMiniDexed->DeletePerformance(nValue) ? "Completed" : "Error", false, false);
					pUIMenu->m_bSplashShow=true;
					CTimer::Get ()->StartKernelTimer (MSEC2HZ (1500), TimerHandlerNoBack, 0, pUIMenu);
					return;
				}
				else
				{
					break;
				}
				
			default:
				return;
		}		
	}
		
	if(!pUIMenu->m_bPerformanceDeleteMode)
	{
		Value = pUIMenu->m_pMiniDexed->GetPerformanceName(nValue);
		unsigned nBankNum = pUIMenu->m_pMiniDexed->GetPerformanceBankID();
		
		std::string nPSelected = "00";
		nPSelected += std::to_string(nBankNum+1);  // Convert to user-facing bank number rather than index
		nPSelected = nPSelected.substr(nPSelected.size()-3);

		std::string nPPerf = "00";
		nPPerf += std::to_string(nValue+1);  // Convert to user-facing performance number rather than index
		nPPerf = nPPerf.substr(nPPerf.size()-3);

		nPSelected += ":"+nPPerf;
		if(nValue == pUIMenu->m_pMiniDexed->GetPerformanceID())
		{
			nPSelected += " [L]";
		}
					
		pUIMenu->m_pUI->DisplayWrite (pUIMenu->m_pParentMenu[pUIMenu->m_nCurrentMenuItem].Name, nPSelected.c_str(),
						  Value.c_str (), true, true);
//						 (int) nValue > 0, (int) nValue < (int) pUIMenu->m_pMiniDexed->GetLastPerformance());
	}
	else
	{
		pUIMenu->m_pUI->DisplayWrite ("", "Delete?", pUIMenu->m_bConfirmDeletePerformance ? "Yes" : "No", false, false);
	}
}

void CUIMenu::SelectPerformanceBank (CUIMenu *pUIMenu, TMenuEvent Event)
{
	bool bPerformanceSelectToLoad = pUIMenu->m_pMiniDexed->GetPerformanceSelectToLoad();
	unsigned nLastPerformanceBank = pUIMenu->m_pMiniDexed->GetLastPerformanceBankID();
	unsigned nValue = pUIMenu->m_nSelectedPerformanceBankID;
	if (nValue == 9999)
	{
		nValue = pUIMenu->m_nSelectedPerformanceBankID = pUIMenu->m_pMiniDexed->GetPerformanceBankID();
	}
	unsigned nStart = nValue;
	std::string Value;

	switch (Event)
	{
		case MenuEventUpdate:
			break;

		case MenuEventStepDown:
			do
			{
				if (nValue == 0)
				{
					// Wrap around
					nValue = nLastPerformanceBank;
				}
				else if (nValue > 0)
				{
					--nValue;
				}
			} while ((pUIMenu->m_pMiniDexed->IsValidPerformanceBank(nValue) != true) && (nValue != nStart));
			pUIMenu->m_nSelectedPerformanceBankID = nValue;
			if (!bPerformanceSelectToLoad)
			{
				// Switch to the new bank and select the first performance voice
				pUIMenu->m_pMiniDexed->LoadPerformanceBank(nValue);
				pUIMenu->m_pMiniDexed->LoadFirstPerformance();
			}
			break;

		case MenuEventStepUp:
			do
			{
				if (nValue == nLastPerformanceBank)
				{
					// Wrap around
					nValue = 0;
				}
				else if (nValue < nLastPerformanceBank)
				{
					++nValue;
				}
			} while ((pUIMenu->m_pMiniDexed->IsValidPerformanceBank(nValue) != true) && (nValue != nStart));
			pUIMenu->m_nSelectedPerformanceBankID = nValue;
			if (!bPerformanceSelectToLoad)
			{
				pUIMenu->m_pMiniDexed->LoadPerformanceBank(nValue);
				pUIMenu->m_pMiniDexed->LoadFirstPerformance();
			}
			break;

		case MenuEventSelect:	
			if (bPerformanceSelectToLoad)
			{
				pUIMenu->m_pMiniDexed->LoadPerformanceBank(nValue);
				pUIMenu->m_pMiniDexed->LoadFirstPerformance();
			}
			break;

		default:
			return;
	}

	Value = pUIMenu->m_pMiniDexed->GetPerformanceConfig ()->GetPerformanceBankName(nValue);
	std::string nPSelected = "00";
	nPSelected += std::to_string(nValue+1);  // Convert to user-facing number rather than index
	nPSelected = nPSelected.substr(nPSelected.size()-3);

	if(nValue == pUIMenu->m_pMiniDexed->GetPerformanceBankID())
	{
		nPSelected += " [L]";
	}

	pUIMenu->m_pUI->DisplayWrite (pUIMenu->m_pParentMenu[pUIMenu->m_nCurrentMenuItem].Name, nPSelected.c_str(),
							Value.c_str (),
							nValue > 0,
							nValue < pUIMenu->m_pMiniDexed->GetLastPerformanceBankID() - 1);
}

void CUIMenu::InputTxt (CUIMenu *pUIMenu, TMenuEvent Event)
{
	unsigned nTG=0;
	string TG ("TG");
	
	std::string MsgOk;
	std::string NoValidChars;
	unsigned MaxChars;
	std::string MenuTitleR;
	std::string MenuTitleL;
	std::string OkTitleL;
	std::string OkTitleR;
	
	switch(pUIMenu->m_nCurrentParameter)
	{
		case 1: // save new performance
			NoValidChars = {92, 47, 58, 42, 63, 34, 60,62, 124};
			MaxChars=14;
			MenuTitleL="Performance Name";
			MenuTitleR="";
			OkTitleL="New Performance"; // \E[?25l
			OkTitleR="";
		 break;
		 
		case 2: // Rename performance - NOT Implemented yet
			NoValidChars = {92, 47, 58, 42, 63, 34, 60,62, 124};
			MaxChars=14;
			MenuTitleL="Performance Name";
			MenuTitleR="";
			OkTitleL="Rename Perf."; // \E[?25l
			OkTitleR="";
		break;
		
		case 3: // Voice name
			nTG = pUIMenu->m_nMenuStackParameter[pUIMenu->m_nCurrentMenuDepth-2];
			NoValidChars = {127};
			MaxChars=10;
			MenuTitleL="Name";
			TG += to_string (nTG+1);
			MenuTitleR=TG;
			OkTitleL="";
			OkTitleR="";
		break;
			
		default:
		return;
	}
	
	bool bOK;
	unsigned nPosition = pUIMenu->m_InputTextPosition;
	unsigned nChar = pUIMenu->m_InputText[nPosition];

	
	switch (Event)
	{
		case MenuEventUpdate:
			if(pUIMenu->m_nCurrentParameter == 1 || pUIMenu->m_nCurrentParameter == 2)
			{
				pUIMenu->m_InputText = pUIMenu->m_pMiniDexed->GetNewPerformanceDefaultName();
				pUIMenu->m_InputText += "              ";
				pUIMenu->m_InputText =  pUIMenu->m_InputText.substr(0,14);
				pUIMenu->m_InputTextPosition=0;
				nPosition=pUIMenu->m_InputTextPosition;
				nChar = pUIMenu->m_InputText[nPosition];
			}
			else
			{
				
				pUIMenu->m_InputText = pUIMenu->m_pMiniDexed->GetVoiceName(nTG);
				pUIMenu->m_InputText += "          ";
				pUIMenu->m_InputText =  pUIMenu->m_InputText.substr(0,10);
				pUIMenu->m_InputTextPosition=0;
				nPosition=pUIMenu->m_InputTextPosition;
				nChar = pUIMenu->m_InputText[nPosition];
			}
			break;

		case MenuEventStepDown:
			if (nChar > 32)
			{
			do	{
				--nChar;
				}
			while (NoValidChars.find(nChar) != std::string::npos);
			}
			pUIMenu->m_InputTextChar = nChar;
			break;

		case MenuEventStepUp:
			if (nChar < 126)
			{
			do	{
					++nChar;
				}
			while (NoValidChars.find(nChar) != std::string::npos);			
			}
			pUIMenu->m_InputTextChar = nChar;
			break;	
			
			
			
		case MenuEventSelect:	
			if(pUIMenu->m_nCurrentParameter == 1)
			{	
				pUIMenu->m_pMiniDexed->SetNewPerformanceName(pUIMenu->m_InputText);
				bOK = pUIMenu->m_pMiniDexed->SavePerformanceNewFile ();
				MsgOk=bOK ? "Completed" : "Error";
				pUIMenu->m_pUI->DisplayWrite (OkTitleR.c_str(), OkTitleL.c_str(), MsgOk.c_str(), false, false);
				CTimer::Get ()->StartKernelTimer (MSEC2HZ (1500), TimerHandler, 0, pUIMenu);
				return;
			}
			else
			{
				break; // Voice Name Edit
			}
		
		case MenuEventPressAndStepDown:
			if (nPosition > 0)
				{
					--nPosition;
				}
			pUIMenu->m_InputTextPosition = nPosition;
			nChar = pUIMenu->m_InputText[nPosition];
			break;
		
		case MenuEventPressAndStepUp:
			if (nPosition < MaxChars-1)
			{
				++nPosition;
			}
			pUIMenu->m_InputTextPosition = nPosition;
			nChar = pUIMenu->m_InputText[nPosition];
			break;

		default:
			return;
	}
	
	
	// \E[2;%dH	Cursor move to row %1 and column %2 (starting at 1)
	// \E[?25h	Normal cursor visible
	// \E[?25l	Cursor invisible
	
	std::string escCursor="\E[?25h\E[2;"; // this is to locate cursor
	escCursor += to_string(nPosition + 2);
	escCursor += "H";
	

	std::string Value = pUIMenu->m_InputText;
	Value[nPosition]=nChar;
	pUIMenu->m_InputText = Value;
	
	if(pUIMenu->m_nCurrentParameter == 3)
	{
		pUIMenu->m_pMiniDexed->SetVoiceName(pUIMenu->m_InputText, nTG);
	}	
		
	Value = Value + " " + escCursor ;
	pUIMenu->m_pUI->DisplayWrite (MenuTitleR.c_str(),MenuTitleL.c_str(), Value.c_str(), false, false);
	
	
}

void CUIMenu::EditTGParameterModulation (CUIMenu *pUIMenu, TMenuEvent Event) 
{

	unsigned nTG = pUIMenu->m_nMenuStackParameter[pUIMenu->m_nCurrentMenuDepth-3]; 
	unsigned nController = pUIMenu->m_nMenuStackParameter[pUIMenu->m_nCurrentMenuDepth-1]; 
	unsigned nParameter = pUIMenu->m_nCurrentParameter + nController;
	
	CMiniDexed::TTGParameter Param = (CMiniDexed::TTGParameter) nParameter;
	const TParameter &rParam = s_TGParameter[Param];

	int nValue = pUIMenu->m_pMiniDexed->GetTGParameter (Param, nTG);

	switch (Event)
	{
		case MenuEventUpdate:
			break;

		case MenuEventStepDown:
			nValue -= rParam.Increment;
			if (nValue < rParam.Minimum)
			{
				nValue = rParam.Minimum;
			}
			pUIMenu->m_pMiniDexed->SetTGParameter (Param, nValue, nTG);
			break;

		case MenuEventStepUp:
			nValue += rParam.Increment;
			if (nValue > rParam.Maximum)
			{
				nValue = rParam.Maximum;
			}
			pUIMenu->m_pMiniDexed->SetTGParameter (Param, nValue, nTG);
			break;

		case MenuEventPressAndStepDown:
		case MenuEventPressAndStepUp:
			pUIMenu->TGShortcutHandler (Event);
			return;

		default:
			return;
	}

	string TG ("TG");
	TG += to_string (nTG+1);

	string Value = GetTGValueString (Param, pUIMenu->m_pMiniDexed->GetTGParameter (Param, nTG));

	pUIMenu->m_pUI->DisplayWrite (TG.c_str (),
				      pUIMenu->m_pParentMenu[pUIMenu->m_nCurrentMenuItem].Name,
				      Value.c_str (),
				      nValue > rParam.Minimum, nValue < rParam.Maximum);
				   
}


