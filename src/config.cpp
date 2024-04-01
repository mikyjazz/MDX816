//
// config.cpp
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
#include "config.h"
#include "performanceconfig.h"
#include "../Synth_Dexed/src/dexed.h"
#include <cstring> 
#include <algorithm>

CConfig::CConfig (FATFS *pFileSystem)
:	m_Properties ("mdx816.ini", pFileSystem),
    m_SessionSettings ("mdx816session.ini", pFileSystem)
{
}

CConfig::~CConfig (void)
{
}

unsigned CConfig::LCDColumns = 20;
unsigned CConfig::LCDRows = 2;

// load configuration files
bool CConfig::Load (void)
{
	// static properties
	if (!m_Properties.Load ()) 
	{
		return false;
	}		

	// last settings
	if (!m_SessionSettings.Load ())
	{
		return false;
	}
		

	m_bUSBGadgetMode = m_Properties.GetNumber ("USBGadget", 0) != 0;

	m_SoundDevice = m_Properties.GetString ("SoundDevice", "pwm");

	m_nSampleRate = m_Properties.GetNumber ("SampleRate", 48000);
	// The CHUNK_SIZE must be a multiple of 2 or a multiple of 384 for HDMI output.
#ifdef ARM_ALLOW_MULTI_CORE
	m_nChunkSize = m_Properties.GetNumber ("ChunkSize", m_SoundDevice == "hdmi" ? 384*6 : 256);
#else
	m_nChunkSize = m_Properties.GetNumber ("ChunkSize", m_SoundDevice == "hdmi" ? 384*6 : 1024);
#endif
	m_nDACI2CAddress = m_Properties.GetNumber ("DACI2CAddress", 0);
	m_bChannelsSwapped = m_Properties.GetNumber ("ChannelsSwapped", 0) != 0;

	unsigned newEngineType = m_Properties.GetNumber ("EngineType", 1);
	if (newEngineType == 2) {
  		m_EngineType = MKI;
	} else if (newEngineType == 3) {
  		m_EngineType = OPL;
	} else {	// 1
  		m_EngineType = MSFA;
	}
	m_bEnableTGCompressor = m_Properties.GetNumber ("EnableTGCompressor", 1) != 0;	
	m_nMIDIBaudRate = m_Properties.GetNumber ("MIDIBaudRate", 31250);

	const char *pMIDIThru = m_Properties.GetString ("MIDIThru");
	if (pMIDIThru)
	{
		std::string Arg (pMIDIThru);

		size_t nPos = Arg.find (',');
		if (nPos != std::string::npos)
		{
			m_MIDIThruIn = Arg.substr (0, nPos);
			m_MIDIThruOut = Arg.substr (nPos+1);

			if (   m_MIDIThruIn.empty ()
			    || m_MIDIThruOut.empty ())
			{
				m_MIDIThruIn.clear ();
				m_MIDIThruOut.clear ();
			}
		}
	}
    m_nMIDIOutTg = m_Properties.GetNumber("MIDIOutTg", 0);	
	m_bMIDIRXProgramChange = m_Properties.GetNumber ("MIDIRXProgramChange", 1) != 0;
	m_bIgnoreAllNotesOff = m_Properties.GetNumber ("IgnoreAllNotesOff", 0) != 0;
	m_bMIDIAutoVoiceDumpOnPC = m_Properties.GetNumber ("MIDIAutoVoiceDumpOnPC", 0) != 0;
	m_bHeaderlessSysExVoices = m_Properties.GetNumber ("HeaderlessSysExVoices", 0) != 0;
	m_bExpandPCAcrossBanks = m_Properties.GetNumber ("ExpandPCAcrossBanks", 1) != 0;

	m_bLCDEnabled = m_Properties.GetNumber ("LCDEnabled", 0) != 0;
	m_nLCDPinEnable = m_Properties.GetNumber ("LCDPinEnable", 4);
	m_nLCDPinRegisterSelect = m_Properties.GetNumber ("LCDPinRegisterSelect", 27);
	m_nLCDPinReadWrite = m_Properties.GetNumber ("LCDPinReadWrite", 0);
	m_nLCDPinData4 = m_Properties.GetNumber ("LCDPinData4", 22);
	m_nLCDPinData5 = m_Properties.GetNumber ("LCDPinData5", 23);
	m_nLCDPinData6 = m_Properties.GetNumber ("LCDPinData6", 24);
	m_nLCDPinData7 = m_Properties.GetNumber ("LCDPinData7", 25);
	m_nLCDI2CAddress = m_Properties.GetNumber ("LCDI2CAddress", 0);

	m_nSSD1306LCDI2CAddress = m_Properties.GetNumber ("SSD1306LCDI2CAddress", 0);
	m_nSSD1306LCDWidth = m_Properties.GetNumber ("SSD1306LCDWidth", 128);
	m_nSSD1306LCDHeight = m_Properties.GetNumber ("SSD1306LCDHeight", 32);
	m_bSSD1306LCDRotate = m_Properties.GetNumber ("SSD1306LCDRotate", 0) != 0;
	m_bSSD1306LCDMirror = m_Properties.GetNumber ("SSD1306LCDMirror", 0) != 0;

	//m_nLCDColumns = m_Properties.GetNumber ("LCDColumns", 16);
	//m_nLCDRows = m_Properties.GetNumber ("LCDRows", 2);
	LCDColumns = m_Properties.GetNumber ("LCDColumns", 16);
	LCDRows = m_Properties.GetNumber ("LCDRows", 2);

	m_nButtonPinPrev = m_Properties.GetNumber ("ButtonPinPrev", 0);
	m_nButtonPinNext = m_Properties.GetNumber ("ButtonPinNext", 0);
	m_nButtonPinBack = m_Properties.GetNumber ("ButtonPinBack", 11);
	m_nButtonPinSelect = m_Properties.GetNumber ("ButtonPinSelect", 11);
	m_nButtonPinHome = m_Properties.GetNumber ("ButtonPinHome", 11);
	m_nButtonPinShortcut = m_Properties.GetNumber ("ButtonPinShortcut", 11);

	m_nButtonPinModeMidi = m_Properties.GetNumber ("ButtonPinModeMidi", 0);
	
	m_ButtonActionPrev = m_Properties.GetString ("ButtonActionPrev", "");
	m_ButtonActionNext = m_Properties.GetString ("ButtonActionNext", "");
	m_ButtonActionBack = m_Properties.GetString ("ButtonActionBack", "doubleclick");
	m_ButtonActionSelect = m_Properties.GetString ("ButtonActionSelect", "click");
	m_ButtonActionHome = m_Properties.GetString ("ButtonActionHome", "longpress");

	m_ButtonActionModeMidi = m_Properties.GetString ("ButtonActionModeMidi", "");

	m_nDoubleClickTimeout = m_Properties.GetNumber ("DoubleClickTimeout", 400);
	m_nLongPressTimeout = m_Properties.GetNumber ("LongPressTimeout", 600);

	m_nButtonPinPgmUp = m_Properties.GetNumber ("ButtonPinPgmUp", 0);
	m_nButtonPinPgmDown = m_Properties.GetNumber ("ButtonPinPgmDown", 0);
	m_nButtonPinTGUp = m_Properties.GetNumber ("ButtonPinTGUp", 0);
	m_nButtonPinTGDown = m_Properties.GetNumber ("ButtonPinTGDown", 0);

	m_ButtonActionPgmUp = m_Properties.GetString ("ButtonActionPgmUp", "");
	m_ButtonActionPgmDown = m_Properties.GetString ("ButtonActionPgmDown", "");
	m_ButtonActionTGUp = m_Properties.GetString ("ButtonActionTGUp", "");
	m_ButtonActionTGDown = m_Properties.GetString ("ButtonActionTGDown", "");

	m_nMIDIButtonCh = m_Properties.GetNumber ("MIDIButtonCh", 0);
	m_nMIDIButtonNotes = m_Properties.GetNumber ("MIDIButtonNotes", 0);
	m_nMIDIButtonPrev = m_Properties.GetNumber ("MIDIButtonPrev", 0);
	m_nMIDIButtonNext = m_Properties.GetNumber ("MIDIButtonNext", 0);
	m_nMIDIButtonBack = m_Properties.GetNumber ("MIDIButtonBack", 0);
	m_nMIDIButtonSelect = m_Properties.GetNumber ("MIDIButtonSelect", 0);
	m_nMIDIButtonHome = m_Properties.GetNumber ("MIDIButtonHome", 0);

	m_nMIDIButtonModeMidi = m_Properties.GetNumber ("MIDIButtonModeMidi", 0);

	m_nMIDIButtonPgmUp = m_Properties.GetNumber ("MIDIButtonPgmUp", 0);
	m_nMIDIButtonPgmDown = m_Properties.GetNumber ("MIDIButtonPgmDown", 0);
	m_nMIDIButtonTGUp = m_Properties.GetNumber ("MIDIButtonTGUp", 0);
	m_nMIDIButtonTGDown = m_Properties.GetNumber ("MIDIButtonTGDown", 0);
	
	m_bEncoderEnabled = m_Properties.GetNumber ("EncoderEnabled", 0) != 0;
	m_nEncoderPinClock = m_Properties.GetNumber ("EncoderPinClock", 10);
	m_nEncoderPinData = m_Properties.GetNumber ("EncoderPinData", 9);

	m_bMIDIDumpEnabled  = m_Properties.GetNumber ("MIDIDumpEnabled", 0) != 0;
	m_bProfileEnabled = m_Properties.GetNumber ("ProfileEnabled", 0) != 0;
	m_bPerformanceSelectToLoad = m_Properties.GetNumber ("PerformanceSelectToLoad", 1) != 0;
	m_nPerformanceSelectChannel = m_Properties.GetNumber ("PerformanceSelectChannel", 0);
	m_bSaveSessionPerformance = m_Properties.GetNumber("SaveSessionPerformance", 0) != 0;

	// mdx816session.ini
	m_nSessionPerformance = m_SessionSettings.GetNumber ("Performance", 1);
	if (m_nSessionPerformance > 0) m_nSessionPerformance--;
	m_nSessionPerformanceBank = m_SessionSettings.GetNumber ("PerformanceBank", 1);
	if (m_nSessionPerformanceBank > 0) m_nSessionPerformanceBank--;
	m_nSessionMasterVolume = m_SessionSettings.GetNumber ("MasterVolume", 100);

	return true;
}

bool CConfig::GetUSBGadgetMode (void) const
{
	return m_bUSBGadgetMode;
}

const char *CConfig::GetSoundDevice (void) const
{
	return m_SoundDevice.c_str ();
}

unsigned CConfig::GetSampleRate (void) const
{
	return m_nSampleRate;
}

unsigned CConfig::GetChunkSize (void) const
{
	return m_nChunkSize;
}

unsigned CConfig::GetDACI2CAddress (void) const
{
	return m_nDACI2CAddress;
}

bool CConfig::GetChannelsSwapped (void) const
{
	return m_bChannelsSwapped;
}

unsigned CConfig::GetEngineType (void) const
{
	return m_EngineType;
}

bool CConfig::GetEnableTGCompressor (void) const
{
	return m_bEnableTGCompressor;
}

unsigned CConfig::GetMIDIBaudRate (void) const
{
	return m_nMIDIBaudRate;
}

const char *CConfig::GetMIDIThruIn (void) const
{
	return m_MIDIThruIn.c_str ();
}

const char *CConfig::GetMIDIThruOut (void) const
{
	return m_MIDIThruOut.c_str ();
}

unsigned CConfig::GetMIDIOutTg(void) const
{
        return m_nMIDIOutTg;
}

bool CConfig::GetMIDIRXProgramChange (void) const
{
	return m_bMIDIRXProgramChange;
}

bool CConfig::GetIgnoreAllNotesOff (void) const
{
	return m_bIgnoreAllNotesOff;
}

bool CConfig::GetMIDIAutoVoiceDumpOnPC (void) const
{
	return m_bMIDIAutoVoiceDumpOnPC;
}

bool CConfig::GetHeaderlessSysExVoices (void) const
{
	return m_bHeaderlessSysExVoices;
}

bool CConfig::GetExpandPCAcrossBanks (void) const
{
	return m_bExpandPCAcrossBanks;
}

bool CConfig::GetLCDEnabled (void) const
{
	return m_bLCDEnabled;
}

unsigned CConfig::GetLCDPinEnable (void) const
{
	return m_nLCDPinEnable;
}

unsigned CConfig::GetLCDPinRegisterSelect (void) const
{
	return m_nLCDPinRegisterSelect;
}

unsigned CConfig::GetLCDPinReadWrite (void) const
{
	return m_nLCDPinReadWrite;
}

unsigned CConfig::GetLCDPinData4 (void) const
{
	return m_nLCDPinData4;
}

unsigned CConfig::GetLCDPinData5 (void) const
{
	return m_nLCDPinData5;
}

unsigned CConfig::GetLCDPinData6 (void) const
{
	return m_nLCDPinData6;
}

unsigned CConfig::GetLCDPinData7 (void) const
{
	return m_nLCDPinData7;
}

unsigned CConfig::GetLCDI2CAddress (void) const
{
	return m_nLCDI2CAddress;
}

unsigned CConfig::GetSSD1306LCDI2CAddress (void) const
{
	return m_nSSD1306LCDI2CAddress;
}

unsigned CConfig::GetSSD1306LCDWidth (void) const
{
	return m_nSSD1306LCDWidth;
}

unsigned CConfig::GetSSD1306LCDHeight (void) const
{
	return m_nSSD1306LCDHeight;
}

bool CConfig::GetSSD1306LCDRotate (void) const
{
	return m_bSSD1306LCDRotate;
}

bool CConfig::GetSSD1306LCDMirror (void) const
{
	return m_bSSD1306LCDMirror;
}

unsigned CConfig::GetLCDColumns (void) const
{
	return LCDColumns;
	//return m_nLCDColumns;
}

unsigned CConfig::GetLCDRows (void) const
{
	return LCDRows;
	//return m_nLCDRows;
}

unsigned CConfig::GetButtonPinPrev (void) const
{
	return m_nButtonPinPrev;
}

unsigned CConfig::GetButtonPinNext (void) const
{
	return m_nButtonPinNext;
}

unsigned CConfig::GetButtonPinBack (void) const
{
	return m_nButtonPinBack;
}

unsigned CConfig::GetButtonPinSelect (void) const
{
	return m_nButtonPinSelect;
}

unsigned CConfig::GetButtonPinHome (void) const
{
	return m_nButtonPinHome;
}

unsigned CConfig::GetButtonPinModeMidi (void) const
{
	return m_nButtonPinModeMidi;
}

unsigned CConfig::GetButtonPinShortcut (void) const
{
	return m_nButtonPinShortcut;
}

const char *CConfig::GetButtonActionPrev (void) const
{
	return m_ButtonActionPrev.c_str();
}

const char *CConfig::GetButtonActionNext (void) const
{
	return m_ButtonActionNext.c_str();
}

const char *CConfig::GetButtonActionBack (void) const
{
	return m_ButtonActionBack.c_str();
}

const char *CConfig::GetButtonActionSelect (void) const
{
	return m_ButtonActionSelect.c_str();
}

const char *CConfig::GetButtonActionHome (void) const
{
	return m_ButtonActionHome.c_str();
}

const char *CConfig::GetButtonActionModeMidi (void) const
{
	return m_ButtonActionModeMidi.c_str();
}

unsigned CConfig::GetDoubleClickTimeout (void) const
{
	return m_nDoubleClickTimeout;
}

unsigned CConfig::GetLongPressTimeout (void) const
{
	return m_nLongPressTimeout;
}

unsigned CConfig::GetButtonPinPgmUp (void) const
{
	return m_nButtonPinPgmUp;
}

unsigned CConfig::GetButtonPinPgmDown (void) const
{
	return m_nButtonPinPgmDown;
}

unsigned CConfig::GetButtonPinTGUp (void) const
{
	return m_nButtonPinTGUp;
}

unsigned CConfig::GetButtonPinTGDown (void) const
{
	return m_nButtonPinTGDown;
}

const char *CConfig::GetButtonActionPgmUp (void) const
{
	return m_ButtonActionPgmUp.c_str();
}

const char *CConfig::GetButtonActionPgmDown (void) const
{
	return m_ButtonActionPgmDown.c_str();
}

const char *CConfig::GetButtonActionTGUp (void) const
{
	return m_ButtonActionTGUp.c_str();
}

const char *CConfig::GetButtonActionTGDown (void) const
{
	return m_ButtonActionTGDown.c_str();
}

unsigned CConfig::GetMIDIButtonCh (void) const
{
	return m_nMIDIButtonCh;
}

unsigned CConfig::GetMIDIButtonNotes (void) const
{
	return m_nMIDIButtonNotes;
}

unsigned CConfig::GetMIDIButtonPrev (void) const
{
	return m_nMIDIButtonPrev;
}

unsigned CConfig::GetMIDIButtonNext (void) const
{
	return m_nMIDIButtonNext;
}

unsigned CConfig::GetMIDIButtonBack (void) const
{
	return m_nMIDIButtonBack;
}

unsigned CConfig::GetMIDIButtonSelect (void) const
{
	return m_nMIDIButtonSelect;
}

unsigned CConfig::GetMIDIButtonHome (void) const
{
	return m_nMIDIButtonHome;
}

unsigned CConfig::GetMIDIButtonModeMidi (void) const
{
	return m_nMIDIButtonModeMidi;
}

unsigned CConfig::GetMIDIButtonPgmUp (void) const
{
	return m_nMIDIButtonPgmUp;
}

unsigned CConfig::GetMIDIButtonPgmDown (void) const
{
	return m_nMIDIButtonPgmDown;
}

unsigned CConfig::GetMIDIButtonTGUp (void) const
{
	return m_nMIDIButtonTGUp;
}

unsigned CConfig::GetMIDIButtonTGDown (void) const
{
	return m_nMIDIButtonTGDown;
}

bool CConfig::GetEncoderEnabled (void) const
{
	return m_bEncoderEnabled;
}

unsigned CConfig::GetEncoderPinClock (void) const
{
	return m_nEncoderPinClock;
}

unsigned CConfig::GetEncoderPinData (void) const
{
	return m_nEncoderPinData;
}

bool CConfig::GetMIDIDumpEnabled (void) const
{
	return m_bMIDIDumpEnabled;
}

bool CConfig::GetProfileEnabled (void) const
{
	return m_bProfileEnabled;
}

bool CConfig::GetPerformanceSelectToLoad (void) const
{
	return m_bPerformanceSelectToLoad;
}

unsigned CConfig::GetPerformanceSelectChannel (void) const
{
	return m_nPerformanceSelectChannel;
}

bool CConfig::GetSaveSessionPerformance (void) const
{
	return m_bSaveSessionPerformance;
}

// mdx816session.ini

unsigned CConfig::GetSessionPerformance (void) const
{
	return m_nSessionPerformance;
}

unsigned CConfig::GetSessionPerformanceBank (void) const
{
	return m_nSessionPerformanceBank;
}

void CConfig::SetSessionPerformance (unsigned nValue)
{
	assert (nValue < NUM_PERFORMANCES);
	m_nSessionPerformance = nValue;
	SaveSessionSettings();
}

void CConfig::SetSessionPerformanceBank (unsigned nValue)
{
	assert (nValue < NUM_PERFORMANCE_BANKS);
	m_nSessionPerformanceBank = nValue;
	SaveSessionSettings();
}

unsigned CConfig::GetSessionMasterVolume (void) const
{
	return m_nSessionMasterVolume;
}

void CConfig::SetSessionMasterVolume (unsigned nValue)
{
	assert (nValue < 128);
	m_nSessionMasterVolume = nValue;
	SaveSessionSettings();
}

// save session settings  file
bool CConfig::SaveSessionSettings (void)
{
	m_SessionSettings.RemoveAll ();

	m_SessionSettings.SetNumber ("Performance", m_nSessionPerformance+1);
	m_SessionSettings.SetNumber ("PerformanceBank", m_nSessionPerformanceBank+1);
	m_SessionSettings.SetNumber ("MasterVolume", m_nSessionMasterVolume);

	return m_SessionSettings.Save ();
}

