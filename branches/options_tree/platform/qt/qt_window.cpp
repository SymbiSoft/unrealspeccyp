/*
Portable ZX-Spectrum emulator.
Copyright (C) 2001-2013 SMT, Dexus, Alone Coder, deathsoft, djdron, scor

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifdef USE_QT

#include <QMenu>
#include <QMenuBar>
#include <QVBoxLayout>
#include <QFileDialog>
#include <QResizeEvent>

#include "qt_window.h"
#include "qt_control.h"
#include "qt_view.h"
#include "../platform.h"
#include "../../options_common.h"

namespace xPlatform
{

OPTION_USING(eOptionBool, op_tape_fast);
OPTION_USING(eOptionInt, op_skip_frame);
OPTION_USING(eOptionB, op_save_state);
OPTION_USING(eOptionB, op_load_state);

//=============================================================================
//	eWindow::eWindow
//-----------------------------------------------------------------------------
eWindow::eWindow(QWidget* parent) : QMainWindow(parent)
{
	setWindowTitle(Handler()->WindowCaption());

#ifndef Q_WS_S60
	QMenu* menu = menuBar()->addMenu(tr("&File"));
	QMenu* dmenu = menuBar()->addMenu(tr("&Device"));
#else//Q_WS_S60
	QAction* acontrol = new QAction(this);
	acontrol->setText(tr("Control"));
	acontrol->setSoftKeyRole(QAction::NegativeSoftKey);
	connect(acontrol, SIGNAL(triggered()), this, SLOT(OnControlToggle()));
	this->addAction(acontrol);
	QMenuBar* menu = menuBar();
	QMenuBar* dmenu = menu;
#endif//Q_WS_S60

	QAction* aopen = new QAction(tr("&Open..."), this);
	aopen->setShortcuts(QKeySequence::Open);
	aopen->setStatusTip(tr("Open file"));
	connect(aopen, SIGNAL(triggered()), this, SLOT(OnOpenFile()));
	menu->addAction(aopen);

	QAction* asave = new QAction(tr("&Save state"), this);
	asave->setShortcuts(QKeySequence::Save);
	connect(asave, SIGNAL(triggered()), this, SLOT(OnSaveState()));
	menu->addAction(asave);

	QAction* aload = new QAction(tr("&Load state"), this);
	aload->setShortcuts(QKeySequence::Undo);
	connect(aload, SIGNAL(triggered()), this, SLOT(OnLoadState()));
	menu->addAction(aload);

	QAction* ajoy_kempston = new QAction(tr("&Kempston"), this);
	QAction* ajoy_cursor = new QAction(tr("&Cursor"), this);
	QAction* ajoy_qaop = new QAction(tr("&QAOP"), this);
	QAction* ajoy_sinclair = new QAction(tr("&Sinclair"), this);
	{
		QMenu* jmenu = dmenu->addMenu(tr("&Joystick"));
		ajoy_kempston->setCheckable(true);
		ajoy_cursor->setCheckable(true);
		ajoy_qaop->setCheckable(true);
		ajoy_sinclair->setCheckable(true);
		connect(ajoy_kempston, SIGNAL(triggered()), this, SLOT(OnJoyKempston()));
		connect(ajoy_cursor, SIGNAL(triggered()), this, SLOT(OnJoyCursor()));
		connect(ajoy_qaop, SIGNAL(triggered()), this, SLOT(OnJoyQAOP()));
		connect(ajoy_sinclair, SIGNAL(triggered()), this, SLOT(OnJoySinclair()));
		jmenu->addAction(ajoy_kempston);
		jmenu->addAction(ajoy_cursor);
		jmenu->addAction(ajoy_qaop);
		jmenu->addAction(ajoy_sinclair);
		QActionGroup* gjoy = new QActionGroup(this);
		gjoy->addAction(ajoy_kempston);
		gjoy->addAction(ajoy_cursor);
		gjoy->addAction(ajoy_qaop);
		gjoy->addAction(ajoy_sinclair);
	}
	QAction* asnd_beeper = new QAction(tr("&Beeper"), this);
	QAction* asnd_ay = new QAction(tr("&AY"), this);
	QAction* asnd_tape = new QAction(tr("&Tape"), this);
	{
		QMenu* smenu = dmenu->addMenu(tr("&Sound"));
		asnd_beeper->setCheckable(true);
		asnd_ay->setCheckable(true);
		asnd_tape->setCheckable(true);
		connect(asnd_beeper, SIGNAL(triggered()), this, SLOT(OnSndBeeper()));
		connect(asnd_ay, SIGNAL(triggered()), this, SLOT(OnSndAY()));
		connect(asnd_tape, SIGNAL(triggered()), this, SLOT(OnSndTape()));
		smenu->addAction(asnd_beeper);
		smenu->addAction(asnd_ay);
		smenu->addAction(asnd_tape);
		QActionGroup* gsnd = new QActionGroup(this);
		gsnd->addAction(asnd_beeper);
		gsnd->addAction(asnd_ay);
		gsnd->addAction(asnd_tape);
	}

	QAction* atape_fast = new QAction(tr("&Fast mode"), this);
	{
		QMenu* tmenu = dmenu->addMenu(tr("&Tape"));
		QAction* atape_toggle = new QAction(tr("&Start/stop"), this);
		connect(atape_toggle, SIGNAL(triggered()), this, SLOT(OnTapeToggle()));
		tmenu->addAction(atape_toggle);

		atape_fast->setCheckable(true);
		connect(atape_fast, SIGNAL(triggered()), this, SLOT(OnTapeFast()));
		tmenu->addAction(atape_fast);
	}

	QAction* areset = new QAction(tr("&Reset"), this);
	areset->setShortcuts(QKeySequence::New);
	connect(areset, SIGNAL(triggered()), this, SLOT(OnReset()));
	dmenu->addAction(areset);

#ifdef Q_WS_S60
	QAction* askip_none = new QAction(tr("&None"), this);
	QAction* askip_2 = new QAction(tr("2"), this);
	QAction* askip_4 = new QAction(tr("4"), this);
	{
		QMenu* smenu = dmenu->addMenu(tr("Skip &frames"));
		askip_none->setCheckable(true);
		askip_2->setCheckable(true);
		askip_4->setCheckable(true);
		connect(askip_none, SIGNAL(triggered()), this, SLOT(OnSkipNone()));
		connect(askip_2, SIGNAL(triggered()), this, SLOT(OnSkip2()));
		connect(askip_4, SIGNAL(triggered()), this, SLOT(OnSkip4()));
		smenu->addAction(askip_none);
		smenu->addAction(askip_2);
		smenu->addAction(askip_4);
		QActionGroup* gkip = new QActionGroup(this);
		gkip->addAction(askip_none);
		gkip->addAction(askip_2);
		gkip->addAction(askip_4);
	}
#endif//Q_WS_S60

	QAction* aexit = new QAction(tr("E&xit"), this);
	aexit->setShortcuts(QKeySequence::Quit);
	connect(aexit, SIGNAL(triggered()), this, SLOT(close()));
	menu->addAction(aexit);

	QWidget* w = new QWidget;
	layout = new QVBoxLayout(w);
	layout->setMargin(0);
	layout->setSpacing(0);
	layout->addWidget(view = new eView);
	layout->addWidget(control = new eControl);
	setCentralWidget(w);
	view->setFocus();

	switch(*OPTION_GET(op_joy))
	{
	case J_KEMPSTON:	ajoy_kempston->setChecked(true);	break;
	case J_CURSOR:		ajoy_cursor->setChecked(true);		break;
	case J_QAOP:		ajoy_qaop->setChecked(true);		break;
	case J_SINCLAIR2:	ajoy_sinclair->setChecked(true);	break;
	default: break;
	}
	switch(*OPTION_GET(op_sound_source))
	{
	case S_BEEPER:		asnd_beeper->setChecked(true);		break;
	case S_AY:			asnd_ay->setChecked(true);			break;
	case S_TAPE:		asnd_tape->setChecked(true);		break;
	default: break;
	}
    xOptions::eOptionBool* ft = OPTION_GET(op_tape_fast);
	atape_fast->setChecked(ft && *ft);
#ifdef Q_WS_S60
	xOptions::eOptionInt* sf = OPTION_GET(op_skip_frames);
	if(sf)
	{
		switch(*sf)
		{
		case 0:			askip_none->setChecked(true);		break;
		case 1:			askip_2->setChecked(true);			break;
		case 2:			askip_4->setChecked(true);			break;
		default: break;
		}
	}
#endif//Q_WS_S60
}
//=============================================================================
//	eWindow::OnOpenFile
//-----------------------------------------------------------------------------
void eWindow::OnOpenFile()
{
	view->Paused(true);
	QString name = QFileDialog::getOpenFileName(this, tr("Open file"), OpLastFolder(),
		tr(	"Supported files (*.sna *.z80 *.szx *.rzx *.trd *.scl *.fdi *.tap *.csw *.tzx *.zip);;"
			"All files (*.*);;"
			"Snapshot files (*.sna *.z80 *.szx);;"
			"Replay files (*.rzx);;"
			"Disk images (*.trd *.scl *.fdi);;"
			"Tape files (*.tap *.csw *.tzx);;"
			"ZIP archives (*.zip)"
			));

	view->Paused(false);
	if(!name.isEmpty())
		Handler()->OnOpenFile(name.toUtf8().data());
}
//=============================================================================
//	eWindow::resizeEvent
//-----------------------------------------------------------------------------
void eWindow::resizeEvent(QResizeEvent* event)
{
	control->setVisible(event->size().width() < event->size().height());
}
//=============================================================================
//	eWindow::OnReset
//-----------------------------------------------------------------------------
void eWindow::OnReset()
{
	Handler()->OnAction(A_RESET);
}
//=============================================================================
//	eWindow::OnControlToggle
//-----------------------------------------------------------------------------
void eWindow::OnControlToggle()
{
	control->ToggleKeyboard();
}
//=============================================================================
//	eWindow::OnSaveState
//-----------------------------------------------------------------------------
void eWindow::OnSaveState()
{
	using namespace xOptions;
	SAFE_CALL(OPTION_GET(op_save_state))->Change();
}
//=============================================================================
//	eWindow::OnLoadState
//-----------------------------------------------------------------------------
void eWindow::OnLoadState()
{
	using namespace xOptions;
	SAFE_CALL(OPTION_GET(op_load_state))->Change();
}
//=============================================================================
//	eWindow::OnJoy*
//-----------------------------------------------------------------------------
void eWindow::OnJoyKempston()	{ OPTION_GET(op_joy)->Set(J_KEMPSTON);	}
void eWindow::OnJoyCursor()		{ OPTION_GET(op_joy)->Set(J_CURSOR);	}
void eWindow::OnJoyQAOP()		{ OPTION_GET(op_joy)->Set(J_QAOP);		}
void eWindow::OnJoySinclair()	{ OPTION_GET(op_joy)->Set(J_SINCLAIR2);	}
void eWindow::OnSndBeeper()		{ OPTION_GET(op_sound_source)->Set(S_BEEPER); }
void eWindow::OnSndAY()			{ OPTION_GET(op_sound_source)->Set(S_AY); }
void eWindow::OnSndTape()		{ OPTION_GET(op_sound_source)->Set(S_TAPE); }

//=============================================================================
//	eWindow::OnTapeToggle
//-----------------------------------------------------------------------------
void eWindow::OnTapeToggle()
{
	Handler()->OnAction(A_TAPE_TOGGLE);
}
//=============================================================================
//	eWindow::OnTapeFast
//-----------------------------------------------------------------------------
void eWindow::OnTapeFast()
{
    SAFE_CALL(OPTION_GET(op_tape_fast))->Change();
}
#ifdef Q_WS_S60
//=============================================================================
//	eWindow::SetSkipFrames
//-----------------------------------------------------------------------------
void eWindow::SetSkipFrames(int sf)
{
	SAFE_CALL(OPTION_GET(op_skip_frames))->Set(sf);
}
void eWindow::OnSkipNone()	{ SetSkipFrames(0);	}
void eWindow::OnSkip2()		{ SetSkipFrames(1);	}
void eWindow::OnSkip4()		{ SetSkipFrames(2);	}
#endif//Q_WS_S60

}
//namespace xPlatform

#endif//USE_QT
