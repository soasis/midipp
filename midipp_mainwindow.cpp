/*-
 * Copyright (c) 2009-2010 Hans Petter Selasky. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

#include <midipp_mainwindow.h>

#include <midipp_scores.h>

uint8_t
MppMainWindow :: noise8(uint8_t factor)
{
	uint32_t temp;
	const uint32_t prime = 0xFFFF1D;

	if (factor == 0)
		return (0);

	if (noiseRem & 1)
		noiseRem += prime;

	noiseRem /= 2;

	temp = noiseRem * factor;

	return (temp >> 24);
}

MppMainWindow :: MppMainWindow(QWidget *parent)
  : QWidget(parent)
{
	int n;
	int x;

	/* set memory default */

	memset(auto_zero_start, 0, auto_zero_end - auto_zero_start);

	CurrMidiFileName = NULL;
	song = NULL;
	track = NULL;

	umidi20_mutex_init(&mtx);

	noiseRem = 1;

	defaultFont.fromString(QString("Sans Serif,12,-1,5,75,0,0,0,0,0"));

	/* Setup GUI */

	main_gl = new QGridLayout(this);
	main_tw = new QTabWidget();
	scores_tw = new QTabWidget();

	/* Watchdog */

	watchdog = new QTimer(this);
	connect(watchdog, SIGNAL(timeout()), this, SLOT(handle_watchdog()));

	/* Tabs */

	main_gl->addWidget(scores_tw,0,0,1,1);
	main_gl->addWidget(main_tw,0,1,1,1);

	for (x = 0; x != MPP_MAX_VIEWS; x++)
		scores_main[x] = new MppScoreMain(this);

	currScoreMain = scores_main[0];

	tab_file_wg = new QWidget();
	tab_play_wg = new QWidget();
	tab_edit_wg = new QWidget();
	tab_config_wg = new QWidget();
	tab_instr_wg = new QWidget();
	tab_volume_wg = new QWidget();

	tab_file_gl = new QGridLayout(tab_file_wg);
	tab_play_gl = new QGridLayout(tab_play_wg);
	tab_edit_gl = new QGridLayout(tab_edit_wg);
	tab_config_gl = new QGridLayout(tab_config_wg);
	tab_instr_gl = new QGridLayout(tab_instr_wg);
	tab_volume_gl = new QGridLayout(tab_volume_wg);

	scores_tw->addTab(scores_main[0]->viewWidget, tr("View A-Scores"));
	scores_tw->addTab(scores_main[0]->editWidget, tr("Edit A-Scores"));

	scores_tw->addTab(scores_main[1]->viewWidget, tr("View B-Scores"));
	scores_tw->addTab(scores_main[1]->editWidget, tr("Edit B-Scores"));

	main_tw->addTab(tab_file_wg, tr("File"));
	main_tw->addTab(tab_play_wg, tr("Play"));
#if 0
	main_tw->addTab(tab_edit_wg, tr("Edit"));
#endif
	main_tw->addTab(tab_config_wg, tr("Config"));
	main_tw->addTab(tab_instr_wg, tr("Instrument"));
	main_tw->addTab(tab_volume_wg, tr("Volume"));

	/* <File> Tab */

	lbl_score_Afile = new QLabel(tr("- A-Score File -"));

	lbl_score_Bfile = new QLabel(tr("- B-Score File -"));

	lbl_midi_file = new QLabel(tr("- MIDI File -"));

	lbl_file_status = new QLabel(QString());

	but_quit = new QPushButton(tr("Quit"));

	but_midi_file_new = new QPushButton(tr("New"));
	but_midi_file_open = new QPushButton(tr("Open"));
	but_midi_file_merge = new QPushButton(tr("Merge"));
	but_midi_file_save = new QPushButton(tr("Save"));
	but_midi_file_save_as = new QPushButton(tr("Save As"));
	but_midi_file_convert = new QPushButton(tr("ToScores"));

	n = 0;

	tab_file_gl->addWidget(lbl_score_Afile, n, 0, 1, 3, Qt::AlignHCenter|Qt::AlignVCenter);
	tab_file_gl->addWidget(lbl_score_Bfile, n, 3, 1, 3, Qt::AlignHCenter|Qt::AlignVCenter);

	n++;

	tab_file_gl->addWidget(scores_main[0]->butScoreFileNew, n, 0, 1, 3);
	tab_file_gl->addWidget(scores_main[1]->butScoreFileNew, n, 3, 1, 3);

	n++;

	tab_file_gl->addWidget(scores_main[0]->butScoreFileOpen, n, 0, 1, 3);
	tab_file_gl->addWidget(scores_main[1]->butScoreFileOpen, n, 3, 1, 3);

	n++;

	tab_file_gl->addWidget(scores_main[0]->butScoreFileSave, n, 0, 1, 3);
	tab_file_gl->addWidget(scores_main[1]->butScoreFileSave, n, 3, 1, 3);

	n++;

	tab_file_gl->addWidget(scores_main[0]->butScoreFileSaveAs, n, 0, 1, 3);
	tab_file_gl->addWidget(scores_main[1]->butScoreFileSaveAs, n, 3, 1, 3);

	n++;

	tab_file_gl->addWidget(scores_main[0]->butScoreFilePrint, n, 0, 1, 3);
	tab_file_gl->addWidget(scores_main[1]->butScoreFilePrint, n, 3, 1, 3);

	n++;
	n++;

	tab_file_gl->addWidget(lbl_file_status, n, 0, 1, 8, Qt::AlignLeft|Qt::AlignVCenter);

	n++;

	tab_file_gl->setRowStretch(n, 3);

	n++;

	tab_file_gl->addWidget(but_quit, n, 0, 1, 8);

	n = 0;

	tab_file_gl->addWidget(lbl_midi_file, n, 6, 1, 2, Qt::AlignHCenter|Qt::AlignVCenter);

	n++;

	tab_file_gl->addWidget(but_midi_file_new, n, 6, 1, 2);

	n++;

	tab_file_gl->addWidget(but_midi_file_open, n, 6, 1, 2);

	n++;

	tab_file_gl->addWidget(but_midi_file_merge, n, 6, 1, 2);

	n++;

	tab_file_gl->addWidget(but_midi_file_save, n, 6, 1, 2);

	n++;

	tab_file_gl->addWidget(but_midi_file_save_as, n, 6, 1, 2);

	n++;

	tab_file_gl->addWidget(but_midi_file_convert, n, 6, 1, 2);

	n++;

	/* <Play> Tab */

	lbl_auto_play = new QLabel(tr("Auto Play BPM (0..6000)"));
	spn_auto_play = new QSpinBox();
	spn_auto_play->setMaximum(6000);
	spn_auto_play->setMinimum(0);
	connect(spn_auto_play, SIGNAL(valueChanged(int)), this, SLOT(handle_auto_play(int)));
	spn_auto_play->setValue(0);

	lbl_bpm_max = new QLabel(tr("Max"));
	lbl_bpm_min = new QLabel(tr("Min"));
	lbl_bpm_avg = new QLabel(tr("Average BPM"));

	lbl_curr_time_val = new QLCDNumber(8);
	lbl_curr_time_val->setMode(QLCDNumber::Dec);
	lbl_curr_time_val->setFrameShape(QLCDNumber::NoFrame);
	lbl_curr_time_val->setSegmentStyle(QLCDNumber::Flat);

	lbl_bpm_min_val = new QLCDNumber(4);
	lbl_bpm_min_val->setMode(QLCDNumber::Dec);
	lbl_bpm_min_val->setFrameShape(QLCDNumber::NoFrame);
	lbl_bpm_min_val->setSegmentStyle(QLCDNumber::Flat);

	lbl_bpm_avg_val = new QLCDNumber(4);
	lbl_bpm_avg_val->setMode(QLCDNumber::Dec);
	lbl_bpm_avg_val->setFrameShape(QLCDNumber::NoFrame);
	lbl_bpm_avg_val->setSegmentStyle(QLCDNumber::Flat);

	lbl_bpm_max_val = new QLCDNumber(4);
	lbl_bpm_max_val->setMode(QLCDNumber::Dec);
	lbl_bpm_max_val->setFrameShape(QLCDNumber::NoFrame);
	lbl_bpm_max_val->setSegmentStyle(QLCDNumber::Flat);

	lbl_score_record = new QLabel(QString());
	lbl_midi_record = new QLabel(QString());
	lbl_midi_pass_thru = new QLabel(QString());
	lbl_midi_play = new QLabel(QString());

	but_jump[0] = new QPushButton(tr("J0"));
	but_jump[1] = new QPushButton(tr("J1"));
	but_jump[2] = new QPushButton(tr("J2"));
	but_jump[3] = new QPushButton(tr("J3"));
	but_jump[4] = new QPushButton(tr("J4"));
	but_jump[5] = new QPushButton(tr("J5"));
	but_jump[6] = new QPushButton(tr("J6"));
	but_jump[7] = new QPushButton(tr("J7"));

	but_midi_pass_thru = new QPushButton(tr("Pass Thru"));
	but_compile = new QPushButton(tr("Compile"));
	but_score_record = new QPushButton(tr("Scores"));
	but_midi_record = new QPushButton(tr("MIDI"));

	but_midi_play = new QPushButton(tr("MIDI"));
	but_midi_pause = new QPushButton(tr("Pause"));
	but_midi_trigger = new QPushButton(tr("Trigger"));
	but_midi_rewind = new QPushButton(tr("Rewind"));

	but_play = new QPushButton(tr(" \nPlay\n "));

	lbl_volume = new QLabel(tr("Volume (0..127..511)"));
	spn_volume = new QSpinBox();
	connect(spn_volume, SIGNAL(valueChanged(int)), this, SLOT(handle_volume_changed(int)));
	spn_volume->setMaximum(511);
	spn_volume->setMinimum(0);

	lbl_play_key = new QLabel(QString());
	spn_play_key = new QSpinBox();
	connect(spn_play_key, SIGNAL(valueChanged(int)), this, SLOT(handle_play_key_changed(int)));
	spn_play_key->setMaximum(127);
	spn_play_key->setMinimum(0);
	spn_play_key->setValue(C4);

	lbl_time_counter = new QLabel(tr(" - Time Counter -"));
	lbl_synth = new QLabel(tr("- Synth Play -"));
	lbl_playback = new QLabel(tr("- Playback -"));
	lbl_recording = new QLabel(tr("- Recording -"));
	lbl_scores = new QLabel(tr("- Scores -"));

	n = 0;

	tab_play_gl->addWidget(lbl_time_counter, n, 0, 1, 4, Qt::AlignHCenter|Qt::AlignVCenter);

	n++;

	tab_play_gl->addWidget(lbl_curr_time_val, n, 0, 1, 4, Qt::AlignHCenter|Qt::AlignVCenter);

	n++;

	tab_play_gl->addWidget(lbl_playback, n, 0, 1, 4, Qt::AlignHCenter|Qt::AlignVCenter);

	n++;

	tab_play_gl->addWidget(lbl_midi_play, n, 3, 1, 1);
	tab_play_gl->addWidget(but_midi_play, n, 0, 1, 3);

	n++;

	tab_play_gl->addWidget(but_midi_pause, n, 0, 1, 4);

	n++;

	tab_play_gl->addWidget(but_midi_trigger, n, 0, 1, 4);

	n++;

	tab_play_gl->addWidget(but_midi_rewind, n, 0, 1, 4);

	n++;

	tab_play_gl->addWidget(lbl_scores, n, 0, 1, 4, Qt::AlignHCenter|Qt::AlignVCenter);

	n++;

	tab_play_gl->addWidget(but_compile, n, 0, 1, 4);

	n++;

	tab_play_gl->addWidget(lbl_recording, n, 0, 1, 4, Qt::AlignHCenter|Qt::AlignVCenter);

	n++;

	tab_play_gl->addWidget(lbl_midi_pass_thru, n, 3, 1, 1);
	tab_play_gl->addWidget(but_midi_pass_thru, n, 0, 1, 3);

	n++;

	tab_play_gl->addWidget(lbl_score_record, n, 3, 1, 1);
	tab_play_gl->addWidget(but_score_record, n, 0, 1, 3);

	n++;

	tab_play_gl->addWidget(lbl_midi_record, n, 3, 1, 1);
	tab_play_gl->addWidget(but_midi_record, n, 0, 1, 3);

	n = 0;

	tab_play_gl->addWidget(lbl_synth, n, 4, 1, 4, Qt::AlignHCenter|Qt::AlignVCenter);

	n++;

	tab_play_gl->addWidget(lbl_volume, n, 4, 1, 3);
	tab_play_gl->addWidget(spn_volume, n, 7, 1, 1);

	n++;

	tab_play_gl->addWidget(lbl_play_key, n, 4, 1, 3);
	tab_play_gl->addWidget(spn_play_key, n, 7, 1, 1);

	n++;

	tab_play_gl->addWidget(lbl_auto_play, n, 4, 1, 3);
	tab_play_gl->addWidget(spn_auto_play, n, 7, 1, 1);

	n++;

	tab_play_gl->addWidget(but_jump[4], n, 5, 1, 1);
	tab_play_gl->addWidget(but_jump[0], n, 4, 1, 1);

	n++;

	tab_play_gl->addWidget(but_jump[5], n, 5, 1, 1);
	tab_play_gl->addWidget(but_jump[1], n, 4, 1, 1);

	n++;

	tab_play_gl->addWidget(but_jump[6], n, 5, 1, 1);
	tab_play_gl->addWidget(but_jump[2], n, 4, 1, 1);

	n++;

	tab_play_gl->addWidget(but_jump[7], n, 5, 1, 1);
	tab_play_gl->addWidget(but_jump[3], n, 4, 1, 1);

	n++;

	tab_play_gl->addWidget(lbl_bpm_max, n, 4, 1, 1, Qt::AlignHCenter|Qt::AlignVCenter);
	tab_play_gl->addWidget(lbl_bpm_avg, n, 5, 1, 2, Qt::AlignHCenter|Qt::AlignVCenter);
	tab_play_gl->addWidget(lbl_bpm_min, n, 7, 1, 1, Qt::AlignHCenter|Qt::AlignVCenter);

	n++;

	tab_play_gl->addWidget(lbl_bpm_max_val, n, 4, 1, 1, Qt::AlignHCenter|Qt::AlignVCenter);
	tab_play_gl->addWidget(lbl_bpm_avg_val, n, 5, 1, 2, Qt::AlignHCenter|Qt::AlignVCenter);
	tab_play_gl->addWidget(lbl_bpm_min_val, n, 7, 1, 1, Qt::AlignHCenter|Qt::AlignVCenter);

	n++;

	tab_play_gl->addWidget(but_play, n, 4, 3, 4);

	n += 3;

	tab_play_gl->setRowStretch(n, 4);

	/* <Configuration> Tab */

	lbl_cmd_key = new QLabel(QString());
	spn_cmd_key = new QSpinBox();
	connect(spn_cmd_key, SIGNAL(valueChanged(int)), this, SLOT(handle_cmd_key_changed(int)));
	spn_cmd_key->setMaximum(127);
	spn_cmd_key->setMinimum(0);
	spn_cmd_key->setValue(C3);

	lbl_base_key = new QLabel(QString());
	spn_base_key = new QSpinBox();
	connect(spn_base_key, SIGNAL(valueChanged(int)), this, SLOT(handle_base_key_changed(int)));
	spn_base_key->setMaximum(127);
	spn_base_key->setMinimum(0);
	spn_base_key->setValue(C4);

	lbl_config_title = new QLabel(tr("- Device configuration -"));
	lbl_config_play = new QLabel(tr("Play"));
	lbl_config_rec = new QLabel(tr("Rec."));
	lbl_config_synth = new QLabel(tr("Synth"));
	lbl_bpm_count = new QLabel(tr("BPM average length (0..32)"));

	lbl_key_delay = new QLabel(tr("Random Key Delay (0..255)"));
	spn_key_delay = new QSpinBox();

	connect(spn_key_delay, SIGNAL(valueChanged(int)), this, SLOT(handle_key_delay_changed(int)));
	spn_key_delay->setMaximum(255);
	spn_key_delay->setMinimum(0);
	spn_key_delay->setValue(0);
	spn_key_delay->setSuffix(tr(" ms"));

	spn_bpm_length = new QSpinBox();
	spn_bpm_length->setMaximum(MPP_MAX_BPM);
	spn_bpm_length->setMinimum(0);
	spn_bpm_length->setValue(0);

	lbl_parse_thres = new QLabel(tr("MIDI To Scores Threshold (0..255)"));
	spn_parse_thres = new QSpinBox();
	spn_parse_thres->setMaximum(255);
	spn_parse_thres->setMinimum(0);
	spn_parse_thres->setValue(30);
	spn_parse_thres->setSuffix(tr(" ms"));

	lbl_fixed_key = new QLabel(tr("Fixed Play Key"));
	cbx_fixed_key = new QCheckBox();

	lbl_config_local = new QLabel(tr("Enable local MIDI on synth"));
	cbx_config_local = new QCheckBox();
	connect(cbx_config_local, SIGNAL(stateChanged(int)), this, SLOT(handle_config_local_changed(int)));
	cbx_config_local->setCheckState(Qt::Checked);

	but_config_apply = new QPushButton(tr("Apply"));
	but_config_revert = new QPushButton(tr("Revert"));
	but_config_fontsel = new QPushButton(tr("Select Font"));

	x = 0;

	tab_config_gl->addWidget(lbl_config_title, x, 0, 1, 5, Qt::AlignHCenter|Qt::AlignVCenter);
	tab_config_gl->addWidget(lbl_config_play, x, 5, 1, 1, Qt::AlignHCenter|Qt::AlignVCenter);
	tab_config_gl->addWidget(lbl_config_rec, x, 6, 1, 1, Qt::AlignHCenter|Qt::AlignVCenter);
	tab_config_gl->addWidget(lbl_config_synth, x, 7, 1, 1, Qt::AlignHCenter|Qt::AlignVCenter);

	x++;

	for (n = 0; n != MPP_MAX_DEVS; n++) {
		char buf[16];

		led_config_dev[n] = new QLineEdit(QString());
		cbx_config_dev[(3*n)+0] = new QCheckBox();
		cbx_config_dev[(3*n)+1] = new QCheckBox();
		cbx_config_dev[(3*n)+2] = new QCheckBox();


		snprintf(buf, sizeof(buf), "Dev%d:", n);

		lbl_config_dev[n] = new QLabel(tr(buf));

		tab_config_gl->addWidget(lbl_config_dev[n], x, 0, 1, 1, Qt::AlignHCenter|Qt::AlignLeft);
		tab_config_gl->addWidget(led_config_dev[n], x, 1, 1, 4, Qt::AlignHCenter|Qt::AlignLeft);
		tab_config_gl->addWidget(cbx_config_dev[(3*n)+0], x, 5, 1, 1, Qt::AlignHCenter|Qt::AlignVCenter);
		tab_config_gl->addWidget(cbx_config_dev[(3*n)+1], x, 6, 1, 1, Qt::AlignHCenter|Qt::AlignVCenter);
		tab_config_gl->addWidget(cbx_config_dev[(3*n)+2], x, 7, 1, 1, Qt::AlignHCenter|Qt::AlignVCenter);

		x++;
	}

	tab_config_gl->addWidget(lbl_bpm_count, x, 0, 1, 6, Qt::AlignLeft|Qt::AlignVCenter);
	tab_config_gl->addWidget(spn_bpm_length, x, 6, 1, 2, Qt::AlignRight|Qt::AlignVCenter);

	x++;

	tab_config_gl->addWidget(lbl_key_delay, x, 0, 1, 6, Qt::AlignLeft|Qt::AlignVCenter);
	tab_config_gl->addWidget(spn_key_delay, x, 6, 1, 2, Qt::AlignRight|Qt::AlignVCenter);

	x++;

	tab_config_gl->addWidget(lbl_cmd_key, x, 0, 1, 7, Qt::AlignLeft|Qt::AlignVCenter);
	tab_config_gl->addWidget(spn_cmd_key, x, 7, 1, 1, Qt::AlignRight|Qt::AlignVCenter);

	x++;

	tab_config_gl->addWidget(lbl_base_key, x, 0, 1, 7, Qt::AlignLeft|Qt::AlignVCenter);
	tab_config_gl->addWidget(spn_base_key, x, 7, 1, 1, Qt::AlignRight|Qt::AlignVCenter);

	x++;

	tab_config_gl->addWidget(lbl_parse_thres, x, 0, 1, 3, Qt::AlignLeft|Qt::AlignVCenter);
	tab_config_gl->addWidget(spn_parse_thres, x, 3, 1, 1, Qt::AlignRight|Qt::AlignVCenter);

	tab_config_gl->addWidget(lbl_fixed_key, x, 4, 1, 3, Qt::AlignLeft|Qt::AlignVCenter);
	tab_config_gl->addWidget(cbx_fixed_key, x, 7, 1, 1, Qt::AlignHCenter|Qt::AlignVCenter);

	x++;

	tab_config_gl->addWidget(lbl_config_local, x, 0, 1, 7, Qt::AlignLeft|Qt::AlignVCenter);
	tab_config_gl->addWidget(cbx_config_local, x, 7, 1, 1, Qt::AlignHCenter|Qt::AlignVCenter);

	x++;

	tab_config_gl->addWidget(but_config_fontsel, x, 0, 1, 2, Qt::AlignLeft|Qt::AlignVCenter);

	x++;

	tab_config_gl->setRowStretch(x, 1);

	x++;

	tab_config_gl->addWidget(but_config_apply, x, 4, 1, 2);
	tab_config_gl->addWidget(but_config_revert, x, 6, 1, 2);

	/* <Edit> Tab */

	lbl_edit_title = new QLabel(tr("- MIDI File Edit -"));
	lbl_edit_channel = new QLabel(tr("Selected Channel (0..15):"));
	lbl_edit_transpose = new QLabel(tr("Transpose Steps (-128..127):"));
	lbl_edit_volume = new QLabel(tr("Average Volume (1..127):"));

	spn_edit_channel = new QSpinBox();
	spn_edit_channel->setMaximum(15);
	spn_edit_channel->setMinimum(0);
	spn_edit_channel->setValue(0);

	spn_edit_transpose = new QSpinBox();
	spn_edit_transpose->setMaximum(127);
	spn_edit_transpose->setMinimum(-128);
	spn_edit_transpose->setValue(0);

	spn_edit_volume = new QSpinBox();
	spn_edit_volume->setMaximum(127);
	spn_edit_volume->setMinimum(1);
	spn_edit_volume->setValue(80);

	but_edit_apply_transpose = new QPushButton(tr("Apply Channel Key Transpose"));
	but_edit_change_volume = new QPushButton(tr("Change Channel Event Volume"));
	but_edit_remove_pedal = new QPushButton(tr("Remove Channel Pedal Events"));
	but_edit_remove_keys = new QPushButton(tr("Remove Channel Key Events"));
	but_edit_remove_all = new QPushButton(tr("Remove All Channel Events"));

	n = 0;

	tab_edit_gl->addWidget(lbl_edit_title, n, 0, 1, 8, Qt::AlignHCenter|Qt::AlignVCenter);

	n++;

	tab_edit_gl->addWidget(lbl_edit_channel, n, 0, 1, 7);
	tab_edit_gl->addWidget(spn_edit_channel, n, 7, 1, 1);

	n++;

	tab_edit_gl->addWidget(lbl_edit_transpose, n, 0, 1, 7);
	tab_edit_gl->addWidget(spn_edit_transpose, n, 7, 1, 1);

	n++;

	tab_edit_gl->addWidget(lbl_edit_volume, n, 0, 1, 7);
	tab_edit_gl->addWidget(spn_edit_volume, n, 7, 1, 1);

	n++;

	tab_edit_gl->addWidget(but_edit_apply_transpose, n, 0, 1, 8);
	n++;
	tab_edit_gl->addWidget(but_edit_change_volume, n, 0, 1, 8);
	n++;
	tab_edit_gl->addWidget(but_edit_remove_pedal, n, 0, 1, 8);
	n++;
	tab_edit_gl->addWidget(but_edit_remove_keys, n, 0, 1, 8);
	n++;
	tab_edit_gl->addWidget(but_edit_remove_all, n, 0, 1, 8);
	n++;

	tab_edit_gl->setRowStretch(n, 4);

	/* <Instrument> tab */

	lbl_instr_title[0] = new QLabel(tr("- Bank/Program/Mute -"));
	lbl_instr_title[1] = new QLabel(tr("- Bank/Program/Mute -"));

	lbl_instr_prog = new QLabel(tr("- Synth/Record Channel and Selected Bank/Program -"));

	but_instr_apply = new QPushButton(tr("Apply"));
	but_instr_revert = new QPushButton(tr("Revert"));
	but_instr_program = new QPushButton(tr("Program"));
	but_instr_reset = new QPushButton(tr("Reset"));

	spn_instr_curr_chan = new QSpinBox();
	connect(spn_instr_curr_chan, SIGNAL(valueChanged(int)), this, SLOT(handle_instr_channel_changed(int)));
	spn_instr_curr_chan->setMaximum(15);
	spn_instr_curr_chan->setMinimum(0);
	spn_instr_curr_chan->setValue(0);

	spn_instr_curr_bank = new QSpinBox();
	spn_instr_curr_bank->setMaximum(16383);
	spn_instr_curr_bank->setMinimum(0);
	spn_instr_curr_bank->setValue(0);

	spn_instr_curr_prog = new QSpinBox();
	spn_instr_curr_prog->setMaximum(127);
	spn_instr_curr_prog->setMinimum(0);
	spn_instr_curr_prog->setValue(0);

	x = 0;

	tab_instr_gl->addWidget(lbl_instr_prog, x, 0, 1, 8, Qt::AlignHCenter|Qt::AlignVCenter);

	x++;

	tab_instr_gl->addWidget(spn_instr_curr_chan, x, 0, 1, 1);
	tab_instr_gl->addWidget(spn_instr_curr_bank, x, 1, 1, 1);
	tab_instr_gl->addWidget(spn_instr_curr_prog, x, 2, 1, 1);
	tab_instr_gl->addWidget(but_instr_program, x, 4, 1, 4);

	x++;

	tab_instr_gl->addWidget(lbl_instr_title[0], x, 0, 1, 4, Qt::AlignHCenter|Qt::AlignVCenter);
	tab_instr_gl->addWidget(lbl_instr_title[1], x, 4, 1, 4, Qt::AlignHCenter|Qt::AlignVCenter);

	x++;

	for (n = 0; n != 16; n++) {
		int y_off = (n & 8) ? 4 : 0;

		char buf[16];

		snprintf(buf, sizeof(buf), "Ch%X", n);

		lbl_instr_desc[n] = new QLabel(tr(buf));

		spn_instr_bank[n] = new QSpinBox();
		spn_instr_bank[n]->setMaximum(16383);
		spn_instr_bank[n]->setMinimum(0);
		spn_instr_bank[n]->setValue(0);

		spn_instr_prog[n] = new QSpinBox();
		spn_instr_prog[n]->setMaximum(127);
		spn_instr_prog[n]->setMinimum(0);
		spn_instr_prog[n]->setValue(0);

		cbx_instr_mute[n] = new QCheckBox();

		tab_instr_gl->addWidget(lbl_instr_desc[n], (n & 7) + x, 0 + y_off, 1, 1, Qt::AlignVCenter|Qt::AlignRight);
		tab_instr_gl->addWidget(spn_instr_bank[n], (n & 7) + x, 1 + y_off, 1, 1, Qt::AlignVCenter|Qt::AlignRight);
		tab_instr_gl->addWidget(spn_instr_prog[n], (n & 7) + x, 2 + y_off, 1, 1, Qt::AlignVCenter|Qt::AlignHCenter);
		tab_instr_gl->addWidget(cbx_instr_mute[n], (n & 7) + x, 3 + y_off, 1, 1, Qt::AlignVCenter|Qt::AlignHCenter);
	}

	x += 8;

	tab_instr_gl->setRowStretch(x, 4);

	x++;

	tab_instr_gl->addWidget(but_instr_reset, x, 2, 1, 2);
	tab_instr_gl->addWidget(but_instr_revert, x, 4, 1, 2);
	tab_instr_gl->addWidget(but_instr_apply, x, 6, 1, 2);

	/* <Volume> tab */

	lbl_volume_title[0] = new QLabel(tr("- Playback -"));
	lbl_volume_title[1] = new QLabel(tr("- Synth/Record -"));

	but_volume_apply = new QPushButton(tr("Apply"));
	but_volume_revert = new QPushButton(tr("Revert"));
	but_volume_reset = new QPushButton(tr("Reset"));

	x = 0;

	tab_volume_gl->addWidget(lbl_volume_title[0], x, 0, 1, 4, Qt::AlignHCenter|Qt::AlignVCenter);
	tab_volume_gl->addWidget(lbl_volume_title[1], x, 4, 1, 4, Qt::AlignHCenter|Qt::AlignVCenter);

	x++;

	for (n = 0; n != 16; n++) {
		int y_off = (n & 8) ? 2 : 0;

		char buf[16];

		snprintf(buf, sizeof(buf), "Ch%X", n);

		lbl_volume_play[n] = new QLabel(tr(buf));
		lbl_volume_synth[n] = new QLabel(tr(buf));

		spn_volume_synth[n] = new QSpinBox();
		spn_volume_synth[n]->setMaximum(511);
		spn_volume_synth[n]->setMinimum(0);
		spn_volume_synth[n]->setValue(MPP_VOLUME_UNIT);

		spn_volume_play[n] = new QSpinBox();
		spn_volume_play[n]->setMaximum(511);
		spn_volume_play[n]->setMinimum(0);
		spn_volume_play[n]->setValue(MPP_VOLUME_UNIT);

		tab_volume_gl->addWidget(lbl_volume_play[n], (n & 7) + x, 0 + y_off, 1, 1, Qt::AlignVCenter|Qt::AlignRight);
		tab_volume_gl->addWidget(spn_volume_play[n], (n & 7) + x, 1 + y_off, 1, 1, Qt::AlignVCenter|Qt::AlignHCenter);
		tab_volume_gl->addWidget(lbl_volume_synth[n], (n & 7) + x, 4 + y_off, 1, 1, Qt::AlignVCenter|Qt::AlignRight);
		tab_volume_gl->addWidget(spn_volume_synth[n], (n & 7) + x, 5 + y_off, 1, 1, Qt::AlignVCenter|Qt::AlignHCenter);
	}

	x += 8;

	tab_volume_gl->setRowStretch(x, 4);

	x++;

	tab_volume_gl->addWidget(but_volume_reset, x, 2, 1, 2);
	tab_volume_gl->addWidget(but_volume_revert, x, 4, 1, 2);
	tab_volume_gl->addWidget(but_volume_apply, x, 6, 1, 2);

	/* Connect all */

	connect(but_jump[0], SIGNAL(pressed()), this, SLOT(handle_jump_0()));
	connect(but_jump[1], SIGNAL(pressed()), this, SLOT(handle_jump_1()));
	connect(but_jump[2], SIGNAL(pressed()), this, SLOT(handle_jump_2()));
	connect(but_jump[3], SIGNAL(pressed()), this, SLOT(handle_jump_3()));
	connect(but_jump[4], SIGNAL(pressed()), this, SLOT(handle_jump_4()));
	connect(but_jump[5], SIGNAL(pressed()), this, SLOT(handle_jump_5()));
	connect(but_jump[6], SIGNAL(pressed()), this, SLOT(handle_jump_6()));
	connect(but_jump[7], SIGNAL(pressed()), this, SLOT(handle_jump_7()));

	connect(but_midi_pass_thru, SIGNAL(pressed()), this, SLOT(handle_pass_thru()));
	connect(but_compile, SIGNAL(pressed()), this, SLOT(handle_compile()));
	connect(but_score_record, SIGNAL(pressed()), this, SLOT(handle_score_record()));
	connect(but_midi_record, SIGNAL(pressed()), this, SLOT(handle_midi_record()));
	connect(but_midi_play, SIGNAL(pressed()), this, SLOT(handle_midi_play()));
	connect(but_play, SIGNAL(pressed()), this, SLOT(handle_play_press()));
	connect(but_play, SIGNAL(released()), this, SLOT(handle_play_release()));
	connect(but_quit, SIGNAL(pressed()), this, SLOT(handle_quit()));

	connect(but_midi_file_new, SIGNAL(pressed()), this, SLOT(handle_midi_file_new()));
	connect(but_midi_file_open, SIGNAL(pressed()), this, SLOT(handle_midi_file_new_open()));
	connect(but_midi_file_merge, SIGNAL(pressed()), this, SLOT(handle_midi_file_merge_open()));
	connect(but_midi_file_save, SIGNAL(pressed()), this, SLOT(handle_midi_file_save()));
	connect(but_midi_file_save_as, SIGNAL(pressed()), this, SLOT(handle_midi_file_save_as()));
	connect(but_midi_file_convert, SIGNAL(pressed()), this, SLOT(handle_midi_file_convert()));

	connect(but_midi_trigger, SIGNAL(pressed()), this, SLOT(handle_midi_trigger()));
	connect(but_midi_rewind, SIGNAL(pressed()), this, SLOT(handle_rewind()));
	connect(but_config_apply, SIGNAL(pressed()), this, SLOT(handle_config_apply()));
	connect(but_config_revert, SIGNAL(pressed()), this, SLOT(handle_config_revert()));
	connect(but_config_fontsel, SIGNAL(pressed()), this, SLOT(handle_config_fontsel()));

	connect(but_instr_program, SIGNAL(pressed()), this, SLOT(handle_instr_program()));
	connect(but_instr_apply, SIGNAL(pressed()), this, SLOT(handle_instr_apply()));
	connect(but_instr_revert, SIGNAL(pressed()), this, SLOT(handle_instr_revert()));
	connect(but_instr_reset, SIGNAL(pressed()), this, SLOT(handle_instr_reset()));

	connect(but_volume_apply, SIGNAL(pressed()), this, SLOT(handle_volume_apply()));
	connect(but_volume_revert, SIGNAL(pressed()), this, SLOT(handle_volume_revert()));
	connect(but_volume_reset, SIGNAL(pressed()), this, SLOT(handle_volume_reset()));

	connect(but_midi_pause, SIGNAL(pressed()), this, SLOT(handle_midi_pause()));

	connect(scores_tw, SIGNAL(currentChanged(int)), this, SLOT(handle_tab_changed(int)));

	MidiInit();

	setWindowTitle(tr("MIDI Player Pro v1.0"));

	watchdog->start(250);
}

MppMainWindow :: ~MppMainWindow()
{
	MidiUnInit();
}

void
MppMainWindow :: handle_quit()
{
	exit(0);
}

void
MppMainWindow :: handle_jump_locked(int index)
{
	int x;

	for (x = 0; x != MPP_MAX_VIEWS; x++)
		scores_main[x]->handleLabelJump(index);
}

void
MppMainWindow :: handle_jump_N(int index)
{
	pthread_mutex_lock(&mtx);

	handle_jump_locked(index);

	pthread_mutex_unlock(&mtx);
}

void
MppMainWindow :: handle_jump_0()
{
	handle_jump_N(0);
}

void
MppMainWindow :: handle_jump_1()
{
	handle_jump_N(1);
}

void
MppMainWindow :: handle_jump_2()
{
	handle_jump_N(2);
}

void
MppMainWindow :: handle_jump_3()
{
	handle_jump_N(3);
}

void
MppMainWindow :: handle_jump_4()
{
	handle_jump_N(4);
}

void
MppMainWindow :: handle_jump_5()
{
	handle_jump_N(5);
}

void
MppMainWindow :: handle_jump_6()
{
	handle_jump_N(6);
}

void
MppMainWindow :: handle_jump_7()
{
	handle_jump_N(7);
}

void
MppMainWindow :: handle_config_local_changed(int state)
{
	pthread_mutex_lock(&mtx);
	synthIsLocal = (state == Qt::Checked);
	pthread_mutex_unlock(&mtx);
}

void
MppMainWindow :: handle_play_key_changed(int key)
{
	key &= 0x7F;

	lbl_play_key->setText(tr("Play Key (0..127) ") + QString(mid_key_str[key]));

	pthread_mutex_lock(&mtx);
	playKey = key;
	pthread_mutex_unlock(&mtx);
}

void
MppMainWindow :: handle_cmd_key_changed(int key)
{
	key &= 0x7F;

	pthread_mutex_lock(&mtx);
	cmdKey = key;
	pthread_mutex_unlock(&mtx);

	lbl_cmd_key->setText(tr("Cmd Key (0..127) ") + QString(mid_key_str[key]));
}

void
MppMainWindow :: handle_base_key_changed(int key)
{
	int x;

	key &= 0x7F;

	pthread_mutex_lock(&mtx);

	for (x = 0; x != MPP_MAX_VIEWS; x++)
		scores_main[x]->baseKey = key;

	pthread_mutex_unlock(&mtx);

	lbl_base_key->setText(tr("Base Key (0..127) ") + QString(mid_key_str[key]));
}

void
MppMainWindow :: handle_key_delay_changed(int delay)
{
	int x;

	delay &= 0xFF;

	pthread_mutex_lock(&mtx);

	for (x = 0; x != MPP_MAX_VIEWS; x++)
		scores_main[x]->delayNoise = delay;

	pthread_mutex_unlock(&mtx);
}

void
MppMainWindow :: handle_pass_thru()
{
	pthread_mutex_lock(&mtx);
	midiPassThruOff = !midiPassThruOff;
	pthread_mutex_unlock(&mtx);

	if (midiPassThruOff == 0)
		lbl_midi_pass_thru->setText(tr("ON"));
	else
		lbl_midi_pass_thru->setText(tr("OFF"));
}

void
MppMainWindow :: handle_compile()
{
	int x;

	pthread_mutex_lock(&mtx);
	handle_stop();
	pthread_mutex_unlock(&mtx);

	for (x = 0; x != MPP_MAX_VIEWS; x++)
		scores_main[x]->handleCompile();
}

void
MppMainWindow :: handle_score_record()
{
	pthread_mutex_lock(&mtx);
	scoreRecordOff = !scoreRecordOff;
	pthread_mutex_unlock(&mtx);

	if (scoreRecordOff == 0)
		lbl_score_record->setText(tr("ON"));
	else
		lbl_score_record->setText(tr("OFF"));
}

void
MppMainWindow :: handle_midi_pause()
{
	uint32_t pos;
	uint8_t triggered;
	uint8_t paused;

	pthread_mutex_lock(&mtx);
	pos = (umidi20_get_curr_position() - startPosition) % 0x40000000UL;
	triggered = midiTriggered;
	paused = midiPaused;
	pthread_mutex_unlock(&mtx);

	if (paused)
		return;		/* nothing to do */

	handle_rewind();

	pthread_mutex_lock(&mtx);
	if (triggered != 0) {
		midiPaused = 1;
		pausePosition = pos;
	}
	pthread_mutex_unlock(&mtx);
}

void
MppMainWindow :: handle_midi_play()
{
	uint8_t triggered;

	pthread_mutex_lock(&mtx);
	midiPlayOff = !midiPlayOff;
	triggered = midiTriggered;
	update_play_device_no();
	pthread_mutex_unlock(&mtx);

	if (midiPlayOff == 0)
		lbl_midi_play->setText(tr("ON"));
	else
		lbl_midi_play->setText(tr("OFF"));

	handle_midi_pause();

	if (triggered)
		handle_midi_trigger();
}

void
MppMainWindow :: handle_midi_record()
{
	uint8_t triggered;

	pthread_mutex_lock(&mtx);
	midiRecordOff = !midiRecordOff;
	triggered = midiTriggered;
	update_play_device_no();
	pthread_mutex_unlock(&mtx);

	if (midiRecordOff == 0)
		lbl_midi_record->setText(tr("ON"));
	else
		lbl_midi_record->setText(tr("OFF"));

	handle_midi_pause();

	if (triggered)
		handle_midi_trigger();
}

void
MppMainWindow :: handle_play_press()
{
	pthread_mutex_lock(&mtx);
	currScoreMain->handleKeyPress(playKey, 90);
	pthread_mutex_unlock(&mtx);
}

void
MppMainWindow :: handle_play_release()
{
	pthread_mutex_lock(&mtx);
	currScoreMain->handleKeyRelease(playKey);
	pthread_mutex_unlock(&mtx);
}

void
MppMainWindow :: handle_watchdog()
{
	QTextCursor cursor(currScoreMain->editWidget->textCursor());
	uint32_t delta;
	char buf[32];
	uint8_t events_copy[MPP_MAX_QUEUE];
	uint8_t num_events;
	uint8_t x;
	uint8_t y;
	uint8_t z;
	uint8_t last_duration;
	uint8_t instr_update;

	pthread_mutex_lock(&mtx);
	instr_update = instrUpdated;
	instrUpdated = 0;
	num_events = numInputEvents;
	if (num_events != 0) {
		delta =  umidi20_get_curr_position() - lastInputEvent;
		if (delta >= ((UMIDI20_BPM + 45 -1) / 45)) {
			numInputEvents = 0;
			memcpy(events_copy, inputEvents, num_events);
		} else {
			/* wait until 2 seconds have elapsed */
			num_events = 0;
		}
	}
	pthread_mutex_unlock(&mtx);

	if (num_events != 0) {
		mid_sort(events_copy, num_events);

		last_duration = 0;

		cursor.movePosition(QTextCursor::StartOfLine, QTextCursor::MoveAnchor, 1);
		cursor.beginEditBlock();

		for (x = 0; x != num_events; x++) {
			for (y = x; y != num_events; y++) {
				if (events_copy[x] != events_copy[y])
					break;
			}

			z = y - 1;
			y = y - x;

			if (y != last_duration) {
				last_duration = y;
				snprintf(buf, sizeof(buf), "U%d %s ",
				    y, mid_key_str[events_copy[x] & 0x7F]);
			} else {
				snprintf(buf, sizeof(buf), "%s ",
				    mid_key_str[events_copy[x] & 0x7F]);
			}

			cursor.insertText(QString(buf));

			x = z;
		}

		cursor.insertText(QString("\n"));
		cursor.endEditBlock();
	}

	if (instr_update)
		handle_instr_revert();

	do_bpm_stats();

	do_clock_stats();

	currScoreMain->viewWidget->repaint();
}


void
MppMainWindow :: handle_midi_file_clear_name()
{
	if (CurrMidiFileName != NULL) {
		delete (CurrMidiFileName);
		CurrMidiFileName = NULL;
	}
	lbl_file_status->setText(QString());
}

void
MppMainWindow :: handle_midi_file_new()
{
	uint8_t x;

	handle_midi_file_clear_name();

	handle_rewind();

	if (track != NULL) {
		pthread_mutex_lock(&mtx);
		umidi20_event_queue_drain(&(track->queue));
		for (x = 0; x != 16; x++) {
			instr[x].bank = 0;
			instr[x].prog = 0;
			instr[x].updated = 1;
			instr[x].muted = 0;
		}
		instrUpdated = 1;
		chanUsageMask = 0;
		pthread_mutex_unlock(&mtx);

		handle_instr_channel_changed(0);
	}
}

void
MppMainWindow :: update_play_device_no()
{
	uint8_t device_no = playDevice;
	struct umidi20_event *event;
	uint8_t vel;

	if (track == NULL)
		return;

	UMIDI20_QUEUE_FOREACH(event, &(track->queue)) {
		event->device_no = device_no;

		vel = umidi20_event_get_velocity(event);

		if (vel != 0) {
			vel |= 1;	/* hint for "MidiEventTxCallback()" */
			umidi20_event_set_velocity(event, vel);
		}
	}
}

void
MppMainWindow :: handle_midi_file_merge_open()
{
	handle_midi_file_open(1);
}

void
MppMainWindow :: handle_midi_file_new_open()
{
	handle_midi_file_open(0);
}

void
MppMainWindow :: handle_midi_file_open(int merge)
{
	QFileDialog *diag = 
	  new QFileDialog(this, tr("Select MIDI File"), 
		QString(), QString("MIDI File (*.mid; *.MID)"));
	struct umidi20_song *song_copy;
	struct umidi20_track *track_copy;
	struct umidi20_event *event;
	struct umidi20_event *event_copy;
	const char *filename;

	diag->setAcceptMode(QFileDialog::AcceptOpen);
	diag->setFileMode(QFileDialog::ExistingFile);

	if (diag->exec()) {

		if (merge) {
			handle_midi_file_clear_name();
			handle_rewind();
		} else {
			handle_midi_file_new();
		}

		CurrMidiFileName = new QString(diag->selectedFiles()[0]);

		filename = MppQStringToAscii(*CurrMidiFileName);

		if (filename != NULL) {
			pthread_mutex_lock(&mtx);
			song_copy = umidi20_load_file(&mtx, filename);
			pthread_mutex_unlock(&mtx);

			free((void *)filename);

			if (song_copy != NULL) {
				lbl_file_status->setText(MppBaseName(*CurrMidiFileName) +
				    tr(": MIDI file opened"));
				goto load_file;
			} else {
				lbl_file_status->setText(MppBaseName(*CurrMidiFileName) +
				    tr(": Could not open MIDI file"));
			}
		}
	}

	goto done;

load_file:

	printf("format %d\n", song_copy->midi_file_format);
	printf("resolution %d\n", song_copy->midi_resolution);
	printf("division_type %d\n", song_copy->midi_division_type);

	pthread_mutex_lock(&mtx);

	UMIDI20_QUEUE_FOREACH(track_copy, &(song_copy->queue)) {

	    printf("track %p\n", track_copy);

	    UMIDI20_QUEUE_FOREACH(event, &(track_copy->queue)) {

	        if (umidi20_event_is_voice(event) ||
		    umidi20_event_is_sysex(event)) {

		    if (do_instr_check(event, NULL)) {
			event_copy = NULL;
		    } else {
			if (umidi20_event_get_what(event) & UMIDI20_WHAT_CHANNEL) {
				uint8_t chan;
				chan = umidi20_event_get_channel(event);
				chanUsageMask |= (1 << chan);
			}
			event_copy = umidi20_event_copy(event, 0);
		    }

		    if (event_copy != NULL) {
			/* reserve low positions for channel program events */
			if (event_copy->position < MPP_MIN_POS)
				event_copy->position = MPP_MIN_POS;

			umidi20_event_queue_insert(&(track->queue),
			    event_copy, UMIDI20_CACHE_INPUT);
		    }
		}
	    }
	}

	umidi20_song_free(song_copy);

	update_play_device_no();

	pthread_mutex_unlock(&mtx);

done:
	/* make sure we save into a new file */
	if (merge)
		handle_midi_file_clear_name();

	delete diag;
}

/* must be called locked */
void
MppMainWindow :: handle_midi_file_instr_prepend()
{
	struct mid_data *d = &mid_data;
	uint8_t x;

	for (x = 0; x != 16; x++) {

		/* 
		 * Don't insert instrument bank and program commands
		 * for channels without events!
		 */
		if (!(chanUsageMask & (1 << x)))
			continue;

		mid_set_channel(d, x);
		mid_set_position(d, 0);
		mid_set_device_no(d, 0xFF);
		mid_set_bank_program(d, x,
		    instr[x].bank,
		    instr[x].prog);
	}
}

/* must be called locked */
void
MppMainWindow :: handle_midi_file_instr_delete()
{
	umidi20_event_queue_move(&(track->queue), NULL, 0,
	    MPP_MIN_POS, 0, 0-1, UMIDI20_CACHE_INPUT);
}

void
MppMainWindow :: handle_midi_file_save()
{
	const char *filename;
	uint8_t status;

	if (CurrMidiFileName != NULL) {

		filename = MppQStringToAscii(*CurrMidiFileName);

		if (filename != NULL) {
			pthread_mutex_lock(&mtx);
			handle_midi_file_instr_prepend();
			status = umidi20_save_file(song, filename);
			handle_midi_file_instr_delete();
			pthread_mutex_unlock(&mtx);
			free((void *)filename);

			if (status) {
				lbl_file_status->setText(MppBaseName(*CurrMidiFileName) + 
				    tr(": Could not save MIDI file"));
			} else {
				lbl_file_status->setText(MppBaseName(*CurrMidiFileName) + 
				    tr(": MIDI file saved"));
			}
		}
	} else {
		handle_midi_file_save_as();
	}
}

void
MppMainWindow :: handle_midi_file_save_as()
{
	QFileDialog *diag = 
	  new QFileDialog(this, tr("Select MIDI File"), 
		QString(), QString("MIDI File (*.mid; *.MID)"));

	diag->setAcceptMode(QFileDialog::AcceptSave);
	diag->setFileMode(QFileDialog::AnyFile);

	if (diag->exec()) {
		if (CurrMidiFileName != NULL)
			delete (CurrMidiFileName);

		CurrMidiFileName = new QString(diag->selectedFiles()[0]);

		if (CurrMidiFileName != NULL)
			handle_midi_file_save();
	}

	delete diag;
}

void
MppMainWindow :: handle_rewind()
{
	pthread_mutex_lock(&mtx);

	midiTriggered = 0;
	midiPaused = 0;
	pausePosition = 0;

	update_play_device_no();

	if (song != NULL) {
		umidi20_song_stop(song,
		    UMIDI20_FLAG_PLAY | UMIDI20_FLAG_RECORD);
		umidi20_song_start(song, 0x40000000, 0x80000000,
		    UMIDI20_FLAG_PLAY | UMIDI20_FLAG_RECORD);
		startPosition = umidi20_get_curr_position() - 0x40000000;
	}

	pthread_mutex_unlock(&mtx);
}

void
MppMainWindow :: handle_midi_trigger()
{
	int x;

	pthread_mutex_lock(&mtx);

	if (midiTriggered == 0) {
		if (midiPlayOff == 0) {
			umidi20_song_stop(song,
			    UMIDI20_FLAG_PLAY | UMIDI20_FLAG_RECORD);
			umidi20_song_start(song, pausePosition, 0x40000000,
			    UMIDI20_FLAG_PLAY | UMIDI20_FLAG_RECORD);
			startPosition = umidi20_get_curr_position() - pausePosition;
		} else {
			umidi20_song_stop(song,
			    UMIDI20_FLAG_PLAY | UMIDI20_FLAG_RECORD);
			umidi20_song_start(song, 0x40000000 + pausePosition, 0x80000000,
			    UMIDI20_FLAG_PLAY | UMIDI20_FLAG_RECORD);
			startPosition = umidi20_get_curr_position() - 0x40000000 - pausePosition;
		}
		midiTriggered = 1;
		midiPaused = 0;
		pausePosition = 0;

		for (x = 0; x != MPP_MAX_VIEWS; x++)
			scores_main[x]->updateTimer();
	}

	pthread_mutex_unlock(&mtx);
}

void
MppMainWindow :: handle_config_reload()
{
	struct umidi20_config cfg;
	int n;
	int x;
	int y;

	/* setup the I/O devices */

	umidi20_config_export(&cfg);

	playDevice = 0;

	for (n = 0; n != MPP_MAX_DEVS; n++) {

		if ((deviceBits & (MPP_DEV0_RECORD << (3 * n))) && 
		    (deviceName[n] != NULL) && 
		    (deviceName[n][0] != 0))  {
			strlcpy(cfg.cfg_dev[n].rec_fname, deviceName[n],
			    sizeof(cfg.cfg_dev[n].rec_fname));
			cfg.cfg_dev[n].rec_enabled_cfg = 1;
		} else {
			cfg.cfg_dev[n].rec_enabled_cfg = 0;
		}

		if ((deviceBits & ((MPP_DEV0_SYNTH | MPP_DEV0_PLAY) << (3 * n))) && 
		    (deviceName[n] != NULL) && 
		    (deviceName[n][0] != 0))  {
			strlcpy(cfg.cfg_dev[n].play_fname, deviceName[n],
			    sizeof(cfg.cfg_dev[n].play_fname));
			cfg.cfg_dev[n].play_enabled_cfg = 1;
		} else {
			cfg.cfg_dev[n].play_enabled_cfg = 0;
		}

		if (deviceBits & (MPP_DEV0_PLAY << (3 * n))) {
			playDevice = n;
		}
	}

	umidi20_config_import(&cfg);

	deviceBits &= ~(MPP_DEV0_PLAY|MPP_DEV1_PLAY|MPP_DEV2_PLAY);
	deviceBits |= MPP_DEV0_PLAY << (3 * playDevice);

	handle_compile();

	bpmAutoPlayOld = spn_auto_play->value();

	handle_config_revert();

	pthread_mutex_lock(&mtx);
	for (y = 0; y != MPP_MAX_DEVS; y++) {
		/* set local on all channels */
		for (x = 0; x != 16; x++) {
			if (check_synth(y, x)) {
				uint8_t buf[4];
				buf[0] = 0xB0 | x;
				buf[1] = 0x7A;
				buf[2] = synthIsLocal ? 0x7F : 0x00;
				mid_add_raw(&mid_data, buf, 3, x);
			}
		}
	}
	pthread_mutex_unlock(&mtx);
}

void
MppMainWindow :: handle_config_revert()
{
	int n;

	for (n = 0; n != MPP_MAX_DEVS; n++) {

		if (deviceName[n] != NULL)
			led_config_dev[n]->setText(QString(deviceName[n]));
		else
			led_config_dev[n]->setText(QString());

		cbx_config_dev[(3*n)+0]->setChecked(
		    (deviceBits & (1UL << ((3*n)+0))) ? 1 : 0);

		cbx_config_dev[(3*n)+1]->setChecked(
		    (deviceBits & (1UL << ((3*n)+1))) ? 1 : 0);

		cbx_config_dev[(3*n)+2]->setChecked(
		    (deviceBits & (1UL << ((3*n)+2))) ? 1 : 0);
	}

	spn_bpm_length->setValue(bpmAvgLength);
	spn_auto_play->setValue(bpmAutoPlayOld);
	cbx_fixed_key->setChecked(playKeyFixed ? 1 : 0);
}

void
MppMainWindow :: handle_config_apply()
{
	int n;
	int m;

	deviceBits = 0;

	for (n = 0; n != MPP_MAX_DEVS; n++) {

		if (deviceName[n] != NULL)
			free(deviceName[n]);

		deviceName[n] = MppQStringToAscii(led_config_dev[n]->text());

		if (cbx_config_dev[(3*n)+0]->isChecked())
			deviceBits |= 1UL << ((3*n)+0);
		if (cbx_config_dev[(3*n)+1]->isChecked())
			deviceBits |= 1UL << ((3*n)+1);
		if (cbx_config_dev[(3*n)+2]->isChecked())
			deviceBits |= 1UL << ((3*n)+2);
	}

	n = spn_bpm_length->value();
	m = cbx_fixed_key->isChecked();

	pthread_mutex_lock(&mtx);
	bpmAvgLength = n;
	playKeyFixed = m;
	pthread_mutex_unlock(&mtx);

	handle_config_reload();
}

void
MppMainWindow :: handle_config_fontsel()
{
	bool success;
	int x;

	QFont font = QFontDialog::getFont(&success, defaultFont, this);

	if (success) {
		defaultFont = font;

		for (x = 0; x != MPP_MAX_VIEWS; x++)
			scores_main[x]->handleCompile();
	}
}

uint8_t
MppMainWindow :: check_synth(uint8_t device_no, uint8_t chan)
{
	struct mid_data *d = &mid_data;
	uint32_t pos;

	if (device_no >= MPP_MAX_DEVS)
		return (0);

	if (deviceBits & (MPP_DEV0_SYNTH << (3 * device_no))) {

		chan &= 0xF;

		handle_midi_trigger();

		pos = umidi20_get_curr_position() - startPosition + 1;

		mid_set_channel(d, chan);
		mid_set_position(d, pos);
		mid_set_device_no(d, device_no);

		return (1);
	}
	return (0);
}

uint8_t
MppMainWindow :: check_record(uint8_t chan)
{
	struct mid_data *d = &mid_data;
	uint32_t pos;

	if (midiRecordOff)
		return (0);

	chan &= 0xF;

	handle_midi_trigger();

	pos = (umidi20_get_curr_position() - startPosition) % 0x40000000UL;
	if (pos < MPP_MIN_POS)
		pos = MPP_MIN_POS;

	chanUsageMask |= (1 << chan);

	mid_set_channel(d, chan);
	mid_set_position(d, pos);
	mid_set_device_no(d, 0xFF);

	return (1);
}

void
MppMainWindow :: do_key_press(int key, int vel, int dur)
{
	struct mid_data *d = &mid_data;
	int ch = mid_get_channel(d);

	if (vel != 0) {
		vel = (synthVolume[ch] * vel) / MPP_VOLUME_UNIT;

		if (vel > 127)
			vel = 127;
		else if (vel < 0)
			vel = 0;
		else if (vel == 1)
			vel = 2;

		vel &= 0x7E;	/* hint for "MidiEventTxCallback()" */
	}

	if (key > 127)
		return;
	else if (key < 0)
		return;

	if (dur < 0)
		return;

	mid_key_press(d, key, vel, dur);
}

void
MppMainWindow :: handle_stop(void)
{
	struct mid_data *d = &mid_data;
	uint32_t *pkey;
	uint8_t ScMidiTriggered;
	uint8_t out_key;
	uint8_t chan;
	uint8_t x;
	uint8_t y;
	uint8_t z;
	uint8_t delay;

	ScMidiTriggered = midiTriggered;
	midiTriggered = 1;

	for (z = 0; z != MPP_MAX_VIEWS; z++) {
	    for (x = 0; x != MPP_PRESSED_MAX; x++) {

		pkey = &scores_main[z]->pressedKeys[x];

		if (*pkey != 0) {

			out_key = (*pkey >> 8) & 0xFF;
			chan = (*pkey >> 16) & 0xFF;
			delay = (*pkey >> 24) & 0xFF;

			*pkey = 0;

			for (y = 0; y != MPP_MAX_DEVS; y++) {
				if (check_synth(y, chan)) {
					mid_delay(d, delay);
					do_key_press(out_key, 0, 0);
				}
			}

			if (check_record(chan)) {
				mid_delay(d, delay);
				do_key_press(out_key, 0, 0);
			}
		}
	    }
	}

	midiTriggered = ScMidiTriggered;
}

/* This function must be called locked */

void
MppMainWindow :: do_update_bpm(void)
{
	uint32_t delta;
	uint32_t curr;

	curr = umidi20_get_curr_position();
	delta = curr - lastKeyPress;
	lastKeyPress = curr;

	/* too big delay */
	if (delta >= (UMIDI20_BPM / 15))
		return;

	/* store statistics */
	bpmData[bpmAvgPos] = delta;
	bpmAvgPos++;

	if (bpmAvgPos >= bpmAvgLength)
		bpmAvgPos = 0;
}

void
MppMainWindow :: do_clock_stats(void)
{
	uint32_t time_offset;
	char buf[32];

	pthread_mutex_lock(&mtx);

	if (midiTriggered == 0) {
		if (midiPaused != 0)
			time_offset = pausePosition;
		else
			time_offset = 0;
	} else {
		time_offset = (umidi20_get_curr_position() - startPosition) % 0x40000000UL;
	}

	time_offset %= 100000000UL;

	pthread_mutex_unlock(&mtx);

	snprintf(buf, sizeof(buf), "%d", time_offset);
	lbl_curr_time_val->display(QString(buf));
}

void
MppMainWindow :: do_bpm_stats(void)
{
	uint32_t min = 0xFFFFFFFFUL;
	uint32_t max = 0;
	uint32_t sum = 0;
	uint32_t val;
	uint8_t x;
	uint8_t len;
	char buf[32];

	len = bpmAvgLength;

	if (len == 0)
		return;

	pthread_mutex_lock(&mtx);

	for (x = 0; x != len; x++) {
		val = bpmData[x];

		sum += val;
		if (val > max)
			max = val;
		if (val < min)
			min = val;
	}

	pthread_mutex_unlock(&mtx);

	if (sum == 0)
		sum = 1;
	if (max == 0)
		max = 1;
	if (min == 0)
		min = 1;

	sum = (len * UMIDI20_BPM) / sum;
	max = UMIDI20_BPM / max;
	min = UMIDI20_BPM / min;

	if (sum > 9999)
		sum = 9999;
	if (min > 9999)
		min = 9999;
	if (max > 9999)
		max = 9999;

	snprintf(buf, sizeof(buf), "%d", min);
	lbl_bpm_max_val->display(QString(buf));

	snprintf(buf, sizeof(buf), "%d", max);
	lbl_bpm_min_val->display(QString(buf));

	snprintf(buf, sizeof(buf), "%d", sum);
	lbl_bpm_avg_val->display(QString(buf));
}

/* is called locked */
static void
MidiEventRxCallback(uint8_t device_no, void *arg, struct umidi20_event *event, uint8_t *drop)
{
	MppMainWindow *mw = (MppMainWindow *)arg;
	struct mid_data *d = &mw->mid_data;
	uint8_t y;
	uint8_t chan;
	int key;
	int vel;
	int lbl;

	*drop = 1;

	chan = mw->currScoreMain->synthChannel;

	if (umidi20_event_get_control_address(event) == 0x40) {

		for (y = 0; y != MPP_MAX_DEVS; y++) {
			if (mw->check_synth(y, chan)) {
				mid_pedal(d, umidi20_event_get_control_value(event));
			}
		}

		if (mw->check_record(chan)) {
			mid_pedal(d, umidi20_event_get_control_value(event));
		}

	} else if (umidi20_event_is_key_start(event)) {

		key = umidi20_event_get_key(event) & 0x7F;
		vel = umidi20_event_get_velocity(event);
		lbl = key - mw->cmdKey;

		if (mw->scoreRecordOff == 0) {
			if (mw->numInputEvents < MPP_MAX_QUEUE) {
				mw->inputEvents[mw->numInputEvents] = key;
				mw->numInputEvents++;
				mw->lastInputEvent = umidi20_get_curr_position();
			}
		}

		if (mw->midiPassThruOff != 0) {
			if (mw->currScoreMain->checkLabelJump(lbl)) {
				mw->handle_jump_locked(lbl);
			} else {
				if (mw->playKeyFixed != 0)
					key = mw->playKey;

				mw->currScoreMain->handleKeyPress(key, vel);
			}
		} else if (mw->currScoreMain->setPressedKey(chan, key, 255, 0) == 0) {
			for (y = 0; y != MPP_MAX_DEVS; y++) {
				if (mw->check_synth(y, chan)) {
					mw->do_key_press(key, vel, 0);
				}
			}

			if (mw->check_record(chan)) {
				mw->do_key_press(key, vel, 0);
			}

			mw->do_update_bpm();
		}
	} else if (umidi20_event_is_key_end(event)) {

		key = umidi20_event_get_key(event) & 0x7F;
		lbl = key - mw->cmdKey;

		if (mw->midiPassThruOff != 0) {

			if (mw->currScoreMain->checkLabelJump(lbl) == 0) {
				mw->currScoreMain->handleKeyRelease(key);
			}

		} else {

			for (y = 0; y != MPP_MAX_DEVS; y++) {
				if (mw->check_synth(y, chan)) {
					mw->do_key_press(key, 0, 0);
				}
			}

			if (mw->check_record(chan)) {
				mw->do_key_press(key, 0, 0);
			}

			mw->currScoreMain->pressedKeys[key] = 0;
		}
	} else if (mw->do_instr_check(event, &chan)) {
		/* found instrument */
		mw->currScoreMain->synthChannel = chan;
	}
}

/* is called locked */
static void
MidiEventTxCallback(uint8_t device_no, void *arg, struct umidi20_event *event, uint8_t *drop)
{
	MppMainWindow *mw = (MppMainWindow *)arg;
	int vel;

	if (umidi20_event_get_what(event) & UMIDI20_WHAT_CHANNEL) {
		uint8_t chan;

		chan = umidi20_event_get_channel(event) & 0xF;

		vel = umidi20_event_get_velocity(event);

		/* update playback velocity, if any */

		if (vel & 1) {
			vel |= 1;	/* hint for "MidiEventTxCallback()" */

			vel = (vel * mw->playVolume[chan]) / MPP_VOLUME_UNIT;

			if (vel > 127)
				vel = 127;
			else if (vel < 0)
				vel = 0;

			umidi20_event_set_velocity(event, vel);
		}

		*drop = mw->instr[chan].muted;
	}
}

/* must be called locked */
uint8_t
MppMainWindow :: do_instr_check(struct umidi20_event *event, uint8_t *pchan)
{
	if (umidi20_event_get_what(event) & UMIDI20_WHAT_CONTROL_VALUE) {
		uint8_t addr;
		uint8_t val;
		uint8_t chan;

		addr = umidi20_event_get_control_address(event);
		val = umidi20_event_get_control_value(event);
		chan = umidi20_event_get_channel(event) & 0xF;

		if (addr == 0x00) {
			instr[chan].bank &= 0x007F;
			instr[chan].bank |= (val << 7);
			instr[chan].updated = 1;
			instr[chan].muted = 0;
			instrUpdated = 1;
			if (pchan != NULL)
				*pchan = chan;
			return (1);
		} else if (addr == 0x20) {
			instr[chan].bank &= 0xFF80;
			instr[chan].bank |= (val & 0x7F);
			instr[chan].updated = 1;
			instr[chan].muted = 0;
			instrUpdated = 1;
			if (pchan != NULL)
				*pchan = chan;
			return (1);
		}
	} else if (umidi20_event_get_what(event) & UMIDI20_WHAT_PROGRAM_VALUE) {
		uint8_t val;
		uint8_t chan;

		val = umidi20_event_get_program_number(event);
		chan = umidi20_event_get_channel(event) & 0xF;

		instr[chan].prog = val;
		instr[chan].updated = 1;
		instr[chan].muted = 0;
		instrUpdated = 1;
		if (pchan != NULL)
			*pchan = chan;
		return (1);
	}
	return (0);
}

void
MppMainWindow :: handle_instr_channel_changed(int chan)
{
	int temp[3];

	pthread_mutex_lock(&mtx);
	currScoreMain->synthChannel = chan;
	temp[0] = instr[chan].bank;
	temp[1] = instr[chan].prog;
	temp[2] = synthVolume[chan];
	pthread_mutex_unlock(&mtx);

	spn_instr_curr_bank->setValue(temp[0]);
	spn_instr_curr_prog->setValue(temp[1]);
	spn_volume->setValue(temp[2]);

	if (spn_instr_curr_chan->value() != chan)
		spn_instr_curr_chan->setValue(chan);
}

void
MppMainWindow :: handle_instr_program()
{
	int chan = spn_instr_curr_chan->value();
	int bank = spn_instr_curr_bank->value();
	int prog = spn_instr_curr_prog->value();

	pthread_mutex_lock(&mtx);
	instr[chan].bank = bank;
	instr[chan].prog = prog;
	instr[chan].muted = 0;
	instr[chan].updated = 1;
	pthread_mutex_unlock(&mtx);

	handle_instr_revert();
}

void 
MppMainWindow :: handle_instr_apply()
{
	int temp[3];
	uint8_t x;
	uint8_t update_curr;

	for (x = 0; x != 16; x++) {

		temp[0] = spn_instr_bank[x]->value();
		temp[1] = spn_instr_prog[x]->value();
		temp[2] = cbx_instr_mute[x]->isChecked();

		pthread_mutex_lock(&mtx);
		instr[x].bank = temp[0];
		instr[x].prog = temp[1];
		instr[x].muted = temp[2];
		instr[x].updated = 1;
		update_curr = (currScoreMain->synthChannel == x);
		pthread_mutex_unlock(&mtx);

		if (update_curr) {
			spn_instr_curr_chan->setValue(x);
			spn_instr_curr_bank->setValue(temp[0]);
			spn_instr_curr_prog->setValue(temp[1]);
		}
	}
	handle_instr_reload();
}

void 
MppMainWindow :: handle_instr_revert()
{
	int temp[3];
	uint8_t x;
	uint8_t update_curr;

	for (x = 0; x != 16; x++) {

		pthread_mutex_lock(&mtx);
		temp[0] = instr[x].bank;
		temp[1] = instr[x].prog;
		temp[2] = instr[x].muted;
		update_curr = (currScoreMain->synthChannel == x);
		pthread_mutex_unlock(&mtx);

		spn_instr_bank[x]->setValue(temp[0]);
		spn_instr_prog[x]->setValue(temp[1]);
		cbx_instr_mute[x]->setChecked(temp[2]);

		if (update_curr) {
			spn_instr_curr_chan->setValue(x);
			spn_instr_curr_bank->setValue(temp[0]);
			spn_instr_curr_prog->setValue(temp[1]);
		}
	}
	handle_instr_reload();
}

void 
MppMainWindow :: handle_instr_reset()
{
	uint8_t x;

	for (x = 0; x != 16; x++) {
		spn_instr_bank[x]->setValue(0);
		spn_instr_prog[x]->setValue(0);
		cbx_instr_mute[x]->setChecked(0);
	}

	spn_instr_curr_chan->setValue(0);
	spn_instr_curr_bank->setValue(0);
	spn_instr_curr_prog->setValue(0);

	handle_instr_reload();
}

void
MppMainWindow :: handle_instr_reload()
{
	struct mid_data *d = &mid_data;
	uint8_t x;
	uint8_t y;
	uint8_t trig;

	pthread_mutex_lock(&mtx);
	trig = midiTriggered;
	midiTriggered = 1;

	for (x = 0; x != 16; x++) {
		if (instr[x].updated == 0)
			continue;

		instr[x].updated = 0;
		for (y = 0; y != MPP_MAX_DEVS; y++) {
			if (check_synth(y, x)) {
				mid_delay(d, (4 * x));
				mid_set_bank_program(d, x, 
				    instr[x].bank,
				    instr[x].prog);
			}
		}
	}

	midiTriggered = trig;
	pthread_mutex_unlock(&mtx);
}

void
MppMainWindow :: handle_volume_changed(int vol)
{
	int x;

	pthread_mutex_lock(&mtx);
	x = currScoreMain->synthChannel;
	synthVolume[x] = vol;
	pthread_mutex_unlock(&mtx);

	spn_volume_synth[x]->setValue(vol);
}

void 
MppMainWindow :: handle_volume_apply()
{
	int temp[2];

	uint8_t x;
	uint8_t update_curr;

	for (x = 0; x != 16; x++) {

		temp[0] = spn_volume_play[x]->value();
		temp[1] = spn_volume_synth[x]->value();

		pthread_mutex_lock(&mtx);
		playVolume[x] = temp[0];
		synthVolume[x] = temp[1];
		update_curr = (currScoreMain->synthChannel == x);
		pthread_mutex_unlock(&mtx);

		if (update_curr)
			spn_volume->setValue(temp[1]);
	}

	handle_volume_reload();
}

void 
MppMainWindow :: handle_volume_revert()
{
	int temp[2];

	uint8_t x;
	uint8_t update_curr;

	for (x = 0; x != 16; x++) {

		pthread_mutex_lock(&mtx);
		temp[0] = playVolume[x];
		temp[1] = synthVolume[x];
		update_curr = (currScoreMain->synthChannel == x);
		pthread_mutex_unlock(&mtx);

		spn_volume_play[x]->setValue(temp[0]);
		spn_volume_synth[x]->setValue(temp[1]);

		if (update_curr)
			spn_volume->setValue(temp[1]);
	}

	handle_volume_reload();
}

void
MppMainWindow :: handle_auto_play(int bpm)
{
	currScoreMain->handleAutoPlay(bpm);
}

void
MppMainWindow :: handle_tab_changed(int index)
{
	QWidget *pw;
	int x;
	int compile = 0;

	pw = scores_tw->widget(index);

	for (x = 0; x != MPP_MAX_VIEWS; x++) {
		if (pw == scores_main[x]->viewWidget) {
			compile = 1;
			break;
		}
		if (pw == scores_main[x]->editWidget)
			break;
	}

	if (x == MPP_MAX_VIEWS)
		currScoreMain = scores_main[0];
	else
		currScoreMain = scores_main[x];

	handle_instr_channel_changed(currScoreMain->synthChannel);

	spn_auto_play->setValue(currScoreMain->bpmAutoPlay);

	if (compile)
		handle_compile();
}

void 
MppMainWindow :: handle_volume_reset()
{
	uint8_t x;

	for (x = 0; x != 16; x++) {
		spn_volume_play[x]->setValue(MPP_VOLUME_UNIT);
		spn_volume_synth[x]->setValue(MPP_VOLUME_UNIT);
	}

	spn_volume->setValue(MPP_VOLUME_UNIT);

	handle_volume_reload();
}

void
MppMainWindow :: handle_volume_reload()
{

}

int
MppMainWindow :: convert_midi_duration(uint32_t thres)
{
	struct umidi20_event *event;

	uint32_t last_pos;
	uint32_t delta;
	uint32_t index;

	last_pos = 0;
	index = 0;

	memset(convLineStart, 0, sizeof(convLineStart));

	UMIDI20_QUEUE_FOREACH(event, &(track->queue)) {

		delta = event->position - last_pos;

		if (!(umidi20_event_get_what(event) & UMIDI20_WHAT_CHANNEL))
			continue;
		if (instr[umidi20_event_get_channel(event) & 0xF].muted)
			continue;

		if (umidi20_event_is_key_start(event)) {

			if (delta > thres) {
				last_pos = event->position;

				if (index < MPP_MAX_LINES) {
					convLineStart[index] = last_pos;
					index++;
				}
			}

		}
	}

	if (index < MPP_MAX_LINES) {
		convLineStart[index] = 0x80000000;
		index++;
	}
	return (index);
}

int
MppMainWindow :: convert_midi_score_duration()
{
	uint32_t retval;
	uint8_t i;
	uint8_t j;

	i = convIndex & 0xFF;

	if (convEndCount[i] == 0 || (convIndex == 0)) {
		retval = 9;
	} else {
		retval = (convEndPos[i] / convEndCount[i]) -
		    convLineStart[convIndex - 1];

		for (j = 0; j != 9; j++) {
			if (retval > (1000U >> (j+1)))
				break;
		}
		retval = j;
	}

	convEndPos[i] = 0;
	convEndCount[i] = 0;

	return (retval);
}

void
MppMainWindow :: handle_midi_file_convert()
{
	QTextCursor cursor(currScoreMain->editWidget->textCursor());

	QString output;
	QString out_block;
	QString out_desc;

	int thres = spn_parse_thres->value();

	struct umidi20_event *event;

	char buf[128];

	uint32_t end;
	uint32_t max_index;
	uint32_t x;
	uint32_t last_u = MPP_MAX_DURATION + 1;

	memset(convEndPos, 0, sizeof(convEndPos));
	memset(convEndCount, 0, sizeof(convEndCount));

	convIndex = 0;

	pthread_mutex_lock(&mtx);

	umidi20_track_compute_max_min(track);

	max_index = convert_midi_duration(thres);

	UMIDI20_QUEUE_FOREACH(event, &(track->queue)) {

		if (!(umidi20_event_get_what(event) & UMIDI20_WHAT_CHANNEL))
			continue;
		if (instr[umidi20_event_get_channel(event) & 0xF].muted)
			continue;

		while ((convIndex < max_index) &&
		       (event->position > convLineStart[convIndex])) {

			uint8_t do_flush;
			uint8_t new_page;
			uint8_t duration;

			convIndex++;

			duration = convert_midi_score_duration();
			do_flush = ((convIndex & 0xF) == 0);
			new_page = ((convIndex & 0xFF) == 0);

			if (duration != 0)
				snprintf(buf, sizeof(buf), ".[%u]   ", (int)duration);
			else
				snprintf(buf, sizeof(buf), ".   ");

			out_desc += buf;
			out_block += "\n";

			if (do_flush) {

				snprintf(buf, sizeof(buf), "%5u", convIndex / 16);

				output += "\nS\"";
				output += buf;
				output += out_desc;
				output += ";\"\n";
				output += out_block;
				if (new_page)
					output += "\nJP\n";
				output += "\n";

				out_desc = "";
				out_block = "";
			}

			last_u = MPP_MAX_DURATION + 1;
		}

		end = event->position + event->duration;

		if (umidi20_event_is_key_start(event)) {

			x = convIndex;
			while (x < max_index) {
				if (convLineStart[x] >= end)
					break;
				x++;
			}

			x = x - convIndex;

			if (x > MPP_MAX_DURATION)
				x = MPP_MAX_DURATION;
			else if (x == 0) {
				x = 1;
				if ((convIndex + 1) < max_index)
					end = convLineStart[convIndex + 1];
				else
					end = 0;	/* should not happen */

			}

			convEndPos[(convIndex + x) & 0xFF] += end;
			convEndCount[(convIndex + x) & 0xFF] += 1;

			if (x != last_u) {
				last_u = x;
				snprintf(buf, sizeof(buf), "U%u ", x);
				out_block += buf;
			}

			out_block += mid_key_str[umidi20_event_get_key(event)];
			out_block += " ";
		}
	}

	pthread_mutex_unlock(&mtx);

	snprintf(buf, sizeof(buf), "%5u", (convIndex + 15) / 16);

	output += "\nS\"";
	output += buf;
	output += out_desc;
	output += ";\"\n";
	output += out_block;
	output += "\n";

	cursor.insertText(output);

	handle_compile();
}

void
MppMainWindow :: MidiInit(void)
{
	int n;

	deviceBits = MPP_DEV0_SYNTH | MPP_DEV0_PLAY | 
	  MPP_DEV1_RECORD | MPP_DEV2_RECORD | MPP_DEV3_RECORD;
	deviceName[0] = strdup("/midi");
	deviceName[1] = strdup("/dev/umidi0.0");
	deviceName[2] = strdup("/dev/umidi1.0");
	deviceName[3] = strdup("/dev/umidi2.0");
	bpmAvgLength = 4;

	handle_midi_record();
	handle_midi_play();
	handle_score_record();
	handle_pass_thru();
	handle_instr_apply();
	handle_volume_apply();

	for (n = 0; n != UMIDI20_N_DEVICES; n++) {
		umidi20_set_record_event_callback(n, &MidiEventRxCallback, this);
		umidi20_set_play_event_callback(n, &MidiEventTxCallback, this);
	}

	pthread_mutex_lock(&mtx);

	song = umidi20_song_alloc(&mtx, UMIDI20_FILE_FORMAT_TYPE_0, 500,
	    UMIDI20_FILE_DIVISION_TYPE_PPQ);

	track = umidi20_track_alloc();

	if (song == NULL || track == NULL) {
		pthread_mutex_unlock(&mtx);
		err(1, "Could not allocate new song or track\n");
	}
	umidi20_song_track_add(song, NULL, track, 0);

	umidi20_song_set_record_track(song, track);

	/* get the MIDI up! */

	mid_init(&mid_data, track);

	umidi20_song_start(song, 0x40000000, 0x80000000,
	    UMIDI20_FLAG_PLAY | UMIDI20_FLAG_RECORD);

	startPosition = umidi20_get_curr_position() - 0x40000000;

	pthread_mutex_unlock(&mtx);

	/* reload the configuration */

	handle_config_reload();
}

void
MppMainWindow :: MidiUnInit(void)
{
	int n;

	handle_rewind();

	pthread_mutex_lock(&mtx);

	umidi20_song_free(song);

	song = NULL;

	umidi20_song_stop(song, UMIDI20_FLAG_PLAY | UMIDI20_FLAG_RECORD);

	pthread_mutex_unlock(&mtx);

	for (n = 0; n != MPP_MAX_DEVS; n++) {
		if (deviceName[n] != NULL) {
			free (deviceName[n]);
			deviceName[n] = NULL;
		}
	}
}

void
MppMainWindow :: keyPressEvent(QKeyEvent *event)
{
#if 0
	if (event->isAutoRepeat())
		return;

	if (event->key() == Qt::Key_PageUp || 
	    event->key() == Qt::Key_PageDown) {
		handle_play_press();
	}
#endif
}

void
MppMainWindow :: keyReleaseEvent(QKeyEvent *event)
{
#if 0
	if (event->isAutoRepeat())
		return;

	if (event->key() == Qt::Key_PageUp || 
	    event->key() == Qt::Key_PageDown) {
		handle_play_release();
	}
#endif
}