/*-
 * Copyright (c) 2010,2012-2013 Hans Petter Selasky. All rights reserved.
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

#include "midipp_decode.h"
#include "midipp_mainwindow.h"
#include "midipp_scores.h"
#include "midipp_groupbox.h"
#include "midipp_import.h"
#include "midipp_button.h"

static int
midipp_import_find_chord(const QString &str)
{
	return (mpp_find_chord(str, NULL, NULL, NULL));
}

static uint8_t
midipp_import_flush(class midipp_import *ps, int i_txt, int i_score)
{
	QString out;
	QString scs;

	int off;
	int x;
	int ai;
	int bi;

	uint8_t any;
	
	out += "S\"";

	for (ai = bi = x = 0; x != ps->max_off; x++) {

		any = 0;

		if (i_score > -1 && ai < ps->n_word[i_score]) {

			any = 1;

			if (ps->d_word[i_score][ai].off == x) {
				QString *ptr = &ps->d_word[i_score][ai].name;

				if (midipp_import_find_chord(*ptr) == 0) {
					MppDecodeTab *ptab = ps->sm->mainWindow->tab_chord_gl;

					out += ".(";
					out += ps->d_word[i_score][ai].name;
					out += ")";

					ptab->setText(*ptr);

					scs += ps->sm->mainWindow->led_config_insert->text() +
					    ptab->getText() +
					    "\n";
				}
				ai++;
			}
		}
	next_word:
		if (i_txt > -1 && bi < ps->n_word[i_txt]) {

			off = ps->d_word[i_txt][bi].off;

			if (x >= off) {
				int y = x - off;

				if (y >= ps->d_word[i_txt][bi].name.size()) {
					bi++;
					goto next_word;
				} else {
					out += ps->d_word[i_txt][bi].name[y];
				}
			} else {
				if (!any)
					break;

				out += " ";
			}
		} else {
			if (!any)
				break;

			out += " ";
		}
	}
	out += "\"\n\n";
	out += scs;
	out += "\n";

	QTextCursor cursor(ps->sm->editWidget->textCursor());
	cursor.beginEditBlock();
	cursor.insertText(out);
	cursor.endEditBlock();

	return (0);
}

static uint8_t
midipp_import_is_chord(QChar ch)
{
	static const QString chordchars("ABCDEFGHM+-/#øØ&|^Δ°");
	int x;
	for (x = 0; x != chordchars.length(); x++) {
		if (ch == chordchars[x])
			return (1);
	}
	char c = ch.toLatin1();
	if (c >= '0' && c <= '9')
		return (1);
	if (c >= 'a' && c <= 'z')
		return (1);
	if (c == ' ' || c == '\t')
		return (2);
	return (0);
}

static uint8_t
midipp_import_parse(class midipp_import *ps)
{
	int n_off;
	int n_word;
	int nchord;
	int fchord;
	int temp;
	uint8_t state;
	uint8_t next;
	QChar c;

	n_word = 0;
	state = midipp_import_is_chord(ps->line_buffer[0]);
	nchord = (state == 1) ? 1 : 0;
	fchord = 0;

	/* reset all the "name" strings */
	for (n_off = 0; n_off != MIDIPP_IMPORT_MW; n_off++) {
		ps->d_word[ps->index][n_off].name = QString();
	}

	/* parse line */
	for (n_off = 0; n_off != ps->line_buffer.size(); n_off++) {

		c = ps->line_buffer[n_off];

		next = midipp_import_is_chord(c);
		if (next != state) {
			ps->d_word[ps->index][n_word].off = n_off -
			    ps->d_word[ps->index][n_word].name.size();

			if (state == 1 &&
			    midipp_import_find_chord(
			    ps->d_word[ps->index][n_word].name) == 0) {
				fchord++;
			}
			n_word ++;

			if (n_word == (MIDIPP_IMPORT_MW - 1))
				break;

			state = next;
			if (state == 1)
				nchord++;
		}
		ps->d_word[ps->index][n_word].name += c;
	}
	if (n_off == ps->line_buffer.size()) {
		ps->d_word[ps->index][n_word].off = n_off -
		    ps->d_word[ps->index][n_word].name.size();

		if (state == 1 &&
		    midipp_import_find_chord(
		    ps->d_word[ps->index][n_word].name) == 0) {
			fchord++;
		}
		n_word ++;
	} else {
		n_off++;
	}
	if (n_off > ps->max_off)
		ps->max_off = n_off;

	ps->n_word[ps->index] = n_word;
	ps->d_chords[ps->index] = ((nchord != 0) && (nchord == fchord));

	if (ps->load_more != 0) {
		/* load another line */
		ps->index ^= 1;
		ps->load_more--;
		return (0);
	}

	temp = 0;
	if (ps->d_chords[0])
		temp |= 1;
	if (ps->d_chords[1])
		temp |= 2;

	switch (temp) {
	case 0:
		if (ps->index == 0) {
			if (midipp_import_flush(ps, 1, -1))
				return (1);
		} else {
			if (midipp_import_flush(ps, 0, -1))
				return (1);
		}
		break;
	case 1:
		if (ps->index == 0) {
			if (midipp_import_flush(ps, 1, -1))
				return (1);
		} else {
			if (midipp_import_flush(ps, 1, 0))
				return (1);
			ps->load_more = 1;
		}
		break;
	case 2:
		if (ps->index == 0) {
			if (midipp_import_flush(ps, 0, 1))
				return (1);
			ps->load_more = 1;
		} else {
			if (midipp_import_flush(ps, 0, -1))
				return (1);
		}
		break;
	default:
		if (ps->index == 0) {
			if (midipp_import_flush(ps, -1, 1))
				return (1);
		} else {
			if (midipp_import_flush(ps, -1, 0))
				return (1);
		}
		break;
	}
	ps->index ^= 1;
	return (0);
}

Q_DECL_EXPORT uint8_t
midipp_import(QString str, class midipp_import *ps, MppScoreMain *sm)
{
	QChar ch;
	int off;

	/* reset parse state */
	for (off = 0; off != (2 * MIDIPP_IMPORT_MW); off++)
		ps->d_word[0][off].off = 0;

	ps->d_chords[0] = 0;
	ps->d_chords[1] = 0;
	ps->sm = sm;
	ps->n_word[0] = 0;
	ps->n_word[1] = 0;
	ps->max_off = 0;
	ps->index = 0;
	ps->load_more = 1;

	for (off = 0; off != str.size(); off++) {
		ch = str[off];

		if (ch == '\r')
			continue;

		/* remove formatting characters */
		if (ch == '(')
			ch = '[';
		if (ch == ')')
			ch = ']';
		if (ch == '"')
			ch = '\'';
		if (ch == '.')
			ch = ' ';

		/* expand tabs to 8 spaces */
		if (ch == '\t')
			ps->line_buffer += "       ";

		if (ch == '\n') {
			/* remove spaces from end of line */
			ps->line_buffer =
			    ps->line_buffer.replace(QRegExp("\\s*$"), "");
			if (midipp_import_parse(ps))
				goto done;
			ps->line_buffer = QString();
		} else {
			ps->line_buffer += ch;
		}
	}
	/* remove spaces from end of line */
	ps->line_buffer =
	    ps->line_buffer.replace(QRegExp("\\s*$"), "");
	midipp_import_parse(ps);

	if (ps->line_buffer.size() != 0 && ps->load_more == 0) {
		ps->line_buffer = QString();
		midipp_import_parse(ps);
	}

 done:
	return (0);
}

MppImportTab :: MppImportTab(MppMainWindow *parent)
{
	int x;

	mainWindow = parent;

	editWidget = new QPlainTextEdit();
	editWidget->setFont(mainWindow->editFont);
	editWidget->setLineWrapMode(QPlainTextEdit::NoWrap);
	editWidget->setPlainText(tr(
	    "Example song:" "\n\n"
	    "C  G  Am" "\n"
	    "Welcome!" "\n"));

	butImportFileNew = new QPushButton(tr("New"));
	butImportFileOpen = new QPushButton(tr("Open"));
	butImportFileSaveAs = new QPushButton(tr("Save As"));

	for (x = 0; x != MPP_MAX_VIEWS; x++) {
		butImport[x] = new MppButton(QString("To %1-Scores").arg(QChar('A' + x)), x);
		connect(butImport[x], SIGNAL(released(int)), this, SLOT(handleImport(int)));
	}
	gbImport = new MppGroupBox(tr("Lyrics"));
	gbImport->addWidget(butImportFileNew, 0, 0, 1, 1);
	gbImport->addWidget(butImportFileOpen, 1, 0, 1, 1);
	gbImport->addWidget(butImportFileSaveAs, 2, 0, 1, 1);
	for (x = 0; x != MPP_MAX_VIEWS; x++)
		gbImport->addWidget(butImport[x], 3 + x, 0, 1, 1);

	connect(butImportFileNew, SIGNAL(released()), this, SLOT(handleImportNew()));
	connect(butImportFileOpen, SIGNAL(released()), this, SLOT(handleImportOpen()));
	connect(butImportFileSaveAs, SIGNAL(released()), this, SLOT(handleImportSaveAs()));
}

MppImportTab :: ~MppImportTab()
{

}

void
MppImportTab :: handleImportNew()
{
	editWidget->setPlainText(QString());

	mainWindow->handle_make_tab_visible(editWidget);
}

void
MppImportTab :: handleImportOpen()
{
	QFileDialog *diag = 
	  new QFileDialog(mainWindow, tr("Select Chord Tabular File"), 
		Mpp.HomeDirTxt,
		QString("Chord Tabular File (*.txt; *.TXT)"));
	QString scores;

	diag->setAcceptMode(QFileDialog::AcceptOpen);
	diag->setFileMode(QFileDialog::ExistingFile);

	if (diag->exec()) {

		Mpp.HomeDirTxt = diag->directory().path();

		handleImportNew();

		scores = MppReadFile(diag->selectedFiles()[0]);

		editWidget->setPlainText(scores);

		mainWindow->handle_make_tab_visible(editWidget);
	}
	delete diag;
}

void
MppImportTab :: handleImportSaveAs()
{
	QFileDialog *diag = 
	  new QFileDialog(mainWindow, tr("Select Chord Tabular File"), 
		Mpp.HomeDirTxt,
		QString("Score File (*.txt *.TXT)"));

	diag->setAcceptMode(QFileDialog::AcceptSave);
	diag->setFileMode(QFileDialog::AnyFile);
	diag->setDefaultSuffix(QString("txt"));

	if (diag->exec()) {
		Mpp.HomeDirTxt = diag->directory().path();
		MppWriteFile(diag->selectedFiles()[0], editWidget->toPlainText());
	}

	delete diag;
}

void
MppImportTab :: handleImport(int n)
{
	class midipp_import ps;

	midipp_import(editWidget->toPlainText(), &ps, mainWindow->scores_main[n]);

	mainWindow->handle_compile();
	mainWindow->handle_make_scores_visible(mainWindow->scores_main[n]);
}
