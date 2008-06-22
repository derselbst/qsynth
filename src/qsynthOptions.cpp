// qsynthOptions.cpp
// qsynthOptions.cpp
//
/****************************************************************************
   Copyright (C) 2003-2007, rncbc aka Rui Nuno Capela. All rights reserved.

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public License
   as published by the Free Software Foundation; either version 2
   of the License, or (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License along
   with this program; if not, write to the Free Software Foundation, Inc.,
   51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.

*****************************************************************************/

#include "qsynthAbout.h"
#include "qsynthOptions.h"

#include "qsynthEngine.h"

#include <QTextStream>
#include <QComboBox>


//-------------------------------------------------------------------------
// qsynthOptions - Prototype settings structure.
//

// Constructor.
qsynthOptions::qsynthOptions (void)
	: m_settings(QSYNTH_DOMAIN, QSYNTH_TITLE)
{
	// Create default setup descriptor.
	m_pDefaultSetup = new qsynthSetup();
	// Load previous/default fluidsynth settings...
	loadSetup(m_pDefaultSetup, QString::null);

	// Load display options...
	m_settings.beginGroup("/Options");
	sMessagesFont   = m_settings.value("/MessagesFont").toString();
	bMessagesLimit  = m_settings.value("/MessagesLimit", true).toBool();
	iMessagesLimitLines = m_settings.value("/MessagesLimitLines", 1000).toInt();
	bMessagesLog    = m_settings.value("/MessagesLog", false).toBool();
	sMessagesLogPath = m_settings.value("/MessagesLogPath", "qsynth.log").toString();
	bQueryClose     = m_settings.value("/QueryClose", true).toBool();
	bKeepOnTop      = m_settings.value("/KeepOnTop", false).toBool();
	bStdoutCapture  = m_settings.value("/StdoutCapture", true).toBool();
	bOutputMeters   = m_settings.value("/OutputMeters", false).toBool();
	bSystemTray     = m_settings.value("/SystemTray", false).toBool();
	bStartMinimized = m_settings.value("/StartMinimized", false).toBool();
	iKnobStyle      = m_settings.value("/KnobStyle", 0).toInt();
	iKnobMotion     = m_settings.value("/KnobMotion", 0).toInt();
	m_settings.endGroup();

	// Load defaults...
	m_settings.beginGroup("/Defaults");
	sSoundFontDir  = m_settings.value("/SoundFontDir").toString();
	bPresetPreview = m_settings.value("/PresetPreview", false).toBool();
	m_settings.endGroup();

	// Load custom additional engines.
	m_settings.beginGroup("/Engines");
	const QString sEnginePrefix = "/Engine%1";
	int iEngine = 0;
	for (;;) {
		QString sItem = m_settings.value(sEnginePrefix.arg(++iEngine)).toString();
		if (sItem.isEmpty())
			break;
		engines.append(sItem);
	}
	m_settings.endGroup();
}


// Default Destructor.
qsynthOptions::~qsynthOptions (void)
{
	// Make program version available in the future.
	m_settings.beginGroup("/Program");
	m_settings.setValue("/Version", QSYNTH_VERSION);
	m_settings.endGroup();

	// Save engines list...
	m_settings.beginGroup("/Engines");
	// Save last preset list.
	const QString sEnginePrefix = "/Engine%1";
	int iEngine = 0;
	QStringListIterator iter(engines);
	while (iter.hasNext())
		m_settings.setValue(sEnginePrefix.arg(++iEngine), iter.next());
	// Cleanup old entries, if any...
	while (!m_settings.value(sEnginePrefix.arg(++iEngine)).toString().isEmpty())
		m_settings.remove(sEnginePrefix.arg(iEngine));
	m_settings.endGroup();

	// Save defaults...
	m_settings.beginGroup("/Defaults");
	m_settings.setValue("/SoundFontDir", sSoundFontDir);
	m_settings.setValue("/PresetPreview", bPresetPreview);
	m_settings.endGroup();

	// Save last display options.
	m_settings.beginGroup("/Options");
	m_settings.setValue("/MessagesFont", sMessagesFont);
	m_settings.setValue("/MessagesLimit", bMessagesLimit);
	m_settings.setValue("/MessagesLimitLines", iMessagesLimitLines);
	m_settings.setValue("/MessagesLog", bMessagesLog);
	m_settings.setValue("/MessagesLogPath", sMessagesLogPath);
	m_settings.setValue("/QueryClose", bQueryClose);
	m_settings.setValue("/KeepOnTop", bKeepOnTop);
	m_settings.setValue("/StdoutCapture", bStdoutCapture);
	m_settings.setValue("/OutputMeters", bOutputMeters);
	m_settings.setValue("/SystemTray", bSystemTray);
	m_settings.setValue("/StartMinimized", bStartMinimized);
	m_settings.setValue("/KnobStyle", iKnobStyle);
	m_settings.setValue("/KnobMotion", iKnobMotion);
	m_settings.endGroup();

	// Create default setup descriptor.
	delete m_pDefaultSetup;
	m_pDefaultSetup = NULL;
}


// Default instance setup accessor.
qsynthSetup *qsynthOptions::defaultSetup (void)
{
	return m_pDefaultSetup;
}


//-------------------------------------------------------------------------
// Command-line argument stuff. Mostly to mimic fluidsynth CLI.
//

// Help about command line options.
void qsynthOptions::print_usage ( const char *arg0 )
{
	QTextStream out(stderr);
	const QString sEot = "\n\t";
	const QString sEol = "\n\n";

	out << QObject::tr("Usage: %1"
		" [options] [soundfonts] [midifiles]").arg(arg0) + sEol;
	out << QSYNTH_TITLE " - " + QObject::tr(QSYNTH_SUBTITLE) + sEol;
	out << QObject::tr("Options") + ":" + sEol;
	out << "  -n, --no-midi-in" + sEot +
		QObject::tr("Don't create a midi driver to read MIDI input events [default = yes]") + sEol;
	out << "  -m, --midi-driver=[label]" + sEot +
		QObject::tr("The name of the midi driver to use [oss,alsa,alsa_seq,...]") + sEol;
	out << "  -K, --midi-channels=[num]" + sEot +
		QObject::tr("The number of midi channels [default = 16]") + sEol;
	out << "  -a, --audio-driver=[label]" + sEot +
		QObject::tr("The audio driver [alsa,jack,oss,dsound,...]") + sEol;
	out << "  -j, --connect-jack-outputs" + sEot +
		QObject::tr("Attempt to connect the jack outputs to the physical ports") + sEol;
	out << "  -L, --audio-channels=[num]" + sEot +
		QObject::tr("The number of stereo audio channels [default = 1]") + sEol;
	out << "  -G, --audio-groups=[num]" + sEot +
		QObject::tr("The number of audio groups [default = 1]") + sEol;
	out << "  -z, --audio-bufsize=[size]" + sEot +
		QObject::tr("Size of each audio buffer") + sEol;
	out << "  -c, --audio-bufcount=[count]" + sEot +
		QObject::tr("Number of audio buffers") + sEol;
	out << "  -r, --sample-rate=[rate]" + sEot +
		QObject::tr("Set the sample rate") + sEol;
	out << "  -R, --reverb=[flag]" + sEot +
		QObject::tr("Turn the reverb on or off [1|0|yes|no|on|off, default = on]") + sEol;
	out << "  -C, --chorus=[flag]" + sEot +
		QObject::tr("Turn the chorus on or off [1|0|yes|no|on|off, default = on]") + sEol;
	out << "  -g, --gain=[gain]" + sEot +
		QObject::tr("Set the master gain [0 < gain < 10, default = 0.2]") + sEol;
	out << "  -o, --option [name=value]" + sEot +
		QObject::tr("Define a setting name=value") + sEol;
	out << "  -s, --server" + sEot +
		QObject::tr("Create and start server [default = no]") + sEol;
	out << "  -i, --no-shell" + sEot +
		QObject::tr("Don't read commands from the shell [ignored]") + sEol;
	out << "  -d, --dump" + sEot +
		QObject::tr("Dump midi router events") + sEol;
	out << "  -v, --verbose" + sEot +
		QObject::tr("Print out verbose messages about midi events") + sEol;
	out << "  -h, --help" + sEot +
		QObject::tr("Show help about command line options") + sEol;
	out << "  -V, --version" + sEot +
		QObject::tr("Show version information") + sEol;
}


// Special parsing of '-o' command-line option into fluidsynth settings.
bool qsynthOptions::parse_option ( char *optarg )
{
	char *val;

	for (val = optarg; *val; val++) {
		if (*val == '=') {
			*val++ = (char) 0;
			break;
		}
	}

	fluid_settings_t *pFluidSettings = m_pDefaultSetup->fluid_settings();

	switch (::fluid_settings_get_type(pFluidSettings, optarg)) {
	case FLUID_NUM_TYPE:
		if (::fluid_settings_setnum(pFluidSettings, optarg, ::atof(val)))
			break;
	case FLUID_INT_TYPE:
		if (::fluid_settings_setint(pFluidSettings, optarg, ::atoi(val)))
			break;
	case FLUID_STR_TYPE:
		if (::fluid_settings_setstr(pFluidSettings, optarg, val))
			break;
	default:
		return false;
	}

	return true;
}


// Parse command line arguments into fluidsynth settings.
bool qsynthOptions::parse_args ( int argc, char **argv )
{
	QTextStream out(stderr);
	const QString sEol = "\n\n";
	int iSoundFontOverride = 0;

	for (int i = 1; i < argc; i++) {

		QString sVal;
		QString sArg = argv[i];
		int iEqual = sArg.indexOf('=');
		if (iEqual >= 0) {
			sVal = sArg.right(sArg.length() - iEqual - 1);
			sArg = sArg.left(iEqual);
		}
		else if (i < argc)
			sVal = argv[i + 1];

		if (sArg == "-n" || sArg == "--no-midi-in") {
			m_pDefaultSetup->bMidiIn = false;
		}
		else if (sArg == "-m" || sArg == "--midi-driver") {
			if (sVal.isEmpty()) {
				out << QObject::tr("Option -m requires an argument (midi-driver).") + sEol;
				return false;
			}
			m_pDefaultSetup->sMidiDriver = sVal;
			if (iEqual < 0)
				i++;
		}
		else if (sArg == "-K" || sArg == "--midi-channels") {
			if (sVal.isEmpty()) {
				out << QObject::tr("Option -K requires an argument (midi-channels).") + sEol;
				return false;
			}
			m_pDefaultSetup->iMidiChannels = sVal.toInt();
			if (iEqual < 0)
				i++;
		}
		else if (sArg == "-a" || sArg == "--audio-driver") {
			if (sVal.isEmpty()) {
				out << QObject::tr("Option -a requires an argument (audio-driver).") + sEol;
				return false;
			}
			m_pDefaultSetup->sAudioDriver = sVal;
			if (iEqual < 0)
				i++;
		}
		else if (sArg == "-j" || sArg == "--connect-jack-outputs") {
			m_pDefaultSetup->bJackAutoConnect = true;
		}
		else if (sArg == "-L" || sArg == "--audio-channels") {
			if (sVal.isEmpty()) {
				out << QObject::tr("Option -L requires an argument (audio-channels).") + sEol;
				return false;
			}
			m_pDefaultSetup->iAudioChannels = sVal.toInt();
			if (iEqual < 0)
				i++;
		}
		else if (sArg == "-G" || sArg == "--audio-groups") {
			if (sVal.isEmpty()) {
				out << QObject::tr("Option -G requires an argument (audio-groups).") + sEol;
				return false;
		}
			m_pDefaultSetup->iAudioGroups = sVal.toInt();
			if (iEqual < 0)
				i++;
		}
		else if (sArg == "-z" || sArg == "--audio-bufsize") {
			if (sVal.isEmpty()) {
				out << QObject::tr("Option -z requires an argument (audio-bufsize).") + sEol;
				return false;
			}
			m_pDefaultSetup->iAudioBufSize = sVal.toInt();
			if (iEqual < 0)
				i++;
		}
		else if (sArg == "-c" || sArg == "--audio-bufcount") {
			if (sVal.isEmpty()) {
				out << QObject::tr("Option -c requires an argument (audio-bufcount).") + sEol;
				return false;
			}
			m_pDefaultSetup->iAudioBufCount = sVal.toInt();
			if (iEqual < 0)
				i++;
		}
		else if (sArg == "-r" || sArg == "--sample-rate") {
			if (sVal.isEmpty()) {
				out << QObject::tr("Option -r requires an argument (sample-rate).") + sEol;
				return false;
			}
			m_pDefaultSetup->fSampleRate = sVal.toFloat();
			if (iEqual < 0)
				i++;
		}
		else if (sArg == "-R" || sArg == "--reverb") {
			if (sVal.isEmpty()) {
				out << QObject::tr("Option -R requires an argument (reverb).") + sEol;
				return false;
			}
			m_pDefaultSetup->bReverbActive = !(sVal == "0" || sVal == "no" || sVal == "off");
			if (iEqual < 0)
				i++;
		}
		else if (sArg == "-C" || sArg == "--chorus") {
			if (sVal.isEmpty()) {
				out << QObject::tr("Option -C requires an argument (chorus).") + sEol;
				return false;
			}
			m_pDefaultSetup->bChorusActive = !(sVal == "0" || sVal == "no" || sVal == "off");
			if (iEqual < 0)
				i++;
		}
		else if (sArg == "-g" || sArg == "--gain") {
			if (sVal.isEmpty()) {
				out << QObject::tr("Option -g requires an argument (gain).") + sEol;
				return false;
			}
			m_pDefaultSetup->fGain = sVal.toFloat();
			if (iEqual < 0)
				i++;
		}
		else if (sArg == "-o" || sArg == "--option") {
			if (++i >= argc) {
				out << QObject::tr("Option -o requires an argument.") + sEol;
				return false;
			}
			if (!parse_option(argv[i])) {
				out << QObject::tr("Option -o failed to set '%1'.").arg(argv[i]) + sEol;
				return false;
			}
		}
		else if (sArg == "-s" || sArg == "--server") {
			m_pDefaultSetup->bServer = true;
		}
		else if (sArg == "-i" || sArg == "--no-shell") {
			// Just ignore this...
		}
		else if (sArg == "-d" || sArg == "--dump") {
			m_pDefaultSetup->bMidiDump = true;
		}
		else if (sArg == "-v" || sArg == "--verbose") {
			m_pDefaultSetup->bVerbose = true;
		}
		else if (sArg == "-h" || sArg == "--help") {
			print_usage(argv[0]);
			return false;
		}
		else if (sArg == "-V" || sArg == "--version") {
			out << QObject::tr("Qt: %1\n").arg(qVersion());
			out << QObject::tr(QSYNTH_TITLE ": %1\n").arg(QSYNTH_VERSION);
			return false;
		}
		else if (::fluid_is_soundfont(argv[i])) {
			if (++iSoundFontOverride == 1) {
				m_pDefaultSetup->soundfonts.clear();
				m_pDefaultSetup->bankoffsets.clear();
			}
			m_pDefaultSetup->soundfonts.append(argv[i]);
			m_pDefaultSetup->bankoffsets.append(QString::null);
		}
		else if (::fluid_is_midifile(argv[i])) {
			m_pDefaultSetup->midifiles.append(argv[i]);
		}
		else {
			out << QObject::tr("Unknown option '%1'.").arg(argv[i]) + sEol;
			print_usage(argv[0]);
			return false;
		}
	}

	// Alright with argument parsing.
	return true;
}


//---------------------------------------------------------------------------
// Engine entry management methods.

void qsynthOptions::newEngine ( qsynthEngine *pEngine )
{
	if (pEngine == NULL)
		return;
	if (pEngine->isDefault())
		return;

	const QString& sName = pEngine->name();
	if (!engines.contains(sName))
		engines.append(sName);
}


bool qsynthOptions::renameEngine ( qsynthEngine *pEngine )
{
	if (pEngine == NULL)
		return false;

	qsynthSetup *pSetup = pEngine->setup();
	if (pSetup == NULL)
		return false;

	const QString sOldName = pEngine->name();
	const QString sNewName = pSetup->sDisplayName;
	if (sOldName == sNewName)
		return false;

	pEngine->setName(sNewName);

	if (!pEngine->isDefault()) {
		engines = engines.replaceInStrings(sOldName, sNewName);
		m_settings.remove("/Engine/" + sOldName);
	}

	return true;
}


void qsynthOptions::deleteEngine ( qsynthEngine *pEngine )
{
	if (pEngine == NULL)
		return;
	if (pEngine->isDefault())
		return;

	const QString& sName = pEngine->name();
	int iEngine = engines.indexOf(sName);
	if (iEngine >= 0)
		engines.removeAt(iEngine);

	m_settings.remove("/Engine/" + sName);
}


//---------------------------------------------------------------------------
// Setup registry methods.

// Load instance m_settings.
void qsynthOptions::loadSetup ( qsynthSetup *pSetup, const QString& sName )
{
	if (pSetup == NULL)
		return;

	// Begin at key group?
	if (!sName.isEmpty())
		m_settings.beginGroup("/Engine/" + sName);

	// Shall we have a default display name.
	QString sDisplayName = sName;
	if (sDisplayName.isEmpty())
		sDisplayName = QObject::tr(QSYNTH_TITLE "1");

	// Load previous/default fluidsynth m_settings...
	m_settings.beginGroup("/Settings");
	pSetup->sDisplayName     = m_settings.value("/DisplayName", sDisplayName).toString();
	pSetup->bMidiIn          = m_settings.value("/MidiIn", true).toBool();
	pSetup->sMidiDriver      = m_settings.value("/MidiDriver", "alsa_seq").toString();
	pSetup->sMidiDevice      = m_settings.value("/MidiDevice").toString();
	pSetup->iMidiChannels    = m_settings.value("/MidiChannels", 16).toInt();
	pSetup->sAlsaName        = m_settings.value("/AlsaName", "pid").toString();
	pSetup->sAudioDriver     = m_settings.value("/AudioDriver", "jack").toString();
	pSetup->sAudioDevice     = m_settings.value("/AudioDevice").toString();
	pSetup->sJackName        = m_settings.value("/JackName", "qsynth").toString();
	pSetup->bJackAutoConnect = m_settings.value("/JackAutoConnect", true).toBool();
	pSetup->bJackMulti       = m_settings.value("/JackMulti", false).toBool();
	pSetup->iAudioChannels   = m_settings.value("/AudioChannels", 1).toInt();
	pSetup->iAudioGroups     = m_settings.value("/AudioGroups", 1).toInt();
	pSetup->iAudioBufSize    = m_settings.value("/AudioBufSize", 64).toInt();
	pSetup->iAudioBufCount   = m_settings.value("/AudioBufCount", 2).toInt();
	pSetup->sSampleFormat    = m_settings.value("/SampleFormat", "16bits").toString();
	pSetup->fSampleRate      = m_settings.value("/SampleRate", 44100.0).toDouble();
	pSetup->iPolyphony       = m_settings.value("/Polyphony", 256).toInt();
	pSetup->bReverbActive    = m_settings.value("/ReverbActive", true).toBool();
	pSetup->fReverbRoom      = m_settings.value("/ReverbRoom",  FLUID_REVERB_DEFAULT_ROOMSIZE).toDouble();
	pSetup->fReverbDamp      = m_settings.value("/ReverbDamp",  FLUID_REVERB_DEFAULT_DAMP).toDouble();
	pSetup->fReverbWidth     = m_settings.value("/ReverbWidth", FLUID_REVERB_DEFAULT_WIDTH).toDouble();
	pSetup->fReverbLevel     = m_settings.value("/ReverbLevel", FLUID_REVERB_DEFAULT_LEVEL).toDouble();
	pSetup->bChorusActive    = m_settings.value("/ChorusActive", true).toBool();
	pSetup->iChorusNr        = m_settings.value("/ChorusNr",    FLUID_CHORUS_DEFAULT_N).toInt();
	pSetup->fChorusLevel     = m_settings.value("/ChorusLevel", FLUID_CHORUS_DEFAULT_LEVEL).toDouble();
	pSetup->fChorusSpeed     = m_settings.value("/ChorusSpeed", FLUID_CHORUS_DEFAULT_SPEED).toDouble();
	pSetup->fChorusDepth     = m_settings.value("/ChorusDepth", FLUID_CHORUS_DEFAULT_DEPTH).toDouble();
	pSetup->iChorusType      = m_settings.value("/ChorusType",  FLUID_CHORUS_DEFAULT_TYPE).toInt();
	pSetup->bLadspaActive    = m_settings.value("/LadspaActive", false).toBool();
	pSetup->fGain            = m_settings.value("/Gain", 1.0).toDouble();
	pSetup->bServer          = m_settings.value("/Server", false).toBool();
	pSetup->bMidiDump        = m_settings.value("/MidiDump", false).toBool();
	pSetup->bVerbose         = m_settings.value("/Verbose", false).toBool();
	m_settings.endGroup();

	// Load soundfont list...
	m_settings.beginGroup("/SoundFonts");
	const QString sSoundFontPrefix  = "/SoundFont%1";
	const QString sBankOffsetPrefix = "/BankOffset%1";
	int i = 0;
	for (;;) {
		++i;
		QString sSoundFont  = m_settings.value(sSoundFontPrefix.arg(i)).toString();
		QString sBankOffset = m_settings.value(sBankOffsetPrefix.arg(i)).toString();
		if (sSoundFont.isEmpty())
			break;
		pSetup->soundfonts.append(sSoundFont);
		pSetup->bankoffsets.append(sBankOffset);
	}
	m_settings.endGroup();

	// Load channel presets list.
	m_settings.beginGroup("/Presets");
	pSetup->sDefPreset = m_settings.value("/DefPreset", pSetup->sDefPresetName).toString();
	const QString sPresetPrefix = "/Preset%1";
	int iPreset = 0;
	for (;;) {
		QString sItem = m_settings.value(sPresetPrefix.arg(++iPreset)).toString();
		if (sItem.isEmpty())
			break;
		pSetup->presets.append(sItem);
	}
	m_settings.endGroup();

	// Done with the key group?
	if (!sName.isEmpty())
		m_settings.endGroup();
}


// Save instance m_settings.
void qsynthOptions::saveSetup ( qsynthSetup *pSetup, const QString& sName )
{
	if (pSetup == NULL)
		return;

	// Begin at key group?
	if (!sName.isEmpty())
		m_settings.beginGroup("/Engine/" + sName);

	// Save presets list...
	m_settings.beginGroup("/Presets");
	m_settings.setValue("/DefPreset", pSetup->sDefPreset);
	// Save last preset list.
	const QString sPresetPrefix = "/Preset%1";
	int iPreset = 0;
	QStringListIterator iter(pSetup->presets);
	while (iter.hasNext())
		m_settings.setValue(sPresetPrefix.arg(++iPreset), iter.next());
	// Cleanup old entries, if any...
	while (!m_settings.value(sPresetPrefix.arg(++iPreset)).toString().isEmpty())
		m_settings.remove(sPresetPrefix.arg(iPreset));
	m_settings.endGroup();

	// Save last soundfont list.
	m_settings.beginGroup("/SoundFonts");
	const QString sSoundFontPrefix  = "/SoundFont%1";
	const QString sBankOffsetPrefix = "/BankOffset%1";
	int i = 0;
	QStringListIterator sfiter(pSetup->soundfonts);
	while (sfiter.hasNext()) {
		m_settings.setValue(sSoundFontPrefix.arg(++i), sfiter.next());
		m_settings.setValue(sBankOffsetPrefix.arg(i), pSetup->bankoffsets[i - 1]);
	}
	// Cleanup old entries, if any...
	for (;;) {
		if (m_settings.value(sSoundFontPrefix.arg(++i)).toString().isEmpty())
			break;
		m_settings.remove(sSoundFontPrefix.arg(i));
		m_settings.remove(sBankOffsetPrefix.arg(i));
	}
	m_settings.endGroup();

	// Save last fluidsynth m_settings.
	m_settings.beginGroup("/Settings");
	m_settings.setValue("/DisplayName",      pSetup->sDisplayName);
	m_settings.setValue("/MidiIn",           pSetup->bMidiIn);
	m_settings.setValue("/MidiDriver",       pSetup->sMidiDriver);
	m_settings.setValue("/MidiDevice",       pSetup->sMidiDevice);
	m_settings.setValue("/MidiChannels",     pSetup->iMidiChannels);
	m_settings.setValue("/AlsaName",         pSetup->sAlsaName);
	m_settings.setValue("/AudioDriver",      pSetup->sAudioDriver);
	m_settings.setValue("/AudioDevice",      pSetup->sAudioDevice);
	m_settings.setValue("/JackName",         pSetup->sJackName);
	m_settings.setValue("/JackAutoConnect",  pSetup->bJackAutoConnect);
	m_settings.setValue("/JackMulti",        pSetup->bJackMulti);
	m_settings.setValue("/AudioChannels",    pSetup->iAudioChannels);
	m_settings.setValue("/AudioGroups",      pSetup->iAudioGroups);
	m_settings.setValue("/AudioBufSize",     pSetup->iAudioBufSize);
	m_settings.setValue("/AudioBufCount",    pSetup->iAudioBufCount);
	m_settings.setValue("/SampleFormat",     pSetup->sSampleFormat);
	m_settings.setValue("/SampleRate",       pSetup->fSampleRate);
	m_settings.setValue("/Polyphony",        pSetup->iPolyphony);
	m_settings.setValue("/ReverbActive",     pSetup->bReverbActive);
	m_settings.setValue("/ReverbRoom",       pSetup->fReverbRoom);
	m_settings.setValue("/ReverbDamp",       pSetup->fReverbDamp);
	m_settings.setValue("/ReverbWidth",      pSetup->fReverbWidth);
	m_settings.setValue("/ReverbLevel",      pSetup->fReverbLevel);
	m_settings.setValue("/ChorusActive",     pSetup->bChorusActive);
	m_settings.setValue("/ChorusNr",         pSetup->iChorusNr);
	m_settings.setValue("/ChorusLevel",      pSetup->fChorusLevel);
	m_settings.setValue("/ChorusSpeed",      pSetup->fChorusSpeed);
	m_settings.setValue("/ChorusDepth",      pSetup->fChorusDepth);
	m_settings.setValue("/ChorusType",       pSetup->iChorusType);
	m_settings.setValue("/LadspaActive",     pSetup->bLadspaActive);
	m_settings.setValue("/Gain",             pSetup->fGain);
	m_settings.setValue("/Server",           pSetup->bServer);
	m_settings.setValue("/MidiDump",         pSetup->bMidiDump);
	m_settings.setValue("/Verbose",          pSetup->bVerbose);
	m_settings.endGroup();

	// Done with the key group?
	if (!sName.isEmpty())
		m_settings.endGroup();
}


//---------------------------------------------------------------------------
// Preset management methods.

bool qsynthOptions::loadPreset ( qsynthEngine *pEngine, const QString& sPreset )
{
	if (pEngine == NULL || pEngine->pSynth == NULL)
		return false;

	qsynthSetup *pSetup = pEngine->setup();
	if (pSetup == NULL)
		return false;

	QString sSuffix;
	if (sPreset != pSetup->sDefPresetName && !sPreset.isEmpty()) {
		sSuffix = '/' + sPreset;
		// Check if on list.
		if (!pSetup->presets.contains(sPreset))
			return false;
	}

	// Begin at key group?
	if (!pEngine->isDefault())
		m_settings.beginGroup("/Engine/" + pEngine->name());

	// Load as current presets.
	const QString sPrefix = "/Chan%1";
	m_settings.beginGroup("/Preset" + sSuffix);
	int iChannels = ::fluid_synth_count_midi_channels(pEngine->pSynth);
	for (int iChan = 0; iChan < iChannels; iChan++) {
		QString sEntry = m_settings.value(sPrefix.arg(iChan + 1)).toString();
		if (!sEntry.isEmpty() && iChan == sEntry.section(':', 0, 0).toInt()) {
			::fluid_synth_bank_select(pEngine->pSynth, iChan, sEntry.section(':', 1, 1).toInt());
			::fluid_synth_program_change(pEngine->pSynth, iChan, sEntry.section(':', 2, 2).toInt());
		}
	}
	m_settings.endGroup();

	// Done with the key group?
	if (!pEngine->isDefault())
		m_settings.endGroup();

	// Recommended to post-stabilize things around.
	::fluid_synth_program_reset(pEngine->pSynth);

	return true;
}

bool qsynthOptions::savePreset ( qsynthEngine *pEngine, const QString& sPreset )
{
	if (pEngine == NULL || pEngine->pSynth == NULL)
		return false;

	qsynthSetup *pSetup = pEngine->setup();
	if (pSetup == NULL)
		return false;

	QString sSuffix;
	if (sPreset != pSetup->sDefPresetName && !sPreset.isEmpty()) {
		sSuffix = '/' + sPreset;
		// Append to list if not already.
		if (!pSetup->presets.contains(sPreset))
			pSetup->presets.prepend(sPreset);
	}

	// Begin at key group?
	if (!pEngine->isDefault())
		m_settings.beginGroup("/Engine/" + pEngine->name());

	// Unload current presets.
	const QString sPrefix = "/Chan%1";
	m_settings.beginGroup("/Preset" + sSuffix);
	int iChannels = ::fluid_synth_count_midi_channels(pEngine->pSynth);
	int iChan = 0;
	for ( ; iChan < iChannels; iChan++) {
		fluid_preset_t *pPreset = ::fluid_synth_get_channel_preset(pEngine->pSynth, iChan);
		if (pPreset) {
			int iBank = pPreset->get_banknum(pPreset);
#ifdef CONFIG_FLUID_BANK_OFFSET
			iBank += ::fluid_synth_get_bank_offset(pEngine->pSynth, (pPreset->sfont)->id);
#endif
			QString sEntry = QString::number(iChan);
			sEntry += ':';
			sEntry += QString::number(iBank);
			sEntry += ':';
			sEntry += QString::number(pPreset->get_num(pPreset));
			m_settings.setValue(sPrefix.arg(iChan + 1), sEntry);
		}
	}
	// Cleanup old entries, if any...
	while (!m_settings.value(sPrefix.arg(++iChan)).toString().isEmpty())
		m_settings.remove(sPrefix.arg(iChan));
	m_settings.endGroup();

	// Done with the key group?
	if (!pEngine->isDefault())
		m_settings.endGroup();

	return true;
}

bool qsynthOptions::deletePreset ( qsynthEngine *pEngine, const QString& sPreset )
{
	if (pEngine == NULL)
		return false;

	qsynthSetup *pSetup = pEngine->setup();
	if (pSetup == NULL)
		return false;

	QString sPrefix;
	if (!pEngine->isDefault())
		sPrefix = "/Engine/" + pEngine->name();
	QString sSuffix;
	if (sPreset != pSetup->sDefPresetName && !sPreset.isEmpty()) {
		sSuffix = "/" + sPreset;
		int iPreset = pSetup->presets.indexOf(sPreset);
		if (iPreset < 0)
			return false;
		pSetup->presets.removeAt(iPreset);
		m_settings.remove(sPrefix + "/Preset" + sSuffix);
	}

	return true;
}


//---------------------------------------------------------------------------
// Combo box history persistence helper implementation.

void qsynthOptions::loadComboBoxHistory ( QComboBox *pComboBox, int iLimit )
{
	// Load combobox list from configuration settings file...
	m_settings.beginGroup("/History/" + pComboBox->objectName());

	if (m_settings.childKeys().count() > 0) {
		pComboBox->setUpdatesEnabled(false);
		pComboBox->setDuplicatesEnabled(false);
		pComboBox->clear();
		for (int i = 0; i < iLimit; i++) {
			const QString& sText = m_settings.value(
				"/Item" + QString::number(i + 1)).toString();
			if (sText.isEmpty())
				break;
			pComboBox->addItem(sText);
		}
		pComboBox->setUpdatesEnabled(true);
	}

	m_settings.endGroup();
}


void qsynthOptions::saveComboBoxHistory ( QComboBox *pComboBox, int iLimit )
{
	// Add current text as latest item...
	const QString& sCurrentText = pComboBox->currentText();
	int iCount = pComboBox->count();
	for (int i = 0; i < iCount; i++) {
		const QString& sText = pComboBox->itemText(i);
		if (sText == sCurrentText) {
			pComboBox->removeItem(i);
			iCount--;
			break;
		}
	}
	while (iCount >= iLimit)
		pComboBox->removeItem(--iCount);
	pComboBox->insertItem(0, sCurrentText);
	iCount++;

	// Save combobox list to configuration settings file...
	m_settings.beginGroup("/History/" + pComboBox->objectName());
	for (int i = 0; i < iCount; i++) {
		const QString& sText = pComboBox->itemText(i);
		if (sText.isEmpty())
			break;
		m_settings.setValue("/Item" + QString::number(i + 1), sText);
	}
	m_settings.endGroup();
}


//---------------------------------------------------------------------------
// Widget geometry persistence helper methods.

void qsynthOptions::loadWidgetGeometry ( QWidget *pWidget, bool bMinimized )
{
	// Try to restore old form window positioning.
	if (pWidget) {
		QPoint fpos;
		QSize  fsize;
		bool bVisible;
		m_settings.beginGroup("/Geometry/" + pWidget->objectName());
		fpos.setX(m_settings.value("/x", -1).toInt());
		fpos.setY(m_settings.value("/y", -1).toInt());
		fsize.setWidth(m_settings.value("/width", -1).toInt());
		fsize.setHeight(m_settings.value("/height", -1).toInt());
		bVisible = m_settings.value("/visible", false).toBool();
		m_settings.endGroup();
		if (fpos.x() > 0 && fpos.y() > 0)
			pWidget->move(fpos);
		if (fsize.width() > 0 && fsize.height() > 0)
			pWidget->resize(fsize);
		else
			pWidget->adjustSize();
		if (bVisible && !bMinimized)
			pWidget->show();
		else
			pWidget->hide();
	}
}


void qsynthOptions::saveWidgetGeometry ( QWidget *pWidget, bool bMinimized )
{
	// Try to save form window position...
	// (due to X11 window managers ideossincrasies, we better
	// only save the form geometry while its up and visible)
	if (pWidget) {
		m_settings.beginGroup("/Geometry/" + pWidget->objectName());
		bool bVisible = pWidget->isVisible();
		const QPoint& fpos  = pWidget->pos();
		const QSize&  fsize = pWidget->size();
		m_settings.setValue("/x", fpos.x());
		m_settings.setValue("/y", fpos.y());
		m_settings.setValue("/width", fsize.width());
		m_settings.setValue("/height", fsize.height());
		m_settings.setValue("/visible", bVisible && !bMinimized);
		m_settings.endGroup();
	}
}


// end of qsynthOptions.cpp
