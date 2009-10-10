/*
 * xboard.c -- X front end for XBoard
 *
 * Copyright 1991 by Digital Equipment Corporation, Maynard,
 * Massachusetts.
 *
 * Enhancements Copyright 1992-2001, 2002, 2003, 2004, 2005, 2006,
 * 2007, 2008, 2009 Free Software Foundation, Inc.
 *
 * The following terms apply to Digital Equipment Corporation's copyright
 * interest in XBoard:
 * ------------------------------------------------------------------------
 * All Rights Reserved
 *
 * Permission to use, copy, modify, and distribute this software and its
 * documentation for any purpose and without fee is hereby granted,
 * provided that the above copyright notice appear in all copies and that
 * both that copyright notice and this permission notice appear in
 * supporting documentation, and that the name of Digital not be
 * used in advertising or publicity pertaining to distribution of the
 * software without specific, written prior permission.
 *
 * DIGITAL DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE, INCLUDING
 * ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO EVENT SHALL
 * DIGITAL BE LIABLE FOR ANY SPECIAL, INDIRECT OR CONSEQUENTIAL DAMAGES OR
 * ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
 * WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION,
 * ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS
 * SOFTWARE.
 * ------------------------------------------------------------------------
 *
 * The following terms apply to the enhanced version of XBoard
 * distributed by the Free Software Foundation:
 * ------------------------------------------------------------------------
 *
 * GNU XBoard is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or (at
 * your option) any later version.
 *
 * GNU XBoard is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see http://www.gnu.org/licenses/.  *
 *
 *------------------------------------------------------------------------
 ** See the file ChangeLog for a revision history.  */

#include "config.h"

#include <stdio.h>
#include <ctype.h>
#include <signal.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <pwd.h>

#if !OMIT_SOCKETS
# if HAVE_SYS_SOCKET_H
#  include <sys/socket.h>
#  include <netinet/in.h>
#  include <netdb.h>
# else /* not HAVE_SYS_SOCKET_H */
#  if HAVE_LAN_SOCKET_H
#   include <lan/socket.h>
#   include <lan/in.h>
#   include <lan/netdb.h>
#  else /* not HAVE_LAN_SOCKET_H */
#   define OMIT_SOCKETS 1
#  endif /* not HAVE_LAN_SOCKET_H */
# endif /* not HAVE_SYS_SOCKET_H */
#endif /* !OMIT_SOCKETS */

#if STDC_HEADERS
# include <stdlib.h>
# include <string.h>
#else /* not STDC_HEADERS */
extern char *getenv();
# if HAVE_STRING_H
#  include <string.h>
# else /* not HAVE_STRING_H */
#  include <strings.h>
# endif /* not HAVE_STRING_H */
#endif /* not STDC_HEADERS */

#if HAVE_SYS_FCNTL_H
# include <sys/fcntl.h>
#else /* not HAVE_SYS_FCNTL_H */
# if HAVE_FCNTL_H
#  include <fcntl.h>
# endif /* HAVE_FCNTL_H */
#endif /* not HAVE_SYS_FCNTL_H */

#if HAVE_SYS_SYSTEMINFO_H
# include <sys/systeminfo.h>
#endif /* HAVE_SYS_SYSTEMINFO_H */

#if TIME_WITH_SYS_TIME
# include <sys/time.h>
# include <time.h>
#else
# if HAVE_SYS_TIME_H
#  include <sys/time.h>
# else
#  include <time.h>
# endif
#endif

#if HAVE_UNISTD_H
# include <unistd.h>
#endif

#if HAVE_SYS_WAIT_H
# include <sys/wait.h>
#endif

#if HAVE_DIRENT_H
# include <dirent.h>
# define NAMLEN(dirent) strlen((dirent)->d_name)
# define HAVE_DIR_STRUCT
#else
# define dirent direct
# define NAMLEN(dirent) (dirent)->d_namlen
# if HAVE_SYS_NDIR_H
#  include <sys/ndir.h>
#  define HAVE_DIR_STRUCT
# endif
# if HAVE_SYS_DIR_H
#  include <sys/dir.h>
#  define HAVE_DIR_STRUCT
# endif
# if HAVE_NDIR_H
#  include <ndir.h>
#  define HAVE_DIR_STRUCT
# endif
#endif

#include <X11/Intrinsic.h>
#include <X11/StringDefs.h>
#include <X11/Shell.h>
#include <X11/cursorfont.h>
#include <X11/Xatom.h>
#if USE_XAW3D
#include <X11/Xaw3d/Dialog.h>
#include <X11/Xaw3d/Form.h>
#include <X11/Xaw3d/List.h>
#include <X11/Xaw3d/Label.h>
#include <X11/Xaw3d/SimpleMenu.h>
#include <X11/Xaw3d/SmeBSB.h>
#include <X11/Xaw3d/SmeLine.h>
#include <X11/Xaw3d/Box.h>
#include <X11/Xaw3d/MenuButton.h>
#include <X11/Xaw3d/Text.h>
#include <X11/Xaw3d/AsciiText.h>
#else
#include <X11/Xaw/Dialog.h>
#include <X11/Xaw/Form.h>
#include <X11/Xaw/List.h>
#include <X11/Xaw/Label.h>
#include <X11/Xaw/SimpleMenu.h>
#include <X11/Xaw/SmeBSB.h>
#include <X11/Xaw/SmeLine.h>
#include <X11/Xaw/Box.h>
#include <X11/Xaw/MenuButton.h>
#include <X11/Xaw/Text.h>
#include <X11/Xaw/AsciiText.h>
#endif

// [HGM] bitmaps: put before incuding the bitmaps / pixmaps, to know how many piece types there are.
#include "common.h"

#if HAVE_LIBXPM
#include <X11/xpm.h>
#include "pixmaps/pixmaps.h"
#define IMAGE_EXT "xpm"
#else
#define IMAGE_EXT "xim"
#include "bitmaps/bitmaps.h"
#endif

#include <gtk/gtk.h>
#include <gdk/gdk.h>
#include <gdk-pixbuf/gdk-pixbuf.h>

#include "bitmaps/icon_white.bm"
#include "bitmaps/icon_black.bm"
#include "bitmaps/checkmark.bm"

#include "frontend.h"
#include "backend.h"
#include "moves.h"
#include "xboard.h"
#include "childio.h"
#include "xgamelist.h"
#include "xhistory.h"
#include "xedittags.h"
#include "gettext.h"
#include "callback.h"
#include "interface.h"

// must be moved to xengineoutput.h

void EngineOutputProc P((Widget w, XEvent *event,
 String *prms, Cardinal *nprms));

void EngineOutputPopDown();


#ifdef __EMX__
#ifndef HAVE_USLEEP
#define HAVE_USLEEP
#endif
#define usleep(t)   _sleep2(((t)+500)/1000)
#endif

#ifdef ENABLE_NLS
# define  _(s) gettext (s)
# define N_(s) gettext_noop (s)
#else
# define  _(s) (s)
# define N_(s)  s
#endif

typedef struct {
    String string;
    XtActionProc proc;
} MenuItem;

typedef struct {
    String name;
    MenuItem *mi;
} Menu;

typedef struct {
    char *name;
    gboolean value;
} Enables;



int main P((int argc, char **argv));
RETSIGTYPE CmailSigHandler P((int sig));
RETSIGTYPE IntSigHandler P((int sig));
void CreateGCs P((void));
void CreateXIMPieces P((void));
void CreateXPMPieces P((void));
void CreatePieces P((void));
void CreatePieceMenus P((void));
Widget CreateMenuBar P((Menu *mb));
Widget CreateButtonBar P ((MenuItem *mi));
char *FindFont P((char *pattern, int targetPxlSize));
void PieceMenuPopup P((Widget w, XEvent *event,
		       String *params, Cardinal *num_params));
static void PieceMenuSelect P((Widget w, ChessSquare piece, caddr_t junk));
static void DropMenuSelect P((Widget w, ChessSquare piece, caddr_t junk));
int EventToSquare P((int x, int limit));
void DrawSquare P((int row, int column, ChessSquare piece, int do_flash));
void HandleUserMove P((Widget w, XEvent *event,
		     String *prms, Cardinal *nprms));
void AnimateUserMove P((Widget w, XEvent * event,
		     String * params, Cardinal * nParams));
void WhiteClock P((Widget w, XEvent *event,
		   String *prms, Cardinal *nprms));
void BlackClock P((Widget w, XEvent *event,
		   String *prms, Cardinal *nprms));
void CommentPopUp P((char *title, char *label));
void CommentPopDown P((void));
void CommentCallback P((Widget w, XtPointer client_data,
			XtPointer call_data));
void ICSInputBoxPopUp P((void));
void ICSInputBoxPopDown P((void));
void FileNamePopUp P((char *label, char *def,
		      FileProc proc, char *openMode));
void FileNamePopDown P((void));
void FileNameCallback P((Widget w, XtPointer client_data,
			 XtPointer call_data));
void FileNameAction P((Widget w, XEvent *event,
		       String *prms, Cardinal *nprms));
void AskQuestionReplyAction P((Widget w, XEvent *event,
			  String *prms, Cardinal *nprms));
void AskQuestionProc P((Widget w, XEvent *event,
			  String *prms, Cardinal *nprms));
void AskQuestionPopDown P((void));
void PromotionPopUp P((void));
void PromotionPopDown P((void));
void PromotionCallback P((Widget w, XtPointer client_data,
			  XtPointer call_data));
void EditCommentPopDown P((void));
void EditCommentCallback P((Widget w, XtPointer client_data,
			    XtPointer call_data));
void SelectCommand P((Widget w, XtPointer client_data, XtPointer call_data));
void LoadPositionProc P((Widget w, XEvent *event,
			 String *prms, Cardinal *nprms));
void LoadNextPositionProc P((Widget w, XEvent *event, String *prms,
			 Cardinal *nprms));
void LoadPrevPositionProc P((Widget w, XEvent *event, String *prms,
			 Cardinal *nprms));
void ReloadPositionProc P((Widget w, XEvent *event, String *prms,
		       Cardinal *nprms));
void CopyPositionProc P((Widget w, XEvent *event, String *prms,
			 Cardinal *nprms));
void PastePositionProc P((Widget w, XEvent *event, String *prms,
			  Cardinal *nprms));
void CopyGameProc P((Widget w, XEvent *event, String *prms, Cardinal *nprms));
void PasteGameProc P((Widget w, XEvent *event, String *prms, Cardinal *nprms));
void SaveGameProc P((Widget w, XEvent *event, String *prms, Cardinal *nprms));
void SavePositionProc P((Widget w, XEvent *event,
			 String *prms, Cardinal *nprms));
void MailMoveProc P((Widget w, XEvent *event, String *prms, Cardinal *nprms));
void ReloadCmailMsgProc P((Widget w, XEvent *event, String *prms,
			    Cardinal *nprms));
void AnalyzeModeProc P((Widget w, XEvent *event,
			 String *prms, Cardinal *nprms));
void AnalyzeFileProc P((Widget w, XEvent *event,
			 String *prms, Cardinal *nprms));
void EditGameProc P((Widget w, XEvent *event, String *prms, Cardinal *nprms));
void EditPositionProc P((Widget w, XEvent *event,
			 String *prms, Cardinal *nprms));
void TrainingProc P((Widget w, XEvent *event, String *prms, Cardinal *nprms));
void EditCommentProc P((Widget w, XEvent *event,
			String *prms, Cardinal *nprms));
void IcsInputBoxProc P((Widget w, XEvent *event,
			String *prms, Cardinal *nprms));
void EnterKeyProc P((Widget w, XEvent *event, String *prms, Cardinal *nprms));
void AlwaysQueenProc P((Widget w, XEvent *event, String *prms,
			Cardinal *nprms));
void AnimateDraggingProc P((Widget w, XEvent *event, String *prms,
			 Cardinal *nprms));
void AnimateMovingProc P((Widget w, XEvent *event, String *prms,
			 Cardinal *nprms));
void AutocommProc P((Widget w, XEvent *event, String *prms,
		     Cardinal *nprms));
void AutoflagProc P((Widget w, XEvent *event, String *prms, Cardinal *nprms));
void AutoflipProc P((Widget w, XEvent *event, String *prms, Cardinal *nprms));
void AutobsProc P((Widget w, XEvent *event, String *prms,
			Cardinal *nprms));
void AutoraiseProc P((Widget w, XEvent *event, String *prms, Cardinal *nprms));
void AutosaveProc P((Widget w, XEvent *event, String *prms, Cardinal *nprms));
void BlindfoldProc P((Widget w, XEvent *event, String *prms, Cardinal *nprms));
void FlashMovesProc P((Widget w, XEvent *event, String *prms,
		       Cardinal *nprms));
void HighlightDraggingProc P((Widget w, XEvent *event, String *prms,
			      Cardinal *nprms));
void HighlightLastMoveProc P((Widget w, XEvent *event, String *prms,
			      Cardinal *nprms));
void MoveSoundProc P((Widget w, XEvent *event, String *prms, Cardinal *nprms));
void IcsAlarmProc P((Widget w, XEvent *event, String *prms, Cardinal *nprms));
void OldSaveStyleProc P((Widget w, XEvent *event, String *prms,
			 Cardinal *nprms));
void PeriodicUpdatesProc P((Widget w, XEvent *event, String *prms,
			 Cardinal *nprms));
void PonderNextMoveProc P((Widget w, XEvent *event, String *prms,
			   Cardinal *nprms));
void PopupMoveErrorsProc P((Widget w, XEvent *event, String *prms,
			Cardinal *nprms));
void PopupExitMessageProc P((Widget w, XEvent *event, String *prms,
			     Cardinal *nprms));
void PremoveProc P((Widget w, XEvent *event, String *prms, Cardinal *nprms));
void QuietPlayProc P((Widget w, XEvent *event, String *prms, Cardinal *nprms));
void TestLegalityProc P((Widget w, XEvent *event, String *prms,
			  Cardinal *nprms));
void AboutGameProc P((Widget w, XEvent *event, String *prms, Cardinal *nprms));
void DebugProc P((Widget w, XEvent *event, String *prms, Cardinal *nprms));
void NothingProc P((Widget w, XEvent *event, String *prms, Cardinal *nprms));
void Iconify P((Widget w, XEvent *event, String *prms, Cardinal *nprms));
void DisplayMove P((int moveNumber));
void DisplayTitle P((char *title));
void ICSInitScript P((void));
int LoadGamePopUp P((FILE *f, int gameNumber, char *title));
void ErrorPopUp P((char *title, char *text, int modal));
void ErrorPopDown P((void));
static char *ExpandPathName P((char *path));
static void CreateAnimVars P((void));
void DragPieceBegin P((int x, int y));
static void DragPieceMove P((int x, int y));
void DragPieceEnd P((int x, int y));
static void DrawDragPiece P((void));
char *ModeToWidgetName P((GameMode mode));
void EngineOutputUpdate( FrontEndProgramStats * stats );
void ShuffleMenuProc P((Widget w, XEvent *event, String *prms, Cardinal *nprms));
void EngineMenuProc P((Widget w, XEvent *event, String *prms, Cardinal *nprms));
void UciMenuProc P((Widget w, XEvent *event, String *prms, Cardinal *nprms));
void TimeControlProc P((Widget w, XEvent *event, String *prms, Cardinal *nprms));
void NewVariantProc P((Widget w, XEvent *event, String *prms, Cardinal *nprms));
void FirstSettingsProc P((Widget w, XEvent *event, String *prms, Cardinal *nprms));
void SecondSettingsProc P((Widget w, XEvent *event, String *prms, Cardinal *nprms));
void ShufflePopDown P(());
void EnginePopDown P(());
void UciPopDown P(());
void TimeControlPopDown P(());
void NewVariantPopDown P(());
void SettingsPopDown P(());
void SetMenuEnables P((Enables *enab));

/*
* XBoard depends on Xt R4 or higher
*/
int xtVersion = XtSpecificationRelease;

int xScreen;
Display *xDisplay;
Window xBoardWindow;
Pixel lightSquareColor, darkSquareColor, whitePieceColor, blackPieceColor,
  jailSquareColor, highlightSquareColor, premoveHighlightColor;
Pixel lowTimeWarningColor;

#define LINE_TYPE_NORMAL 0
#define LINE_TYPE_HIGHLIGHT 1
#define LINE_TYPE_PRE 2


GC lightSquareGC, darkSquareGC, jailSquareGC,  wdPieceGC, wlPieceGC,
  bdPieceGC, blPieceGC, wbPieceGC, bwPieceGC, coordGC,
  wjPieceGC, bjPieceGC;
Pixmap iconPixmap, wIconPixmap, bIconPixmap, xMarkPixmap;
Widget shellWidget, layoutWidget, formWidget, boardWidget, messageWidget,
  whiteTimerWidget, blackTimerWidget, titleWidget, widgetList[16],
  commentShell, promotionShell, whitePieceMenu, blackPieceMenu, dropMenu,
  menuBarWidget, buttonBarWidget, editShell, errorShell, analysisShell,
  ICSInputShell, fileNameShell, askQuestionShell;
Font clockFontID, coordFontID, countFontID;
XFontStruct *clockFontStruct, *coordFontStruct, *countFontStruct;
XtAppContext appContext;
char *layoutName;
char *oldICSInteractionTitle;

FileProc fileProc;
char *fileOpenMode;
char installDir[] = "."; // [HGM] UCI: needed for UCI; probably needs run-time initializtion

Position commentX = -1, commentY = -1;
Dimension commentW, commentH;

int squareSize, smallLayout = 0, tinyLayout = 0,
  marginW, marginH, // [HGM] for run-time resizing
  fromX = -1, fromY = -1, toX, toY, commentUp = False, analysisUp = False,
  ICSInputBoxUp = False, askQuestionUp = False,
  filenameUp = False, promotionUp = False, pmFromX = -1, pmFromY = -1,
  editUp = False, errorUp = False, errorExitStatus = -1, lineGap;
Pixel timerForegroundPixel, timerBackgroundPixel;
Pixel buttonForegroundPixel, buttonBackgroundPixel;
char *chessDir, *programName, *programVersion,
  *gameCopyFilename, *gamePasteFilename;

#define SOLID 0
#define OUTLINE 1
Pixmap pieceBitmap[2][(int)BlackPawn];
Pixmap pieceBitmap2[2][(int)BlackPawn+4];       /* [HGM] pieces */
Pixmap xpmPieceBitmap[4][(int)BlackPawn];	/* LL, LD, DL, DD actually used*/
Pixmap xpmPieceBitmap2[4][(int)BlackPawn+4];	/* LL, LD, DL, DD set to select from */
Pixmap xpmLightSquare, xpmDarkSquare, xpmJailSquare;
int useImages=0, useImageSqs;
XImage *ximPieceBitmap[4][(int)BlackPawn+4];	/* LL, LD, DL, DD */
Pixmap ximMaskPm[(int)BlackPawn];               /* clipmasks, used for XIM pieces */
Pixmap ximMaskPm2[(int)BlackPawn+4];            /* clipmasks, used for XIM pieces */
XImage *ximLightSquare, *ximDarkSquare;
XImage *xim_Cross;

#define pieceToSolid(piece) &pieceBitmap[SOLID][(piece) % (int)BlackPawn]
#define pieceToOutline(piece) &pieceBitmap[OUTLINE][(piece) % (int)BlackPawn]

#define White(piece) ((int)(piece) < (int)BlackPawn)

/* Variables for doing smooth animation. This whole thing
   would be much easier if the board was double-buffered,
   but that would require a fairly major rewrite.	*/

typedef struct {
	Pixmap  saveBuf;
	Pixmap	newBuf;
	GC	blitGC, pieceGC, outlineGC;
	XPoint	startSquare, prevFrame, mouseDelta;
	int	startColor;
	int	dragPiece;
	Boolean	dragActive;
        int     startBoardX, startBoardY;
    } AnimState;

/* There can be two pieces being animated at once: a player
   can begin dragging a piece before the remote opponent has moved. */

static AnimState game, player;

/* Bitmaps for use as masks when drawing XPM pieces.
   Need one for each black and white piece.		*/
static Pixmap xpmMask[BlackKing + 1];

/* This magic number is the number of intermediate frames used
   in each half of the animation. For short moves it's reduced
   by 1. The total number of frames will be factor * 2 + 1.  */
#define kFactor	   4

SizeDefaults sizeDefaults[] = SIZE_DEFAULTS;

Enables icsEnables[] = {
    { "menuFile.Mail Move", False },
    { "menuFile.Reload CMail Message", False },
    { "menuMode.Machine Black", False },
    { "menuMode.Machine White", False },
    { "menuMode.Analysis Mode", False },
    { "menuMode.Analyze File", False },
    { "menuMode.Two Machines", False },
#ifndef ZIPPY
    { "menuHelp.Hint", False },
    { "menuHelp.Book", False },
    { "menuStep.Move Now", False },
    { "menuOptions.Periodic Updates", False },
    { "menuOptions.Hide Thinking", False },
    { "menuOptions.Ponder Next Move", False },
#endif
    { NULL, False }
};

Enables ncpEnables[] = {
    { "menuFile.Mail Move", False },
    { "menuFile.Reload CMail Message", False },
    { "menuMode.Machine White", False },
    { "menuMode.Machine Black", False },
    { "menuMode.Analysis Mode", False },
    { "menuMode.Analyze File", False },
    { "menuMode.Two Machines", False },
    { "menuMode.ICS Client", False },
    { "menuMode.ICS Input Box", False },
    { "Action", False },
    { "menuStep.Revert", False },
    { "menuStep.Move Now", False },
    { "menuStep.Retract Move", False },
    { "menuOptions.Auto Comment", False },
    { "menuOptions.Auto Flag", False },
    { "menuOptions.Auto Flip View", False },
    { "menuOptions.Auto Observe", False },
    { "menuOptions.Auto Raise Board", False },
    { "menuOptions.Get Move List", False },
    { "menuOptions.ICS Alarm", False },
    { "menuOptions.Move Sound", False },
    { "menuOptions.Quiet Play", False },
    { "menuOptions.Hide Thinking", False },
    { "menuOptions.Periodic Updates", False },
    { "menuOptions.Ponder Next Move", False },
    { "menuHelp.Hint", False },
    { "menuHelp.Book", False },
    { NULL, False }
};

Enables gnuEnables[] = {
    { "menuMode.ICS Client", False },
    { "menuMode.ICS Input Box", False },
    { "menuAction.Accept", False },
    { "menuAction.Decline", False },
    { "menuAction.Rematch", False },
    { "menuAction.Adjourn", False },
    { "menuAction.Stop Examining", False },
    { "menuAction.Stop Observing", False },
    { "menuStep.Revert", False },
    { "menuOptions.Auto Comment", False },
    { "menuOptions.Auto Observe", False },
    { "menuOptions.Auto Raise Board", False },
    { "menuOptions.Get Move List", False },
    { "menuOptions.Premove", False },
    { "menuOptions.Quiet Play", False },

    /* The next two options rely on SetCmailMode being called *after*    */
    /* SetGNUMode so that when GNU is being used to give hints these     */
    /* menu options are still available                                  */

    { "menuFile.Mail Move", False },
    { "menuFile.Reload CMail Message", False },
    { NULL, False }
};

Enables cmailEnables[] = {
    { "Action", True },
    { "menuAction.Call Flag", False },
    { "menuAction.Draw", True },
    { "menuAction.Adjourn", False },
    { "menuAction.Abort", False },
    { "menuAction.Stop Observing", False },
    { "menuAction.Stop Examining", False },
    { "menuFile.Mail Move", True },
    { "menuFile.Reload CMail Message", True },
    { NULL, False }
};

Enables trainingOnEnables[] = {
  { "menuMode.Edit Comment", False },
  { "menuMode.Pause", False },
  { "menuStep.Forward", False },
  { "menuStep.Backward", False },
  { "menuStep.Forward to End", False },
  { "menuStep.Back to Start", False },
  { "menuStep.Move Now", False },
  { "menuStep.Truncate Game", False },
  { NULL, False }
};

Enables trainingOffEnables[] = {
  { "menuMode.Edit Comment", True },
  { "menuMode.Pause", True },
  { "menuStep.Forward", True },
  { "menuStep.Backward", True },
  { "menuStep.Forward to End", True },
  { "menuStep.Back to Start", True },
  { "menuStep.Move Now", True },
  { "menuStep.Truncate Game", True },
  { NULL, False }
};

Enables machineThinkingEnables[] = {
  { "menuFile.Load Game", False },
  { "menuFile.Load Next Game", False },
  { "menuFile.Load Previous Game", False },
  { "menuFile.Reload Same Game", False },
  { "menuFile.Paste Game", False },
  { "menuFile.Load Position", False },
  { "menuFile.Load Next Position", False },
  { "menuFile.Load Previous Position", False },
  { "menuFile.Reload Same Position", False },
  { "menuFile.Paste Position", False },
  { "menuMode.Machine White", False },
  { "menuMode.Machine Black", False },
  { "menuMode.Two Machines", False },
  { "menuStep.Retract Move", False },
  { NULL, False }
};

Enables userThinkingEnables[] = {
  { "menuFile.Load Game", True },
  { "menuFile.Load Next Game", True },
  { "menuFile.Load Previous Game", True },
  { "menuFile.Reload Same Game", True },
  { "menuFile.Paste Game", True },
  { "menuFile.Load Position", True },
  { "menuFile.Load Next Position", True },
  { "menuFile.Load Previous Position", True },
  { "menuFile.Reload Same Position", True },
  { "menuFile.Paste Position", True },
  { "menuMode.Machine White", True },
  { "menuMode.Machine Black", True },
  { "menuMode.Two Machines", True },
  { "menuStep.Retract Move", True },
  { NULL, False }
};



MenuItem fileMenu[] = {
    {N_("New Shuffle Game ..."), ShuffleMenuProc},
    {N_("New Variant ..."), NewVariantProc},      // [HGM] variant: not functional yet
    {"----", NothingProc},
    {N_("Save Game"), SaveGameProc},
    {"----", NothingProc},
    {N_("Copy Game"), CopyGameProc},
    {N_("Paste Game"), PasteGameProc},
    {"----", NothingProc},
    {N_("Load Position"), LoadPositionProc},
    {N_("Load Next Position"), LoadNextPositionProc},
    {N_("Load Previous Position"), LoadPrevPositionProc},
    {N_("Reload Same Position"), ReloadPositionProc},
    {N_("Save Position"), SavePositionProc},
    {"----", NothingProc},
    {N_("Copy Position"), CopyPositionProc},
    {N_("Paste Position"), PastePositionProc},
    {"----", NothingProc},
    {N_("Mail Move"), MailMoveProc},
    {N_("Reload CMail Message"), ReloadCmailMsgProc},
    {"----", NothingProc},
    {NULL, NULL}
};

MenuItem modeMenu[] = {
  //    {N_("Machine White"), MachineWhiteProc},
  //    {N_("Machine Black"), MachineBlackProc},
  //    {N_("Two Machines"), TwoMachinesProc},
    {N_("Analysis Mode"), AnalyzeModeProc},
    {N_("Analyze File"), AnalyzeFileProc },
    //    {N_("ICS Client"), IcsClientProc},
    {N_("Edit Game"), EditGameProc},
    {N_("Edit Position"), EditPositionProc},
    {N_("Training"), TrainingProc},
    {"----", NothingProc},
    {N_("Show Engine Output"), EngineOutputProc},
    {N_("Show Evaluation Graph"), NothingProc}, // [HGM] evalgr: not functional yet
    {N_("Show Game List"), ShowGameListProc},
    //    {"Show Move History", HistoryShowProc}, // [HGM] hist: activate 4.2.7 code
    {"----", NothingProc},
    {N_("Edit Tags"), EditTagsProc},
    {N_("Edit Comment"), EditCommentProc},
    {N_("ICS Input Box"), IcsInputBoxProc},
    {NULL, NULL}
};

MenuItem optionsMenu[] = {
  //    {N_("Flip View"), FlipViewProc},
  //    {"----", NothingProc},
    {N_("Adjudications ..."), EngineMenuProc},
    {N_("General Settings ..."), UciMenuProc},
    {N_("Engine #1 Settings ..."), FirstSettingsProc},
    {N_("Engine #2 Settings ..."), SecondSettingsProc},
    {N_("Time Control ..."), TimeControlProc},
    {"----", NothingProc},
    {N_("Always Queen"), AlwaysQueenProc},
    {N_("Animate Dragging"), AnimateDraggingProc},
    {N_("Animate Moving"), AnimateMovingProc},
    {N_("Auto Comment"), AutocommProc},
    {N_("Auto Flag"), AutoflagProc},
    {N_("Auto Flip View"), AutoflipProc},
    {N_("Auto Observe"), AutobsProc},
    {N_("Auto Raise Board"), AutoraiseProc},
    {N_("Auto Save"), AutosaveProc},
    {N_("Blindfold"), BlindfoldProc},
    {N_("Flash Moves"), FlashMovesProc},
 //    {N_("Get Move List"), GetMoveListProc},
#if HIGHDRAG
    {N_("Highlight Dragging"), HighlightDraggingProc},
#endif
    {N_("Highlight Last Move"), HighlightLastMoveProc},
    {N_("Move Sound"), MoveSoundProc},
    {N_("ICS Alarm"), IcsAlarmProc},
    {N_("Old Save Style"), OldSaveStyleProc},
    {N_("Periodic Updates"), PeriodicUpdatesProc},
    {N_("Ponder Next Move"), PonderNextMoveProc},
    {N_("Popup Exit Message"), PopupExitMessageProc},
    {N_("Popup Move Errors"), PopupMoveErrorsProc},
    {N_("Premove"), PremoveProc},
    {N_("Quiet Play"), QuietPlayProc},
    //    {N_("Hide Thinking"), HideThinkingProc},
    {N_("Test Legality"), TestLegalityProc},
    {NULL, NULL}
};

Menu menuBar[] = {
    {N_("File"), fileMenu},
    {N_("Mode"), modeMenu},
    {N_("Options"), optionsMenu},
    {NULL, NULL}
};

#define PAUSE_BUTTON N_("P")
MenuItem buttonBar[] = {
  //    {"<<", ToStartProc},
    //    {"<", BackwardProc},
    //    {PAUSE_BUTTON, PauseProc},
    //    {">", ForwardProc},
  //    {">>", ToEndProc},
    {NULL, NULL}
};

#define PIECE_MENU_SIZE 18
String pieceMenuStrings[2][PIECE_MENU_SIZE] = {
    { N_("White"), "----", N_("Pawn"), N_("Knight"), N_("Bishop"), N_("Rook"),
      N_("Queen"), N_("King"), "----", N_("Elephant"), N_("Cannon"),
      N_("Archbishop"), N_("Chancellor"), "----", N_("Promote"), N_("Demote"),
      N_("Empty square"), N_("Clear board") },
    { N_("Black"), "----", N_("Pawn"), N_("Knight"), N_("Bishop"), N_("Rook"),
      N_("Queen"), N_("King"), "----", N_("Elephant"), N_("Cannon"),
      N_("Archbishop"), N_("Chancellor"), "----", N_("Promote"), N_("Demote"),
      N_("Empty square"), N_("Clear board") }
};
/* must be in same order as PieceMenuStrings! */
ChessSquare pieceMenuTranslation[2][PIECE_MENU_SIZE] = {
    { WhitePlay, (ChessSquare) 0, WhitePawn, WhiteKnight, WhiteBishop,
	WhiteRook, WhiteQueen, WhiteKing, (ChessSquare) 0, WhiteAlfil,
	WhiteCannon, WhiteAngel, WhiteMarshall, (ChessSquare) 0,
	PromotePiece, DemotePiece, EmptySquare, ClearBoard },
    { BlackPlay, (ChessSquare) 0, BlackPawn, BlackKnight, BlackBishop,
	BlackRook, BlackQueen, BlackKing, (ChessSquare) 0, BlackAlfil,
	BlackCannon, BlackAngel, BlackMarshall, (ChessSquare) 0,
	PromotePiece, DemotePiece, EmptySquare, ClearBoard },
};

#define DROP_MENU_SIZE 6
String dropMenuStrings[DROP_MENU_SIZE] = {
    "----", N_("Pawn"), N_("Knight"), N_("Bishop"), N_("Rook"), N_("Queen")
  };
/* must be in same order as PieceMenuStrings! */
ChessSquare dropMenuTranslation[DROP_MENU_SIZE] = {
    (ChessSquare) 0, WhitePawn, WhiteKnight, WhiteBishop,
    WhiteRook, WhiteQueen
};

typedef struct {
    char piece;
    char* widget;
} DropMenuEnables;

DropMenuEnables dmEnables[] = {
    { 'P', "Pawn" },
    { 'N', "Knight" },
    { 'B', "Bishop" },
    { 'R', "Rook" },
    { 'Q', "Queen" }
};

Arg layoutArgs[] = {
    { XtNborderWidth, 0 },
    { XtNdefaultDistance, 0 },
};

Arg formArgs[] = {
    { XtNborderWidth, 0 },
    { XtNresizable, (XtArgVal) True },
};

Arg boardArgs[] = {
    { XtNborderWidth, 0 },
    { XtNwidth, 0 },
    { XtNheight, 0 }
};

XtResource clientResources[] = {
    { "whitePieceColor", "whitePieceColor", XtRString, sizeof(String),
	XtOffset(AppDataPtr, whitePieceColor), XtRString,
	WHITE_PIECE_COLOR },
    { "blackPieceColor", "blackPieceColor", XtRString, sizeof(String),
	XtOffset(AppDataPtr, blackPieceColor), XtRString,
	BLACK_PIECE_COLOR },
    { "lightSquareColor", "lightSquareColor", XtRString,
	sizeof(String), XtOffset(AppDataPtr, lightSquareColor),
	XtRString, LIGHT_SQUARE_COLOR },
    { "darkSquareColor", "darkSquareColor", XtRString, sizeof(String),
	XtOffset(AppDataPtr, darkSquareColor), XtRString,
	DARK_SQUARE_COLOR },
    { "highlightSquareColor", "highlightSquareColor", XtRString,
	sizeof(String),	XtOffset(AppDataPtr, highlightSquareColor),
	XtRString, HIGHLIGHT_SQUARE_COLOR },
    { "premoveHighlightColor", "premoveHighlightColor", XtRString,
	sizeof(String),	XtOffset(AppDataPtr, premoveHighlightColor),
	XtRString, PREMOVE_HIGHLIGHT_COLOR },
    { "movesPerSession", "movesPerSession", XtRInt, sizeof(int),
	XtOffset(AppDataPtr, movesPerSession), XtRImmediate,
	(XtPointer) MOVES_PER_SESSION },
    { "timeIncrement", "timeIncrement", XtRInt, sizeof(int),
	XtOffset(AppDataPtr, timeIncrement), XtRImmediate,
	(XtPointer) TIME_INCREMENT },
    { "initString", "initString", XtRString, sizeof(String),
	XtOffset(AppDataPtr, initString), XtRString, INIT_STRING },
    { "secondInitString", "secondInitString", XtRString, sizeof(String),
	XtOffset(AppDataPtr, secondInitString), XtRString, INIT_STRING },
    { "firstComputerString", "firstComputerString", XtRString,
        sizeof(String),	XtOffset(AppDataPtr, firstComputerString), XtRString,
      COMPUTER_STRING },
    { "secondComputerString", "secondComputerString", XtRString,
        sizeof(String),	XtOffset(AppDataPtr, secondComputerString), XtRString,
      COMPUTER_STRING },
    { "firstChessProgram", "firstChessProgram", XtRString,
	sizeof(String), XtOffset(AppDataPtr, firstChessProgram),
	XtRString, FIRST_CHESS_PROGRAM },
    { "secondChessProgram", "secondChessProgram", XtRString,
	sizeof(String), XtOffset(AppDataPtr, secondChessProgram),
	XtRString, SECOND_CHESS_PROGRAM },
    { "firstPlaysBlack", "firstPlaysBlack", XtRBoolean,
	sizeof(Boolean), XtOffset(AppDataPtr, firstPlaysBlack),
	XtRImmediate, (XtPointer) False },
    { "noChessProgram", "noChessProgram", XtRBoolean,
	sizeof(Boolean), XtOffset(AppDataPtr, noChessProgram),
	XtRImmediate, (XtPointer) False },
    { "firstHost", "firstHost", XtRString, sizeof(String),
	XtOffset(AppDataPtr, firstHost), XtRString, FIRST_HOST },
    { "secondHost", "secondHost", XtRString, sizeof(String),
	XtOffset(AppDataPtr, secondHost), XtRString, SECOND_HOST },
    { "firstDirectory", "firstDirectory", XtRString, sizeof(String),
	XtOffset(AppDataPtr, firstDirectory), XtRString, "." },
    { "secondDirectory", "secondDirectory", XtRString, sizeof(String),
	XtOffset(AppDataPtr, secondDirectory), XtRString, "." },
    { "bitmapDirectory", "bitmapDirectory", XtRString,
	sizeof(String), XtOffset(AppDataPtr, bitmapDirectory),
	XtRString, "" },
    { "remoteShell", "remoteShell", XtRString, sizeof(String),
	XtOffset(AppDataPtr, remoteShell), XtRString, REMOTE_SHELL },
    { "remoteUser", "remoteUser", XtRString, sizeof(String),
	XtOffset(AppDataPtr, remoteUser), XtRString, "" },
    { "timeDelay", "timeDelay", XtRFloat, sizeof(float),
	XtOffset(AppDataPtr, timeDelay), XtRString,
	(XtPointer) TIME_DELAY_QUOTE },
    { "timeControl", "timeControl", XtRString, sizeof(String),
	XtOffset(AppDataPtr, timeControl), XtRString,
	(XtPointer) TIME_CONTROL },
    { "internetChessServerMode", "internetChessServerMode",
	XtRBoolean, sizeof(Boolean),
	XtOffset(AppDataPtr, icsActive), XtRImmediate,
	(XtPointer) False },
    { "internetChessServerHost", "internetChessServerHost",
	XtRString, sizeof(String),
	XtOffset(AppDataPtr, icsHost),
	XtRString, (XtPointer) ICS_HOST },
    { "internetChessServerPort", "internetChessServerPort",
	XtRString, sizeof(String),
	XtOffset(AppDataPtr, icsPort), XtRString,
	(XtPointer) ICS_PORT },
    { "internetChessServerCommPort", "internetChessServerCommPort",
	XtRString, sizeof(String),
	XtOffset(AppDataPtr, icsCommPort), XtRString,
	ICS_COMM_PORT },
    { "internetChessServerLogonScript", "internetChessServerLogonScript",
	XtRString, sizeof(String),
	XtOffset(AppDataPtr, icsLogon), XtRString,
	ICS_LOGON },
    { "internetChessServerHelper", "internetChessServerHelper",
	XtRString, sizeof(String),
	XtOffset(AppDataPtr, icsHelper), XtRString, "" },
    { "internetChessServerInputBox", "internetChessServerInputBox",
	XtRBoolean, sizeof(Boolean),
	XtOffset(AppDataPtr, icsInputBox), XtRImmediate,
	(XtPointer) False },
    { "icsAlarm", "icsAlarm",
	XtRBoolean, sizeof(Boolean),
	XtOffset(AppDataPtr, icsAlarm), XtRImmediate,
	(XtPointer) True },
    { "icsAlarmTime", "icsAlarmTime",
	XtRInt, sizeof(int),
	XtOffset(AppDataPtr, icsAlarmTime), XtRImmediate,
	(XtPointer) 5000 },
    { "useTelnet", "useTelnet", XtRBoolean, sizeof(Boolean),
	XtOffset(AppDataPtr, useTelnet), XtRImmediate,
	(XtPointer) False },
    { "telnetProgram", "telnetProgram", XtRString, sizeof(String),
	XtOffset(AppDataPtr, telnetProgram), XtRString, TELNET_PROGRAM },
    { "gateway", "gateway", XtRString, sizeof(String),
	XtOffset(AppDataPtr, gateway), XtRString, "" },
    { "loadGameFile", "loadGameFile", XtRString, sizeof(String),
	XtOffset(AppDataPtr, loadGameFile), XtRString, "" },
    { "loadGameIndex", "loadGameIndex",
	XtRInt, sizeof(int),
	XtOffset(AppDataPtr, loadGameIndex), XtRImmediate,
	(XtPointer) 0 },
    { "saveGameFile", "saveGameFile", XtRString, sizeof(String),
	XtOffset(AppDataPtr, saveGameFile), XtRString, "" },
    { "autoRaiseBoard", "autoRaiseBoard", XtRBoolean,
	sizeof(Boolean), XtOffset(AppDataPtr, autoRaiseBoard),
	XtRImmediate, (XtPointer) True },
    { "autoSaveGames", "autoSaveGames", XtRBoolean,
	sizeof(Boolean), XtOffset(AppDataPtr, autoSaveGames),
	XtRImmediate, (XtPointer) False },
    { "blindfold", "blindfold", XtRBoolean,
	sizeof(Boolean), XtOffset(AppDataPtr, blindfold),
	XtRImmediate, (XtPointer) False },
    { "loadPositionFile", "loadPositionFile", XtRString,
	sizeof(String), XtOffset(AppDataPtr, loadPositionFile),
	XtRString, "" },
    { "loadPositionIndex", "loadPositionIndex",
	XtRInt, sizeof(int),
	XtOffset(AppDataPtr, loadPositionIndex), XtRImmediate,
	(XtPointer) 1 },
    { "savePositionFile", "savePositionFile", XtRString,
	sizeof(String), XtOffset(AppDataPtr, savePositionFile),
	XtRString, "" },
    { "matchMode", "matchMode", XtRBoolean, sizeof(Boolean),
	XtOffset(AppDataPtr, matchMode), XtRImmediate, (XtPointer) False },
    { "matchGames", "matchGames", XtRInt, sizeof(int),
	XtOffset(AppDataPtr, matchGames), XtRImmediate,
	(XtPointer) 0 },
    { "monoMode", "monoMode", XtRBoolean, sizeof(Boolean),
	XtOffset(AppDataPtr, monoMode), XtRImmediate,
	(XtPointer) False },
    { "debugMode", "debugMode", XtRBoolean, sizeof(Boolean),
	XtOffset(AppDataPtr, debugMode), XtRImmediate,
	(XtPointer) False },
    { "clockMode", "clockMode", XtRBoolean, sizeof(Boolean),
	XtOffset(AppDataPtr, clockMode), XtRImmediate,
	(XtPointer) True },
    { "boardSize", "boardSize", XtRString, sizeof(String),
	XtOffset(AppDataPtr, boardSize), XtRString, "" },
    { "searchTime", "searchTime", XtRString, sizeof(String),
	XtOffset(AppDataPtr, searchTime), XtRString,
	(XtPointer) "" },
    { "searchDepth", "searchDepth", XtRInt, sizeof(int),
	XtOffset(AppDataPtr, searchDepth), XtRImmediate,
	(XtPointer) 0 },
    { "showCoords", "showCoords", XtRBoolean, sizeof(Boolean),
	XtOffset(AppDataPtr, showCoords), XtRImmediate,
	(XtPointer) False },
    { "showJail", "showJail", XtRInt, sizeof(int),
	XtOffset(AppDataPtr, showJail), XtRImmediate,
	(XtPointer) 0 },
    { "showThinking", "showThinking", XtRBoolean, sizeof(Boolean),
	XtOffset(AppDataPtr, showThinking), XtRImmediate,
	(XtPointer) True },
    { "ponderNextMove", "ponderNextMove", XtRBoolean, sizeof(Boolean),
	XtOffset(AppDataPtr, ponderNextMove), XtRImmediate,
	(XtPointer) True },
    { "periodicUpdates", "periodicUpdates", XtRBoolean, sizeof(Boolean),
	XtOffset(AppDataPtr, periodicUpdates), XtRImmediate,
	(XtPointer) True },
    { "clockFont", "clockFont", XtRString, sizeof(String),
	XtOffset(AppDataPtr, clockFont), XtRString, CLOCK_FONT },
    { "coordFont", "coordFont", XtRString, sizeof(String),
	XtOffset(AppDataPtr, coordFont), XtRString, COORD_FONT },
    { "font", "font", XtRString, sizeof(String),
	XtOffset(AppDataPtr, font), XtRString, DEFAULT_FONT },
    { "ringBellAfterMoves", "ringBellAfterMoves",
	XtRBoolean, sizeof(Boolean),
	XtOffset(AppDataPtr, ringBellAfterMoves),
	XtRImmediate, (XtPointer) False	},
    { "autoCallFlag", "autoCallFlag", XtRBoolean,
	sizeof(Boolean), XtOffset(AppDataPtr, autoCallFlag),
	XtRImmediate, (XtPointer) False },
    { "autoFlipView", "autoFlipView", XtRBoolean,
	sizeof(Boolean), XtOffset(AppDataPtr, autoFlipView),
	XtRImmediate, (XtPointer) True },
    { "autoObserve", "autoObserve", XtRBoolean,
	sizeof(Boolean), XtOffset(AppDataPtr, autoObserve),
	XtRImmediate, (XtPointer) False },
    { "autoComment", "autoComment", XtRBoolean,
	sizeof(Boolean), XtOffset(AppDataPtr, autoComment),
	XtRImmediate, (XtPointer) False },
    { "getMoveList", "getMoveList", XtRBoolean,
	sizeof(Boolean), XtOffset(AppDataPtr, getMoveList),
	XtRImmediate, (XtPointer) True },
#if HIGHDRAG
    { "highlightDragging", "highlightDragging", XtRBoolean,
	sizeof(Boolean), XtOffset(AppDataPtr, highlightDragging),
	XtRImmediate, (XtPointer) False },
#endif
    { "highlightLastMove", "highlightLastMove", XtRBoolean,
	sizeof(Boolean), XtOffset(AppDataPtr, highlightLastMove),
	XtRImmediate, (XtPointer) False },
    { "premove", "premove", XtRBoolean,
	sizeof(Boolean), XtOffset(AppDataPtr, premove),
        XtRImmediate, (XtPointer) True },
    { "testLegality", "testLegality", XtRBoolean,
	sizeof(Boolean), XtOffset(AppDataPtr, testLegality),
	XtRImmediate, (XtPointer) True },
    { "flipView", "flipView", XtRBoolean,
	sizeof(Boolean), XtOffset(AppDataPtr, flipView),
	XtRImmediate, (XtPointer) False },
    { "cmail", "cmailGameName", XtRString, sizeof(String),
	XtOffset(AppDataPtr, cmailGameName), XtRString, "" },
    { "alwaysPromoteToQueen", "alwaysPromoteToQueen", XtRBoolean,
	sizeof(Boolean), XtOffset(AppDataPtr, alwaysPromoteToQueen),
	XtRImmediate, (XtPointer) False },
    { "oldSaveStyle", "oldSaveStyle", XtRBoolean,
	sizeof(Boolean), XtOffset(AppDataPtr, oldSaveStyle),
	XtRImmediate, (XtPointer) False },
    { "quietPlay", "quietPlay", XtRBoolean,
	sizeof(Boolean), XtOffset(AppDataPtr, quietPlay),
	XtRImmediate, (XtPointer) False },
    { "titleInWindow", "titleInWindow", XtRBoolean,
	sizeof(Boolean), XtOffset(AppDataPtr, titleInWindow),
	XtRImmediate, (XtPointer) False },
    { "localLineEditing", "localLineEditing", XtRBoolean,
	sizeof(Boolean), XtOffset(AppDataPtr, localLineEditing),
	XtRImmediate, (XtPointer) True }, /* not implemented, must be True */
#if ZIPPY
    { "zippyTalk", "zippyTalk", XtRBoolean,
	sizeof(Boolean), XtOffset(AppDataPtr, zippyTalk),
	XtRImmediate, (XtPointer) ZIPPY_TALK },
    { "zippyPlay", "zippyPlay", XtRBoolean,
	sizeof(Boolean), XtOffset(AppDataPtr, zippyPlay),
	XtRImmediate, (XtPointer) ZIPPY_PLAY },
    { "zippyLines", "zippyLines", XtRString, sizeof(String),
	XtOffset(AppDataPtr, zippyLines), XtRString, ZIPPY_LINES },
    { "zippyPinhead", "zippyPinhead", XtRString, sizeof(String),
        XtOffset(AppDataPtr, zippyPinhead), XtRString, ZIPPY_PINHEAD },
    { "zippyPassword", "zippyPassword", XtRString, sizeof(String),
	XtOffset(AppDataPtr, zippyPassword), XtRString, ZIPPY_PASSWORD },
    { "zippyPassword2", "zippyPassword2", XtRString, sizeof(String),
	XtOffset(AppDataPtr, zippyPassword2), XtRString, ZIPPY_PASSWORD2 },
    { "zippyWrongPassword", "zippyWrongPassword", XtRString, sizeof(String),
	XtOffset(AppDataPtr, zippyWrongPassword), XtRString,
        ZIPPY_WRONG_PASSWORD },
    { "zippyAcceptOnly", "zippyAcceptOnly", XtRString, sizeof(String),
	XtOffset(AppDataPtr, zippyAcceptOnly), XtRString, ZIPPY_ACCEPT_ONLY },
    { "zippyUseI", "zippyUseI", XtRBoolean,
	sizeof(Boolean), XtOffset(AppDataPtr, zippyUseI),
        XtRImmediate, (XtPointer) ZIPPY_USE_I },
    { "zippyBughouse", "zippyBughouse", XtRInt,
	sizeof(int), XtOffset(AppDataPtr, zippyBughouse),
	XtRImmediate, (XtPointer) ZIPPY_BUGHOUSE },
    { "zippyNoplayCrafty", "zippyNoplayCrafty", XtRBoolean,
	sizeof(Boolean), XtOffset(AppDataPtr, zippyNoplayCrafty),
        XtRImmediate, (XtPointer) ZIPPY_NOPLAY_CRAFTY },
    { "zippyGameEnd", "zippyGameEnd", XtRString, sizeof(String),
	XtOffset(AppDataPtr, zippyGameEnd), XtRString, ZIPPY_GAME_END },
    { "zippyGameStart", "zippyGameStart", XtRString, sizeof(String),
	XtOffset(AppDataPtr, zippyGameStart), XtRString, ZIPPY_GAME_START },
    { "zippyAdjourn", "zippyAdjourn", XtRBoolean,
	sizeof(Boolean), XtOffset(AppDataPtr, zippyAdjourn),
	XtRImmediate, (XtPointer) ZIPPY_ADJOURN },
    { "zippyAbort", "zippyAbort", XtRBoolean,
	sizeof(Boolean), XtOffset(AppDataPtr, zippyAbort),
	XtRImmediate, (XtPointer) ZIPPY_ABORT },
    { "zippyVariants", "zippyVariants", XtRString, sizeof(String),
	XtOffset(AppDataPtr, zippyVariants), XtRString, ZIPPY_VARIANTS },
    { "zippyMaxGames", "zippyMaxGames", XtRInt, sizeof(int),
	XtOffset(AppDataPtr, zippyMaxGames), XtRImmediate,
        (XtPointer) ZIPPY_MAX_GAMES },
    { "zippyReplayTimeout", "zippyReplayTimeout", XtRInt, sizeof(int),
	XtOffset(AppDataPtr, zippyReplayTimeout), XtRImmediate,
        (XtPointer) ZIPPY_REPLAY_TIMEOUT },
    { "zippyShortGame", "zippyShortGame", XtRInt, sizeof(int),
	XtOffset(AppDataPtr, zippyShortGame), XtRImmediate,
        (XtPointer) 0 },
#endif
    { "flashCount", "flashCount", XtRInt, sizeof(int),
	XtOffset(AppDataPtr, flashCount), XtRImmediate,
	(XtPointer) FLASH_COUNT  },
    { "flashRate", "flashRate", XtRInt, sizeof(int),
	XtOffset(AppDataPtr, flashRate), XtRImmediate,
	(XtPointer) FLASH_RATE },
    { "pixmapDirectory", "pixmapDirectory", XtRString,
	sizeof(String), XtOffset(AppDataPtr, pixmapDirectory),
	XtRString, "" },
    { "msLoginDelay", "msLoginDelay", XtRInt, sizeof(int),
	XtOffset(AppDataPtr, msLoginDelay), XtRImmediate,
	(XtPointer) MS_LOGIN_DELAY },
    { "colorizeMessages", "colorizeMessages", XtRBoolean,
	sizeof(Boolean), XtOffset(AppDataPtr, colorize),
	XtRImmediate, (XtPointer) False },
    { "colorShout", "colorShout", XtRString,
	sizeof(String), XtOffset(AppDataPtr, colorShout),
	XtRString, COLOR_SHOUT },
    { "colorSShout", "colorSShout", XtRString,
	sizeof(String), XtOffset(AppDataPtr, colorSShout),
	XtRString, COLOR_SSHOUT },
    { "colorChannel1", "colorChannel1", XtRString,
	sizeof(String), XtOffset(AppDataPtr, colorChannel1),
	XtRString, COLOR_CHANNEL1 },
    { "colorChannel", "colorChannel", XtRString,
	sizeof(String), XtOffset(AppDataPtr, colorChannel),
	XtRString, COLOR_CHANNEL },
    { "colorKibitz", "colorKibitz", XtRString,
	sizeof(String), XtOffset(AppDataPtr, colorKibitz),
	XtRString, COLOR_KIBITZ },
    { "colorTell", "colorTell", XtRString,
	sizeof(String), XtOffset(AppDataPtr, colorTell),
	XtRString, COLOR_TELL },
    { "colorChallenge", "colorChallenge", XtRString,
	sizeof(String), XtOffset(AppDataPtr, colorChallenge),
	XtRString, COLOR_CHALLENGE },
    { "colorRequest", "colorRequest", XtRString,
	sizeof(String), XtOffset(AppDataPtr, colorRequest),
	XtRString, COLOR_REQUEST },
    { "colorSeek", "colorSeek", XtRString,
	sizeof(String), XtOffset(AppDataPtr, colorSeek),
	XtRString, COLOR_SEEK },
    { "colorNormal", "colorNormal", XtRString,
	sizeof(String), XtOffset(AppDataPtr, colorNormal),
	XtRString, COLOR_NORMAL },
    { "soundProgram", "soundProgram", XtRString,
      sizeof(String), XtOffset(AppDataPtr, soundProgram),
      XtRString, "play" },
    { "soundShout", "soundShout", XtRString,
      sizeof(String), XtOffset(AppDataPtr, soundShout),
      XtRString, "" },
    { "soundSShout", "soundSShout", XtRString,
      sizeof(String), XtOffset(AppDataPtr, soundSShout),
      XtRString, "" },
    { "soundChannel1", "soundChannel1", XtRString,
      sizeof(String), XtOffset(AppDataPtr, soundChannel1),
      XtRString, "" },
    { "soundChannel", "soundChannel", XtRString,
      sizeof(String), XtOffset(AppDataPtr, soundChannel),
      XtRString, "" },
    { "soundKibitz", "soundKibitz", XtRString,
      sizeof(String), XtOffset(AppDataPtr, soundKibitz),
      XtRString, "" },
    { "soundTell", "soundTell", XtRString,
      sizeof(String), XtOffset(AppDataPtr, soundTell),
      XtRString, "" },
    { "soundChallenge", "soundChallenge", XtRString,
      sizeof(String), XtOffset(AppDataPtr, soundChallenge),
      XtRString, "" },
    { "soundRequest", "soundRequest", XtRString,
      sizeof(String), XtOffset(AppDataPtr, soundRequest),
      XtRString, "" },
    { "soundSeek", "soundSeek", XtRString,
      sizeof(String), XtOffset(AppDataPtr, soundSeek),
      XtRString, "" },
    { "soundMove", "soundMove", XtRString,
      sizeof(String), XtOffset(AppDataPtr, soundMove),
      XtRString, "$" },
    { "soundIcsWin", "soundIcsWin", XtRString,
      sizeof(String), XtOffset(AppDataPtr, soundIcsWin),
      XtRString, "" },
    { "soundIcsLoss", "soundIcsLoss", XtRString,
      sizeof(String), XtOffset(AppDataPtr, soundIcsLoss),
      XtRString, "" },
    { "soundIcsDraw", "soundIcsDraw", XtRString,
      sizeof(String), XtOffset(AppDataPtr, soundIcsDraw),
      XtRString, "" },
    { "soundIcsUnfinished", "soundIcsUnfinished", XtRString,
      sizeof(String), XtOffset(AppDataPtr, soundIcsUnfinished),
      XtRString, "" },
    { "soundIcsAlarm", "soundIcsAlarm", XtRString,
      sizeof(String), XtOffset(AppDataPtr, soundIcsAlarm),
      XtRString, "$" },
    { "reuseFirst", "reuseFirst", XtRBoolean,
	sizeof(Boolean), XtOffset(AppDataPtr, reuseFirst),
	XtRImmediate, (XtPointer) True },
    { "reuseSecond", "reuseSecond", XtRBoolean,
	sizeof(Boolean), XtOffset(AppDataPtr, reuseSecond),
	XtRImmediate, (XtPointer) True },
    { "animateDragging", "animateDragging", XtRBoolean,
        sizeof(Boolean), XtOffset(AppDataPtr, animateDragging),
	XtRImmediate, (XtPointer) True },
    { "animateMoving", "animateMoving", XtRBoolean,
        sizeof(Boolean), XtOffset(AppDataPtr, animate),
	XtRImmediate, (XtPointer) True },
    { "animateSpeed", "animateSpeed", XtRInt,
        sizeof(int), XtOffset(AppDataPtr, animSpeed),
	XtRImmediate, (XtPointer)10 },
    { "popupExitMessage", "popupExitMessage", XtRBoolean,
	sizeof(Boolean), XtOffset(AppDataPtr, popupExitMessage),
	XtRImmediate, (XtPointer) True },
    { "popupMoveErrors", "popupMoveErrors", XtRBoolean,
	sizeof(Boolean), XtOffset(AppDataPtr, popupMoveErrors),
	XtRImmediate, (XtPointer) False },
    { "fontSizeTolerance", "fontSizeTolerance", XtRInt,
        sizeof(int), XtOffset(AppDataPtr, fontSizeTolerance),
	XtRImmediate, (XtPointer)4 },
    { "initialMode", "initialMode", XtRString,
        sizeof(String), XtOffset(AppDataPtr, initialMode),
	XtRImmediate, (XtPointer) "" },
    { "variant", "variant", XtRString,
        sizeof(String), XtOffset(AppDataPtr, variant),
	XtRImmediate, (XtPointer) "normal" },
    { "firstProtocolVersion", "firstProtocolVersion", XtRInt,
        sizeof(int), XtOffset(AppDataPtr, firstProtocolVersion),
	XtRImmediate, (XtPointer)PROTOVER },
    { "secondProtocolVersion", "secondProtocolVersion", XtRInt,
        sizeof(int), XtOffset(AppDataPtr, secondProtocolVersion),
	XtRImmediate, (XtPointer)PROTOVER },
    { "showButtonBar", "showButtonBar", XtRBoolean,
	sizeof(Boolean), XtOffset(AppDataPtr, showButtonBar),
	XtRImmediate, (XtPointer) True },
    { "lowTimeWarningColor", "lowTimeWarningColor", XtRString,
      sizeof(String), XtOffset(AppDataPtr, lowTimeWarningColor),
      XtRString, COLOR_LOWTIMEWARNING },
    { "lowTimeWarning", "lowTimeWarning", XtRBoolean,
      sizeof(Boolean), XtOffset(AppDataPtr, lowTimeWarning),
      XtRImmediate, (XtPointer) False },
    {"icsEngineAnalyze", "icsEngineAnalyze", XtRBoolean,        /* [DM] icsEngineAnalyze */
        sizeof(Boolean), XtOffset(AppDataPtr, icsEngineAnalyze),
        XtRImmediate, (XtPointer) False },
    { "firstScoreAbs", "firstScoreAbs", XtRBoolean,
	sizeof(Boolean), XtOffset(AppDataPtr, firstScoreIsAbsolute),
	XtRImmediate, (XtPointer) False },
    { "secondScoreAbs", "secondScoreAbs", XtRBoolean,
	sizeof(Boolean), XtOffset(AppDataPtr, secondScoreIsAbsolute),
	XtRImmediate, (XtPointer) False },
    { "pgnExtendedInfo", "pgnExtendedInfo", XtRBoolean,
	sizeof(Boolean), XtOffset(AppDataPtr, saveExtendedInfoInPGN),
	XtRImmediate, (XtPointer) False },
    { "hideThinkingFromHuman", "hideThinkingFromHuman", XtRBoolean,
	sizeof(Boolean), XtOffset(AppDataPtr, hideThinkingFromHuman),
	XtRImmediate, (XtPointer) True },
    { "adjudicateLossThreshold", "adjudicateLossThreshold", XtRInt,
	sizeof(int), XtOffset(AppDataPtr, adjudicateLossThreshold),
	XtRImmediate, (XtPointer) 0},
    { "adjudicateDrawMoves", "adjudicateDrawMoves", XtRInt,
	sizeof(int), XtOffset(AppDataPtr, adjudicateDrawMoves),
	XtRImmediate, (XtPointer) 0},
    { "pgnEventHeader", "pgnEventHeader", XtRString,
        sizeof(String), XtOffset(AppDataPtr, pgnEventHeader),
	XtRImmediate, (XtPointer) "Computer Chess Game" },
    { "defaultFrcPosition", "defaultFrcPositon", XtRInt,
	sizeof(int), XtOffset(AppDataPtr, defaultFrcPosition),
	XtRImmediate, (XtPointer) -1},
    { "gameListTags", "gameListTags", XtRString,
        sizeof(String), XtOffset(AppDataPtr, gameListTags),
	XtRImmediate, (XtPointer) GLT_DEFAULT_TAGS },

    // [HGM] 4.3.xx options
    { "boardWidth", "boardWidth", XtRInt,
	sizeof(int), XtOffset(AppDataPtr, NrFiles),
	XtRImmediate, (XtPointer) -1},
    { "boardHeight", "boardHeight", XtRInt,
	sizeof(int), XtOffset(AppDataPtr, NrRanks),
	XtRImmediate, (XtPointer) -1},
    { "matchPause", "matchPause", XtRInt,
	sizeof(int), XtOffset(AppDataPtr, matchPause),
	XtRImmediate, (XtPointer) 10000},
    { "holdingsSize", "holdingsSize", XtRInt,
	sizeof(int), XtOffset(AppDataPtr, holdingsSize),
	XtRImmediate, (XtPointer) -1},
    { "flipBlack", "flipBlack", XtRBoolean,
	sizeof(Boolean), XtOffset(AppDataPtr, upsideDown),
	XtRImmediate, (XtPointer) False},
    { "allWhite", "allWhite", XtRBoolean,
	sizeof(Boolean), XtOffset(AppDataPtr, allWhite),
	XtRImmediate, (XtPointer) False},
    { "pieceToCharTable", "pieceToCharTable", XtRString,
	sizeof(String), XtOffset(AppDataPtr, pieceToCharTable),
	XtRImmediate, (XtPointer) 0},
    { "alphaRank", "alphaRank", XtRBoolean,
	sizeof(Boolean), XtOffset(AppDataPtr, alphaRank),
	XtRImmediate, (XtPointer) False},
    { "testClaims", "testClaims", XtRBoolean,
	sizeof(Boolean), XtOffset(AppDataPtr, testClaims),
	XtRImmediate, (XtPointer) True},
    { "checkMates", "checkMates", XtRBoolean,
	sizeof(Boolean), XtOffset(AppDataPtr, checkMates),
	XtRImmediate, (XtPointer) True},
    { "materialDraws", "materialDraws", XtRBoolean,
	sizeof(Boolean), XtOffset(AppDataPtr, materialDraws),
	XtRImmediate, (XtPointer) True},
    { "trivialDraws", "trivialDraws", XtRBoolean,
	sizeof(Boolean), XtOffset(AppDataPtr, trivialDraws),
	XtRImmediate, (XtPointer) False},
    { "ruleMoves", "ruleMoves", XtRInt,
	sizeof(int), XtOffset(AppDataPtr, ruleMoves),
	XtRImmediate, (XtPointer) 51},
    { "repeatsToDraw", "repeatsToDraw", XtRInt,
	sizeof(int), XtOffset(AppDataPtr, drawRepeats),
	XtRImmediate, (XtPointer) 6},
    { "engineDebugOutput", "engineDebugOutput", XtRInt,
	sizeof(int), XtOffset(AppDataPtr, engineComments),
	XtRImmediate, (XtPointer) 1},
    { "userName", "userName", XtRString,
	sizeof(int), XtOffset(AppDataPtr, userName),
	XtRImmediate, (XtPointer) 0},
    { "autoKibitz", "autoKibitz", XtRBoolean,
	sizeof(Boolean), XtOffset(AppDataPtr, autoKibitz),
	XtRImmediate, (XtPointer) False},
    { "firstTimeOdds", "firstTimeOdds", XtRInt,
	sizeof(int), XtOffset(AppDataPtr, firstTimeOdds),
	XtRImmediate, (XtPointer) 1},
    { "secondTimeOdds", "secondTimeOdds", XtRInt,
	sizeof(int), XtOffset(AppDataPtr, secondTimeOdds),
	XtRImmediate, (XtPointer) 1},
    { "timeOddsMode", "timeOddsMode", XtRInt,
	sizeof(int), XtOffset(AppDataPtr, timeOddsMode),
	XtRImmediate, (XtPointer) 0},
    { "firstAccumulateTC", "firstAccumulateTC", XtRInt,
	sizeof(int), XtOffset(AppDataPtr, firstAccumulateTC),
	XtRImmediate, (XtPointer) 1},
    { "secondAccumulateTC", "secondAccumulateTC", XtRInt,
	sizeof(int), XtOffset(AppDataPtr, secondAccumulateTC),
	XtRImmediate, (XtPointer) 1},
    { "firstNPS", "firstNPS", XtRInt,
	sizeof(int), XtOffset(AppDataPtr, firstNPS),
	XtRImmediate, (XtPointer) -1},
    { "secondNPS", "secondNPS", XtRInt,
	sizeof(int), XtOffset(AppDataPtr, secondNPS),
	XtRImmediate, (XtPointer) -1},
    { "serverMoves", "serverMoves", XtRString,
	sizeof(String), XtOffset(AppDataPtr, serverMovesName),
	XtRImmediate, (XtPointer) 0},
    { "serverPause", "serverPause", XtRInt,
	sizeof(int), XtOffset(AppDataPtr, serverPause),
	XtRImmediate, (XtPointer) 0},
    { "suppressLoadMoves", "suppressLoadMoves", XtRBoolean,
	sizeof(Boolean), XtOffset(AppDataPtr, suppressLoadMoves),
	XtRImmediate, (XtPointer) False},
    { "userName", "userName", XtRString,
	sizeof(String), XtOffset(AppDataPtr, userName),
	XtRImmediate, (XtPointer) 0},
    { "egtFormats", "egtFormats", XtRString,
	sizeof(String), XtOffset(AppDataPtr, egtFormats),
	XtRImmediate, (XtPointer) 0},
    { "rewindIndex", "rewindIndex", XtRInt,
	sizeof(int), XtOffset(AppDataPtr, rewindIndex),
	XtRImmediate, (XtPointer) 0},
    { "sameColorGames", "sameColorGames", XtRInt,
	sizeof(int), XtOffset(AppDataPtr, sameColorGames),
	XtRImmediate, (XtPointer) 0},
    { "smpCores", "smpCores", XtRInt,
	sizeof(int), XtOffset(AppDataPtr, smpCores),
	XtRImmediate, (XtPointer) 1},
    { "niceEngines", "niceEngines", XtRInt,
	sizeof(int), XtOffset(AppDataPtr, niceEngines),
	XtRImmediate, (XtPointer) 0},
    { "nameOfDebugFile", "nameOfDebugFile", XtRString,
	sizeof(String), XtOffset(AppDataPtr, nameOfDebugFile),
	XtRImmediate, (XtPointer) "xboard.debug"},
    { "engineDebugOutput", "engineDebugOutput", XtRInt,
	sizeof(int), XtOffset(AppDataPtr, engineComments),
	XtRImmediate, (XtPointer) 1},
    { "noGUI", "noGUI", XtRBoolean,
	sizeof(Boolean), XtOffset(AppDataPtr, noGUI),
	XtRImmediate, (XtPointer) 0},
    { "firstOptions", "firstOptions", XtRString,
        sizeof(String), XtOffset(AppDataPtr, firstOptions),
	XtRImmediate, (XtPointer) "" },
    { "secondOptions", "secondOptions", XtRString,
        sizeof(String), XtOffset(AppDataPtr, secondOptions),
	XtRImmediate, (XtPointer) "" },
    { "firstNeedsNoncompliantFEN", "firstNeedsNoncompliantFEN", XtRString,
        sizeof(String), XtOffset(AppDataPtr, fenOverride1),
	XtRImmediate, (XtPointer) 0 },
    { "secondNeedsNoncompliantFEN", "secondNeedsNoncompliantFEN", XtRString,
        sizeof(String), XtOffset(AppDataPtr, fenOverride2),
	XtRImmediate, (XtPointer) 0 },

    // [HGM] Winboard_x UCI options
    { "firstIsUCI", "firstIsUCI", XtRBoolean,
	sizeof(Boolean), XtOffset(AppDataPtr, firstIsUCI),
	XtRImmediate, (XtPointer) False},
    { "secondIsUCI", "secondIsUCI", XtRBoolean,
	sizeof(Boolean), XtOffset(AppDataPtr, secondIsUCI),
	XtRImmediate, (XtPointer) False},
    { "firstHasOwnBookUCI", "firstHasOwnBookUCI", XtRBoolean,
	sizeof(Boolean), XtOffset(AppDataPtr, firstHasOwnBookUCI),
	XtRImmediate, (XtPointer) True},
    { "secondHasOwnBookUCI", "secondHasOwnBookUCI", XtRBoolean,
	sizeof(Boolean), XtOffset(AppDataPtr, secondHasOwnBookUCI),
	XtRImmediate, (XtPointer) True},
    { "usePolyglotBook", "usePolyglotBook", XtRBoolean,
	sizeof(Boolean), XtOffset(AppDataPtr, usePolyglotBook),
	XtRImmediate, (XtPointer) False},
    { "defaultHashSize", "defaultHashSize", XtRInt,
	sizeof(int), XtOffset(AppDataPtr, defaultHashSize),
	XtRImmediate, (XtPointer) 64},
    { "defaultCacheSizeEGTB", "defaultCacheSizeEGTB", XtRInt,
	sizeof(int), XtOffset(AppDataPtr, defaultCacheSizeEGTB),
	XtRImmediate, (XtPointer) 4},
    { "polyglotDir", "polyglotDir", XtRString,
        sizeof(String), XtOffset(AppDataPtr, polyglotDir),
	XtRImmediate, (XtPointer) "." },
    { "polyglotBook", "polyglotBook", XtRString,
        sizeof(String), XtOffset(AppDataPtr, polyglotBook),
	XtRImmediate, (XtPointer) "" },
    { "defaultPathEGTB", "defaultPathEGTB", XtRString,
	sizeof(String), XtOffset(AppDataPtr, defaultPathEGTB),
	XtRImmediate, (XtPointer) "/usr/local/share/egtb"},
    { "delayBeforeQuit", "delayBeforeQuit", XtRInt,
	sizeof(int), XtOffset(AppDataPtr, delayBeforeQuit),
	XtRImmediate, (XtPointer) 0},
    { "delayAfterQuit", "delayAfterQuit", XtRInt,
	sizeof(int), XtOffset(AppDataPtr, delayAfterQuit),
	XtRImmediate, (XtPointer) 0},
    { "keepAlive", "keepAlive", XtRInt,
	sizeof(int), XtOffset(AppDataPtr, keepAlive),
	XtRImmediate, (XtPointer) 0},
    { "forceIllegalMoves", "forceIllegalMoves", XtRBoolean,
	sizeof(Boolean), XtOffset(AppDataPtr, forceIllegal),
	XtRImmediate, (XtPointer) False},
};

XrmOptionDescRec shellOptions[] = {
    { "-whitePieceColor", "whitePieceColor", XrmoptionSepArg, NULL },
    { "-blackPieceColor", "blackPieceColor", XrmoptionSepArg, NULL },
    { "-lightSquareColor", "lightSquareColor", XrmoptionSepArg, NULL },
    { "-darkSquareColor", "darkSquareColor", XrmoptionSepArg, NULL },
    { "-highlightSquareColor", "highlightSquareColor", XrmoptionSepArg, NULL },
    { "-premoveHighlightColor", "premoveHighlightColor", XrmoptionSepArg,NULL},
    { "-movesPerSession", "movesPerSession", XrmoptionSepArg, NULL },
    { "-mps", "movesPerSession", XrmoptionSepArg, NULL },
    { "-timeIncrement", "timeIncrement", XrmoptionSepArg, NULL },
    { "-inc", "timeIncrement", XrmoptionSepArg, NULL },
    { "-initString", "initString", XrmoptionSepArg, NULL },
    { "-firstInitString", "initString", XrmoptionSepArg, NULL },
    { "-secondInitString", "secondInitString", XrmoptionSepArg, NULL },
    { "-firstComputerString", "firstComputerString", XrmoptionSepArg, NULL },
    { "-secondComputerString", "secondComputerString", XrmoptionSepArg, NULL },
    { "-firstChessProgram", "firstChessProgram", XrmoptionSepArg, NULL },
    { "-fcp", "firstChessProgram", XrmoptionSepArg, NULL },
    { "-secondChessProgram", "secondChessProgram", XrmoptionSepArg, NULL },
    { "-scp", "secondChessProgram", XrmoptionSepArg, NULL },
    { "-firstPlaysBlack", "firstPlaysBlack", XrmoptionSepArg, NULL },
    { "-fb", "firstPlaysBlack", XrmoptionNoArg, "True" },
    { "-xfb", "firstPlaysBlack", XrmoptionNoArg, "False" },
    { "-noChessProgram", "noChessProgram", XrmoptionSepArg, NULL },
    { "-ncp", "noChessProgram", XrmoptionNoArg, "True" },
    { "-xncp", "noChessProgram", XrmoptionNoArg, "False" },
    { "-firstHost", "firstHost", XrmoptionSepArg, NULL },
    { "-fh", "firstHost", XrmoptionSepArg, NULL },
    { "-secondHost", "secondHost", XrmoptionSepArg, NULL },
    { "-sh", "secondHost", XrmoptionSepArg, NULL },
    { "-firstDirectory", "firstDirectory", XrmoptionSepArg, NULL },
    { "-fd", "firstDirectory", XrmoptionSepArg, NULL },
    { "-secondDirectory", "secondDirectory", XrmoptionSepArg, NULL },
    { "-sd", "secondDirectory", XrmoptionSepArg, NULL },
    { "-bitmapDirectory", "bitmapDirectory", XrmoptionSepArg, NULL },
    { "-bm", "bitmapDirectory", XrmoptionSepArg, NULL },
    { "-remoteShell", "remoteShell", XrmoptionSepArg, NULL },
    { "-rsh", "remoteShell", XrmoptionSepArg, NULL },
    { "-remoteUser", "remoteUser", XrmoptionSepArg, NULL },
    { "-ruser", "remoteUser", XrmoptionSepArg, NULL },
    { "-timeDelay", "timeDelay", XrmoptionSepArg, NULL },
    { "-td", "timeDelay", XrmoptionSepArg, NULL },
    { "-timeControl", "timeControl", XrmoptionSepArg, NULL },
    { "-tc", "timeControl", XrmoptionSepArg, NULL },
    { "-internetChessServerMode", "internetChessServerMode",
	XrmoptionSepArg, NULL },
    { "-ics", "internetChessServerMode", XrmoptionNoArg, "True" },
    { "-xics", "internetChessServerMode", XrmoptionNoArg, "False" },
    { "-internetChessServerHost", "internetChessServerHost",
	XrmoptionSepArg, NULL },
    { "-icshost", "internetChessServerHost", XrmoptionSepArg, NULL },
    { "-internetChessServerPort", "internetChessServerPort",
	XrmoptionSepArg, NULL },
    { "-icsport", "internetChessServerPort", XrmoptionSepArg, NULL },
    { "-internetChessServerCommPort", "internetChessServerCommPort",
	XrmoptionSepArg, NULL },
    { "-icscomm", "internetChessServerCommPort", XrmoptionSepArg, NULL },
    { "-internetChessServerLogonScript", "internetChessServerLogonScript",
	XrmoptionSepArg, NULL },
    { "-icslogon", "internetChessServerLogonScript", XrmoptionSepArg, NULL },
    { "-internetChessServerHelper", "internetChessServerHelper",
	XrmoptionSepArg, NULL },
    { "-icshelper", "internetChessServerHelper", XrmoptionSepArg, NULL },
    { "-internetChessServerInputBox", "internetChessServerInputBox",
	XrmoptionSepArg, NULL },
    { "-icsinput", "internetChessServerInputBox", XrmoptionNoArg, "True" },
    { "-xicsinput", "internetChessServerInputBox", XrmoptionNoArg, "False" },
    { "-icsAlarm", "icsAlarm", XrmoptionSepArg, NULL },
    { "-alarm", "icsAlarm", XrmoptionNoArg, "True" },
    { "-xalarm", "icsAlarm", XrmoptionNoArg, "False" },
    { "-icsAlarmTime", "icsAlarmTime", XrmoptionSepArg, NULL },
    { "-useTelnet", "useTelnet", XrmoptionSepArg, NULL },
    { "-telnet", "useTelnet", XrmoptionNoArg, "True" },
    { "-xtelnet", "useTelnet", XrmoptionNoArg, "False" },
    { "-telnetProgram", "telnetProgram", XrmoptionSepArg, NULL },
    { "-gateway", "gateway", XrmoptionSepArg, NULL },
    { "-loadGameFile", "loadGameFile", XrmoptionSepArg, NULL },
    { "-lgf", "loadGameFile", XrmoptionSepArg, NULL },
    { "-loadGameIndex", "loadGameIndex", XrmoptionSepArg, NULL },
    { "-lgi", "loadGameIndex", XrmoptionSepArg, NULL },
    { "-saveGameFile", "saveGameFile", XrmoptionSepArg, NULL },
    { "-sgf", "saveGameFile", XrmoptionSepArg, NULL },
    { "-autoSaveGames", "autoSaveGames", XrmoptionSepArg, NULL },
    { "-autosave", "autoSaveGames", XrmoptionNoArg, "True" },
    { "-xautosave", "autoSaveGames", XrmoptionNoArg, "False" },
    { "-autoRaiseBoard", "autoRaiseBoard", XrmoptionSepArg, NULL },
    { "-autoraise", "autoRaiseBoard", XrmoptionNoArg, "True" },
    { "-xautoraise", "autoRaiseBoard", XrmoptionNoArg, "False" },
    { "-blindfold", "blindfold", XrmoptionSepArg, NULL },
    { "-blind", "blindfold", XrmoptionNoArg, "True" },
    { "-xblind", "blindfold", XrmoptionNoArg, "False" },
    { "-loadPositionFile", "loadPositionFile", XrmoptionSepArg, NULL },
    { "-lpf", "loadPositionFile", XrmoptionSepArg, NULL },
    { "-loadPositionIndex", "loadPositionIndex", XrmoptionSepArg, NULL },
    { "-lpi", "loadPositionIndex", XrmoptionSepArg, NULL },
    { "-savePositionFile", "savePositionFile", XrmoptionSepArg, NULL },
    { "-spf", "savePositionFile", XrmoptionSepArg, NULL },
    { "-matchMode", "matchMode", XrmoptionSepArg, NULL },
    { "-mm", "matchMode", XrmoptionNoArg, "True" },
    { "-xmm", "matchMode", XrmoptionNoArg, "False" },
    { "-matchGames", "matchGames", XrmoptionSepArg, NULL },
    { "-mg", "matchGames", XrmoptionSepArg, NULL },
    { "-monoMode", "monoMode", XrmoptionSepArg, NULL },
    { "-mono", "monoMode", XrmoptionNoArg, "True" },
    { "-xmono", "monoMode", XrmoptionNoArg, "False" },
    { "-debugMode", "debugMode", XrmoptionSepArg, NULL },
    { "-debug", "debugMode", XrmoptionNoArg, "True" },
    { "-xdebug", "debugMode", XrmoptionNoArg, "False" },
    { "-clockMode", "clockMode", XrmoptionSepArg, NULL },
    { "-clock", "clockMode", XrmoptionNoArg, "True" },
    { "-xclock", "clockMode", XrmoptionNoArg, "False" },
    { "-boardSize", "boardSize", XrmoptionSepArg, NULL },
    { "-size", "boardSize", XrmoptionSepArg, NULL },
    { "-searchTime", "searchTime", XrmoptionSepArg, NULL },
    { "-st", "searchTime", XrmoptionSepArg, NULL },
    { "-searchDepth", "searchDepth", XrmoptionSepArg, NULL },
    { "-depth", "searchDepth", XrmoptionSepArg, NULL },
    { "-showCoords", "showCoords", XrmoptionSepArg, NULL },
    { "-coords", "showCoords", XrmoptionNoArg, "True" },
    { "-xcoords", "showCoords", XrmoptionNoArg, "False" },
#if JAIL
    { "-showJail", "showJail", XrmoptionSepArg, NULL },
    { "-jail", "showJail", XrmoptionNoArg, "1" },
    { "-sidejail", "showJail", XrmoptionNoArg, "2" },
    { "-xjail", "showJail", XrmoptionNoArg, "0" },
#endif
    { "-showThinking", "showThinking", XrmoptionSepArg, NULL },
    { "-thinking", "showThinking", XrmoptionNoArg, "True" },
    { "-xthinking", "showThinking", XrmoptionNoArg, "False" },
    { "-ponderNextMove", "ponderNextMove", XrmoptionSepArg, NULL },
    { "-ponder", "ponderNextMove", XrmoptionNoArg, "True" },
    { "-xponder", "ponderNextMove", XrmoptionNoArg, "False" },
    { "-periodicUpdates", "periodicUpdates", XrmoptionSepArg, NULL },
    { "-periodic", "periodicUpdates", XrmoptionNoArg, "True" },
    { "-xperiodic", "periodicUpdates", XrmoptionNoArg, "False" },
    { "-clockFont", "clockFont", XrmoptionSepArg, NULL },
    { "-coordFont", "coordFont", XrmoptionSepArg, NULL },
    { "-font", "font", XrmoptionSepArg, NULL },
    { "-ringBellAfterMoves", "ringBellAfterMoves", XrmoptionSepArg, NULL },
    { "-bell", "ringBellAfterMoves", XrmoptionNoArg, "True" },
    { "-xbell", "ringBellAfterMoves", XrmoptionNoArg, "False" },
    { "-movesound", "ringBellAfterMoves", XrmoptionNoArg, "True" },
    { "-xmovesound", "ringBellAfterMoves", XrmoptionNoArg, "False" },
    { "-autoCallFlag", "autoCallFlag", XrmoptionSepArg, NULL },
    { "-autoflag", "autoCallFlag", XrmoptionNoArg, "True" },
    { "-xautoflag", "autoCallFlag", XrmoptionNoArg, "False" },
    { "-autoFlipView", "autoFlipView", XrmoptionSepArg, NULL },
    { "-autoflip", "autoFlipView", XrmoptionNoArg, "True" },
    { "-xautoflip", "autoFlipView", XrmoptionNoArg, "False" },
    { "-autoObserve", "autoObserve", XrmoptionSepArg, NULL },
    { "-autobs", "autoObserve", XrmoptionNoArg, "True" },
    { "-xautobs", "autoObserve", XrmoptionNoArg, "False" },
    { "-autoComment", "autoComment", XrmoptionSepArg, NULL },
    { "-autocomm", "autoComment", XrmoptionNoArg, "True" },
    { "-xautocomm", "autoComment", XrmoptionNoArg, "False" },
    { "-getMoveList", "getMoveList", XrmoptionSepArg, NULL },
    { "-moves", "getMoveList", XrmoptionNoArg, "True" },
    { "-xmoves", "getMoveList", XrmoptionNoArg, "False" },
#if HIGHDRAG
    { "-highlightDragging", "highlightDragging", XrmoptionSepArg, NULL },
    { "-highdrag", "highlightDragging", XrmoptionNoArg, "True" },
    { "-xhighdrag", "highlightDragging", XrmoptionNoArg, "False" },
#endif
    { "-highlightLastMove", "highlightLastMove", XrmoptionSepArg, NULL },
    { "-highlight", "highlightLastMove", XrmoptionNoArg, "True" },
    { "-xhighlight", "highlightLastMove", XrmoptionNoArg, "False" },
    { "-premove", "premove", XrmoptionSepArg, NULL },
    { "-pre", "premove", XrmoptionNoArg, "True" },
    { "-xpre", "premove", XrmoptionNoArg, "False" },
    { "-testLegality", "testLegality", XrmoptionSepArg, NULL },
    { "-legal", "testLegality", XrmoptionNoArg, "True" },
    { "-xlegal", "testLegality", XrmoptionNoArg, "False" },
    { "-flipView", "flipView", XrmoptionSepArg, NULL },
    { "-flip", "flipView", XrmoptionNoArg, "True" },
    { "-xflip", "flipView", XrmoptionNoArg, "False" },
    { "-cmail", "cmailGameName", XrmoptionSepArg, NULL },
    { "-alwaysPromoteToQueen", "alwaysPromoteToQueen",
	XrmoptionSepArg, NULL },
    { "-queen", "alwaysPromoteToQueen", XrmoptionNoArg, "True" },
    { "-xqueen", "alwaysPromoteToQueen", XrmoptionNoArg, "False" },
    { "-oldSaveStyle", "oldSaveStyle", XrmoptionSepArg, NULL },
    { "-oldsave", "oldSaveStyle", XrmoptionNoArg, "True" },
    { "-xoldsave", "oldSaveStyle", XrmoptionNoArg, "False" },
    { "-quietPlay", "quietPlay", XrmoptionSepArg, NULL },
    { "-quiet", "quietPlay", XrmoptionNoArg, "True" },
    { "-xquiet", "quietPlay", XrmoptionNoArg, "False" },
    { "-titleInWindow", "titleInWindow", XrmoptionSepArg, NULL },
    { "-title", "titleInWindow", XrmoptionNoArg, "True" },
    { "-xtitle", "titleInWindow", XrmoptionNoArg, "False" },
#ifdef ZIPPY
    { "-zippyTalk", "zippyTalk", XrmoptionSepArg, NULL },
    { "-zt", "zippyTalk", XrmoptionNoArg, "True" },
    { "-xzt", "zippyTalk", XrmoptionNoArg, "False" },
    { "-zippyPlay", "zippyPlay", XrmoptionSepArg, NULL },
    { "-zp", "zippyPlay", XrmoptionNoArg, "True" },
    { "-xzp", "zippyPlay", XrmoptionNoArg, "False" },
    { "-zippyLines", "zippyLines", XrmoptionSepArg, NULL },
    { "-zippyPinhead", "zippyPinhead", XrmoptionSepArg, NULL },
    { "-zippyPassword", "zippyPassword", XrmoptionSepArg, NULL },
    { "-zippyPassword2", "zippyPassword2", XrmoptionSepArg, NULL },
    { "-zippyWrongPassword", "zippyWrongPassword", XrmoptionSepArg, NULL },
    { "-zippyAcceptOnly", "zippyAcceptOnly", XrmoptionSepArg, NULL },
    { "-zippyUseI", "zippyUseI", XrmoptionSepArg, NULL },
    { "-zui", "zippyUseI", XrmoptionNoArg, "True" },
    { "-xzui", "zippyUseI", XrmoptionNoArg, "False" },
    { "-zippyBughouse", "zippyBughouse", XrmoptionSepArg, NULL },
    { "-zippyNoplayCrafty", "zippyNoplayCrafty", XrmoptionSepArg, NULL },
    { "-znc", "zippyNoplayCrafty", XrmoptionNoArg, "True" },
    { "-xznc", "zippyNoplayCrafty", XrmoptionNoArg, "False" },
    { "-zippyGameEnd", "zippyGameEnd", XrmoptionSepArg, NULL },
    { "-zippyGameStart", "zippyGameStart", XrmoptionSepArg, NULL },
    { "-zippyAdjourn", "zippyAdjourn", XrmoptionSepArg, NULL },
    { "-zadj", "zippyAdjourn", XrmoptionNoArg, "True" },
    { "-xzadj", "zippyAdjourn", XrmoptionNoArg, "False" },
    { "-zippyAbort", "zippyAbort", XrmoptionSepArg, NULL },
    { "-zab", "zippyAbort", XrmoptionNoArg, "True" },
    { "-xzab", "zippyAbort", XrmoptionNoArg, "False" },
    { "-zippyVariants", "zippyVariants", XrmoptionSepArg, NULL },
    { "-zippyMaxGames", "zippyMaxGames", XrmoptionSepArg, NULL },
    { "-zippyReplayTimeout", "zippyReplayTimeout", XrmoptionSepArg, NULL },
    { "-zippyShortGame", "zippyShortGame", XrmoptionSepArg, NULL },
#endif
    { "-flashCount", "flashCount", XrmoptionSepArg, NULL },
    { "-flash", "flashCount", XrmoptionNoArg, "3" },
    { "-xflash", "flashCount", XrmoptionNoArg, "0" },
    { "-flashRate", "flashRate", XrmoptionSepArg, NULL },
    { "-pixmapDirectory", "pixmapDirectory", XrmoptionSepArg, NULL },
    { "-msLoginDelay", "msLoginDelay", XrmoptionSepArg, NULL },
    { "-pixmap", "pixmapDirectory", XrmoptionSepArg, NULL },
    { "-colorizeMessages", "colorizeMessages", XrmoptionSepArg, NULL },
    { "-colorize", "colorizeMessages", XrmoptionNoArg, "True" },
    { "-xcolorize", "colorizeMessages", XrmoptionNoArg, "False" },
    { "-colorShout", "colorShout", XrmoptionSepArg, NULL },
    { "-colorSShout", "colorSShout", XrmoptionSepArg, NULL },
    { "-colorCShout", "colorSShout", XrmoptionSepArg, NULL }, /*FICS name*/
    { "-colorChannel1", "colorChannel1", XrmoptionSepArg, NULL },
    { "-colorChannel", "colorChannel", XrmoptionSepArg, NULL },
    { "-colorKibitz", "colorKibitz", XrmoptionSepArg, NULL },
    { "-colorTell", "colorTell", XrmoptionSepArg, NULL },
    { "-colorChallenge", "colorChallenge", XrmoptionSepArg, NULL },
    { "-colorRequest", "colorRequest", XrmoptionSepArg, NULL },
    { "-colorSeek", "colorSeek", XrmoptionSepArg, NULL },
    { "-colorNormal", "colorNormal", XrmoptionSepArg, NULL },
    { "-soundProgram", "soundProgram", XrmoptionSepArg, NULL },
    { "-soundShout", "soundShout", XrmoptionSepArg, NULL },
    { "-soundSShout", "soundSShout", XrmoptionSepArg, NULL },
    { "-soundCShout", "soundSShout", XrmoptionSepArg, NULL }, /*FICS name*/
    { "-soundChannel1", "soundChannel1", XrmoptionSepArg, NULL },
    { "-soundChannel", "soundChannel", XrmoptionSepArg, NULL },
    { "-soundKibitz", "soundKibitz", XrmoptionSepArg, NULL },
    { "-soundTell", "soundTell", XrmoptionSepArg, NULL },
    { "-soundChallenge", "soundChallenge", XrmoptionSepArg, NULL },
    { "-soundRequest", "soundRequest", XrmoptionSepArg, NULL },
    { "-soundSeek", "soundSeek", XrmoptionSepArg, NULL },
    { "-soundMove", "soundMove", XrmoptionSepArg, NULL },
    { "-soundIcsWin", "soundIcsWin", XrmoptionSepArg, NULL },
    { "-soundIcsLoss", "soundIcsLoss", XrmoptionSepArg, NULL },
    { "-soundIcsDraw", "soundIcsDraw", XrmoptionSepArg, NULL },
    { "-soundIcsUnfinished", "soundIcsUnfinished", XrmoptionSepArg, NULL },
    { "-soundIcsAlarm", "soundIcsAlarm", XrmoptionSepArg, NULL },
    { "-reuseFirst", "reuseFirst", XrmoptionSepArg, NULL },
    { "-reuseChessPrograms", "reuseFirst", XrmoptionSepArg, NULL }, /*compat*/
    { "-reuse", "reuseFirst", XrmoptionNoArg, "True" },
    { "-xreuse", "reuseFirst", XrmoptionNoArg, "False" },
    { "-reuseSecond", "reuseSecond", XrmoptionSepArg, NULL },
    { "-reuse2", "reuseSecond", XrmoptionNoArg, "True" },
    { "-xreuse2", "reuseSecond", XrmoptionNoArg, "False" },
    { "-animateMoving", "animateMoving", XrmoptionSepArg, NULL },
    { "-animate", "animateMoving", XrmoptionNoArg, "True" },
    { "-xanimate", "animateMoving", XrmoptionNoArg, "False" },
    { "-animateDragging", "animateDragging", XrmoptionSepArg, NULL },
    { "-drag", "animateDragging", XrmoptionNoArg, "True" },
    { "-xdrag", "animateDragging", XrmoptionNoArg, "False" },
    { "-animateSpeed", "animateSpeed", XrmoptionSepArg, NULL },
    { "-popupExitMessage", "popupExitMessage", XrmoptionSepArg, NULL },
    { "-exit", "popupExitMessage", XrmoptionNoArg, "True" },
    { "-xexit", "popupExitMessage", XrmoptionNoArg, "False" },
    { "-popupMoveErrors", "popupMoveErrors", XrmoptionSepArg, NULL },
    { "-popup", "popupMoveErrors", XrmoptionNoArg, "True" },
    { "-xpopup", "popupMoveErrors", XrmoptionNoArg, "False" },
    { "-fontSizeTolerance", "fontSizeTolerance", XrmoptionSepArg, NULL },
    { "-initialMode", "initialMode", XrmoptionSepArg, NULL },
    { "-mode", "initialMode", XrmoptionSepArg, NULL },
    { "-variant", "variant", XrmoptionSepArg, NULL },
    { "-firstProtocolVersion", "firstProtocolVersion", XrmoptionSepArg, NULL },
    { "-secondProtocolVersion","secondProtocolVersion",XrmoptionSepArg, NULL },
    { "-showButtonBar", "showButtonBar", XrmoptionSepArg, NULL },
    { "-buttons", "showButtonBar", XrmoptionNoArg, "True" },
    { "-xbuttons", "showButtonBar", XrmoptionNoArg, "False" },
    { "-lowTimeWarningColor", "lowTimeWarningColor", XrmoptionSepArg, NULL },
    { "-lowTimeWarning", "lowTimeWarning", XrmoptionSepArg, NULL },
    /* [AS,HR] New features */
    { "-firstScoreAbs", "firstScoreAbs", XrmoptionSepArg, NULL },
    { "-secondScoreAbs", "secondScoreAbs", XrmoptionSepArg, NULL },
    { "-pgnExtendedInfo", "pgnExtendedInfo", XrmoptionSepArg, NULL },
    { "-hideThinkingFromHuman", "hideThinkingFromHuman", XrmoptionSepArg, NULL },
    { "-adjudicateLossThreshold", "adjudicateLossThreshold", XrmoptionSepArg, NULL },
    { "-adjudicateDrawMoves", "adjudicateDrawMoves", XrmoptionSepArg, NULL },
    { "-pgnEventHeader", "pgnEventHeader", XrmoptionSepArg, NULL },
    { "-firstIsUCI", "firstIsUCI", XrmoptionSepArg, NULL },
    { "-secondIsUCI", "secondIsUCI", XrmoptionSepArg, NULL },
    { "-fUCI", "firstIsUCI", XrmoptionNoArg, "True" },
    { "-sUCI", "secondIsUCI", XrmoptionNoArg, "True" },
    { "-firstHasOwnBookUCI", "firstHasOwnBookUCI", XrmoptionSepArg, NULL },
    { "-secondHasOwnBookUCI", "secondHasOwnBookUCI", XrmoptionSepArg, NULL },
    { "-fNoOwnBookUCI", "firstHasOwnBookUCI", XrmoptionNoArg, "False" },
    { "-sNoOwnBookUCI", "secondHasOwnBookUCI", XrmoptionNoArg, "False" },
    { "-firstXBook", "firstHasOwnBookUCI", XrmoptionNoArg, "False" },
    { "-secondXBook", "secondHasOwnBookUCI", XrmoptionNoArg, "False" },
    { "-polyglotDir", "polyglotDir", XrmoptionSepArg, NULL },
    { "-usePolyglotBook", "usePolyglotBook", XrmoptionSepArg, NULL },
    { "-polyglotBook", "polyglotBook", XrmoptionSepArg, NULL },
    { "-defaultHashSize", "defaultHashSize", XrmoptionSepArg, NULL },
    { "-defaultCacheSizeEGTB", "defaultCacheSizeEGTB", XrmoptionSepArg, NULL },
    { "-defaultPathEGTB", "defaultPathEGTB", XrmoptionSepArg, NULL },
    { "-defaultFrcPosition", "defaultFrcPosition", XrmoptionSepArg, NULL },
    { "-gameListTags", "gameListTags", XrmoptionSepArg, NULL },
    // [HGM] I am sure AS added many more options, but we have to fish them out, from the list in winboard.c

    /* [HGM,HR] User-selectable board size */
    { "-boardWidth", "boardWidth", XrmoptionSepArg, NULL },
    { "-boardHeight", "boardHeight", XrmoptionSepArg, NULL },
    { "-matchPause", "matchPause", XrmoptionSepArg, NULL },

    /* [HGM] new arguments of 4.3.xx. All except first three are back-end options, which should work immediately */
    { "-holdingsSize", "holdingsSize", XrmoptionSepArg, NULL }, // requires extensive front-end changes to work
    { "-flipBlack", "flipBlack", XrmoptionSepArg, NULL },       // requires front-end changes to work
    { "-allWhite", "allWhite", XrmoptionSepArg, NULL },         // requires front-end changes to work
    { "-pieceToCharTable", "pieceToCharTable", XrmoptionSepArg, NULL },
    { "-alphaRank", "alphaRank", XrmoptionSepArg, NULL },
    { "-testClaims", "testClaims", XrmoptionSepArg, NULL },
    { "-checkMates", "checkMates", XrmoptionSepArg, NULL },
    { "-materialDraws", "materialDraws", XrmoptionSepArg, NULL },
    { "-trivialDraws", "trivialDraws", XrmoptionSepArg, NULL },
    { "-ruleMoves", "ruleMoves", XrmoptionSepArg, NULL },
    { "-repeatsToDraw", "repeatsToDraw", XrmoptionSepArg, NULL },
    { "-engineDebugOutput", "engineDebugOutput", XrmoptionSepArg, NULL },
    { "-userName", "userName", XrmoptionSepArg, NULL },
    { "-autoKibitz", "autoKibitz", XrmoptionNoArg, "True" },
    { "-firstTimeOdds", "firstTimeOdds", XrmoptionSepArg, NULL },
    { "-secondTimeOdds", "secondTimeOdds", XrmoptionSepArg, NULL },
    { "-timeOddsMode", "timeOddsMode", XrmoptionSepArg, NULL },
    { "-firstAccumulateTC", "firstAccumulateTC", XrmoptionSepArg, NULL },
    { "-secondAccumulateTC", "secondAccumulateTC", XrmoptionSepArg, NULL },
    { "-firstNPS", "firstNPS", XrmoptionSepArg, NULL },
    { "-secondNPS", "secondNPS", XrmoptionSepArg, NULL },
    { "-serverMoves", "serverMoves", XrmoptionSepArg, NULL },
    { "-serverPause", "serverPause", XrmoptionSepArg, NULL },
    { "-suppressLoadMoves", "suppressLoadMoves", XrmoptionSepArg, NULL },
    { "-egtFormats", "egtFormats", XrmoptionSepArg, NULL },
    { "-userName", "userName", XrmoptionSepArg, NULL },
    { "-smpCores", "smpCores", XrmoptionSepArg, NULL },
    { "-sameColorGames", "sameColorGames", XrmoptionSepArg, NULL },
    { "-rewindIndex", "rewindIndex", XrmoptionSepArg, NULL },
    { "-niceEngines", "niceEngines", XrmoptionSepArg, NULL },
    { "-delayBeforeQuit", "delayBeforeQuit", XrmoptionSepArg, NULL },
    { "-delayAfterQuit", "delayAfterQuit", XrmoptionSepArg, NULL },
    { "-nameOfDebugFile", "nameOfDebugFile", XrmoptionSepArg, NULL },
    { "-debugFile", "nameOfDebugFile", XrmoptionSepArg, NULL },
    { "-engineDebugOutput", "engineDebugOutput", XrmoptionSepArg, NULL },
    { "-noGUI", "noGUI", XrmoptionNoArg, "True" },
    { "-firstOptions", "firstOptions", XrmoptionSepArg, NULL },
    { "-secondOptions", "secondOptions", XrmoptionSepArg, NULL },
    { "-firstNeedsNoncompliantFEN", "firstNeedsNoncompliantFEN", XrmoptionSepArg, NULL },
    { "-secondNeedsNoncompliantFEN", "secondNeedsNoncompliantFEN", XrmoptionSepArg, NULL },
    { "-keepAlive", "keepAlive", XrmoptionSepArg, NULL },
    { "-forceIllegalMoves", "forceIllegalMoves", XrmoptionNoArg, "True" },
};


XtActionsRec boardActions[] = {
    { "HandleUserMove", HandleUserMove },
    { "AnimateUserMove", AnimateUserMove },
    { "FileNameAction", FileNameAction },
    { "AskQuestionProc", AskQuestionProc },
    { "AskQuestionReplyAction", AskQuestionReplyAction },
    { "PieceMenuPopup", PieceMenuPopup },
    { "WhiteClock", WhiteClock },
    { "BlackClock", BlackClock },
    { "Iconify", Iconify },
    { "LoadSelectedProc", LoadSelectedProc },
    { "LoadPositionProc", LoadPositionProc },
    { "LoadNextPositionProc", LoadNextPositionProc },
    { "LoadPrevPositionProc", LoadPrevPositionProc },
    { "ReloadPositionProc", ReloadPositionProc },
    { "CopyPositionProc", CopyPositionProc },
    { "PastePositionProc", PastePositionProc },
    { "CopyGameProc", CopyGameProc },
    { "PasteGameProc", PasteGameProc },
    { "SaveGameProc", SaveGameProc },
    { "SavePositionProc", SavePositionProc },
    { "MailMoveProc", MailMoveProc },
    { "ReloadCmailMsgProc", ReloadCmailMsgProc },
    //    { "MachineWhiteProc", MachineWhiteProc },
    //    { "MachineBlackProc", MachineBlackProc },
    { "AnalysisModeProc", AnalyzeModeProc },
    { "AnalyzeFileProc", AnalyzeFileProc },
    //    { "TwoMachinesProc", TwoMachinesProc },
    //    { "IcsClientProc", IcsClientProc },
    { "EditGameProc", EditGameProc },
    { "EditPositionProc", EditPositionProc },
    { "TrainingProc", EditPositionProc },
    { "EngineOutputProc", EngineOutputProc}, // [HGM] Winboard_x engine-output window
    { "ShowGameListProc", ShowGameListProc },
    //    { "ShowMoveListProc", HistoryShowProc},
    { "EditTagsProc", EditCommentProc },
    { "EditCommentProc", EditCommentProc },
    { "IcsAlarmProc", IcsAlarmProc },
    { "IcsInputBoxProc", IcsInputBoxProc },
    //    { "AcceptProc", AcceptProc },
    //    { "DeclineProc", DeclineProc },
    //    { "RematchProc", RematchProc },
    //    { "CallFlagProc", CallFlagProc },
    //    { "DrawProc", DrawProc },
    //    { "AdjournProc", AdjournProc },
    //    { "AbortProc", AbortProc },
    //    { "ResignProc", ResignProc },
    //    { "AdjuWhiteProc", AdjuWhiteProc },
    //    { "AdjuBlackProc", AdjuBlackProc },
    //    { "AdjuDrawProc", AdjuDrawProc },
    { "EnterKeyProc", EnterKeyProc },
    //    { "StopObservingProc", StopObservingProc },
    //    { "StopExaminingProc", StopExaminingProc },
    //    { "BackwardProc", BackwardProc },
    //    { "ForwardProc", ForwardProc },
    //    { "ToStartProc", ToStartProc },
    //    { "ToEndProc", ToEndProc },
    //    { "RevertProc", RevertProc },
    //    { "TruncateGameProc", TruncateGameProc },
    //    { "MoveNowProc", MoveNowProc },
    //    { "RetractMoveProc", RetractMoveProc },
    { "AlwaysQueenProc", AlwaysQueenProc },
    { "AnimateDraggingProc", AnimateDraggingProc },
    { "AnimateMovingProc", AnimateMovingProc },
    { "AutoflagProc", AutoflagProc },
    { "AutoflipProc", AutoflipProc },
    { "AutobsProc", AutobsProc },
    { "AutoraiseProc", AutoraiseProc },
    { "AutosaveProc", AutosaveProc },
    { "BlindfoldProc", BlindfoldProc },
    { "FlashMovesProc", FlashMovesProc },
    //    { "FlipViewProc", FlipViewProc },
    //    { "GetMoveListProc", GetMoveListProc },
#if HIGHDRAG
    { "HighlightDraggingProc", HighlightDraggingProc },
#endif
    { "HighlightLastMoveProc", HighlightLastMoveProc },
    { "IcsAlarmProc", IcsAlarmProc },
    { "MoveSoundProc", MoveSoundProc },
    { "OldSaveStyleProc", OldSaveStyleProc },
    { "PeriodicUpdatesProc", PeriodicUpdatesProc },
    { "PonderNextMoveProc", PonderNextMoveProc },
    { "PopupExitMessageProc", PopupExitMessageProc },
    { "PopupMoveErrorsProc", PopupMoveErrorsProc },
    { "PremoveProc", PremoveProc },
    { "QuietPlayProc", QuietPlayProc },
    //    { "ShowThinkingProc", ShowThinkingProc },
    //    { "HideThinkingProc", HideThinkingProc },
    { "TestLegalityProc", TestLegalityProc },
    //    { "InfoProc", InfoProc },
    //    { "ManProc", ManProc },
    //    { "HintProc", HintProc },
    //    { "BookProc", BookProc },
    { "AboutGameProc", AboutGameProc },
    { "DebugProc", DebugProc },
    { "NothingProc", NothingProc },
    { "CommentPopDown", (XtActionProc) CommentPopDown },
    { "EditCommentPopDown", (XtActionProc) EditCommentPopDown },
    { "TagsPopDown", (XtActionProc) TagsPopDown },
    { "ErrorPopDown", (XtActionProc) ErrorPopDown },
    { "ICSInputBoxPopDown", (XtActionProc) ICSInputBoxPopDown },
    { "AnalysisPopDown", (XtActionProc) AnalysisPopDown },
    { "FileNamePopDown", (XtActionProc) FileNamePopDown },
    { "AskQuestionPopDown", (XtActionProc) AskQuestionPopDown },
    { "GameListPopDown", (XtActionProc) GameListPopDown },
    { "PromotionPopDown", (XtActionProc) PromotionPopDown },
    //    { "HistoryPopDown", (XtActionProc) HistoryPopDown },
    { "EngineOutputPopDown", (XtActionProc) EngineOutputPopDown },
    { "ShufflePopDown", (XtActionProc) ShufflePopDown },
    { "EnginePopDown", (XtActionProc) EnginePopDown },
    { "UciPopDown", (XtActionProc) UciPopDown },
    { "TimeControlPopDown", (XtActionProc) TimeControlPopDown },
    { "NewVariantPopDown", (XtActionProc) NewVariantPopDown },
    { "SettingsPopDown", (XtActionProc) SettingsPopDown },
};


char ICSInputTranslations[] =
    "<Key>Return: EnterKeyProc() \n";

String xboardResources[] = {
    "*fileName*value.translations: #override\\n <Key>Return: FileNameAction()",
    "*question*value.translations: #override\\n <Key>Return: AskQuestionReplyAction()",
    "*errorpopup*translations: #override\\n <Key>Return: ErrorPopDown()",
    NULL
  };

static char *cnames[9] = { "black", "red", "green", "yellow", "blue",
			     "magenta", "cyan", "white" };
typedef struct {
    int attr, bg, fg;
} TextColors;
TextColors textColors[(int)NColorClasses];

/* String is: "fg, bg, attr". Which is 0, 1, 2 */
static int
parse_color(str, which)
     char *str;
     int which;
{
    char *p, buf[100], *d;
    int i;

    if (strlen(str) > 99)	/* watch bounds on buf */
      return -1;

    p = str;
    d = buf;
    for (i=0; i<which; ++i) {
	p = strchr(p, ',');
	if (!p)
	  return -1;
	++p;
    }

    /* Could be looking at something like:
       black, , 1
       .. in which case we want to stop on a comma also */
    while (*p && *p != ',' && !isalpha(*p) && !isdigit(*p))
      ++p;

    if (*p == ',') {
	return -1;		/* Use default for empty field */
    }

    if (which == 2 || isdigit(*p))
      return atoi(p);

    while (*p && isalpha(*p))
      *(d++) = *(p++);

    *d = 0;

    for (i=0; i<8; ++i) {
	if (!StrCaseCmp(buf, cnames[i]))
	  return which? (i+40) : (i+30);
    }
    if (!StrCaseCmp(buf, "default")) return -1;

    fprintf(stderr, _("%s: unrecognized color %s\n"), programName, buf);
    return -2;
}

static int
parse_cpair(cc, str)
     ColorClass cc;
     char *str;
{
    if ((textColors[(int)cc].fg=parse_color(str, 0)) == -2) {
	fprintf(stderr, _("%s: can't parse foreground color in `%s'\n"),
		programName, str);
	return -1;
    }

    /* bg and attr are optional */
    textColors[(int)cc].bg = parse_color(str, 1);
    if ((textColors[(int)cc].attr = parse_color(str, 2)) < 0) {
	textColors[(int)cc].attr = 0;
    }
    return 0;
}


/* Arrange to catch delete-window events */
Atom wm_delete_window;
void
CatchDeleteWindow(Widget w, String procname)
{
  char buf[MSG_SIZ];
  XSetWMProtocols(xDisplay, XtWindow(w), &wm_delete_window, 1);
  snprintf(buf, sizeof(buf), "<Message>WM_PROTOCOLS: %s() \n", procname);
  XtAugmentTranslations(w, XtParseTranslationTable(buf));
}

void
BoardToTop()
{
  /* this should raise the board to the top */
  gtk_window_present(GTK_WINDOW(GUI_Window));
  return;
}

#define BoardSize int
void InitDrawingSizes(BoardSize boardSize, int flags)
{   // [HGM] resize is functional now, but for board format changes only (nr of ranks, files)
    Dimension timerWidth, boardWidth, boardHeight, w, h, sep, bor, wr, hr;
    Arg args[16];
    XtGeometryResult gres;
    int i;

    boardWidth  = lineGap + BOARD_WIDTH  * (squareSize + lineGap);
    boardHeight = lineGap + BOARD_HEIGHT * (squareSize + lineGap);

    timerWidth = (boardWidth - sep) / 2;

    if (appData.titleInWindow)
      {
	i = 0;
	if (smallLayout)
	  {
	    w = boardWidth - 2*bor;
	  }
	else
	  {
	    w = boardWidth - w - sep - 2*bor - 2; // WIDTH_FUDGE
	  }
      }

    if(!formWidget) return;

    /*
     * Inhibit shell resizing.
     */

    // [HGM] pieces: tailor piece bitmaps to needs of specific variant
    // (only for xpm)
    if(useImages) {
      for(i=0; i<4; i++) {
	int p;
	for(p=0; p<=(int)WhiteKing; p++)
	   xpmPieceBitmap[i][p] = xpmPieceBitmap2[i][p]; // defaults
	if(gameInfo.variant == VariantShogi) {
	   xpmPieceBitmap[i][(int)WhiteCannon] = xpmPieceBitmap2[i][(int)WhiteKing+1];
	   xpmPieceBitmap[i][(int)WhiteNightrider] = xpmPieceBitmap2[i][(int)WhiteKing+2];
	   xpmPieceBitmap[i][(int)WhiteSilver] = xpmPieceBitmap2[i][(int)WhiteKing+3];
	   xpmPieceBitmap[i][(int)WhiteGrasshopper] = xpmPieceBitmap2[i][(int)WhiteKing+4];
	   xpmPieceBitmap[i][(int)WhiteQueen] = xpmPieceBitmap2[i][(int)WhiteLance];
	}
#ifdef GOTHIC
	if(gameInfo.variant == VariantGothic) {
	   xpmPieceBitmap[i][(int)WhiteMarshall] = xpmPieceBitmap2[i][(int)WhiteSilver];
	}
#endif
#if !HAVE_LIBXPM
	// [HGM] why are thee ximMasks used at all? the ximPieceBitmaps seem to be never used!
	for(p=0; p<=(int)WhiteKing; p++)
	   ximMaskPm[p] = ximMaskPm2[p]; // defaults
	if(gameInfo.variant == VariantShogi) {
	   ximMaskPm[(int)WhiteCannon] = ximMaskPm2[(int)WhiteKing+1];
	   ximMaskPm[(int)WhiteNightrider] = ximMaskPm2[(int)WhiteKing+2];
	   ximMaskPm[(int)WhiteSilver] = ximMaskPm2[(int)WhiteKing+3];
	   ximMaskPm[(int)WhiteGrasshopper] = ximMaskPm2[(int)WhiteKing+4];
	   ximMaskPm[(int)WhiteQueen] = ximMaskPm2[(int)WhiteLance];
	}
#ifdef GOTHIC
	if(gameInfo.variant == VariantGothic) {
           ximMaskPm[(int)WhiteMarshall] = ximMaskPm2[(int)WhiteSilver];
	}
#endif
#endif
      }
    } else {
      for(i=0; i<2; i++) {
	int p;
	for(p=0; p<=(int)WhiteKing; p++)
	   pieceBitmap[i][p] = pieceBitmap2[i][p]; // defaults
	if(gameInfo.variant == VariantShogi) {
	   pieceBitmap[i][(int)WhiteCannon] = pieceBitmap2[i][(int)WhiteKing+1];
	   pieceBitmap[i][(int)WhiteNightrider] = pieceBitmap2[i][(int)WhiteKing+2];
	   pieceBitmap[i][(int)WhiteSilver] = pieceBitmap2[i][(int)WhiteKing+3];
	   pieceBitmap[i][(int)WhiteGrasshopper] = pieceBitmap2[i][(int)WhiteKing+4];
	   pieceBitmap[i][(int)WhiteQueen] = pieceBitmap2[i][(int)WhiteLance];
	}
#ifdef GOTHIC
	if(gameInfo.variant == VariantGothic) {
	   pieceBitmap[i][(int)WhiteMarshall] = pieceBitmap2[i][(int)WhiteSilver];
	}
#endif
      }
    }
#if HAVE_LIBXPM
    CreateAnimVars();
#endif
}

void EscapeExpand(char *p, char *q)
{	// [HGM] initstring: routine to shape up string arguments
	while(*p++ = *q++) if(p[-1] == '\\')
	    switch(*q++) {
		case 'n': p[-1] = '\n'; break;
		case 'r': p[-1] = '\r'; break;
		case 't': p[-1] = '\t'; break;
		case '\\': p[-1] = '\\'; break;
		case 0: *p = 0; return;
		default: p[-1] = q[-1]; break;
	    }
}

int
main(argc, argv)
     int argc;
     char **argv;
{
    int i, j, clockFontPxlSize, coordFontPxlSize, fontPxlSize;
    XSetWindowAttributes window_attributes;
    Arg args[16];
    Dimension timerWidth, boardWidth, boardHeight, w, h, sep, bor, wr, hr;
    XrmValue vFrom, vTo;
    XtGeometryResult gres;
    char *p;
    XrmDatabase xdb;
    int forceMono = False;

#define INDIRECTION
#ifdef INDIRECTION
    // [HGM] before anything else, expand any indirection files amongst options
    char *argvCopy[1000]; // 1000 seems enough
    char newArgs[10000];  // holds actual characters
    int k = 0;

    srandom(time(0)); // [HGM] book: make random truly random

    j = 0;
    for(i=0; i<argc; i++) {
	if(j >= 1000-2) { printf(_("too many arguments\n")); exit(-1); }
	//fprintf(stderr, "arg %s\n", argv[i]);
	if(argv[i][0] != '@') argvCopy[j++] = argv[i]; else {
	    char c;
	    FILE *f = fopen(argv[i]+1, "rb");
	    if(f == NULL) { fprintf(stderr, _("ignore %s\n"), argv[i]); continue; } // do not expand non-existing
	    argvCopy[j++] = newArgs + k; // get ready for first argument from file
	    while((c = fgetc(f)) != EOF) { // each line of file inserts 1 argument in the list
		if(c == '\n') {
		    if(j >= 1000-2) { printf(_("too many arguments\n")); exit(-1); }
		    newArgs[k++] = 0;  // terminate current arg
		    if(k >= 10000-1) { printf(_("too long arguments\n")); exit(-1); }
		    argvCopy[j++] = newArgs + k; // get ready for next
		} else {
		    if(k >= 10000-1) { printf(_("too long arguments\n")); exit(-1); }
		    newArgs[k++] = c;
		}
	    }
	    newArgs[k] = 0;
	    j--;
	    fclose(f);
	}
    }
    argvCopy[j] = NULL;
    argv = argvCopy;
    argc = j;
#endif

    setbuf(stdout, NULL);
    setbuf(stderr, NULL);
    debugFP = stderr;

    programName = strrchr(argv[0], '/');
    if (programName == NULL)
      programName = argv[0];
    else
      programName++;

#ifdef ENABLE_NLS
    XtSetLanguageProc(NULL, NULL, NULL);
    bindtextdomain(PACKAGE, LOCALEDIR);
    textdomain(PACKAGE);
#endif

    shellWidget =
      XtAppInitialize(&appContext, "XBoard", shellOptions,
		      XtNumber(shellOptions),
		      &argc, argv, xboardResources, NULL, 0);

    /* set up GTK */
    gtk_init (&argc, &argv);

    /* parse glade file to build widgets */

    builder = gtk_builder_new ();
    gtk_builder_add_from_file (builder, "gtk-interface.xml", NULL);

    /* test if everything worked ok */

    GUI_Window = GTK_WIDGET (gtk_builder_get_object (builder, "MainWindow"));
    if(!GUI_Window) printf("Error: gtk_builder didn't work!\n");

    GUI_History = GTK_WIDGET (gtk_builder_get_object (builder, "MoveHistory"));
    if(!GUI_History) printf("Error: gtk_builder didn't work!\n");

    GUI_Menubar  = GTK_WIDGET (gtk_builder_get_object (builder, "MenuBar"));
    if(!GUI_Menubar) printf("Error: gtk_builder didn't work!\n");
    GUI_Timer  = GTK_WIDGET (gtk_builder_get_object (builder, "Timer"));
    if(!GUI_Timer) printf("Error: gtk_builder didn't work!\n");
    GUI_Buttonbar  = GTK_WIDGET (gtk_builder_get_object (builder, "ButtonBar"));
    if(!GUI_Buttonbar) printf("Error: gtk_builder didn't work!\n");
    GUI_Board  = GTK_WIDGET (gtk_builder_get_object (builder, "Board"));
    if(!GUI_Board) printf("Error: gtk_builder didn't work!\n");

    GUI_Whiteclock  = GTK_WIDGET (gtk_builder_get_object (builder, "WhiteClock"));
    if(!GUI_Whiteclock) printf("Error: gtk_builder didn't work!\n");

    GUI_Blackclock  = GTK_WIDGET (gtk_builder_get_object (builder, "BlackClock"));
    if(!GUI_Blackclock) printf("Error: gtk_builder didn't work!\n");

    LIST_MoveHistory = GTK_LIST_STORE (gtk_builder_get_object (builder, "MoveHistoryStore"));
    if(!LIST_MoveHistory) printf("Error: gtk_builder didn't work!\n");


    gtk_builder_connect_signals (builder, NULL);

    // don't unref the builder, since we use it to get references to widgets
    //    g_object_unref (G_OBJECT (builder));

    /* end parse glade file */

    if (argc > 1)
      {
	fprintf(stderr, _("%s: unrecognized argument %s\n"),
		programName, argv[1]);

	fprintf(stderr, "Recognized options:\n");
	for(i = 0; i < XtNumber(shellOptions); i++) 
	  {
	    /* print first column */
	    j = fprintf(stderr, "  %s%s", shellOptions[i].option,
		        (shellOptions[i].argKind == XrmoptionSepArg
			 ? " ARG" : ""));
	    /* print second column and end line */
	    if (++i < XtNumber(shellOptions)) 
	      {		
		fprintf(stderr, "%*c%s%s\n", 40 - j, ' ',
			shellOptions[i].option,
			(shellOptions[i].argKind == XrmoptionSepArg
			 ? " ARG" : ""));
	      } 
	    else 
	      {
		fprintf(stderr, "\n");
	      };
	  };
	exit(2);
      };

    p = getenv("HOME");
    if (p == NULL) p = "/tmp";
    i = strlen(p) + strlen("/.xboardXXXXXx.pgn") + 1;
    gameCopyFilename = (char*) malloc(i);
    gamePasteFilename = (char*) malloc(i);
    snprintf(gameCopyFilename,i, "%s/.xboard%05uc.pgn", p, getpid());
    snprintf(gamePasteFilename,i, "%s/.xboard%05up.pgn", p, getpid());

    XtGetApplicationResources(shellWidget, (XtPointer) &appData,
			      clientResources, XtNumber(clientResources),
			      NULL, 0);

    { // [HGM] initstring: kludge to fix bad bug. expand '\n' characters in init string and computer string.
	static char buf[MSG_SIZ];
	EscapeExpand(buf, appData.initString);
	appData.initString = strdup(buf);
	EscapeExpand(buf, appData.secondInitString);
	appData.secondInitString = strdup(buf);
	EscapeExpand(buf, appData.firstComputerString);
	appData.firstComputerString = strdup(buf);
	EscapeExpand(buf, appData.secondComputerString);
	appData.secondComputerString = strdup(buf);
    }

    if ((chessDir = (char *) getenv("CHESSDIR")) == NULL) {
	chessDir = ".";
    } else {
	if (chdir(chessDir) != 0) {
	    fprintf(stderr, _("%s: can't cd to CHESSDIR: "), programName);
	    perror(chessDir);
	    exit(1);
	}
    }

    if (appData.debugMode && appData.nameOfDebugFile && strcmp(appData.nameOfDebugFile, "stderr")) {
	/* [DM] debug info to file [HGM] make the filename a command-line option, and allow it to remain stderr */
        if ((debugFP = fopen(appData.nameOfDebugFile, "w")) == NULL)  {
           printf(_("Failed to open file '%s'\n"), appData.nameOfDebugFile);
           exit(errno);
        }
        setbuf(debugFP, NULL);
    }

    /* [HGM,HR] make sure board size is acceptable */
    if(appData.NrFiles > BOARD_SIZE ||
       appData.NrRanks > BOARD_SIZE   )
      DisplayFatalError(_("Recompile with BOARD_SIZE > 12, to support this size"), 0, 2);

#if !HIGHDRAG
    /* This feature does not work; animation needs a rewrite */
    appData.highlightDragging = FALSE;
#endif
    InitBackEnd1();

    xDisplay = XtDisplay(shellWidget);
    xScreen = DefaultScreen(xDisplay);
    wm_delete_window = XInternAtom(xDisplay, "WM_DELETE_WINDOW", True);

    gameInfo.variant = StringToVariant(appData.variant);
    InitPosition(FALSE);

    /* calc board size */
    if (isdigit(appData.boardSize[0])) 
      {
	i = sscanf(appData.boardSize, "%d,%d,%d,%d,%d,%d,%d", &squareSize,
		   &lineGap, &clockFontPxlSize, &coordFontPxlSize,
		   &fontPxlSize, &smallLayout, &tinyLayout);
	if (i == 0) 
	  {
	    fprintf(stderr, _("%s: bad boardSize syntax %s\n"),
		    programName, appData.boardSize);
	    exit(2);
	  }
	if (i < 7) 
	  {
	    /* Find some defaults; use the nearest known size */
	    SizeDefaults *szd, *nearest;
	    int distance = 99999;
	    nearest = szd = sizeDefaults;
	    while (szd->name != NULL) 
	      {
		if (abs(szd->squareSize - squareSize) < distance) 
		  {
		    nearest = szd;
		    distance = abs(szd->squareSize - squareSize);
		    if (distance == 0) break;
		  }
		szd++;
	      };
	    if (i < 2) lineGap = nearest->lineGap;
	    if (i < 3) clockFontPxlSize = nearest->clockFontPxlSize;
	    if (i < 4) coordFontPxlSize = nearest->coordFontPxlSize;
	    if (i < 5) fontPxlSize = nearest->fontPxlSize;
	    if (i < 6) smallLayout = nearest->smallLayout;
	    if (i < 7) tinyLayout = nearest->tinyLayout;
	  }
      } 
    else 
      {
        SizeDefaults *szd = sizeDefaults;
        if (*appData.boardSize == NULLCHAR) 
	  {
	    while (DisplayWidth(xDisplay, xScreen) < szd->minScreenSize 
		   || DisplayHeight(xDisplay, xScreen) < szd->minScreenSize) 
	      {
		szd++;
	      }
	    if (szd->name == NULL) szd--;
	  } 
	else 
	  {
	    while (szd->name != NULL 
		   && StrCaseCmp(szd->name, appData.boardSize) != 0) 
	      szd++;
	    if (szd->name == NULL) 
	      {
		fprintf(stderr, _("%s: unrecognized boardSize name %s\n"),
			programName, appData.boardSize);
		exit(2);
	      }
	  }
	squareSize = szd->squareSize;
	lineGap = szd->lineGap;
	clockFontPxlSize = szd->clockFontPxlSize;
	coordFontPxlSize = szd->coordFontPxlSize;
	fontPxlSize = szd->fontPxlSize;
	smallLayout = szd->smallLayout;
	tinyLayout = szd->tinyLayout;
      }
    /* end figuring out what size to use */
    
    boardWidth  = lineGap + BOARD_WIDTH * (squareSize + lineGap);
    boardHeight = lineGap + BOARD_HEIGHT * (squareSize + lineGap);
    
    /*
     * Determine what fonts to use.
     */
    appData.clockFont = FindFont(appData.clockFont, clockFontPxlSize);
    clockFontID = XLoadFont(xDisplay, appData.clockFont);
    clockFontStruct = XQueryFont(xDisplay, clockFontID);
    appData.coordFont = FindFont(appData.coordFont, coordFontPxlSize);
    coordFontID = XLoadFont(xDisplay, appData.coordFont);
    coordFontStruct = XQueryFont(xDisplay, coordFontID);
    appData.font = FindFont(appData.font, fontPxlSize);
    countFontID = XLoadFont(xDisplay, appData.coordFont); // [HGM] holdings
    countFontStruct = XQueryFont(xDisplay, countFontID);
//    appData.font = FindFont(appData.font, fontPxlSize);

    xdb = XtDatabase(xDisplay);
    XrmPutStringResource(&xdb, "*font", appData.font);

    /*
     * Detect if there are not enough colors available and adapt.
     */
    if (DefaultDepth(xDisplay, xScreen) <= 2) {
      appData.monoMode = True;
    }

    if (!appData.monoMode) {
	vFrom.addr = (caddr_t) appData.lightSquareColor;
	vFrom.size = strlen(appData.lightSquareColor);
	XtConvert(shellWidget, XtRString, &vFrom, XtRPixel, &vTo);
	if (vTo.addr == NULL) {
	  appData.monoMode = True;
	  forceMono = True;
	} else {
	  lightSquareColor = *(Pixel *) vTo.addr;
	}
    }
    if (!appData.monoMode) {
	vFrom.addr = (caddr_t) appData.darkSquareColor;
	vFrom.size = strlen(appData.darkSquareColor);
	XtConvert(shellWidget, XtRString, &vFrom, XtRPixel, &vTo);
	if (vTo.addr == NULL) {
	  appData.monoMode = True;
	  forceMono = True;
	} else {
	  darkSquareColor = *(Pixel *) vTo.addr;
	}
    }
    if (!appData.monoMode) {
	vFrom.addr = (caddr_t) appData.whitePieceColor;
	vFrom.size = strlen(appData.whitePieceColor);
	XtConvert(shellWidget, XtRString, &vFrom, XtRPixel, &vTo);
	if (vTo.addr == NULL) {
	  appData.monoMode = True;
	  forceMono = True;
	} else {
	  whitePieceColor = *(Pixel *) vTo.addr;
	}
    }
    if (!appData.monoMode) {
	vFrom.addr = (caddr_t) appData.blackPieceColor;
	vFrom.size = strlen(appData.blackPieceColor);
	XtConvert(shellWidget, XtRString, &vFrom, XtRPixel, &vTo);
	if (vTo.addr == NULL) {
	  appData.monoMode = True;
	  forceMono = True;
	} else {
	  blackPieceColor = *(Pixel *) vTo.addr;
	}
    }

    if (!appData.monoMode) {
	vFrom.addr = (caddr_t) appData.highlightSquareColor;
	vFrom.size = strlen(appData.highlightSquareColor);
	XtConvert(shellWidget, XtRString, &vFrom, XtRPixel, &vTo);
	if (vTo.addr == NULL) {
	  appData.monoMode = True;
	  forceMono = True;
	} else {
	  highlightSquareColor = *(Pixel *) vTo.addr;
	}
    }

    if (!appData.monoMode) {
	vFrom.addr = (caddr_t) appData.premoveHighlightColor;
	vFrom.size = strlen(appData.premoveHighlightColor);
	XtConvert(shellWidget, XtRString, &vFrom, XtRPixel, &vTo);
	if (vTo.addr == NULL) {
	  appData.monoMode = True;
	  forceMono = True;
	} else {
	  premoveHighlightColor = *(Pixel *) vTo.addr;
	}
    }

    if (forceMono) {
      fprintf(stderr, _("%s: too few colors available; trying monochrome mode\n"),
	      programName);

      if (appData.bitmapDirectory == NULL ||
	      appData.bitmapDirectory[0] == NULLCHAR)
	    appData.bitmapDirectory = DEF_BITMAP_DIR;
    }

    if (appData.lowTimeWarning && !appData.monoMode) {
      vFrom.addr = (caddr_t) appData.lowTimeWarningColor;
      vFrom.size = strlen(appData.lowTimeWarningColor);
      XtConvert(shellWidget, XtRString, &vFrom, XtRPixel, &vTo);
      if (vTo.addr == NULL)
		appData.monoMode = True;
      else
		lowTimeWarningColor = *(Pixel *) vTo.addr;
    }

    if (appData.monoMode && appData.debugMode) {
	fprintf(stderr, _("white pixel = 0x%lx, black pixel = 0x%lx\n"),
		(unsigned long) XWhitePixel(xDisplay, xScreen),
		(unsigned long) XBlackPixel(xDisplay, xScreen));
    }

    if (parse_cpair(ColorShout, appData.colorShout) < 0 ||
	parse_cpair(ColorSShout, appData.colorSShout) < 0 ||
	parse_cpair(ColorChannel1, appData.colorChannel1) < 0  ||
	parse_cpair(ColorChannel, appData.colorChannel) < 0  ||
	parse_cpair(ColorKibitz, appData.colorKibitz) < 0 ||
	parse_cpair(ColorTell, appData.colorTell) < 0 ||
	parse_cpair(ColorChallenge, appData.colorChallenge) < 0  ||
	parse_cpair(ColorRequest, appData.colorRequest) < 0  ||
	parse_cpair(ColorSeek, appData.colorSeek) < 0  ||
	parse_cpair(ColorNormal, appData.colorNormal) < 0)
      {
	  if (appData.colorize) {
	      fprintf(stderr,
		      _("%s: can't parse color names; disabling colorization\n"),
		      programName);
	  }
	  appData.colorize = FALSE;
      }
    textColors[ColorNone].fg = textColors[ColorNone].bg = -1;
    textColors[ColorNone].attr = 0;

    //    XtAppAddActions(appContext, boardActions, XtNumber(boardActions));

    /*
     * widget hierarchy
     */
    if (tinyLayout) {
	layoutName = "tinyLayout";
    } else if (smallLayout) {
	layoutName = "smallLayout";
    } else {
	layoutName = "normalLayout";
    }

    if (appData.titleInWindow) {
      /* todo check what this appdata does */
    }

    if (appData.showButtonBar) {
      /* TODO hide button bar if requested */
    }

    /*
     *  gtk set properties of widgets
     */

    /* set board size */
    gtk_widget_set_size_request(GTK_WIDGET(GUI_Board),
				boardWidth,boardHeight);

    /* end gtk set properties of widgets */

    if (appData.titleInWindow)
      {
	if (smallLayout)
	  {
	    /* make it small */
	    if (appData.showButtonBar)
	      {

	      }
	  }
	else
	  {
	    if (appData.showButtonBar)
	      {
	      }
	  }
      }
    else
      {
      }


    /* set some checkboxes in the menu according to appData */

    if (appData.alwaysPromoteToQueen)
      gtk_check_menu_item_set_active (GTK_CHECK_MENU_ITEM (gtk_builder_get_object (builder, "menuOptions.Always Queen")),TRUE);

    if (appData.animateDragging)
      gtk_check_menu_item_set_active (GTK_CHECK_MENU_ITEM (gtk_builder_get_object (builder, "menuOptions.Animate Dragging")),TRUE);

    if (appData.animate)
      gtk_check_menu_item_set_active (GTK_CHECK_MENU_ITEM (gtk_builder_get_object (builder, "menuOptions.Animate Moving")),TRUE);

    if (appData.autoComment)
      gtk_check_menu_item_set_active (GTK_CHECK_MENU_ITEM (gtk_builder_get_object (builder, "menuOptions.Auto Comment")),TRUE);

    if (appData.autoCallFlag)
      gtk_check_menu_item_set_active (GTK_CHECK_MENU_ITEM (gtk_builder_get_object (builder, "menuOptions.Auto Flag")),TRUE);

    if (appData.autoFlipView)
      gtk_check_menu_item_set_active (GTK_CHECK_MENU_ITEM (gtk_builder_get_object (builder, "menuOptions.Auto Flip View")),TRUE);

    if (appData.autoObserve)
      gtk_check_menu_item_set_active (GTK_CHECK_MENU_ITEM (gtk_builder_get_object (builder, "menuOptions.Auto Observe")),TRUE);

    if (appData.autoRaiseBoard)
      gtk_check_menu_item_set_active (GTK_CHECK_MENU_ITEM (gtk_builder_get_object (builder, "menuOptions.Auto Raise Board")),TRUE);

    if (appData.autoSaveGames)
      gtk_check_menu_item_set_active (GTK_CHECK_MENU_ITEM (gtk_builder_get_object (builder, "menuOptions.Auto Save")),TRUE);

    if (appData.saveGameFile[0] != NULLCHAR)
      {
	/* Can't turn this off from menu */
	gtk_check_menu_item_set_active (GTK_CHECK_MENU_ITEM (gtk_builder_get_object (builder, "menuOptions.Auto Save")),TRUE);
	gtk_action_set_sensitive(GTK_ACTION (gtk_builder_get_object (builder, "menuOptions.Auto Save")),FALSE);
      }

    if (appData.blindfold)
      gtk_check_menu_item_set_active (GTK_CHECK_MENU_ITEM (gtk_builder_get_object (builder, "menuOptions.Blindfold")),TRUE);

    if (appData.flashCount > 0)
      gtk_check_menu_item_set_active (GTK_CHECK_MENU_ITEM (gtk_builder_get_object (builder, "menuOptions.Flash Moves")),TRUE);

    if (appData.getMoveList)
      gtk_check_menu_item_set_active (GTK_CHECK_MENU_ITEM (gtk_builder_get_object (builder, "menuOptions.Get Move List")),TRUE);

#if HIGHDRAG
    if (appData.highlightDragging)
      gtk_check_menu_item_set_active (GTK_CHECK_MENU_ITEM (gtk_builder_get_object (builder, "menuOptions.Highlight Dragging")),TRUE);
#endif

    if (appData.highlightLastMove)
      gtk_check_menu_item_set_active (GTK_CHECK_MENU_ITEM (gtk_builder_get_object (builder, "menuOptions.Highlight Last Move")),TRUE);

    if (appData.icsAlarm)
      gtk_check_menu_item_set_active (GTK_CHECK_MENU_ITEM (gtk_builder_get_object (builder, "menuOptions.ICS Alarm")),TRUE);

    if (appData.ringBellAfterMoves)
      gtk_check_menu_item_set_active (GTK_CHECK_MENU_ITEM (gtk_builder_get_object (builder, "menuOptions.Move Sound")),TRUE);

    if (appData.oldSaveStyle)
      gtk_check_menu_item_set_active (GTK_CHECK_MENU_ITEM (gtk_builder_get_object (builder, "menuOptions.Old Save Style")),TRUE);

    if (appData.periodicUpdates)
      gtk_check_menu_item_set_active (GTK_CHECK_MENU_ITEM (gtk_builder_get_object (builder, "menuOptions.Periodic Updates")),TRUE);

    if (appData.ponderNextMove)
      gtk_check_menu_item_set_active (GTK_CHECK_MENU_ITEM (gtk_builder_get_object (builder, "menuOptions.Ponder Next Move")),TRUE);

    if (appData.popupExitMessage)
      gtk_check_menu_item_set_active (GTK_CHECK_MENU_ITEM (gtk_builder_get_object (builder, "menuOptions.Popup Exit Message")),TRUE);

    if (appData.popupMoveErrors)
      gtk_check_menu_item_set_active (GTK_CHECK_MENU_ITEM (gtk_builder_get_object (builder, "menuOptions.Popup Move Errors")),TRUE);

    if (appData.premove)
      gtk_check_menu_item_set_active (GTK_CHECK_MENU_ITEM (gtk_builder_get_object (builder, "menuOptions.Premove")),TRUE);

    if (appData.quietPlay)
      gtk_check_menu_item_set_active (GTK_CHECK_MENU_ITEM (gtk_builder_get_object (builder, "menuOptions.Quit Play")),TRUE);

    if (appData.showCoords)
      gtk_check_menu_item_set_active (GTK_CHECK_MENU_ITEM (gtk_builder_get_object (builder, "menuOptions.Show Coords")),TRUE);

    if (appData.showThinking)
      gtk_check_menu_item_set_active (GTK_CHECK_MENU_ITEM (gtk_builder_get_object (builder, "menuOptions.Show Thinking")),TRUE);

    if (appData.testLegality)
      gtk_check_menu_item_set_active (GTK_CHECK_MENU_ITEM (gtk_builder_get_object (builder, "menuOptions.Test Legality")),TRUE);

    /* end setting check boxes */

    /* load square colors */
    SVGLightSquare   = load_pixbuf("svg/LightSquare.svg",squareSize);
    SVGDarkSquare    = load_pixbuf("svg/DarkSquare.svg",squareSize);
    SVGNeutralSquare = load_pixbuf("svg/NeutralSquare.svg",squareSize);

    /* use two icons to indicate if it is white's or black's turn */
    WhiteIcon  = load_pixbuf("svg/icon_white.svg",0);
    BlackIcon  = load_pixbuf("svg/icon_black.svg",0);
    WindowIcon = WhiteIcon;
    gtk_window_set_icon(GTK_WINDOW(GUI_Window),WindowIcon);


    /* realize window */
    gtk_widget_show (GUI_Window);

    /* do resizing to a fixed aspect ratio */
    {
      GtkRequisition w;
      int totalh=boardHeight;
      float ratio;

      gtk_widget_size_request(GTK_WIDGET(GUI_Menubar),   &w);
      totalh += w.height;

      gtk_widget_size_request(GTK_WIDGET(GUI_Timer),     &w);
      totalh += w.height;

      gtk_widget_size_request(GTK_WIDGET(GUI_Buttonbar), &w);
      totalh += w.height;

      ratio  = (totalh)/(boardWidth) ;
      GUI_SetAspectRatio(ratio);
    }

    CreateGCs();
    CreatePieces();
    CreatePieceMenus();

    if (appData.animate || appData.animateDragging)
      CreateAnimVars();

    InitBackEnd2();

    if (errorExitStatus == -1) {
	if (appData.icsActive) {
	    /* We now wait until we see "login:" from the ICS before
	       sending the logon script (problems with timestamp otherwise) */
	    /*ICSInitScript();*/
	    if (appData.icsInputBox) ICSInputBoxPopUp();
	}

	signal(SIGINT, IntSigHandler);
	signal(SIGTERM, IntSigHandler);
	if (*appData.cmailGameName != NULLCHAR) {
	    signal(SIGUSR1, CmailSigHandler);
	}
    }
    gameInfo.boardWidth = 0; // [HGM] pieces: kludge to ensure InitPosition() calls InitDrawingSizes()
    InitPosition(TRUE);

    /*
     * Create a cursor for the board widget.
     * (This needs to be called after the window has been created to have access to board-window)
     */

    BoardCursor = gdk_cursor_new(GDK_HAND2);
    gdk_window_set_cursor(GUI_Board->window, BoardCursor);
    gdk_cursor_destroy(BoardCursor);

    /* end cursor */
    gtk_main ();

    if (appData.debugMode) fclose(debugFP); // [DM] debug
    return 0;
}

void
ShutDownFrontEnd()
{
    if (appData.icsActive && oldICSInteractionTitle != NULL) {
        DisplayIcsInteractionTitle(oldICSInteractionTitle);
    }
    unlink(gameCopyFilename);
    unlink(gamePasteFilename);
}

RETSIGTYPE
IntSigHandler(sig)
     int sig;
{
    ExitEvent(sig);
}

RETSIGTYPE
CmailSigHandler(sig)
     int sig;
{
    int dummy = 0;
    int error;

    signal(SIGUSR1, SIG_IGN);	/* suspend handler     */

    /* Activate call-back function CmailSigHandlerCallBack()             */
    OutputToProcess(cmailPR, (char *)(&dummy), sizeof(int), &error);

    signal(SIGUSR1, CmailSigHandler); /* re-activate handler */
}

void
CmailSigHandlerCallBack(isr, closure, message, count, error)
     InputSourceRef isr;
     VOIDSTAR closure;
     char *message;
     int count;
     int error;
{
    BoardToTop();
    ReloadCmailMsgEvent(TRUE);	/* Reload cmail msg  */
}
/**** end signal code ****/


void
ICSInitScript()
{
    FILE *f;
    char buf[MSG_SIZ];
    char *p;

    f = fopen(appData.icsLogon, "r");
    if (f == NULL) {
	p = getenv("HOME");
	if (p != NULL) {
	    strcpy(buf, p);
	    strcat(buf, "/");
	    strcat(buf, appData.icsLogon);
	    f = fopen(buf, "r");
	}
    }
    if (f != NULL)
      ProcessICSInitScript(f);
}

void
ResetFrontEnd()
{
    CommentPopDown();
    EditCommentPopDown();
    TagsPopDown();
    return;
}

void
SetMenuEnables(enab)
     Enables *enab;
{
  GObject *o;

  if (!builder) return;
  while (enab->name != NULL) {
    o = gtk_builder_get_object(builder, enab->name);
    if(GTK_IS_WIDGET(o))
      gtk_widget_set_sensitive(GTK_WIDGET (o),enab->value);
    else
      {
	if(GTK_IS_ACTION(o))
	  gtk_action_set_sensitive(GTK_ACTION (o),enab->value);
	else
	  DisplayError(enab->name, 0);
      }
    enab++;
  }
}

void SetICSMode()
{
  SetMenuEnables(icsEnables);

#ifdef ZIPPY
  if (appData.zippyPlay && !appData.noChessProgram)   /* [DM] icsEngineAnalyze */
    {}; //     XtSetSensitive(XtNameToWidget(menuBarWidget, "menuMode.Analysis Mode"), True);
#endif
}

void
SetNCPMode()
{
  SetMenuEnables(ncpEnables);
}

void
SetGNUMode()
{
  SetMenuEnables(gnuEnables);
}

void
SetCmailMode()
{
  SetMenuEnables(cmailEnables);
}

void
SetTrainingModeOn()
{
  SetMenuEnables(trainingOnEnables);
  if (appData.showButtonBar) {
    //    XtSetSensitive(buttonBarWidget, False);
  }
  CommentPopDown();
}

void
SetTrainingModeOff()
{
  SetMenuEnables(trainingOffEnables);
  if (appData.showButtonBar) {
    //    XtSetSensitive(buttonBarWidget, True);
  }
}

void
SetUserThinkingEnables()
{
  if (appData.noChessProgram) return;
  SetMenuEnables(userThinkingEnables);
}

void
SetMachineThinkingEnables()
{
  if (appData.noChessProgram) return;
  SetMenuEnables(machineThinkingEnables);
  switch (gameMode) {
  case MachinePlaysBlack:
  case MachinePlaysWhite:
  case TwoMachinesPlay:
//    XtSetSensitive(XtNameToWidget(menuBarWidget,
//				  ModeToWidgetName(gameMode)), True);
    break;
  default:
    break;
  }
}

#define Abs(n) ((n)<0 ? -(n) : (n))

/*
 * Find a font that matches "pattern" that is as close as
 * possible to the targetPxlSize.  Prefer fonts that are k
 * pixels smaller to fonts that are k pixels larger.  The
 * pattern must be in the X Consortium standard format,
 * e.g. "-*-helvetica-bold-r-normal--*-*-*-*-*-*-*-*".
 * The return value should be freed with XtFree when no
 * longer needed.
 */
char *FindFont(pattern, targetPxlSize)
     char *pattern;
     int targetPxlSize;
{
    char **fonts, *p, *best, *scalable, *scalableTail;
    int i, j, nfonts, minerr, err, pxlSize;

#ifdef ENABLE_NLS
    char **missing_list;
    int missing_count;
    char *def_string, *base_fnt_lst, strInt[3];
    XFontSet fntSet;
    XFontStruct **fnt_list;

    base_fnt_lst = calloc(1, strlen(pattern) + 3);
    sprintf(strInt, "%d", targetPxlSize);
    p = strstr(pattern, "--");
    strncpy(base_fnt_lst, pattern, p - pattern + 2);
    strcat(base_fnt_lst, strInt);
    strcat(base_fnt_lst, strchr(p + 2, '-'));

    if ((fntSet = XCreateFontSet(xDisplay,
                                 base_fnt_lst,
                                 &missing_list,
                                 &missing_count,
                                 &def_string)) == NULL) {

       fprintf(stderr, _("Unable to create font set.\n"));
       exit (2);
    }

    nfonts = XFontsOfFontSet(fntSet, &fnt_list, &fonts);
#else
    fonts = XListFonts(xDisplay, pattern, 999999, &nfonts);
    if (nfonts < 1) {
	fprintf(stderr, _("%s: no fonts match pattern %s\n"),
		programName, pattern);
	exit(2);
    }
#endif

    best = fonts[0];
    scalable = NULL;
    minerr = 999999;
    for (i=0; i<nfonts; i++) {
	j = 0;
	p = fonts[i];
	if (*p != '-') continue;
	while (j < 7) {
	    if (*p == NULLCHAR) break;
	    if (*p++ == '-') j++;
	}
	if (j < 7) continue;
	pxlSize = atoi(p);
	if (pxlSize == 0) {
	    scalable = fonts[i];
	    scalableTail = p;
	} else {
	    err = pxlSize - targetPxlSize;
	    if (Abs(err) < Abs(minerr) ||
	        (minerr > 0 && err < 0 && -err == minerr)) {
	        best = fonts[i];
	        minerr = err;
	    }
	}
    }
    if (scalable && Abs(minerr) > appData.fontSizeTolerance) {
        /* If the error is too big and there is a scalable font,
	   use the scalable font. */
        int headlen = scalableTail - scalable;
        p = (char *) XtMalloc(strlen(scalable) + 10);
	while (isdigit(*scalableTail)) scalableTail++;
	sprintf(p, "%.*s%d%s", headlen, scalable, targetPxlSize, scalableTail);
    } else {
        p = (char *) XtMalloc(strlen(best) + 1);
        strcpy(p, best);
    }
    if (appData.debugMode) {
        fprintf(debugFP, _("resolved %s at pixel size %d\n  to %s\n"),
		pattern, targetPxlSize, p);
    }
#ifdef ENABLE_NLS
    if (missing_count > 0)
       XFreeStringList(missing_list);
    XFreeFontSet(xDisplay, fntSet);
#else
     XFreeFontNames(fonts);
#endif
    return p;
}

void CreateGCs()
{
    XtGCMask value_mask = GCLineWidth | GCLineStyle | GCForeground
      | GCBackground | GCFunction | GCPlaneMask;
    XGCValues gc_values;
    GC copyInvertedGC;

    gc_values.plane_mask = AllPlanes;
    gc_values.line_width = lineGap;
    gc_values.line_style = LineSolid;
    gc_values.function = GXcopy;

    gc_values.foreground = XBlackPixel(xDisplay, xScreen);
    gc_values.background = XWhitePixel(xDisplay, xScreen);
    coordGC = XtGetGC(shellWidget, value_mask, &gc_values);
    XSetFont(xDisplay, coordGC, coordFontID);

    if (appData.monoMode) {
	gc_values.foreground = XWhitePixel(xDisplay, xScreen);
	gc_values.background = XBlackPixel(xDisplay, xScreen);
	lightSquareGC = wbPieceGC
	  = XtGetGC(shellWidget, value_mask, &gc_values);

	gc_values.foreground = XBlackPixel(xDisplay, xScreen);
	gc_values.background = XWhitePixel(xDisplay, xScreen);
	darkSquareGC = bwPieceGC
	  = XtGetGC(shellWidget, value_mask, &gc_values);

	if (DefaultDepth(xDisplay, xScreen) == 1) {
	    /* Avoid XCopyPlane on 1-bit screens to work around Sun bug */
	    gc_values.function = GXcopyInverted;
	    copyInvertedGC = XtGetGC(shellWidget, value_mask, &gc_values);
	    gc_values.function = GXcopy;
	    if (XBlackPixel(xDisplay, xScreen) == 1) {
		bwPieceGC = darkSquareGC;
		wbPieceGC = copyInvertedGC;
	    } else {
		bwPieceGC = copyInvertedGC;
		wbPieceGC = lightSquareGC;
	    }
	}
    } else {
	gc_values.foreground = lightSquareColor;
	gc_values.background = darkSquareColor;
	lightSquareGC = XtGetGC(shellWidget, value_mask, &gc_values);

	gc_values.foreground = darkSquareColor;
	gc_values.background = lightSquareColor;
	darkSquareGC = XtGetGC(shellWidget, value_mask, &gc_values);

	gc_values.foreground = jailSquareColor;
	gc_values.background = jailSquareColor;
	jailSquareGC = XtGetGC(shellWidget, value_mask, &gc_values);

	gc_values.foreground = whitePieceColor;
	gc_values.background = darkSquareColor;
	wdPieceGC = XtGetGC(shellWidget, value_mask, &gc_values);

	gc_values.foreground = whitePieceColor;
	gc_values.background = lightSquareColor;
	wlPieceGC = XtGetGC(shellWidget, value_mask, &gc_values);

	gc_values.foreground = whitePieceColor;
	gc_values.background = jailSquareColor;
	wjPieceGC = XtGetGC(shellWidget, value_mask, &gc_values);

	gc_values.foreground = blackPieceColor;
	gc_values.background = darkSquareColor;
	bdPieceGC = XtGetGC(shellWidget, value_mask, &gc_values);

	gc_values.foreground = blackPieceColor;
	gc_values.background = lightSquareColor;
	blPieceGC = XtGetGC(shellWidget, value_mask, &gc_values);

	gc_values.foreground = blackPieceColor;
	gc_values.background = jailSquareColor;
	bjPieceGC = XtGetGC(shellWidget, value_mask, &gc_values);
    }
}

void CreatePieces()
{
  /* order of pieces
  WhitePawn, WhiteKnight, WhiteBishop, WhiteRook, WhiteQueen, WhiteKing,
  BlackPawn, BlackKnight, BlackBishop, BlackRook, BlackQueen, BlackKing,
  */
  int i;

  /* get some defaults going */
  for(i=WhitePawn; i<DemotePiece+1; i++)
    SVGpieces[i]   = load_pixbuf("svg/NeutralSquare.svg",squareSize);
    

  SVGpieces[WhitePawn]   = load_pixbuf("svg/WhitePawn.svg",squareSize);
  SVGpieces[WhiteKnight] = load_pixbuf("svg/WhiteKnight.svg",squareSize);
  SVGpieces[WhiteBishop] = load_pixbuf("svg/WhiteBishop.svg",squareSize);
  SVGpieces[WhiteRook]   = load_pixbuf("svg/WhiteRook.svg",squareSize);
  SVGpieces[WhiteQueen]  = load_pixbuf("svg/WhiteQueen.svg",squareSize);
  SVGpieces[WhiteKing]   = load_pixbuf("svg/WhiteKing.svg",squareSize);

  SVGpieces[BlackPawn]   = load_pixbuf("svg/BlackPawn.svg",squareSize);
  SVGpieces[BlackKnight] = load_pixbuf("svg/BlackKnight.svg",squareSize);
  SVGpieces[BlackBishop] = load_pixbuf("svg/BlackBishop.svg",squareSize);
  SVGpieces[BlackRook]   = load_pixbuf("svg/BlackRook.svg",squareSize);
  SVGpieces[BlackQueen]  = load_pixbuf("svg/BlackQueen.svg",squareSize);
  SVGpieces[BlackKing]   = load_pixbuf("svg/BlackKing.svg",squareSize);

  return;
}


static void MenuBarSelect(w, addr, index)
     Widget w;
     caddr_t addr;
     caddr_t index;
{
    XtActionProc proc = (XtActionProc) addr;

    (proc)(NULL, NULL, NULL, NULL);
}

void CreateMenuBarPopup(parent, name, mb)
     Widget parent;
     String name;
     Menu *mb;
{
    int j;
    Widget menu, entry;
    MenuItem *mi;
    Arg args[16];

    menu = XtCreatePopupShell(name, simpleMenuWidgetClass,
			      parent, NULL, 0);
    j = 0;
    XtSetArg(args[j], XtNleftMargin, 20);   j++;
    XtSetArg(args[j], XtNrightMargin, 20);  j++;
    mi = mb->mi;
    while (mi->string != NULL) {
	if (strcmp(mi->string, "----") == 0) {
	    entry = XtCreateManagedWidget(mi->string, smeLineObjectClass,
					  menu, args, j);
	} else {
          XtSetArg(args[j], XtNlabel, XtNewString(_(mi->string)));
	    entry = XtCreateManagedWidget(mi->string, smeBSBObjectClass,
					  menu, args, j+1);
	    XtAddCallback(entry, XtNcallback,
			  (XtCallbackProc) MenuBarSelect,
			  (caddr_t) mi->proc);
	}
	mi++;
    }
}

Widget CreateMenuBar(mb)
     Menu *mb;
{
    int j;
    Widget anchor, menuBar;
    Arg args[16];
    char menuName[MSG_SIZ];

    j = 0;
    XtSetArg(args[j], XtNorientation, XtorientHorizontal);  j++;
    XtSetArg(args[j], XtNvSpace, 0);                        j++;
    XtSetArg(args[j], XtNborderWidth, 0);                   j++;
    menuBar = XtCreateWidget("menuBar", boxWidgetClass,
			     formWidget, args, j);

    while (mb->name != NULL) {
	strcpy(menuName, "menu");
	strcat(menuName, mb->name);
	j = 0;
	XtSetArg(args[j], XtNmenuName, XtNewString(menuName));  j++;
	if (tinyLayout) {
	    char shortName[2];
            shortName[0] = _(mb->name)[0];
	    shortName[1] = NULLCHAR;
	    XtSetArg(args[j], XtNlabel, XtNewString(shortName)); j++;
	}
      else {
          XtSetArg(args[j], XtNlabel, XtNewString(_(mb->name))); j++;
      }

	XtSetArg(args[j], XtNborderWidth, 0);                   j++;
	anchor = XtCreateManagedWidget(mb->name, menuButtonWidgetClass,
				       menuBar, args, j);
	CreateMenuBarPopup(menuBar, menuName, mb);
	mb++;
    }
    return menuBar;
}

Widget CreateButtonBar(mi)
     MenuItem *mi;
{
    int j;
    Widget button, buttonBar;
    Arg args[16];

    j = 0;
    XtSetArg(args[j], XtNorientation, XtorientHorizontal); j++;
    if (tinyLayout) {
	XtSetArg(args[j], XtNhSpace, 0); j++;
    }
    XtSetArg(args[j], XtNborderWidth, 0); j++;
    XtSetArg(args[j], XtNvSpace, 0);                        j++;
    buttonBar = XtCreateWidget("buttonBar", boxWidgetClass,
			       formWidget, args, j);

    while (mi->string != NULL) {
	j = 0;
	if (tinyLayout) {
	    XtSetArg(args[j], XtNinternalWidth, 2); j++;
	    XtSetArg(args[j], XtNborderWidth, 0); j++;
	}
      XtSetArg(args[j], XtNlabel, XtNewString(_(mi->string))); j++;
	button = XtCreateManagedWidget(mi->string, commandWidgetClass,
				       buttonBar, args, j);
	XtAddCallback(button, XtNcallback,
		      (XtCallbackProc) MenuBarSelect,
		      (caddr_t) mi->proc);
	mi++;
    }
    return buttonBar;
}

Widget
CreatePieceMenu(name, color)
     char *name;
     int color;
{
    int i;
    Widget entry, menu;
    Arg args[16];
    ChessSquare selection;

    menu = XtCreatePopupShell(name, simpleMenuWidgetClass,
			      boardWidget, args, 0);

    for (i = 0; i < PIECE_MENU_SIZE; i++) {
	String item = pieceMenuStrings[color][i];

	if (strcmp(item, "----") == 0) {
	    entry = XtCreateManagedWidget(item, smeLineObjectClass,
					  menu, NULL, 0);
	} else {
          XtSetArg(args[0], XtNlabel, XtNewString(_(item)));
	    entry = XtCreateManagedWidget(item, smeBSBObjectClass,
                                menu, args, 1);
	    selection = pieceMenuTranslation[color][i];
	    XtAddCallback(entry, XtNcallback,
			  (XtCallbackProc) PieceMenuSelect,
			  (caddr_t) selection);
	    if (selection == WhitePawn || selection == BlackPawn) {
		XtSetArg(args[0], XtNpopupOnEntry, entry);
		XtSetValues(menu, args, 1);
	    }
	}
    }
    return menu;
}

void
CreatePieceMenus()
{
    int i;
    Widget entry;
    Arg args[16];
    ChessSquare selection;

//    whitePieceMenu = CreatePieceMenu("menuW", 0);
//    blackPieceMenu = CreatePieceMenu("menuB", 1);
//
//    XtRegisterGrabAction(PieceMenuPopup, True,
//			 (unsigned)(ButtonPressMask|ButtonReleaseMask),
//			 GrabModeAsync, GrabModeAsync);
//
//    XtSetArg(args[0], XtNlabel, _("Drop"));
//    dropMenu = XtCreatePopupShell("menuD", simpleMenuWidgetClass,
//				  boardWidget, args, 1);
//    for (i = 0; i < DROP_MENU_SIZE; i++) {
//	String item = dropMenuStrings[i];
//
//	if (strcmp(item, "----") == 0) {
//	    entry = XtCreateManagedWidget(item, smeLineObjectClass,
//					  dropMenu, NULL, 0);
//	} else {
//          XtSetArg(args[0], XtNlabel, XtNewString(_(item)));
//	    entry = XtCreateManagedWidget(item, smeBSBObjectClass,
//                                dropMenu, args, 1);
//	    selection = dropMenuTranslation[i];
//	    XtAddCallback(entry, XtNcallback,
//			  (XtCallbackProc) DropMenuSelect,
//			  (caddr_t) selection);
//	}
//    }
}

void SetupDropMenu()
{
    int i, j, count;
    char label[32];
    Arg args[16];
    Widget entry;
    char* p;

    for (i=0; i<sizeof(dmEnables)/sizeof(DropMenuEnables); i++) {
	entry = XtNameToWidget(dropMenu, dmEnables[i].widget);
	p = strchr(gameMode == IcsPlayingWhite ? white_holding : black_holding,
		   dmEnables[i].piece);
	XtSetSensitive(entry, p != NULL || !appData.testLegality
		       /*!!temp:*/ || (gameInfo.variant == VariantCrazyhouse
				       && !appData.icsActive));
	count = 0;
	while (p && *p++ == dmEnables[i].piece) count++;
	snprintf(label, sizeof(label), "%s  %d", dmEnables[i].widget, count);
	j = 0;
	XtSetArg(args[j], XtNlabel, label); j++;
	XtSetValues(entry, args, j);
    }
}

void PieceMenuPopup(w, event, params, num_params)
     Widget w;
     XEvent *event;
     String *params;
     Cardinal *num_params;
{
    String whichMenu;
    if (event->type != ButtonPress) return;
    if (errorUp) ErrorPopDown();
    switch (gameMode) {
      case EditPosition:
      case IcsExamining:
	whichMenu = params[0];
	break;
      case IcsPlayingWhite:
      case IcsPlayingBlack:
      case EditGame:
      case MachinePlaysWhite:
      case MachinePlaysBlack:
	if (appData.testLegality &&
	    gameInfo.variant != VariantBughouse &&
	    gameInfo.variant != VariantCrazyhouse) return;
	SetupDropMenu();
	whichMenu = "menuD";
	break;
      default:
	return;
    }

    if (((pmFromX = EventToSquare(event->xbutton.x, BOARD_WIDTH)) < 0) ||
	((pmFromY = EventToSquare(event->xbutton.y, BOARD_HEIGHT)) < 0)) {
	pmFromX = pmFromY = -1;
	return;
    }
    if (flipView)
      pmFromX = BOARD_WIDTH - 1 - pmFromX;
    else
      pmFromY = BOARD_HEIGHT - 1 - pmFromY;

    XtPopupSpringLoaded(XtNameToWidget(boardWidget, whichMenu));
}

static void PieceMenuSelect(w, piece, junk)
     Widget w;
     ChessSquare piece;
     caddr_t junk;
{
    if (pmFromX < 0 || pmFromY < 0) return;
    EditPositionMenuEvent(piece, pmFromX, pmFromY);
}

static void DropMenuSelect(w, piece, junk)
     Widget w;
     ChessSquare piece;
     caddr_t junk;
{
    if (pmFromX < 0 || pmFromY < 0) return;
    DropMenuEvent(piece, pmFromX, pmFromY);
}

void WhiteClock(w, event, prms, nprms)
     Widget w;
     XEvent *event;
     String *prms;
     Cardinal *nprms;
{
    if (gameMode == EditPosition || gameMode == IcsExamining) {
	SetWhiteToPlayEvent();
    } else if (gameMode == IcsPlayingBlack || gameMode == MachinePlaysWhite) {
	CallFlagEvent();
    }
}

void BlackClock(w, event, prms, nprms)
     Widget w;
     XEvent *event;
     String *prms;
     Cardinal *nprms;
{
    if (gameMode == EditPosition || gameMode == IcsExamining) {
	SetBlackToPlayEvent();
    } else if (gameMode == IcsPlayingWhite || gameMode == MachinePlaysBlack) {
	CallFlagEvent();
    }
}


/*
 * If the user selects on a border boundary, return -1; if off the board,
 *   return -2.  Otherwise map the event coordinate to the square.
 */
int EventToSquare(x, limit)
     int x;
{
    if (x <= 0)
      return -2;
    if (x < lineGap)
      return -1;
    x -= lineGap;
    if ((x % (squareSize + lineGap)) >= squareSize)
      return -1;
    x /= (squareSize + lineGap);
    if (x >= limit)
      return -2;
    return x;
}

static void do_flash_delay(msec)
     unsigned long msec;
{
    TimeDelay(msec);
}

static void drawHighlight(file, rank, line_type)
     int file, rank, line_type;
{
    int x, y;
    cairo_t *cr;

    if (lineGap == 0 || appData.blindfold) return;

    if (flipView)
      {
	x = lineGap/2 + ((BOARD_WIDTH-1)-file) *
	  (squareSize + lineGap);
	y = lineGap/2 + rank * (squareSize + lineGap);
      }
    else
      {
	x = lineGap/2 + file * (squareSize + lineGap);
	y = lineGap/2 + ((BOARD_HEIGHT-1)-rank) *
	  (squareSize + lineGap);
      }

    /* get a cairo_t */
    cr = gdk_cairo_create (GDK_WINDOW(GUI_Board->window));

    /* draw the highlight */
    cairo_move_to (cr, x, y);
    cairo_rel_line_to (cr, 0,squareSize+lineGap);
    cairo_rel_line_to (cr, squareSize+lineGap,0);
    cairo_rel_line_to (cr, 0,-squareSize-lineGap);
    cairo_close_path (cr);

    cairo_set_line_width (cr, lineGap);
    switch(line_type)
      {
	/* TODO: use appdata colors */
      case LINE_TYPE_HIGHLIGHT:
	cairo_set_source_rgba (cr, 1, 1, 0, 1.0);
	break;
      case LINE_TYPE_PRE:
	cairo_set_source_rgba (cr, 1, 0, 0, 1.0);
	break;
      case LINE_TYPE_NORMAL:
      default:
	cairo_set_source_rgba (cr, 0, 1, 0, 1.0);
      }

    cairo_stroke (cr);

    /* free memory */
    cairo_destroy (cr);

    return;
}

int hi1X = -1, hi1Y = -1, hi2X = -1, hi2Y = -1;
int pm1X = -1, pm1Y = -1, pm2X = -1, pm2Y = -1;

void
SetHighlights(fromX, fromY, toX, toY)
     int fromX, fromY, toX, toY;
{
    if (hi1X != fromX || hi1Y != fromY)
      {
	if (hi1X >= 0 && hi1Y >= 0)
	  {
	    drawHighlight(hi1X, hi1Y, LINE_TYPE_NORMAL);
	  }
	if (fromX >= 0 && fromY >= 0)
	  {
	    drawHighlight(fromX, fromY, LINE_TYPE_HIGHLIGHT);
	  }
      }
    if (hi2X != toX || hi2Y != toY)
      {
	if (hi2X >= 0 && hi2Y >= 0)
	  {
	    drawHighlight(hi2X, hi2Y, LINE_TYPE_NORMAL);
	  }
	if (toX >= 0 && toY >= 0)
	  {
	    drawHighlight(toX, toY, LINE_TYPE_HIGHLIGHT);
	  }
      }
    hi1X = fromX;
    hi1Y = fromY;
    hi2X = toX;
    hi2Y = toY;

    return;
}

void
ClearHighlights()
{
    SetHighlights(-1, -1, -1, -1);
}


void
SetPremoveHighlights(fromX, fromY, toX, toY)
     int fromX, fromY, toX, toY;
{
    if (pm1X != fromX || pm1Y != fromY)
      {
	if (pm1X >= 0 && pm1Y >= 0)
	  {
	    drawHighlight(pm1X, pm1Y, LINE_TYPE_NORMAL);
	  }
	if (fromX >= 0 && fromY >= 0)
	  {
	    drawHighlight(fromX, fromY, LINE_TYPE_PRE);
	  }
      }
    if (pm2X != toX || pm2Y != toY)
      {
	if (pm2X >= 0 && pm2Y >= 0)
	  {
	    drawHighlight(pm2X, pm2Y, LINE_TYPE_NORMAL);
	  }
	if (toX >= 0 && toY >= 0)
	  {
	    drawHighlight(toX, toY, LINE_TYPE_PRE);
	  }
      }

    pm1X = fromX;
    pm1Y = fromY;
    pm2X = toX;
    pm2Y = toY;

    return;
}

void
ClearPremoveHighlights()
{
  SetPremoveHighlights(-1, -1, -1, -1);
}

static void BlankSquare(x, y, color, piece, dest)
     int x, y, color;
     ChessSquare piece;
     Drawable dest;
{
      GdkPixbuf *pb;

      switch (color) 
	{
	case 0: /* dark */
	  pb = SVGDarkSquare;
	  break;
	case 1: /* light */
	  pb = SVGLightSquare;
	  break;
	case 2: /* neutral */
	default:
	  pb = SVGNeutralSquare;
	  break;
	}
      gdk_draw_pixbuf(GDK_WINDOW(GUI_Board->window),NULL,pb,0,0,x,y,-1,-1, GDK_RGB_DITHER_NORMAL, 0, 0);
      return;
}

static void DrawPiece(piece, square_color, x, y, dest)
     ChessSquare piece;
     int square_color, x, y;
     Drawable dest;
{
  /* redraw background, since piece might be transparent in some areas */
  BlankSquare(x,y,square_color,piece,dest);

  /* draw piece */
  gdk_draw_pixbuf(GDK_WINDOW(GUI_Board->window),NULL,
		  GDK_PIXBUF(SVGpieces[piece]),0,0,x,y,-1,-1,
		  GDK_RGB_DITHER_NORMAL, 0, 0);
  return ;
}

/* [HR] determine square color depending on chess variant. */
static int SquareColor(row, column)
     int row, column;
{
    int square_color;

    if (gameInfo.variant == VariantXiangqi) {
        if (column >= 3 && column <= 5 && row >= 0 && row <= 2) {
            square_color = 1;
        } else if (column >= 3 && column <= 5 && row >= 7 && row <= 9) {
            square_color = 0;
        } else if (row <= 4) {
            square_color = 0;
        } else {
            square_color = 1;
        }
    } else {
        square_color = ((column + row) % 2) == 1;
    }

    /* [hgm] holdings: next line makes all holdings squares light */
    if(column < BOARD_LEFT || column >= BOARD_RGHT) square_color = 1;

    return square_color;
}

void DrawSquare(row, column, piece, do_flash)
     int row, column, do_flash;
     ChessSquare piece;
{
    int square_color, x, y;
    int i;
    char string[2];
    int flash_delay;

    /* Calculate delay in milliseconds (2-delays per complete flash) */
    flash_delay = 500 / appData.flashRate;

    /* calculate x and y coordinates from row and column */
    if (flipView)
      {
	x = lineGap + ((BOARD_WIDTH-1)-column) *
	  (squareSize + lineGap);
	y = lineGap + row * (squareSize + lineGap);
      }
    else
      {
	x = lineGap + column * (squareSize + lineGap);
	y = lineGap + ((BOARD_HEIGHT-1)-row) *
	  (squareSize + lineGap);
      }

    square_color = SquareColor(row, column);

    // [HGM] holdings: blank out area between board and holdings
    if ( column == BOARD_LEFT-1 ||  column == BOARD_RGHT
	 || (column == BOARD_LEFT-2 && row < BOARD_HEIGHT-gameInfo.holdingsSize)
	 || (column == BOARD_RGHT+1 && row >= gameInfo.holdingsSize) )
      {
	BlankSquare(x, y, 2, EmptySquare, xBoardWindow);

	// [HGM] print piece counts next to holdings
	string[1] = NULLCHAR;
	if(piece > 1)
	  {
	    cairo_text_extents_t extents;
	    cairo_t *cr;
	    int  xpos, ypos;

	    /* get a cairo_t */
	    cr = gdk_cairo_create (GDK_WINDOW(GUI_Board->window));

	    string[0] = '0' + piece;

	    /* TODO this has to go into the font-selection */
	    cairo_select_font_face (cr, "Sans",
				    CAIRO_FONT_SLANT_NORMAL,
				    CAIRO_FONT_WEIGHT_NORMAL);

	    cairo_set_font_size (cr, 12.0);
	    cairo_text_extents (cr, string, &extents);

	    if (column == (flipView ? BOARD_LEFT-1 : BOARD_RGHT) )
	      {
		xpos= x + squareSize - extents.width - 2;
		ypos= y + extents.y_bearing + 1;
	      }
	    if (column == (flipView ? BOARD_RGHT : BOARD_LEFT-1) && piece > 1)
	      {
		xpos= x + 2;
		ypos = y + extents.y_bearing + 1;
	      }

	    /* TODO mono mode? */
	    cairo_move_to (cr, xpos, ypos);
	    cairo_text_path (cr, string);
	    cairo_set_source_rgb (cr, 1.0, 1.0, 1);
	    cairo_fill_preserve (cr);
	    cairo_set_source_rgb (cr, 0, 0, 0);
	    cairo_set_line_width (cr, 0.1);
	    cairo_stroke (cr);

	    /* free memory */
	    cairo_destroy (cr);
	  }
      }
    else
      {
	/* square on the board */
	if (piece == EmptySquare || appData.blindfold)
	  {
	    BlankSquare(x, y, square_color, piece, xBoardWindow);
	  }
	else
	  {
	    if (do_flash && appData.flashCount > 0)
	      {
		for (i=0; i<appData.flashCount; ++i)
		  {

		    DrawPiece(piece, square_color, x, y, xBoardWindow);
		    do_flash_delay(flash_delay);

		    BlankSquare(x, y, square_color, piece, xBoardWindow);
		    do_flash_delay(flash_delay);
		  }
	      }
	    DrawPiece(piece, square_color, x, y, xBoardWindow);
	  }
      }

    /* show coordinates if necessary */
    if(appData.showCoords)
      {
	cairo_text_extents_t extents;
	cairo_t *cr;
	int  xpos, ypos;

	/* TODO this has to go into the font-selection */
	cairo_select_font_face (cr, "Sans",
				CAIRO_FONT_SLANT_NORMAL,
				CAIRO_FONT_WEIGHT_NORMAL);
	cairo_set_font_size (cr, 12.0);

	string[1] = NULLCHAR;

	/* get a cairo_t */
	cr = gdk_cairo_create (GDK_WINDOW(GUI_Board->window));

	if (row == (flipView ? BOARD_HEIGHT-1 : 0) &&
	    column >= BOARD_LEFT && column < BOARD_RGHT)
	  {
	    string[0] = 'a' + column - BOARD_LEFT;
	    cairo_text_extents (cr, string, &extents);

	    xpos = x + squareSize - extents.width - 2;
	    ypos = y + squareSize - extents.height - extents.y_bearing - 1;

	    if (appData.monoMode)
	      { /*TODO*/
	      }
	    else
	      {
	      }

	    cairo_move_to (cr, xpos, ypos);
	    cairo_text_path (cr, string);
	    cairo_set_source_rgb (cr, 0.0, 0.0, 0);
	    cairo_fill_preserve (cr);
	    cairo_set_source_rgb (cr, 0, 1.0, 0);
	    cairo_set_line_width (cr, 0.1);
	    cairo_stroke (cr);
	  }
	if ( column == (flipView ? BOARD_RGHT-1 : BOARD_LEFT))
	  {

	    string[0] = ONE + row;
	    cairo_text_extents (cr, string, &extents);

	    xpos = x + 2;
	    ypos = y + extents.height + 1;

	    if (appData.monoMode)
	      { /*TODO*/
	      }
	    else
	      {
	      }

	    cairo_move_to (cr, xpos, ypos);
	    cairo_text_path (cr, string);
	    cairo_set_source_rgb (cr, 0.0, 0.0, 0.0);
	    cairo_fill_preserve (cr);
	    cairo_set_source_rgb (cr, 0, 0, 1.0);
	    cairo_set_line_width (cr, 0.1);
	    cairo_stroke (cr);

	  }
	/* free memory */
	cairo_destroy (cr);
      }

    return;
}


/* Returns 1 if there are "too many" differences between b1 and b2
   (i.e. more than 1 move was made) */
static int too_many_diffs(b1, b2)
     Board b1, b2;
{
    int i, j;
    int c = 0;

    for (i=0; i<BOARD_HEIGHT; ++i) {
	for (j=0; j<BOARD_WIDTH; ++j) {
	    if (b1[i][j] != b2[i][j]) {
		if (++c > 4)	/* Castling causes 4 diffs */
		  return 1;
	    }
	}
    }

    return 0;
}

/* Matrix describing castling maneuvers */
/* Row, ColRookFrom, ColKingFrom, ColRookTo, ColKingTo */
static int castling_matrix[4][5] = {
    { 0, 0, 4, 3, 2 },		/* 0-0-0, white */
    { 0, 7, 4, 5, 6 },		/* 0-0,   white */
    { 7, 0, 4, 3, 2 },		/* 0-0-0, black */
    { 7, 7, 4, 5, 6 }		/* 0-0,   black */
};

/* Checks whether castling occurred. If it did, *rrow and *rcol
   are set to the destination (row,col) of the rook that moved.

   Returns 1 if castling occurred, 0 if not.

   Note: Only handles a max of 1 castling move, so be sure
   to call too_many_diffs() first.
   */
static int check_castle_draw(newb, oldb, rrow, rcol)
     Board newb, oldb;
     int *rrow, *rcol;
{
    int i, *r, j;
    int match;

    /* For each type of castling... */
    for (i=0; i<4; ++i) {
	r = castling_matrix[i];

	/* Check the 4 squares involved in the castling move */
	match = 0;
	for (j=1; j<=4; ++j) {
	    if (newb[r[0]][r[j]] == oldb[r[0]][r[j]]) {
		match = 1;
		break;
	    }
	}

	if (!match) {
	    /* All 4 changed, so it must be a castling move */
	    *rrow = r[0];
	    *rcol = r[3];
	    return 1;
	}
    }
    return 0;
}

static int damage[BOARD_SIZE][BOARD_SIZE];

/*
 * event handler for redrawing the board
 */
void DrawPosition( repaint, board)
     /*Boolean*/int repaint;
		Board board;
{
  int i, j, do_flash;
  static int lastFlipView = 0;
  static int lastBoardValid = 0;
  static Board lastBoard;
  int rrow, rcol;

  if (board == NULL) {
    if (!lastBoardValid) return;
    board = lastBoard;
  }
  if (!lastBoardValid || lastFlipView != flipView) {
    //    XtSetArg(args[0], XtNleftBitmap, (flipView ? xMarkPixmap : None));
    // XtSetValues(XtNameToWidget(menuBarWidget, "menuOptions.Flip View"),
    //	args, 1);
  }

  /*
   * It would be simpler to clear the window with XClearWindow()
   * but this causes a very distracting flicker.
   */

  if (!repaint && lastBoardValid && lastFlipView == flipView)
    {
      /* If too much changes (begin observing new game, etc.), don't
	 do flashing */
      do_flash = too_many_diffs(board, lastBoard) ? 0 : 1;

      /* Special check for castling so we don't flash both the king
	 and the rook (just flash the king). */
      if (do_flash)
	{
	  if (check_castle_draw(board, lastBoard, &rrow, &rcol))
	    {
	      /* Draw rook with NO flashing. King will be drawn flashing later */
	      DrawSquare(rrow, rcol, board[rrow][rcol], 0);
	      lastBoard[rrow][rcol] = board[rrow][rcol];
	    }
	}

      /* First pass -- Draw (newly) empty squares and repair damage.
	 This prevents you from having a piece show up twice while it
	 is flashing on its new square */
      for (i = 0; i < BOARD_HEIGHT; i++)
	for (j = 0; j < BOARD_WIDTH; j++)
	  if ((board[i][j] != lastBoard[i][j] && board[i][j] == EmptySquare)
	      || damage[i][j])
	    {
	      DrawSquare(i, j, board[i][j], 0);
	      damage[i][j] = False;
	    }

      /* Second pass -- Draw piece(s) in new position and flash them */
      for (i = 0; i < BOARD_HEIGHT; i++)
	for (j = 0; j < BOARD_WIDTH; j++)
	  if (board[i][j] != lastBoard[i][j])
	    {
	      DrawSquare(i, j, board[i][j], do_flash);
	    }
    }
  else
    {
      /* redraw Grid */
      if (lineGap > 0)
	{
	  int x1,x2,y1,y2;
	  cairo_t *cr;

	  /* get a cairo_t */
	  cr = gdk_cairo_create (GDK_WINDOW(GUI_Board->window));

	  cairo_set_line_width (cr, lineGap);

	  /* TODO: use appdata colors */
	  cairo_set_source_rgba (cr, 0, 1, 0, 1.0);

	  cairo_stroke (cr);

	  for (i = 0; i < BOARD_HEIGHT + 1; i++)
	    {
	      x1 = 0;
	      x2 = lineGap + BOARD_WIDTH * (squareSize + lineGap);
	      y1 = y2 = lineGap / 2 + (i * (squareSize + lineGap));

	      cairo_move_to (cr, x1, y1);
	      cairo_rel_line_to (cr, x2,0);
	      cairo_stroke (cr);
	    }

	  for (j = 0; j < BOARD_WIDTH + 1; j++)
	    {
	      y1 = 0;
	      y2 = lineGap + BOARD_HEIGHT * (squareSize + lineGap);
	      x1 = x2  = lineGap / 2 + (j * (squareSize + lineGap));

	      cairo_move_to (cr, x1, y1);
	      cairo_rel_line_to (cr, 0, y2);
	      cairo_stroke (cr);
	    }

	  /* free memory */
	  cairo_destroy (cr);
	}

      /* draw pieces */
      for (i = 0; i < BOARD_HEIGHT; i++)
	for (j = 0; j < BOARD_WIDTH; j++)
	  {
	    DrawSquare(i, j, board[i][j], 0);
	    damage[i][j] = False;
	  }
    }

  CopyBoard(lastBoard, board);
  lastBoardValid = 1;
  lastFlipView = flipView;

  /* Draw highlights */
  if (pm1X >= 0 && pm1Y >= 0)
    {
      drawHighlight(pm1X, pm1Y, LINE_TYPE_PRE);
    }
  if (pm2X >= 0 && pm2Y >= 0)
    {
      drawHighlight(pm2X, pm2Y, LINE_TYPE_PRE);
    }
  if (hi1X >= 0 && hi1Y >= 0)
    {
      drawHighlight(hi1X, hi1Y, LINE_TYPE_HIGHLIGHT);
    }
  if (hi2X >= 0 && hi2Y >= 0)
    {
      drawHighlight(hi2X, hi2Y, LINE_TYPE_HIGHLIGHT);
    }

  /* If piece being dragged around board, must redraw that too */
  DrawDragPiece();

  return;
}

/*
 * event handler for parsing user moves
 */
// [HGM] This routine will need quite some reworking. Although the backend still supports the old
//       way of doing things, by calling UserMoveEvent() to test the legality of the move and then perform
//       it at the end, and doing all kind of preliminary tests here (e.g. to weed out self-captures), it
//       should be made to use the new way, of calling UserMoveTest early  to determine the legality of the
//       move, (which will weed out the illegal selfcaptures and moves into the holdings, and flag promotions),
//       and at the end FinishMove() to perform the move after optional promotion popups.
//       For now I patched it to allow self-capture with King, and suppress clicks between board and holdings.
void HandleUserMove(w, event, prms, nprms)
     Widget w;
     XEvent *event;
     String *prms;
     Cardinal *nprms;
{
    int x, y;
    Boolean saveAnimate;
    static int second = 0, promotionChoice = 0;
    ChessMove moveType;

    if (w != boardWidget || errorExitStatus != -1) return;

    x = EventToSquare(event->xbutton.x, BOARD_WIDTH);
    y = EventToSquare(event->xbutton.y, BOARD_HEIGHT);
    if (!flipView && y >= 0) {
	y = BOARD_HEIGHT - 1 - y;
    }
    if (flipView && x >= 0) {
	x = BOARD_WIDTH - 1 - x;
    }

    if(promotionChoice) { // we are waiting for a click to indicate promotion piece
	if(event->type == ButtonRelease) return; // ignore upclick of click-click destination
	promotionChoice = FALSE; // only one chance: if click not OK it is interpreted as cancel
	if(appData.debugMode) fprintf(debugFP, "promotion click, x=%d, y=%d\n", x, y);
	if(gameInfo.holdingsWidth && 
		(WhiteOnMove(currentMove) 
			? x == BOARD_WIDTH-1 && y < gameInfo.holdingsSize && y > 0
			: x == 0 && y >= BOARD_HEIGHT - gameInfo.holdingsSize && y < BOARD_HEIGHT-1) ) {
	    // click in right holdings, for determining promotion piece
	    ChessSquare p = boards[currentMove][y][x];
	    if(appData.debugMode) fprintf(debugFP, "square contains %d\n", (int)p);
	    if(p != EmptySquare) {
		FinishMove(NormalMove, fromX, fromY, toX, toY, ToLower(PieceToChar(p)));
		fromX = fromY = -1;
		return;
	    }
	}
	DrawPosition(FALSE, boards[currentMove]);
	return;
    }
    if (event->type == ButtonPress) ErrorPopDown();

    if (promotionUp) {
	if (event->type == ButtonPress) {
//	    XtPopdown(promotionShell);
//	    XtDestroyWidget(promotionShell);
	    promotionUp = False;
	    ClearHighlights();
	    fromX = fromY = -1;
	} else {
	    return;
	}
    }

    /* [HGM] holdings: next 5 lines: ignore all clicks between board and holdings */
    if(event->type == ButtonPress
            && ( x == BOARD_LEFT-1 || 
		 x == BOARD_RGHT   || 
		 (x == BOARD_LEFT-2 && y < BOARD_HEIGHT-gameInfo.holdingsSize ) || 
		 (x == BOARD_RGHT+1 && y >= gameInfo.holdingsSize)) )
	return;

    if (fromX == -1) {
	if (event->type == ButtonPress) {
	    /* First square, prepare to drag */
	    if (OKToStartUserMove(x, y)) {
		fromX = x;
		fromY = y;
		second = 0;
		DragPieceBegin(event->xbutton.x, event->xbutton.y);
		if (appData.highlightDragging) {
		    SetHighlights(x, y, -1, -1);
		}
	    }
	}
	return;
    }

    /* fromX != -1 */
    if (event->type == ButtonRelease &&	x == fromX && y == fromY) {
    /* Click on single square in stead of drag-drop */
	DragPieceEnd(event->xbutton.x, event->xbutton.y);
	if (appData.animateDragging) {
	    /* Undo animation damage if any */
	    DrawPosition(FALSE, NULL);
	}
	if (second) {
	    /* Second up/down in same square; just abort move */
	    second = 0;
	    fromX = fromY = -1;
	    ClearHighlights();
	    gotPremove = 0;
	    ClearPremoveHighlights();
	} else {
	    /* First upclick in same square; start click-click mode */
	    SetHighlights(x, y, -1, -1);
	}
	return;
    }

    moveType = UserMoveTest(fromX, fromY, x, y, NULLCHAR, event->type == ButtonRelease);

    if (moveType == Comment) { // kludge for indicating capture-own on Press
      /* Clicked again on same color piece -- changed his mind */
      /* note that re-clicking same square always hits same color piece */
      second = (x == fromX && y == fromY);
      if (appData.highlightDragging) {
	SetHighlights(x, y, -1, -1);
      } else {
	ClearHighlights();
      }
      if (OKToStartUserMove(x, y)) {
	fromX = x;
	fromY = y;
	DragPieceBegin(event->xbutton.x, event->xbutton.y);
      }
      return;
    }

    if(moveType == AmbiguousMove) { // kludge to indicate edit-position move
      fromX = fromY = -1; 
      ClearHighlights();
      DragPieceEnd(event->xbutton.x, event->xbutton.y);
      DrawPosition(FALSE, boards[currentMove]);
      return;
    }

    /* Complete move; (x,y) is now different from (fromX, fromY) on both Press and Release */
    toX = x;
    toY = y;
    saveAnimate = appData.animate;
    if (event->type == ButtonPress) {
	/* Finish clickclick move */
	if (appData.animate || appData.highlightLastMove) {
	    SetHighlights(fromX, fromY, toX, toY);
	} else {
	    ClearHighlights();
	}
    } else {
	/* Finish drag move */
	if (appData.highlightLastMove) {
	    SetHighlights(fromX, fromY, toX, toY);
	} else {
	    ClearHighlights();
	}
	DragPieceEnd(event->xbutton.x, event->xbutton.y);
	/* Don't animate move and drag both */
	appData.animate = FALSE;
    }
    if (moveType == WhitePromotionKnight || moveType == BlackPromotionKnight ||
        (moveType == WhitePromotionQueen || moveType == BlackPromotionQueen) &&
            appData.alwaysPromoteToQueen) { // promotion, but no choice
      FinishMove(moveType, fromX, fromY, toX, toY, 'q');
    } else
    if (moveType == WhitePromotionQueen || moveType == BlackPromotionQueen ) {
	SetHighlights(fromX, fromY, toX, toY);
	if(gameInfo.variant == VariantSuper || gameInfo.variant == VariantGreat) {
	    // [HGM] super: promotion to captured piece selected from holdings
	    ChessSquare p = boards[currentMove][fromY][fromX], q = boards[currentMove][toY][toX];
	    promotionChoice = TRUE;
	    // kludge follows to temporarily execute move on display, without promoting yet
	    boards[currentMove][fromY][fromX] = EmptySquare; // move Pawn to 8th rank
	    boards[currentMove][toY][toX] = p;
	    DrawPosition(FALSE, boards[currentMove]);
	    boards[currentMove][fromY][fromX] = p; // take back, but display stays
	    boards[currentMove][toY][toX] = q;
	    DisplayMessage("Click in holdings to choose piece", "");
	    return;
	}
	PromotionPopUp();
	goto skipClearingFrom; // the skipped stuff is done asynchronously by PromotionCallback
    } else
    if(moveType != ImpossibleMove) { // valid move, but no promotion
      FinishMove(moveType, fromX, fromY, toX, toY, NULLCHAR);
    } else { // invalid move; could have set premove
      ClearHighlights();
    }
	    if (!appData.highlightLastMove || gotPremove) ClearHighlights();
	    if (gotPremove) SetPremoveHighlights(fromX, fromY, toX, toY);
	    fromX = fromY = -1;
skipClearingFrom:
    appData.animate = saveAnimate;
    if (appData.animate || appData.animateDragging) {
	/* Undo animation damage if needed */
	DrawPosition(FALSE, NULL);
    }
}

void AnimateUserMove (Widget w, XEvent * event,
		      String * params, Cardinal * nParams)
{
    DragPieceMove(event->xmotion.x, event->xmotion.y);
}

Widget CommentCreate(name, text, mutable, callback, lines)
     char *name, *text;
     int /*Boolean*/ mutable;
     XtCallbackProc callback;
     int lines;
{
    Arg args[16];
    Widget shell, layout, form, edit, b_ok, b_cancel, b_clear, b_close, b_edit;
    Dimension bw_width;
    int j;

    j = 0;
    XtSetArg(args[j], XtNwidth, &bw_width);  j++;
    XtGetValues(boardWidget, args, j);

    j = 0;
    XtSetArg(args[j], XtNresizable, True);  j++;
#if TOPLEVEL
    shell =
      XtCreatePopupShell(name, topLevelShellWidgetClass,
			 shellWidget, args, j);
#else
    shell =
      XtCreatePopupShell(name, transientShellWidgetClass,
			 shellWidget, args, j);
#endif
    layout =
      XtCreateManagedWidget(layoutName, formWidgetClass, shell,
			    layoutArgs, XtNumber(layoutArgs));
    form =
      XtCreateManagedWidget("form", formWidgetClass, layout,
			    formArgs, XtNumber(formArgs));

    j = 0;
    if (mutable) {
	XtSetArg(args[j], XtNeditType, XawtextEdit);  j++;
	XtSetArg(args[j], XtNuseStringInPlace, False);  j++;
    }
    XtSetArg(args[j], XtNstring, text);  j++;
    XtSetArg(args[j], XtNtop, XtChainTop);  j++;
    XtSetArg(args[j], XtNbottom, XtChainBottom);  j++;
    XtSetArg(args[j], XtNleft, XtChainLeft);  j++;
    XtSetArg(args[j], XtNright, XtChainRight);  j++;
    XtSetArg(args[j], XtNresizable, True);  j++;
    XtSetArg(args[j], XtNwidth, bw_width);  j++; /*force wider than buttons*/
    /* !!Work around an apparent bug in XFree86 4.0.1 (X11R6.4.3) */
    XtSetArg(args[j], XtNscrollVertical, XawtextScrollAlways);  j++;
    XtSetArg(args[j], XtNautoFill, True);  j++;
    XtSetArg(args[j], XtNwrap, XawtextWrapWord); j++;
    edit =
      XtCreateManagedWidget("text", asciiTextWidgetClass, form, args, j);

    if (mutable) {
	j = 0;
	XtSetArg(args[j], XtNfromVert, edit);  j++;
	XtSetArg(args[j], XtNtop, XtChainBottom); j++;
	XtSetArg(args[j], XtNbottom, XtChainBottom); j++;
	XtSetArg(args[j], XtNleft, XtChainLeft); j++;
	XtSetArg(args[j], XtNright, XtChainLeft); j++;
	b_ok =
	  XtCreateManagedWidget(_("ok"), commandWidgetClass, form, args, j);
	XtAddCallback(b_ok, XtNcallback, callback, (XtPointer) 0);

	j = 0;
	XtSetArg(args[j], XtNfromVert, edit);  j++;
	XtSetArg(args[j], XtNfromHoriz, b_ok);  j++;
	XtSetArg(args[j], XtNtop, XtChainBottom); j++;
	XtSetArg(args[j], XtNbottom, XtChainBottom); j++;
	XtSetArg(args[j], XtNleft, XtChainLeft); j++;
	XtSetArg(args[j], XtNright, XtChainLeft); j++;
	b_cancel =
	  XtCreateManagedWidget(_("cancel"), commandWidgetClass, form, args, j);
	XtAddCallback(b_cancel, XtNcallback, callback, (XtPointer) 0);

	j = 0;
	XtSetArg(args[j], XtNfromVert, edit);  j++;
	XtSetArg(args[j], XtNfromHoriz, b_cancel);  j++;
	XtSetArg(args[j], XtNtop, XtChainBottom); j++;
	XtSetArg(args[j], XtNbottom, XtChainBottom); j++;
	XtSetArg(args[j], XtNleft, XtChainLeft); j++;
	XtSetArg(args[j], XtNright, XtChainLeft); j++;
	b_clear =
	  XtCreateManagedWidget(_("clear"), commandWidgetClass, form, args, j);
	XtAddCallback(b_clear, XtNcallback, callback, (XtPointer) 0);
    } else {
	j = 0;
	XtSetArg(args[j], XtNfromVert, edit);  j++;
	XtSetArg(args[j], XtNtop, XtChainBottom); j++;
	XtSetArg(args[j], XtNbottom, XtChainBottom); j++;
	XtSetArg(args[j], XtNleft, XtChainLeft); j++;
	XtSetArg(args[j], XtNright, XtChainLeft); j++;
	b_close =
	  XtCreateManagedWidget(_("close"), commandWidgetClass, form, args, j);
	XtAddCallback(b_close, XtNcallback, callback, (XtPointer) 0);

	j = 0;
	XtSetArg(args[j], XtNfromVert, edit);  j++;
	XtSetArg(args[j], XtNfromHoriz, b_close);  j++;
	XtSetArg(args[j], XtNtop, XtChainBottom); j++;
	XtSetArg(args[j], XtNbottom, XtChainBottom); j++;
	XtSetArg(args[j], XtNleft, XtChainLeft); j++;
	XtSetArg(args[j], XtNright, XtChainLeft); j++;
	b_edit =
	  XtCreateManagedWidget(_("edit"), commandWidgetClass, form, args, j);
	XtAddCallback(b_edit, XtNcallback, callback, (XtPointer) 0);
    }

    XtRealizeWidget(shell);

    if (commentX == -1) {
	int xx, yy;
	Window junk;
	Dimension pw_height;
	Dimension ew_height;

	j = 0;
	XtSetArg(args[j], XtNheight, &ew_height);  j++;
	XtGetValues(edit, args, j);

	j = 0;
	XtSetArg(args[j], XtNheight, &pw_height);  j++;
	XtGetValues(shell, args, j);
	commentH = pw_height + (lines - 1) * ew_height;
	commentW = bw_width - 16;

	XSync(xDisplay, False);
#ifdef NOTDEF
	/* This code seems to tickle an X bug if it is executed too soon
	   after xboard starts up.  The coordinates get transformed as if
	   the main window was positioned at (0, 0).
	   */
	XtTranslateCoords(shellWidget,
			  (bw_width - commentW) / 2, 0 - commentH / 2,
			  &commentX, &commentY);
#else  /*!NOTDEF*/
        XTranslateCoordinates(xDisplay, XtWindow(shellWidget),
			      RootWindowOfScreen(XtScreen(shellWidget)),
			      (bw_width - commentW) / 2, 0 - commentH / 2,
			      &xx, &yy, &junk);
	commentX = xx;
	commentY = yy;
#endif /*!NOTDEF*/
	if (commentY < 0) commentY = 0; /*avoid positioning top offscreen*/
    }
    j = 0;
    XtSetArg(args[j], XtNheight, commentH);  j++;
    XtSetArg(args[j], XtNwidth, commentW);  j++;
    XtSetArg(args[j], XtNx, commentX);  j++;
    XtSetArg(args[j], XtNy, commentY);  j++;
    XtSetValues(shell, args, j);
    XtSetKeyboardFocus(shell, edit);

    return shell;
}

/* Used for analysis window and ICS input window */
Widget MiscCreate(name, text, mutable, callback, lines)
     char *name, *text;
     int /*Boolean*/ mutable;
     XtCallbackProc callback;
     int lines;
{
    Arg args[16];
    Widget shell, layout, form, edit;
    Position x, y;
    Dimension bw_width, pw_height, ew_height, w, h;
    int j;
    int xx, yy;
    Window junk;

    j = 0;
    XtSetArg(args[j], XtNresizable, True);  j++;
#if TOPLEVEL
    shell =
      XtCreatePopupShell(name, topLevelShellWidgetClass,
			 shellWidget, args, j);
#else
    shell =
      XtCreatePopupShell(name, transientShellWidgetClass,
			 shellWidget, args, j);
#endif
    layout =
      XtCreateManagedWidget(layoutName, formWidgetClass, shell,
			    layoutArgs, XtNumber(layoutArgs));
    form =
      XtCreateManagedWidget("form", formWidgetClass, layout,
			    formArgs, XtNumber(formArgs));

    j = 0;
    if (mutable) {
	XtSetArg(args[j], XtNeditType, XawtextEdit);  j++;
	XtSetArg(args[j], XtNuseStringInPlace, False);  j++;
    }
    XtSetArg(args[j], XtNstring, text);  j++;
    XtSetArg(args[j], XtNtop, XtChainTop);  j++;
    XtSetArg(args[j], XtNbottom, XtChainBottom);  j++;
    XtSetArg(args[j], XtNleft, XtChainLeft);  j++;
    XtSetArg(args[j], XtNright, XtChainRight);  j++;
    XtSetArg(args[j], XtNresizable, True);  j++;
    /* !!Work around an apparent bug in XFree86 4.0.1 (X11R6.4.3) */
    XtSetArg(args[j], XtNscrollVertical, XawtextScrollAlways);  j++;
    XtSetArg(args[j], XtNautoFill, True);  j++;
    XtSetArg(args[j], XtNwrap, XawtextWrapWord); j++;
    edit =
      XtCreateManagedWidget("text", asciiTextWidgetClass, form, args, j);

    XtRealizeWidget(shell);

    j = 0;
    XtSetArg(args[j], XtNwidth, &bw_width);  j++;
    XtGetValues(boardWidget, args, j);

    j = 0;
    XtSetArg(args[j], XtNheight, &ew_height);  j++;
    XtGetValues(edit, args, j);

    j = 0;
    XtSetArg(args[j], XtNheight, &pw_height);  j++;
    XtGetValues(shell, args, j);
    h = pw_height + (lines - 1) * ew_height;
    w = bw_width - 16;

    XSync(xDisplay, False);
#ifdef NOTDEF
    /* This code seems to tickle an X bug if it is executed too soon
       after xboard starts up.  The coordinates get transformed as if
       the main window was positioned at (0, 0).
    */
    XtTranslateCoords(shellWidget, (bw_width - w) / 2, 0 - h / 2, &x, &y);
#else  /*!NOTDEF*/
    XTranslateCoordinates(xDisplay, XtWindow(shellWidget),
			  RootWindowOfScreen(XtScreen(shellWidget)),
			  (bw_width - w) / 2, 0 - h / 2, &xx, &yy, &junk);
#endif /*!NOTDEF*/
    x = xx;
    y = yy;
    if (y < 0) y = 0; /*avoid positioning top offscreen*/

    j = 0;
    XtSetArg(args[j], XtNheight, h);  j++;
    XtSetArg(args[j], XtNwidth, w);  j++;
    XtSetArg(args[j], XtNx, x);  j++;
    XtSetArg(args[j], XtNy, y);  j++;
    XtSetValues(shell, args, j);

    return shell;
}


static int savedIndex;  /* gross that this is global */

void EditCommentPopUp(index, title, text)
     int index;
     char *title, *text;
{
    Widget edit;
    Arg args[16];
    int j;

    savedIndex = index;
    if (text == NULL) text = "";

    if (editShell == NULL) {
	editShell =
	  CommentCreate(title, text, True, EditCommentCallback, 4);
	XtRealizeWidget(editShell);
	CatchDeleteWindow(editShell, "EditCommentPopDown");
    } else {
	edit = XtNameToWidget(editShell, "*form.text");
	j = 0;
	XtSetArg(args[j], XtNstring, text); j++;
	XtSetValues(edit, args, j);
	j = 0;
	XtSetArg(args[j], XtNiconName, (XtArgVal) title);   j++;
	XtSetArg(args[j], XtNtitle, (XtArgVal) title);      j++;
	XtSetValues(editShell, args, j);
    }

    XtPopup(editShell, XtGrabNone);

    editUp = True;
    j = 0;
    XtSetArg(args[j], XtNleftBitmap, xMarkPixmap); j++;
    XtSetValues(XtNameToWidget(menuBarWidget, "menuMode.Edit Comment"),
		args, j);
}

void EditCommentCallback(w, client_data, call_data)
     Widget w;
     XtPointer client_data, call_data;
{
    String name, val;
    Arg args[16];
    int j;
    Widget edit;

    j = 0;
    XtSetArg(args[j], XtNlabel, &name);  j++;
    XtGetValues(w, args, j);

    if (strcmp(name, _("ok")) == 0) {
	edit = XtNameToWidget(editShell, "*form.text");
	j = 0;
	XtSetArg(args[j], XtNstring, &val); j++;
	XtGetValues(edit, args, j);
	ReplaceComment(savedIndex, val);
	EditCommentPopDown();
    } else if (strcmp(name, _("cancel")) == 0) {
	EditCommentPopDown();
    } else if (strcmp(name, _("clear")) == 0) {
	edit = XtNameToWidget(editShell, "*form.text");
	XtCallActionProc(edit, "select-all", NULL, NULL, 0);
	XtCallActionProc(edit, "kill-selection", NULL, NULL, 0);
    }
}

void EditCommentPopDown()
{
    Arg args[16];
    int j;

    if (!editUp) return;
    j = 0;
    XtSetArg(args[j], XtNx, &commentX); j++;
    XtSetArg(args[j], XtNy, &commentY); j++;
    XtSetArg(args[j], XtNheight, &commentH); j++;
    XtSetArg(args[j], XtNwidth, &commentW); j++;
    XtGetValues(editShell, args, j);
    XtPopdown(editShell);
    editUp = False;
    j = 0;
    XtSetArg(args[j], XtNleftBitmap, None); j++;
    XtSetValues(XtNameToWidget(menuBarWidget, "menuMode.Edit Comment"),
		args, j);
}

void ICSInputBoxPopUp()
{
    Widget edit;
    Arg args[16];
    int j;
    char *title = _("ICS Input");
    XtTranslations tr;

    if (ICSInputShell == NULL) {
	ICSInputShell = MiscCreate(title, "", True, NULL, 1);
	tr = XtParseTranslationTable(ICSInputTranslations);
	edit = XtNameToWidget(ICSInputShell, "*form.text");
	XtOverrideTranslations(edit, tr);
	XtRealizeWidget(ICSInputShell);
	CatchDeleteWindow(ICSInputShell, "ICSInputBoxPopDown");

    } else {
	edit = XtNameToWidget(ICSInputShell, "*form.text");
	j = 0;
	XtSetArg(args[j], XtNstring, ""); j++;
	XtSetValues(edit, args, j);
	j = 0;
	XtSetArg(args[j], XtNiconName, (XtArgVal) title);   j++;
	XtSetArg(args[j], XtNtitle, (XtArgVal) title);      j++;
	XtSetValues(ICSInputShell, args, j);
    }

    XtPopup(ICSInputShell, XtGrabNone);
    XtSetKeyboardFocus(ICSInputShell, edit);

    ICSInputBoxUp = True;
    j = 0;
    XtSetArg(args[j], XtNleftBitmap, xMarkPixmap); j++;
    XtSetValues(XtNameToWidget(menuBarWidget, "menuMode.ICS Input Box"),
		args, j);
}

void ICSInputSendText()
{
    Widget edit;
    int j;
    Arg args[16];
    String val;

    edit = XtNameToWidget(ICSInputShell, "*form.text");
    j = 0;
    XtSetArg(args[j], XtNstring, &val); j++;
    XtGetValues(edit, args, j);
    SendMultiLineToICS(val);
    XtCallActionProc(edit, "select-all", NULL, NULL, 0);
    XtCallActionProc(edit, "kill-selection", NULL, NULL, 0);
}

void ICSInputBoxPopDown()
{
    Arg args[16];
    int j;

    if (!ICSInputBoxUp) return;
    j = 0;
    XtPopdown(ICSInputShell);
    ICSInputBoxUp = False;
    j = 0;
    XtSetArg(args[j], XtNleftBitmap, None); j++;
    XtSetValues(XtNameToWidget(menuBarWidget, "menuMode.ICS Input Box"),
		args, j);
}

void CommentPopUp(title, text)
     char *title, *text;
{
    Arg args[16];
    int j;
    Widget edit;

    if (commentShell == NULL) {
	commentShell =
	  CommentCreate(title, text, False, CommentCallback, 4);
	XtRealizeWidget(commentShell);
	CatchDeleteWindow(commentShell, "CommentPopDown");
    } else {
	edit = XtNameToWidget(commentShell, "*form.text");
	j = 0;
	XtSetArg(args[j], XtNstring, text); j++;
	XtSetValues(edit, args, j);
	j = 0;
	XtSetArg(args[j], XtNiconName, (XtArgVal) title);   j++;
	XtSetArg(args[j], XtNtitle, (XtArgVal) title);      j++;
	XtSetValues(commentShell, args, j);
    }

    XtPopup(commentShell, XtGrabNone);
    XSync(xDisplay, False);

    commentUp = True;
}

void AnalysisPopUp(title, text)
     char *title, *text;
{
    Arg args[16];
    int j;
    Widget edit;

    if (analysisShell == NULL) {
	analysisShell = MiscCreate(title, text, False, NULL, 4);
	XtRealizeWidget(analysisShell);
	CatchDeleteWindow(analysisShell, "AnalysisPopDown");

    } else {
	edit = XtNameToWidget(analysisShell, "*form.text");
	j = 0;
	XtSetArg(args[j], XtNstring, text); j++;
	XtSetValues(edit, args, j);
	j = 0;
	XtSetArg(args[j], XtNiconName, (XtArgVal) title);   j++;
	XtSetArg(args[j], XtNtitle, (XtArgVal) title);      j++;
	XtSetValues(analysisShell, args, j);
    }

    if (!analysisUp) {
	XtPopup(analysisShell, XtGrabNone);
    }
    XSync(xDisplay, False);

    analysisUp = True;
}

void CommentCallback(w, client_data, call_data)
     Widget w;
     XtPointer client_data, call_data;
{
    String name;
    Arg args[16];
    int j;

    j = 0;
    XtSetArg(args[j], XtNlabel, &name);  j++;
    XtGetValues(w, args, j);

    if (strcmp(name, _("close")) == 0) {
	CommentPopDown();
    } else if (strcmp(name, _("edit")) == 0) {
	CommentPopDown();
	EditCommentEvent();
    }
}


void CommentPopDown()
{
    Arg args[16];
    int j;

    if (!commentUp) return;
    j = 0;
    XtSetArg(args[j], XtNx, &commentX); j++;
    XtSetArg(args[j], XtNy, &commentY); j++;
    XtSetArg(args[j], XtNwidth, &commentW); j++;
    XtSetArg(args[j], XtNheight, &commentH); j++;
    XtGetValues(commentShell, args, j);
    XtPopdown(commentShell);
    XSync(xDisplay, False);
    commentUp = False;
}

void AnalysisPopDown()
{
    if (!analysisUp) return;
    XtPopdown(analysisShell);
    XSync(xDisplay, False);
    analysisUp = False;
    if (appData.icsEngineAnalyze) ExitAnalyzeMode();    /* [DM] icsEngineAnalyze */
}


void FileNamePopUp(label, def, proc, openMode)
     char *label;
     char *def;
     FileProc proc;
     char *openMode;
{
    Arg args[16];
    Widget popup, layout, dialog, edit;
    Window root, child;
    int x, y, i;
    int win_x, win_y;
    unsigned int mask;

    fileProc = proc;		/* I can't see a way not */
    fileOpenMode = openMode;	/*   to use globals here */

    i = 0;
    XtSetArg(args[i], XtNresizable, True); i++;
    XtSetArg(args[i], XtNwidth, DIALOG_SIZE); i++;
    XtSetArg(args[i], XtNtitle, XtNewString(_("File name prompt"))); i++;
    fileNameShell = popup =
      XtCreatePopupShell("File name prompt", transientShellWidgetClass,
			 shellWidget, args, i);

    layout =
      XtCreateManagedWidget(layoutName, formWidgetClass, popup,
			    layoutArgs, XtNumber(layoutArgs));

    i = 0;
    XtSetArg(args[i], XtNlabel, label); i++;
    XtSetArg(args[i], XtNvalue, def); i++;
    XtSetArg(args[i], XtNborderWidth, 0); i++;
    dialog = XtCreateManagedWidget("fileName", dialogWidgetClass,
				   layout, args, i);

    XawDialogAddButton(dialog, _("ok"), FileNameCallback, (XtPointer) dialog);
    XawDialogAddButton(dialog, _("cancel"), FileNameCallback,
		       (XtPointer) dialog);

    XtRealizeWidget(popup);
    CatchDeleteWindow(popup, "FileNamePopDown");

    XQueryPointer(xDisplay, xBoardWindow, &root, &child,
		  &x, &y, &win_x, &win_y, &mask);

    XtSetArg(args[0], XtNx, x - 10);
    XtSetArg(args[1], XtNy, y - 30);
    XtSetValues(popup, args, 2);

    XtPopup(popup, XtGrabExclusive);
    filenameUp = True;

    edit = XtNameToWidget(dialog, "*value");
    XtSetKeyboardFocus(popup, edit);
}

void FileNamePopDown()
{
    if (!filenameUp) return;
    XtPopdown(fileNameShell);
    XtDestroyWidget(fileNameShell);
    filenameUp = False;
    ModeHighlight();
}

void FileNameCallback(w, client_data, call_data)
     Widget w;
     XtPointer client_data, call_data;
{
    String name;
    Arg args[16];

    XtSetArg(args[0], XtNlabel, &name);
    XtGetValues(w, args, 1);

    if (strcmp(name, _("cancel")) == 0) {
        FileNamePopDown();
        return;
    }

    FileNameAction(w, NULL, NULL, NULL);
}

void FileNameAction(w, event, prms, nprms)
     Widget w;
     XEvent *event;
     String *prms;
     Cardinal *nprms;
{
    char buf[MSG_SIZ];
    String name;
    FILE *f;
    char *p, *fullname;
    int index;

    name = XawDialogGetValueString(w = XtParent(w));

    if ((name != NULL) && (*name != NULLCHAR)) {
	strcpy(buf, name);
	XtPopdown(w = XtParent(XtParent(w)));
	XtDestroyWidget(w);
	filenameUp = False;

	p = strrchr(buf, ' ');
	if (p == NULL) {
	    index = 0;
	} else {
	    *p++ = NULLCHAR;
	    index = atoi(p);
	}
	fullname = ExpandPathName(buf);
	if (!fullname) {
	    ErrorPopUp(_("Error"), _("Can't open file"), FALSE);
	}
	else {
	    f = fopen(fullname, fileOpenMode);
	    if (f == NULL) {
		DisplayError(_("Failed to open file"), errno);
	    } else {
		(void) (*fileProc)(f, index, buf);
	    }
	}
	ModeHighlight();
	return;
    }

    XtPopdown(w = XtParent(XtParent(w)));
    XtDestroyWidget(w);
    filenameUp = False;
    ModeHighlight();
}

void PromotionPopUp()
{
    Arg args[16];
    Widget dialog, layout;
    Position x, y;
    Dimension bw_width, pw_width;
    int j;

    j = 0;
    XtSetArg(args[j], XtNwidth, &bw_width); j++;
    XtGetValues(boardWidget, args, j);

    j = 0;
    XtSetArg(args[j], XtNresizable, True); j++;
    XtSetArg(args[j], XtNtitle, XtNewString(_("Promotion"))); j++;
    promotionShell =
      XtCreatePopupShell("Promotion", transientShellWidgetClass,
			 shellWidget, args, j);
    layout =
      XtCreateManagedWidget(layoutName, formWidgetClass, promotionShell,
			    layoutArgs, XtNumber(layoutArgs));

    j = 0;
    XtSetArg(args[j], XtNlabel, _("Promote to what?")); j++;
    XtSetArg(args[j], XtNborderWidth, 0); j++;
    dialog = XtCreateManagedWidget("promotion", dialogWidgetClass,
				   layout, args, j);

  if(gameInfo.variant != VariantShogi) {
    XawDialogAddButton(dialog, _("Queen"), PromotionCallback,
		       (XtPointer) dialog);
    XawDialogAddButton(dialog, _("Rook"), PromotionCallback,
		       (XtPointer) dialog);
    XawDialogAddButton(dialog, _("Bishop"), PromotionCallback,
		       (XtPointer) dialog);
    XawDialogAddButton(dialog, _("Knight"), PromotionCallback,
		       (XtPointer) dialog);
    if (!appData.testLegality || gameInfo.variant == VariantSuicide ||
        gameInfo.variant == VariantGiveaway) {
      XawDialogAddButton(dialog, _("King"), PromotionCallback,
			 (XtPointer) dialog);
    }
    if(gameInfo.variant == VariantCapablanca ||
       gameInfo.variant == VariantGothic ||
       gameInfo.variant == VariantCapaRandom) {
      XawDialogAddButton(dialog, _("Archbishop"), PromotionCallback,
			 (XtPointer) dialog);
      XawDialogAddButton(dialog, _("Chancellor"), PromotionCallback,
			 (XtPointer) dialog);
    }
  } else // [HGM] shogi
  {
      XawDialogAddButton(dialog, _("Promote"), PromotionCallback,
			 (XtPointer) dialog);
      XawDialogAddButton(dialog, _("Defer"), PromotionCallback,
			 (XtPointer) dialog);
  }
    XawDialogAddButton(dialog, _("cancel"), PromotionCallback,
		       (XtPointer) dialog);

    XtRealizeWidget(promotionShell);
    CatchDeleteWindow(promotionShell, "PromotionPopDown");

    j = 0;
    XtSetArg(args[j], XtNwidth, &pw_width); j++;
    XtGetValues(promotionShell, args, j);

    XtTranslateCoords(boardWidget, (bw_width - pw_width) / 2,
		      lineGap + squareSize/3 +
		      ((toY == BOARD_HEIGHT-1) ^ (flipView) ?
		       0 : 6*(squareSize + lineGap)), &x, &y);

    j = 0;
    XtSetArg(args[j], XtNx, x); j++;
    XtSetArg(args[j], XtNy, y); j++;
    XtSetValues(promotionShell, args, j);

    XtPopup(promotionShell, XtGrabNone);

    promotionUp = True;
}

void PromotionPopDown()
{
    if (!promotionUp) return;
    XtPopdown(promotionShell);
    XtDestroyWidget(promotionShell);
    promotionUp = False;
}

void PromotionCallback(w, client_data, call_data)
     Widget w;
     XtPointer client_data, call_data;
{
    String name;
    Arg args[16];
    int promoChar;

    XtSetArg(args[0], XtNlabel, &name);
    XtGetValues(w, args, 1);

    PromotionPopDown();

    if (fromX == -1) return;

    if (strcmp(name, _("cancel")) == 0) {
	fromX = fromY = -1;
	ClearHighlights();
	return;
    } else if (strcmp(name, _("Knight")) == 0) {
	promoChar = 'n';
    } else if (strcmp(name, _("Promote")) == 0) {
	promoChar = '+';
    } else if (strcmp(name, _("Defer")) == 0) {
	promoChar = '=';
    } else {
	promoChar = ToLower(name[0]);
    }

    FinishMove(NormalMove, fromX, fromY, toX, toY, promoChar);

    if (!appData.highlightLastMove || gotPremove) ClearHighlights();
    if (gotPremove) SetPremoveHighlights(fromX, fromY, toX, toY);
    fromX = fromY = -1;
}


void ErrorCallback(w, client_data, call_data)
     Widget w;
     XtPointer client_data, call_data;
{
    errorUp = False;
    XtPopdown(w = XtParent(XtParent(XtParent(w))));
    XtDestroyWidget(w);
    if (errorExitStatus != -1) ExitEvent(errorExitStatus);
}


void ErrorPopDown()
{
    if (!errorUp) return;
    errorUp = False;

    if(GUI_Error)
      gtk_widget_destroy(GTK_WIDGET(GUI_Error));

    if (errorExitStatus != -1) ExitEvent(errorExitStatus);

    return;
}

void ErrorPopUp(title, label, modal)
     char *title, *label;
     int modal;
{
  GUI_Error = gtk_message_dialog_new(GTK_WINDOW(GUI_Window),
                                  GTK_DIALOG_DESTROY_WITH_PARENT,
                                  GTK_MESSAGE_ERROR,
                                  GTK_BUTTONS_CLOSE,
                                  (gchar *)label);

  gtk_window_set_title(GTK_WINDOW(GUI_Error),(gchar *) title);
  if(modal)
    {
      gtk_dialog_run(GTK_DIALOG(GUI_Error));
      gtk_widget_destroy(GTK_WIDGET(GUI_Error));
    }
  else
    {
      g_signal_connect_swapped (GUI_Error, "response",
                                G_CALLBACK (ErrorPopDownProc),
                                GUI_Error);
      errorUp = True;
      gtk_widget_show(GTK_WIDGET(GUI_Error));
    }

  return;
}

/* Disable all user input other than deleting the window */
static int frozen = 0;
void FreezeUI()
{
  if (frozen) return;
  /* Grab by a widget that doesn't accept input */
  //  XtAddGrab(messageWidget, TRUE, FALSE);
  frozen = 1;
}

/* Undo a FreezeUI */
void ThawUI()
{
  if (!frozen) return;
  //  XtRemoveGrab(messageWidget);
  frozen = 0;
}

char *ModeToWidgetName(mode)
     GameMode mode;
{
    switch (mode) {
      case BeginningOfGame:
	if (appData.icsActive)
	  return "menuMode.ICS Client";
	else if (appData.noChessProgram ||
		 *appData.cmailGameName != NULLCHAR)
	  return "menuMode.Edit Game";
	else
	  return "menuMode.Machine Black";
      case MachinePlaysBlack:
	return "menuMode.Machine Black";
      case MachinePlaysWhite:
	return "menuMode.Machine White";
      case AnalyzeMode:
	return "menuMode.Analysis Mode";
      case AnalyzeFile:
	return "menuMode.Analyze File";
      case TwoMachinesPlay:
	return "menuMode.Two Machines";
      case EditGame:
	return "menuMode.Edit Game";
      case PlayFromGameFile:
	return "menuFile.Load Game";
      case EditPosition:
	return "menuMode.Edit Position";
      case Training:
	return "menuMode.Training";
      case IcsPlayingWhite:
      case IcsPlayingBlack:
      case IcsObserving:
      case IcsIdle:
      case IcsExamining:
	return "menuMode.ICS Client";
      default:
      case EndOfGame:
	return NULL;
    }
}

void ModeHighlight()
{
    static int oldPausing = FALSE;
    static GameMode oldmode = (GameMode) -1;
    char *wname;

   // todo this toggling of the pause button doesn't seem to work?
    // e.g. select pause from buttonbar doesn't activate menumode.pause

    //    if (!boardWidget || !XtIsRealized(boardWidget)) return;

    if (pausing != oldPausing) {
      oldPausing = pausing;
      gtk_button_set_relief(GTK_BUTTON (gtk_builder_get_object (builder, "menuMode.Pause")),pausing?GTK_RELIEF_NORMAL:GTK_RELIEF_NONE);
      /* toggle background color in showbuttonbar */
      if (appData.showButtonBar) {
	if (pausing) {
	  gtk_button_pressed(GTK_BUTTON (gtk_builder_get_object (builder, "buttonbar.Pause")));
	} else {
	  gtk_button_released(GTK_BUTTON (gtk_builder_get_object (builder, "buttonbar.Pause")));
	}
      }
    }

    wname = ModeToWidgetName(oldmode);
    if(wname)
       gtk_check_menu_item_set_active (GTK_CHECK_MENU_ITEM (gtk_builder_get_object (builder, wname)),True);

    oldmode = gameMode;

    /* Maybe all the enables should be handled here, not just this one */
    gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM (gtk_builder_get_object (builder, "menuMode.Training")),
			     gameMode == Training || gameMode == PlayFromGameFile);
}


/*
 * Button/menu procedures
 */

int LoadGamePopUp(f, gameNumber, title)
     FILE *f;
     int gameNumber;
     char *title;
{
    cmailMsgLoaded = FALSE;

    if (gameNumber == 0) 
      {
	int error = GameListBuild(f);

	if (error) 
	  {
	    DisplayError(_("Cannot build game list"), error);
	  } 
	else if (!ListEmpty(&gameList) 
		 && ((ListGame *) gameList.tailPred)->number > 1) 
	  {
	    GameListPopUp(f, title);
	    return TRUE;
	  };

	GameListDestroy();
	gameNumber = 1;
      };

    return LoadGame(f, gameNumber, title, FALSE);
}


void LoadNextPositionProc(w, event, prms, nprms)
     Widget w;
     XEvent *event;
     String *prms;
     Cardinal *nprms;
{
    ReloadPosition(1);
}

void LoadPrevPositionProc(w, event, prms, nprms)
     Widget w;
     XEvent *event;
     String *prms;
     Cardinal *nprms;
{
    ReloadPosition(-1);
}

void ReloadPositionProc(w, event, prms, nprms)
     Widget w;
     XEvent *event;
     String *prms;
     Cardinal *nprms;
{
    ReloadPosition(0);
}

void LoadPositionProc(w, event, prms, nprms)
     Widget w;
     XEvent *event;
     String *prms;
     Cardinal *nprms;
{
    if (gameMode == AnalyzeMode || gameMode == AnalyzeFile) {
	Reset(FALSE, TRUE);
    }
    FileNamePopUp(_("Load position file name?"), "", LoadPosition, "rb");
}

void SaveGameProc(w, event, prms, nprms)
     Widget w;
     XEvent *event;
     String *prms;
     Cardinal *nprms;
{
    FileNamePopUp(_("Save game file name?"),
		  DefaultFileName(appData.oldSaveStyle ? "game" : "pgn"),
		  SaveGame, "a");
}

void SavePositionProc(w, event, prms, nprms)
     Widget w;
     XEvent *event;
     String *prms;
     Cardinal *nprms;
{
    FileNamePopUp(_("Save position file name?"),
		  DefaultFileName(appData.oldSaveStyle ? "pos" : "fen"),
		  SavePosition, "a");
}

void ReloadCmailMsgProc(w, event, prms, nprms)
     Widget w;
     XEvent *event;
     String *prms;
     Cardinal *nprms;
{
    ReloadCmailMsgEvent(FALSE);
}

void MailMoveProc(w, event, prms, nprms)
     Widget w;
     XEvent *event;
     String *prms;
     Cardinal *nprms;
{
    MailMoveEvent();
}

/* this variable is shared between CopyPositionProc and SendPositionSelection */
static char *selected_fen_position=NULL;

static Boolean
SendPositionSelection(Widget w, Atom *selection, Atom *target,
		 Atom *type_return, XtPointer *value_return,
		 unsigned long *length_return, int *format_return)
{
  char *selection_tmp;

  if (!selected_fen_position) return False; /* should never happen */
  if (*target == XA_STRING){
    /* note: since no XtSelectionDoneProc was registered, Xt will
     * automatically call XtFree on the value returned.  So have to
     * make a copy of it allocated with XtMalloc */
    selection_tmp= XtMalloc(strlen(selected_fen_position)+16);
    strcpy(selection_tmp, selected_fen_position);

    *value_return=selection_tmp;
    *length_return=strlen(selection_tmp);
    *type_return=XA_STRING;
    *format_return = 8; /* bits per byte */
    return True;
  } else {
    return False;
  }
}

/* note: when called from menu all parameters are NULL, so no clue what the
 * Widget which was clicked on was, or what the click event was
 */
void CopyPositionProc(w, event, prms, nprms)
  Widget w;
  XEvent *event;
  String *prms;
  Cardinal *nprms;
  {
    int ret;

    if (selected_fen_position) free(selected_fen_position);
    selected_fen_position = (char *)PositionToFEN(currentMove, NULL);
    if (!selected_fen_position) return;
    ret = XtOwnSelection(menuBarWidget, XA_PRIMARY,
			 CurrentTime,
			 SendPositionSelection,
			 NULL/* lose_ownership_proc */ ,
			 NULL/* transfer_done_proc */);
    if (!ret) {
      free(selected_fen_position);
      selected_fen_position=NULL;
    }
  }

/* function called when the data to Paste is ready */
static void
PastePositionCB(Widget w, XtPointer client_data, Atom *selection,
	   Atom *type, XtPointer value, unsigned long *len, int *format)
{
  char *fenstr=value;
  if (value==NULL || *len==0) return; /* nothing had been selected to copy */
  fenstr[*len]='\0'; /* normally this string is terminated, but be safe */
  EditPositionPasteFEN(fenstr);
  XtFree(value);
}

/* called when Paste Position button is pressed,
 * all parameters will be NULL */
void PastePositionProc(w, event, prms, nprms)
  Widget w;
  XEvent *event;
  String *prms;
  Cardinal *nprms;
{
    XtGetSelectionValue(menuBarWidget, XA_PRIMARY, XA_STRING,
      /* (XtSelectionCallbackProc) */ PastePositionCB,
      NULL, /* client_data passed to PastePositionCB */

      /* better to use the time field from the event that triggered the
       * call to this function, but that isn't trivial to get
       */
      CurrentTime
    );
    return;
}

static Boolean
SendGameSelection(Widget w, Atom *selection, Atom *target,
		  Atom *type_return, XtPointer *value_return,
		  unsigned long *length_return, int *format_return)
{
  char *selection_tmp;

  if (*target == XA_STRING){
    FILE* f = fopen(gameCopyFilename, "r");
    long len;
    size_t count;
    if (f == NULL) return False;
    fseek(f, 0, 2);
    len = ftell(f);
    rewind(f);
    selection_tmp = XtMalloc(len + 1);
    count = fread(selection_tmp, 1, len, f);
    if (len != count) {
      XtFree(selection_tmp);
      return False;
    }
    selection_tmp[len] = NULLCHAR;
    *value_return = selection_tmp;
    *length_return = len;
    *type_return = XA_STRING;
    *format_return = 8; /* bits per byte */
    return True;
  } else {
    return False;
  }
}

/* note: when called from menu all parameters are NULL, so no clue what the
 * Widget which was clicked on was, or what the click event was
 */
void CopyGameProc(w, event, prms, nprms)
  Widget w;
  XEvent *event;
  String *prms;
  Cardinal *nprms;
{
  int ret;

  ret = SaveGameToFile(gameCopyFilename, FALSE);
  if (!ret) return;

  ret = XtOwnSelection(menuBarWidget, XA_PRIMARY,
		       CurrentTime,
		       SendGameSelection,
		       NULL/* lose_ownership_proc */ ,
		       NULL/* transfer_done_proc */);
}

/* function called when the data to Paste is ready */
static void
PasteGameCB(Widget w, XtPointer client_data, Atom *selection,
	    Atom *type, XtPointer value, unsigned long *len, int *format)
{
  FILE* f;
  if (value == NULL || *len == 0) {
    return; /* nothing had been selected to copy */
  }
  f = fopen(gamePasteFilename, "w");
  if (f == NULL) {
    DisplayError(_("Can't open temp file"), errno);
    return;
  }
  fwrite(value, 1, *len, f);
  fclose(f);
  XtFree(value);
  LoadGameFromFile(gamePasteFilename, 0, gamePasteFilename, TRUE);
}

/* called when Paste Game button is pressed,
 * all parameters will be NULL */
void PasteGameProc(w, event, prms, nprms)
  Widget w;
  XEvent *event;
  String *prms;
  Cardinal *nprms;
{
    XtGetSelectionValue(menuBarWidget, XA_PRIMARY, XA_STRING,
      /* (XtSelectionCallbackProc) */ PasteGameCB,
      NULL, /* client_data passed to PasteGameCB */

      /* better to use the time field from the event that triggered the
       * call to this function, but that isn't trivial to get
       */
      CurrentTime
    );
    return;
}


void AutoSaveGame()
{
    SaveGameProc(NULL, NULL, NULL, NULL);
}

void AnalyzeModeProc(w, event, prms, nprms)
     Widget w;
     XEvent *event;
     String *prms;
     Cardinal *nprms;
{
    char buf[MSG_SIZ];

    if (!first.analysisSupport) {
      snprintf(buf, sizeof(buf), _("%s does not support analysis"), first.tidy);
      DisplayError(buf, 0);
      return;
    }
    /* [DM] icsEngineAnalyze [HGM] This is horrible code; reverse the gameMode and isEngineAnalyze tests! */
    if (appData.icsActive) {
        if (gameMode != IcsObserving) {
            sprintf(buf,_("You are not observing a game"));
            DisplayError(buf, 0);
            /* secure check */
            if (appData.icsEngineAnalyze) {
                if (appData.debugMode)
                    fprintf(debugFP, _("Found unexpected active ICS engine analyze \n"));
                ExitAnalyzeMode();
                ModeHighlight();
            }
            return;
        }
        /* if enable, use want disable icsEngineAnalyze */
        if (appData.icsEngineAnalyze) {
                ExitAnalyzeMode();
                ModeHighlight();
                return;
        }
        appData.icsEngineAnalyze = TRUE;
        if (appData.debugMode)
            fprintf(debugFP, _("ICS engine analyze starting... \n"));
    }
    if (!appData.showThinking)
      ShowThinkingProc(NULL,NULL);

    AnalyzeModeEvent();
}

void AnalyzeFileProc(w, event, prms, nprms)
     Widget w;
     XEvent *event;
     String *prms;
     Cardinal *nprms;
{
    if (!first.analysisSupport) {
      char buf[MSG_SIZ];
      snprintf(buf, sizeof(buf), _("%s does not support analysis"), first.tidy);
      DisplayError(buf, 0);
      return;
    }
    Reset(FALSE, TRUE);

    if (!appData.showThinking)
      ShowThinkingProc(NULL,NULL);

    AnalyzeFileEvent();
    FileNamePopUp(_("File to analyze"), "", LoadGamePopUp, "rb");
    AnalysisPeriodicEvent(1);
}


void EditGameProc(w, event, prms, nprms)
     Widget w;
     XEvent *event;
     String *prms;
     Cardinal *nprms;
{
    EditGameEvent();
}

void EditPositionProc(w, event, prms, nprms)
     Widget w;
     XEvent *event;
     String *prms;
     Cardinal *nprms;
{
    EditPositionEvent();
}

void TrainingProc(w, event, prms, nprms)
     Widget w;
     XEvent *event;
     String *prms;
     Cardinal *nprms;
{
    TrainingEvent();
}

void EditCommentProc(w, event, prms, nprms)
     Widget w;
     XEvent *event;
     String *prms;
     Cardinal *nprms;
{
    if (editUp) {
	EditCommentPopDown();
    } else {
	EditCommentEvent();
    }
}

void IcsInputBoxProc(w, event, prms, nprms)
     Widget w;
     XEvent *event;
     String *prms;
     Cardinal *nprms;
{
    if (ICSInputBoxUp) {
	ICSInputBoxPopDown();
    } else {
	ICSInputBoxPopUp();
    }
}


void EnterKeyProc(w, event, prms, nprms)
     Widget w;
     XEvent *event;
     String *prms;
     Cardinal *nprms;
{
    if (ICSInputBoxUp == True)
      ICSInputSendText();
}

void AlwaysQueenProc(w, event, prms, nprms)
     Widget w;
     XEvent *event;
     String *prms;
     Cardinal *nprms;
{
    Arg args[16];

    appData.alwaysPromoteToQueen = !appData.alwaysPromoteToQueen;

    if (appData.alwaysPromoteToQueen) {
	XtSetArg(args[0], XtNleftBitmap, xMarkPixmap);
    } else {
	XtSetArg(args[0], XtNleftBitmap, None);
    }
    XtSetValues(XtNameToWidget(menuBarWidget, "menuOptions.Always Queen"),
		args, 1);
}

void AnimateDraggingProc(w, event, prms, nprms)
     Widget w;
     XEvent *event;
     String *prms;
     Cardinal *nprms;
{
    Arg args[16];

    appData.animateDragging = !appData.animateDragging;

    if (appData.animateDragging) {
	XtSetArg(args[0], XtNleftBitmap, xMarkPixmap);
        CreateAnimVars();
    } else {
	XtSetArg(args[0], XtNleftBitmap, None);
    }
    XtSetValues(XtNameToWidget(menuBarWidget, "menuOptions.Animate Dragging"),
		args, 1);
}

void AnimateMovingProc(w, event, prms, nprms)
     Widget w;
     XEvent *event;
     String *prms;
     Cardinal *nprms;
{
    Arg args[16];

    appData.animate = !appData.animate;

    if (appData.animate) {
	XtSetArg(args[0], XtNleftBitmap, xMarkPixmap);
        CreateAnimVars();
    } else {
	XtSetArg(args[0], XtNleftBitmap, None);
    }
    XtSetValues(XtNameToWidget(menuBarWidget, "menuOptions.Animate Moving"),
		args, 1);
}

void AutocommProc(w, event, prms, nprms)
     Widget w;
     XEvent *event;
     String *prms;
     Cardinal *nprms;
{
    Arg args[16];

    appData.autoComment = !appData.autoComment;

    if (appData.autoComment) {
	XtSetArg(args[0], XtNleftBitmap, xMarkPixmap);
    } else {
	XtSetArg(args[0], XtNleftBitmap, None);
    }
    XtSetValues(XtNameToWidget(menuBarWidget, "menuOptions.Auto Comment"),
		args, 1);
}


void AutoflagProc(w, event, prms, nprms)
     Widget w;
     XEvent *event;
     String *prms;
     Cardinal *nprms;
{
    Arg args[16];

    appData.autoCallFlag = !appData.autoCallFlag;

    if (appData.autoCallFlag) {
	XtSetArg(args[0], XtNleftBitmap, xMarkPixmap);
    } else {
	XtSetArg(args[0], XtNleftBitmap, None);
    }
    XtSetValues(XtNameToWidget(menuBarWidget, "menuOptions.Auto Flag"),
		args, 1);
}

void AutoflipProc(w, event, prms, nprms)
     Widget w;
     XEvent *event;
     String *prms;
     Cardinal *nprms;
{
    Arg args[16];

    appData.autoFlipView = !appData.autoFlipView;

    if (appData.autoFlipView) {
	XtSetArg(args[0], XtNleftBitmap, xMarkPixmap);
    } else {
	XtSetArg(args[0], XtNleftBitmap, None);
    }
    XtSetValues(XtNameToWidget(menuBarWidget, "menuOptions.Auto Flip View"),
		args, 1);
}

void AutobsProc(w, event, prms, nprms)
     Widget w;
     XEvent *event;
     String *prms;
     Cardinal *nprms;
{
    Arg args[16];

    appData.autoObserve = !appData.autoObserve;

    if (appData.autoObserve) {
	XtSetArg(args[0], XtNleftBitmap, xMarkPixmap);
    } else {
	XtSetArg(args[0], XtNleftBitmap, None);
    }
    XtSetValues(XtNameToWidget(menuBarWidget, "menuOptions.Auto Observe"),
		args, 1);
}

void AutoraiseProc(w, event, prms, nprms)
     Widget w;
     XEvent *event;
     String *prms;
     Cardinal *nprms;
{
    Arg args[16];

    appData.autoRaiseBoard = !appData.autoRaiseBoard;

    if (appData.autoRaiseBoard) {
	XtSetArg(args[0], XtNleftBitmap, xMarkPixmap);
    } else {
	XtSetArg(args[0], XtNleftBitmap, None);
    }
    XtSetValues(XtNameToWidget(menuBarWidget, "menuOptions.Auto Raise Board"),
		args, 1);
}

void AutosaveProc(w, event, prms, nprms)
     Widget w;
     XEvent *event;
     String *prms;
     Cardinal *nprms;
{
    Arg args[16];

    appData.autoSaveGames = !appData.autoSaveGames;

    if (appData.autoSaveGames) {
	XtSetArg(args[0], XtNleftBitmap, xMarkPixmap);
    } else {
	XtSetArg(args[0], XtNleftBitmap, None);
    }
    XtSetValues(XtNameToWidget(menuBarWidget, "menuOptions.Auto Save"),
		args, 1);
}

void BlindfoldProc(w, event, prms, nprms)
     Widget w;
     XEvent *event;
     String *prms;
     Cardinal *nprms;
{
    Arg args[16];

    appData.blindfold = !appData.blindfold;

    if (appData.blindfold) {
	XtSetArg(args[0], XtNleftBitmap, xMarkPixmap);
    } else {
	XtSetArg(args[0], XtNleftBitmap, None);
    }
    XtSetValues(XtNameToWidget(menuBarWidget, "menuOptions.Blindfold"),
		args, 1);

    DrawPosition(True, NULL);
}

void TestLegalityProc(w, event, prms, nprms)
     Widget w;
     XEvent *event;
     String *prms;
     Cardinal *nprms;
{
    Arg args[16];

    appData.testLegality = !appData.testLegality;

    if (appData.testLegality) {
	XtSetArg(args[0], XtNleftBitmap, xMarkPixmap);
    } else {
	XtSetArg(args[0], XtNleftBitmap, None);
    }
    XtSetValues(XtNameToWidget(menuBarWidget, "menuOptions.Test Legality"),
		args, 1);
}


void FlashMovesProc(w, event, prms, nprms)
     Widget w;
     XEvent *event;
     String *prms;
     Cardinal *nprms;
{
    Arg args[16];

    if (appData.flashCount == 0) {
	appData.flashCount = 3;
    } else {
	appData.flashCount = -appData.flashCount;
    }

    if (appData.flashCount > 0) {
	XtSetArg(args[0], XtNleftBitmap, xMarkPixmap);
    } else {
	XtSetArg(args[0], XtNleftBitmap, None);
    }
    XtSetValues(XtNameToWidget(menuBarWidget, "menuOptions.Flash Moves"),
		args, 1);
}

#if HIGHDRAG
void HighlightDraggingProc(w, event, prms, nprms)
     Widget w;
     XEvent *event;
     String *prms;
     Cardinal *nprms;
{
    Arg args[16];

    appData.highlightDragging = !appData.highlightDragging;

    if (appData.highlightDragging) {
	XtSetArg(args[0], XtNleftBitmap, xMarkPixmap);
    } else {
	XtSetArg(args[0], XtNleftBitmap, None);
    }
    XtSetValues(XtNameToWidget(menuBarWidget,
			       "menuOptions.Highlight Dragging"), args, 1);
}
#endif

void HighlightLastMoveProc(w, event, prms, nprms)
     Widget w;
     XEvent *event;
     String *prms;
     Cardinal *nprms;
{
    Arg args[16];

    appData.highlightLastMove = !appData.highlightLastMove;

    if (appData.highlightLastMove) {
	XtSetArg(args[0], XtNleftBitmap, xMarkPixmap);
    } else {
	XtSetArg(args[0], XtNleftBitmap, None);
    }
    XtSetValues(XtNameToWidget(menuBarWidget,
			       "menuOptions.Highlight Last Move"), args, 1);
}

void IcsAlarmProc(w, event, prms, nprms)
     Widget w;
     XEvent *event;
     String *prms;
     Cardinal *nprms;
{
    Arg args[16];

    appData.icsAlarm = !appData.icsAlarm;

    if (appData.icsAlarm) {
	XtSetArg(args[0], XtNleftBitmap, xMarkPixmap);
    } else {
	XtSetArg(args[0], XtNleftBitmap, None);
    }
    XtSetValues(XtNameToWidget(menuBarWidget,
			       "menuOptions.ICS Alarm"), args, 1);
}

void MoveSoundProc(w, event, prms, nprms)
     Widget w;
     XEvent *event;
     String *prms;
     Cardinal *nprms;
{
    Arg args[16];

    appData.ringBellAfterMoves = !appData.ringBellAfterMoves;

    if (appData.ringBellAfterMoves) {
	XtSetArg(args[0], XtNleftBitmap, xMarkPixmap);
    } else {
	XtSetArg(args[0], XtNleftBitmap, None);
    }
    XtSetValues(XtNameToWidget(menuBarWidget, "menuOptions.Move Sound"),
		args, 1);
}


void OldSaveStyleProc(w, event, prms, nprms)
     Widget w;
     XEvent *event;
     String *prms;
     Cardinal *nprms;
{
    Arg args[16];

    appData.oldSaveStyle = !appData.oldSaveStyle;

    if (appData.oldSaveStyle) {
	XtSetArg(args[0], XtNleftBitmap, xMarkPixmap);
    } else {
	XtSetArg(args[0], XtNleftBitmap, None);
    }
    XtSetValues(XtNameToWidget(menuBarWidget, "menuOptions.Old Save Style"),
		args, 1);
}

void PeriodicUpdatesProc(w, event, prms, nprms)
     Widget w;
     XEvent *event;
     String *prms;
     Cardinal *nprms;
{
    Arg args[16];

    PeriodicUpdatesEvent(!appData.periodicUpdates);

    if (appData.periodicUpdates) {
	XtSetArg(args[0], XtNleftBitmap, xMarkPixmap);
    } else {
	XtSetArg(args[0], XtNleftBitmap, None);
    }
    XtSetValues(XtNameToWidget(menuBarWidget, "menuOptions.Periodic Updates"),
		args, 1);
}

void PonderNextMoveProc(w, event, prms, nprms)
     Widget w;
     XEvent *event;
     String *prms;
     Cardinal *nprms;
{
    Arg args[16];

    PonderNextMoveEvent(!appData.ponderNextMove);

    if (appData.ponderNextMove) {
	XtSetArg(args[0], XtNleftBitmap, xMarkPixmap);
    } else {
	XtSetArg(args[0], XtNleftBitmap, None);
    }
    XtSetValues(XtNameToWidget(menuBarWidget, "menuOptions.Ponder Next Move"),
		args, 1);
}

void PopupExitMessageProc(w, event, prms, nprms)
     Widget w;
     XEvent *event;
     String *prms;
     Cardinal *nprms;
{
    Arg args[16];

    appData.popupExitMessage = !appData.popupExitMessage;

    if (appData.popupExitMessage) {
	XtSetArg(args[0], XtNleftBitmap, xMarkPixmap);
    } else {
	XtSetArg(args[0], XtNleftBitmap, None);
    }
    XtSetValues(XtNameToWidget(menuBarWidget,
			       "menuOptions.Popup Exit Message"), args, 1);
}

void PopupMoveErrorsProc(w, event, prms, nprms)
     Widget w;
     XEvent *event;
     String *prms;
     Cardinal *nprms;
{
    Arg args[16];

    appData.popupMoveErrors = !appData.popupMoveErrors;

    if (appData.popupMoveErrors) {
	XtSetArg(args[0], XtNleftBitmap, xMarkPixmap);
    } else {
	XtSetArg(args[0], XtNleftBitmap, None);
    }
    XtSetValues(XtNameToWidget(menuBarWidget, "menuOptions.Popup Move Errors"),
		args, 1);
}

void PremoveProc(w, event, prms, nprms)
     Widget w;
     XEvent *event;
     String *prms;
     Cardinal *nprms;
{
    Arg args[16];

    appData.premove = !appData.premove;

    if (appData.premove) {
	XtSetArg(args[0], XtNleftBitmap, xMarkPixmap);
    } else {
	XtSetArg(args[0], XtNleftBitmap, None);
    }
    XtSetValues(XtNameToWidget(menuBarWidget,
			       "menuOptions.Premove"), args, 1);
}

void QuietPlayProc(w, event, prms, nprms)
     Widget w;
     XEvent *event;
     String *prms;
     Cardinal *nprms;
{
    Arg args[16];

    appData.quietPlay = !appData.quietPlay;

    if (appData.quietPlay) {
	XtSetArg(args[0], XtNleftBitmap, xMarkPixmap);
    } else {
	XtSetArg(args[0], XtNleftBitmap, None);
    }
    XtSetValues(XtNameToWidget(menuBarWidget, "menuOptions.Quiet Play"),
		args, 1);
}

void DebugProc(w, event, prms, nprms)
     Widget w;
     XEvent *event;
     String *prms;
     Cardinal *nprms;
{
    appData.debugMode = !appData.debugMode;
}

void AboutGameProc(w, event, prms, nprms)
     Widget w;
     XEvent *event;
     String *prms;
     Cardinal *nprms;
{
    AboutGameEvent();
}

void NothingProc(w, event, prms, nprms)
     Widget w;
     XEvent *event;
     String *prms;
     Cardinal *nprms;
{
    return;
}

void Iconify(w, event, prms, nprms)
     Widget w;
     XEvent *event;
     String *prms;
     Cardinal *nprms;
{
    Arg args[16];

    fromX = fromY = -1;
    XtSetArg(args[0], XtNiconic, True);
    XtSetValues(shellWidget, args, 1);
}

void DisplayMessage(message, extMessage)
     gchar *message, *extMessage;
{
    char buf[MSG_SIZ];
    Arg arg;

    if (extMessage) {
	if (*message) {
	    snprintf(buf, sizeof(buf), "%s  %s", message, extMessage);
	    message = buf;
	} else {
	    message = extMessage;
	}
    }
    gtk_label_set_text( GTK_LABEL(gtk_builder_get_object (builder, "Messages")),message);

    return;
}

void DisplayTitle(text)
     char *text;
{
    gchar title[MSG_SIZ];

    if (text == NULL) text = "";

    if (appData.titleInWindow)
      {
	/* TODO */
      }

    if (*text != NULLCHAR)
      {
	strcpy(title, text);
      }
    else if (appData.icsActive)
      {
	snprintf(title, sizeof(title), "%s: %s", programName, appData.icsHost);
      }
    else if (appData.cmailGameName[0] != NULLCHAR)
      {
	snprintf(title,sizeof(title), "%s: %s", programName, "CMail");
#ifdef GOTHIC
    // [HGM] license: This stuff should really be done in back-end, but WinBoard already had a pop-up for it
      }
    else if (gameInfo.variant == VariantGothic)
      {
	strcpy(title, GOTHIC);
#endif
#ifdef FALCON
      }
    else if (gameInfo.variant == VariantFalcon)
      {
	strcpy(title, FALCON);
#endif
      }
    else if (appData.noChessProgram)
      {
	strcpy(title, programName);
      }
    else
      {
	snprintf(title,sizeof(title), "%s: %s", programName, first.tidy);
      }
    gtk_window_set_title(GTK_WINDOW(GUI_Window),title);

    return;
}


void DisplayError(message, error)
     String message;
     int error;
{
    char buf[MSG_SIZ];

    if (error == 0) {
	if (appData.debugMode || appData.matchMode) {
	    fprintf(stderr, "%s: %s\n", programName, message);
	}
    } else {
	if (appData.debugMode || appData.matchMode) {
	    fprintf(stderr, "%s: %s: %s\n",
		    programName, message, strerror(error));
	}
	snprintf(buf, sizeof(buf), "%s: %s", message, strerror(error));
	message = buf;
    }
    ErrorPopUp(_("Error"), message, FALSE);
}


void DisplayMoveError(message)
     String message;
{
    fromX = fromY = -1;
    ClearHighlights();
    DrawPosition(FALSE, NULL);
    if (appData.debugMode || appData.matchMode) {
	fprintf(stderr, "%s: %s\n", programName, message);
    }
    if (appData.popupMoveErrors) {
	ErrorPopUp(_("Error"), message, FALSE);
    } else {
	DisplayMessage(message, "");
    }
}


void DisplayFatalError(message, error, status)
     String message;
     int error, status;
{
    char buf[MSG_SIZ];

    errorExitStatus = status;
    if (error == 0) {
	fprintf(stderr, "%s: %s\n", programName, message);
    } else {
	fprintf(stderr, "%s: %s: %s\n",
		programName, message, strerror(error));
	snprintf(buf, sizeof(buf), "%s: %s", message, strerror(error));
	message = buf;
    }
    if (appData.popupExitMessage && boardWidget && XtIsRealized(boardWidget)) {
      ErrorPopUp(status ? _("Fatal Error") : _("Exiting"), message, TRUE);
    } else {
      ExitEvent(status);
    }
}

void DisplayInformation(message)
     String message;
{
    ErrorPopDown();
    ErrorPopUp(_("Information"), message, TRUE);
}

void DisplayNote(message)
     String message;
{
    ErrorPopDown();
    ErrorPopUp(_("Note"), message, FALSE);
}

static int
NullXErrorCheck(dpy, error_event)
     Display *dpy;
     XErrorEvent *error_event;
{
    return 0;
}

void DisplayIcsInteractionTitle(message)
     String message;
{
  if (oldICSInteractionTitle == NULL) {
    /* Magic to find the old window title, adapted from vim */
    char *wina = getenv("WINDOWID");
    if (wina != NULL) {
      Window win = (Window) atoi(wina);
      Window root, parent, *children;
      unsigned int nchildren;
      int (*oldHandler)() = XSetErrorHandler(NullXErrorCheck);
      for (;;) {
	if (XFetchName(xDisplay, win, &oldICSInteractionTitle)) break;
	if (!XQueryTree(xDisplay, win, &root, &parent,
			&children, &nchildren)) break;
	if (children) XFree((void *)children);
	if (parent == root || parent == 0) break;
	win = parent;
      }
      XSetErrorHandler(oldHandler);
    }
    if (oldICSInteractionTitle == NULL) {
      oldICSInteractionTitle = "xterm";
    }
  }
  printf("\033]0;%s\007", message);
  fflush(stdout);
}

char pendingReplyPrefix[MSG_SIZ];
ProcRef pendingReplyPR;

void AskQuestionProc(w, event, prms, nprms)
     Widget w;
     XEvent *event;
     String *prms;
     Cardinal *nprms;
{
    if (*nprms != 4) {
	fprintf(stderr, _("AskQuestionProc needed 4 parameters, got %d\n"),
		*nprms);
	return;
    }
    AskQuestionEvent(prms[0], prms[1], prms[2], prms[3]);
}

void AskQuestionPopDown()
{
    if (!askQuestionUp) return;
    XtPopdown(askQuestionShell);
    XtDestroyWidget(askQuestionShell);
    askQuestionUp = False;
}

void AskQuestionReplyAction(w, event, prms, nprms)
     Widget w;
     XEvent *event;
     String *prms;
     Cardinal *nprms;
{
    char buf[MSG_SIZ];
    int err;
    String reply;

    reply = XawDialogGetValueString(w = XtParent(w));
    strcpy(buf, pendingReplyPrefix);
    if (*buf) strcat(buf, " ");
    strcat(buf, reply);
    strcat(buf, "\n");
    OutputToProcess(pendingReplyPR, buf, strlen(buf), &err);
    AskQuestionPopDown();

    if (err) DisplayFatalError(_("Error writing to chess program"), err, 0);
}

void AskQuestionCallback(w, client_data, call_data)
     Widget w;
     XtPointer client_data, call_data;
{
    String name;
    Arg args[16];

    XtSetArg(args[0], XtNlabel, &name);
    XtGetValues(w, args, 1);

    if (strcmp(name, _("cancel")) == 0) {
        AskQuestionPopDown();
    } else {
	AskQuestionReplyAction(w, NULL, NULL, NULL);
    }
}

void AskQuestion(title, question, replyPrefix, pr)
     char *title, *question, *replyPrefix;
     ProcRef pr;
{
    Arg args[16];
    Widget popup, layout, dialog, edit;
    Window root, child;
    int x, y, i;
    int win_x, win_y;
    unsigned int mask;

    strcpy(pendingReplyPrefix, replyPrefix);
    pendingReplyPR = pr;

    i = 0;
    XtSetArg(args[i], XtNresizable, True); i++;
    XtSetArg(args[i], XtNwidth, DIALOG_SIZE); i++;
    askQuestionShell = popup =
      XtCreatePopupShell(title, transientShellWidgetClass,
			 shellWidget, args, i);

    layout =
      XtCreateManagedWidget(layoutName, formWidgetClass, popup,
			    layoutArgs, XtNumber(layoutArgs));

    i = 0;
    XtSetArg(args[i], XtNlabel, question); i++;
    XtSetArg(args[i], XtNvalue, ""); i++;
    XtSetArg(args[i], XtNborderWidth, 0); i++;
    dialog = XtCreateManagedWidget("question", dialogWidgetClass,
				   layout, args, i);

    XawDialogAddButton(dialog, _("enter"), AskQuestionCallback,
		       (XtPointer) dialog);
    XawDialogAddButton(dialog, _("cancel"), AskQuestionCallback,
		       (XtPointer) dialog);

    XtRealizeWidget(popup);
    CatchDeleteWindow(popup, "AskQuestionPopDown");

    XQueryPointer(xDisplay, xBoardWindow, &root, &child,
		  &x, &y, &win_x, &win_y, &mask);

    XtSetArg(args[0], XtNx, x - 10);
    XtSetArg(args[1], XtNy, y - 30);
    XtSetValues(popup, args, 2);

    XtPopup(popup, XtGrabExclusive);
    askQuestionUp = True;

    edit = XtNameToWidget(dialog, "*value");
    XtSetKeyboardFocus(popup, edit);
}


void
PlaySound(name)
     char *name;
{
  if (*name == NULLCHAR) {
    return;
  } else if (strcmp(name, "$") == 0) {
    putc(BELLCHAR, stderr);
  } else {
    char buf[2048];
    snprintf(buf, sizeof(buf), "%s '%s' &", appData.soundProgram, name);
    system(buf);
  }
}

void
RingBell()
{
  PlaySound(appData.soundMove);
}

void
PlayIcsWinSound()
{
  PlaySound(appData.soundIcsWin);
}

void
PlayIcsLossSound()
{
  PlaySound(appData.soundIcsLoss);
}

void
PlayIcsDrawSound()
{
  PlaySound(appData.soundIcsDraw);
}

void
PlayIcsUnfinishedSound()
{
  PlaySound(appData.soundIcsUnfinished);
}

void
PlayAlarmSound()
{
  PlaySound(appData.soundIcsAlarm);
}

void
EchoOn()
{
    system("stty echo");
}

void
EchoOff()
{
    system("stty -echo");
}

void
Colorize(cc, continuation)
     ColorClass cc;
     int continuation;
{
    char buf[MSG_SIZ];
    int count, outCount, error;

    if (textColors[(int)cc].bg > 0) {
	if (textColors[(int)cc].fg > 0) {
	    sprintf(buf, "\033[0;%d;%d;%dm", textColors[(int)cc].attr,
		    textColors[(int)cc].fg, textColors[(int)cc].bg);
	} else {
	    sprintf(buf, "\033[0;%d;%dm", textColors[(int)cc].attr,
		    textColors[(int)cc].bg);
	}
    } else {
	if (textColors[(int)cc].fg > 0) {
	    sprintf(buf, "\033[0;%d;%dm", textColors[(int)cc].attr,
		    textColors[(int)cc].fg);
	} else {
	    sprintf(buf, "\033[0;%dm", textColors[(int)cc].attr);
	}
    }
    count = strlen(buf);
    outCount = OutputToProcess(NoProc, buf, count, &error);
    if (outCount < count) {
	DisplayFatalError(_("Error writing to display"), error, 1);
    }

    if (continuation) return;
    switch (cc) {
    case ColorShout:
      PlaySound(appData.soundShout);
      break;
    case ColorSShout:
      PlaySound(appData.soundSShout);
      break;
    case ColorChannel1:
      PlaySound(appData.soundChannel1);
      break;
    case ColorChannel:
      PlaySound(appData.soundChannel);
      break;
    case ColorKibitz:
      PlaySound(appData.soundKibitz);
      break;
    case ColorTell:
      PlaySound(appData.soundTell);
      break;
    case ColorChallenge:
      PlaySound(appData.soundChallenge);
      break;
    case ColorRequest:
      PlaySound(appData.soundRequest);
      break;
    case ColorSeek:
      PlaySound(appData.soundSeek);
      break;
    case ColorNormal:
    case ColorNone:
    default:
      break;
    }
}

char *UserName()
{
    return getpwuid(getuid())->pw_name;
}

static char *ExpandPathName(path)
     char *path;
{
    static char static_buf[2000];
    char *d, *s, buf[2000];
    struct passwd *pwd;

    s = path;
    d = static_buf;

    while (*s && isspace(*s))
      ++s;

    if (!*s) {
	*d = 0;
	return static_buf;
    }

    if (*s == '~') {
	if (*(s+1) == '/') {
	    strcpy(d, getpwuid(getuid())->pw_dir);
	    strcat(d, s+1);
	}
	else {
	    strcpy(buf, s+1);
	    *strchr(buf, '/') = 0;
	    pwd = getpwnam(buf);
	    if (!pwd)
	      {
		  fprintf(stderr, _("ERROR: Unknown user %s (in path %s)\n"),
			  buf, path);
		  return NULL;
	      }
	    strcpy(d, pwd->pw_dir);
	    strcat(d, strchr(s+1, '/'));
	}
    }
    else
      strcpy(d, s);

    return static_buf;
}

char *HostName()
{
    static char host_name[MSG_SIZ];

#if HAVE_GETHOSTNAME
    gethostname(host_name, MSG_SIZ);
    return host_name;
#else  /* not HAVE_GETHOSTNAME */
# if HAVE_SYSINFO && HAVE_SYS_SYSTEMINFO_H
    sysinfo(SI_HOSTNAME, host_name, MSG_SIZ);
    return host_name;
# else /* not (HAVE_SYSINFO && HAVE_SYS_SYSTEMINFO_H) */
    return "localhost";
# endif/* not (HAVE_SYSINFO && HAVE_SYS_SYSTEMINFO_H) */
#endif /* not HAVE_GETHOSTNAME */
}

guint delayedEventTimerTag = 0;
DelayedEventCallback delayedEventCallback = 0;

void
FireDelayedEvent(data)
     gpointer data;
{
  /* remove timer */
  g_source_remove(delayedEventTimerTag);
  delayedEventTimerTag = 0;

  /* call function */
  delayedEventCallback();

  return;
}

void
ScheduleDelayedEvent(cb, millisec)
     DelayedEventCallback cb; guint millisec;
{
    if(delayedEventTimerTag && delayedEventCallback == cb)
	// [HGM] alive: replace, rather than add or flush identical event
	g_source_remove(delayedEventTimerTag);
    delayedEventCallback = cb;
    delayedEventTimerTag = g_timeout_add(millisec,(GSourceFunc) FireDelayedEvent, NULL);
    return;
}

DelayedEventCallback
GetDelayedEvent()
{
  if (delayedEventTimerTag)
    {
      return delayedEventCallback;
    }
  else
    {
      return NULL;
    }
}

void
CancelDelayedEvent()
{
  if (delayedEventTimerTag)
    {
      g_source_remove(delayedEventTimerTag);
      delayedEventTimerTag = 0;
    }

  return;
}

guint loadGameTimerTag = 0;

int LoadGameTimerRunning()
{
    return loadGameTimerTag != 0;
}

int StopLoadGameTimer()
{
    if (loadGameTimerTag != 0) {
	g_source_remove(loadGameTimerTag);
	loadGameTimerTag = 0;
	return TRUE;
    } else {
	return FALSE;
    }
}

void
LoadGameTimerCallback(data)
     gpointer data;
{
  /* remove timer */
  g_source_remove(loadGameTimerTag);
  loadGameTimerTag = 0;

  AutoPlayGameLoop();
  return;
}

void
StartLoadGameTimer(millisec)
     long millisec;
{
  loadGameTimerTag =
    g_timeout_add( millisec, (GSourceFunc) LoadGameTimerCallback, NULL);
  return;
}

guint analysisClockTag = 0;

gboolean
AnalysisClockCallback(data)
     gpointer data;
{
    if (gameMode == AnalyzeMode || gameMode == AnalyzeFile
         || appData.icsEngineAnalyze)
      {
	AnalysisPeriodicEvent(0);
	return 1; /* keep on going */
      }
    return 0; /* stop timer */
}

void
StartAnalysisClock()
{
  analysisClockTag =
    g_timeout_add( 2000,(GSourceFunc) AnalysisClockCallback, NULL);
  return;
}

guint clockTimerTag = 0;

int ClockTimerRunning()
{
    return clockTimerTag != 0;
}

int StopClockTimer()
{
    if (clockTimerTag != 0)
      {
	g_source_remove(clockTimerTag);
	clockTimerTag = 0;
	return TRUE;
      }
    else
      {
	return FALSE;
      }
}

void
ClockTimerCallback(data)
     gpointer data;
{
  /* remove timer */
  g_source_remove(clockTimerTag);
  clockTimerTag = 0;

  DecrementClocks();
  return;
}

void
StartClockTimer(millisec)
     long millisec;
{
  clockTimerTag = g_timeout_add(millisec,(GSourceFunc) ClockTimerCallback,NULL);
  return;
}

void
DisplayTimerLabel(w, color, timer, highlight)
     GtkWidget *w;
     char *color;
     long timer;
     int highlight;
{
  gchar buf[MSG_SIZ];


  if (appData.clockMode) {
    sprintf(buf, "%s: %s", color, TimeString(timer));
  } else {
    sprintf(buf, "%s  ", color);
  }
  gtk_label_set_text(GTK_LABEL(w),buf);

  /* check for low time warning */
//    Pixel foregroundOrWarningColor = timerForegroundPixel;

//    if (timer > 0 &&
//        appData.lowTimeWarning &&
//        (timer / 1000) < appData.icsAlarmTime)
//      foregroundOrWarningColor = lowTimeWarningColor;
//
//    if (appData.clockMode) {
//	sprintf(buf, "%s: %s", color, TimeString(timer));
//	XtSetArg(args[0], XtNlabel, buf);
//    } else {
//	sprintf(buf, "%s  ", color);
//	XtSetArg(args[0], XtNlabel, buf);
//    }
//
//    if (highlight) {
//
//	XtSetArg(args[1], XtNbackground, foregroundOrWarningColor);
//	XtSetArg(args[2], XtNforeground, timerBackgroundPixel);
//    } else {
//	XtSetArg(args[1], XtNbackground, timerBackgroundPixel);
//	XtSetArg(args[2], XtNforeground, foregroundOrWarningColor);
//    }
//
//    XtSetValues(w, args, 3);
//
}

void
DisplayWhiteClock(timeRemaining, highlight)
     long timeRemaining;
     int highlight;
{
  if(appData.noGUI) return;

  DisplayTimerLabel(GUI_Whiteclock, _("White"), timeRemaining, highlight);
  if (highlight && WindowIcon == BlackIcon)
    {
      WindowIcon = WhiteIcon;
      gtk_window_set_icon(GTK_WINDOW(GUI_Window),WindowIcon);
    }
}

void
DisplayBlackClock(timeRemaining, highlight)
     long timeRemaining;
     int highlight;
{
    if(appData.noGUI) return;

    DisplayTimerLabel(GUI_Blackclock, _("Black"), timeRemaining, highlight);
    if (highlight && WindowIcon == WhiteIcon)
      {
        WindowIcon = BlackIcon;
        gtk_window_set_icon(GTK_WINDOW(GUI_Window),WindowIcon);
      }
}

#define CPNone 0
#define CPReal 1
#define CPComm 2
#define CPSock 3
#define CPLoop 4
typedef int CPKind;

typedef struct {
    CPKind kind;
    int pid;
    int fdTo, fdFrom;
} ChildProc;


int StartChildProcess(cmdLine, dir, pr)
     char *cmdLine;
     char *dir;
     ProcRef *pr;
{
    char *argv[64], *p;
    int i, pid;
    int to_prog[2], from_prog[2];
    ChildProc *cp;
    char buf[MSG_SIZ];

    if (appData.debugMode) {
	fprintf(stderr, "StartChildProcess (dir=\"%s\") %s\n",dir, cmdLine);
    }

    /* We do NOT feed the cmdLine to the shell; we just
       parse it into blank-separated arguments in the
       most simple-minded way possible.
       */
    i = 0;
    strcpy(buf, cmdLine);
    p = buf;
    for (;;) {
	argv[i++] = p;
	p = strchr(p, ' ');
	if (p == NULL) break;
	*p++ = NULLCHAR;
    }
    argv[i] = NULL;

    SetUpChildIO(to_prog, from_prog);

    if ((pid = fork()) == 0) {
	/* Child process */
	// [HGM] PSWBTM: made order resistant against case where fd of created pipe was 0 or 1
	close(to_prog[1]);     // first close the unused pipe ends
	close(from_prog[0]);
	dup2(to_prog[0], 0);   // to_prog was created first, nd is the only one to use 0 or 1
	dup2(from_prog[1], 1);
	if(to_prog[0] >= 2) close(to_prog[0]); // if 0 or 1, the dup2 already cosed the original
	close(from_prog[1]);                   // and closing again loses one of the pipes!
	if(fileno(stderr) >= 2) // better safe than sorry...
		dup2(1, fileno(stderr)); /* force stderr to the pipe */

	if (dir[0] != NULLCHAR && chdir(dir) != 0) {
	    perror(dir);
	    exit(1);
	}

	nice(appData.niceEngines); // [HGM] nice: adjust priority of engine proc

        execvp(argv[0], argv);

	/* If we get here, exec failed */
	perror(argv[0]);
	exit(1);
    }

    /* Parent process */
    close(to_prog[0]);
    close(from_prog[1]);

    cp = (ChildProc *) calloc(1, sizeof(ChildProc));
    cp->kind = CPReal;
    cp->pid = pid;
    cp->fdFrom = from_prog[0];
    cp->fdTo = to_prog[1];
    *pr = (ProcRef) cp;
    return 0;
}

// [HGM] kill: implement the 'hard killing' of AS's Winboard_x
static RETSIGTYPE AlarmCallBack(int n)
{
    return;
}

void
DestroyChildProcess(pr, signalType)
     ProcRef pr;
     int signalType;
{
    ChildProc *cp = (ChildProc *) pr;

    if (cp->kind != CPReal) return;
    cp->kind = CPNone;
    if (signalType == 10) { // [HGM] kill: if it does not terminate in 3 sec, kill
	signal(SIGALRM, AlarmCallBack);
	alarm(3);
	if(wait((int *) 0) == -1) { // process does not terminate on its own accord
	    kill(cp->pid, SIGKILL); // kill it forcefully
	    wait((int *) 0);        // and wait again
	}
    } else {
	if (signalType) {
	    kill(cp->pid, signalType == 9 ? SIGKILL : SIGTERM); // [HGM] kill: use hard kill if so requested
	}
	/* Process is exiting either because of the kill or because of
	   a quit command sent by the backend; either way, wait for it to die.
	*/
	wait((int *) 0);
    }
    close(cp->fdFrom);
    close(cp->fdTo);
}

void
InterruptChildProcess(pr)
     ProcRef pr;
{
    ChildProc *cp = (ChildProc *) pr;

    if (cp->kind != CPReal) return;
    (void) kill(cp->pid, SIGINT); /* stop it thinking */
}

int OpenTelnet(host, port, pr)
     char *host;
     char *port;
     ProcRef *pr;
{
    char cmdLine[MSG_SIZ];

    if (port[0] == NULLCHAR) {
      snprintf(cmdLine, sizeof(cmdLine), "%s %s", appData.telnetProgram, host);
    } else {
      snprintf(cmdLine, sizeof(cmdLine), "%s %s %s", appData.telnetProgram, host, port);
    }
    return StartChildProcess(cmdLine, "", pr);
}

int OpenTCP(host, port, pr)
     char *host;
     char *port;
     ProcRef *pr;
{
#if OMIT_SOCKETS
    DisplayFatalError(_("Socket support is not configured in"), 0, 2);
#else  /* !OMIT_SOCKETS */
    int s;
    struct sockaddr_in sa;
    struct hostent     *hp;
    unsigned short uport;
    ChildProc *cp;

    if ((s = socket(AF_INET, SOCK_STREAM, 6)) < 0) {
	return errno;
    }

    memset((char *) &sa, (int)0, sizeof(struct sockaddr_in));
    sa.sin_family = AF_INET;
    sa.sin_addr.s_addr = INADDR_ANY;
    uport = (unsigned short) 0;
    sa.sin_port = htons(uport);
    if (bind(s, (struct sockaddr *) &sa, sizeof(struct sockaddr_in)) < 0) {
	return errno;
    }

    memset((char *) &sa, (int)0, sizeof(struct sockaddr_in));
    if (!(hp = gethostbyname(host))) {
	int b0, b1, b2, b3;
	if (sscanf(host, "%d.%d.%d.%d", &b0, &b1, &b2, &b3) == 4) {
	    hp = (struct hostent *) calloc(1, sizeof(struct hostent));
	    hp->h_addrtype = AF_INET;
	    hp->h_length = 4;
	    hp->h_addr_list = (char **) calloc(2, sizeof(char *));
	    hp->h_addr_list[0] = (char *) malloc(4);
	    hp->h_addr_list[0][0] = b0;
	    hp->h_addr_list[0][1] = b1;
	    hp->h_addr_list[0][2] = b2;
	    hp->h_addr_list[0][3] = b3;
	} else {
	    return ENOENT;
	}
    }
    sa.sin_family = hp->h_addrtype;
    uport = (unsigned short) atoi(port);
    sa.sin_port = htons(uport);
    memcpy((char *) &sa.sin_addr, hp->h_addr, hp->h_length);

    if (connect(s, (struct sockaddr *) &sa,
		sizeof(struct sockaddr_in)) < 0) {
	return errno;
    }

    cp = (ChildProc *) calloc(1, sizeof(ChildProc));
    cp->kind = CPSock;
    cp->pid = 0;
    cp->fdFrom = s;
    cp->fdTo = s;
    *pr = (ProcRef) cp;

#endif /* !OMIT_SOCKETS */

    return 0;
}

int OpenCommPort(name, pr)
     char *name;
     ProcRef *pr;
{
    int fd;
    ChildProc *cp;

    fd = open(name, 2, 0);
    if (fd < 0) return errno;

    cp = (ChildProc *) calloc(1, sizeof(ChildProc));
    cp->kind = CPComm;
    cp->pid = 0;
    cp->fdFrom = fd;
    cp->fdTo = fd;
    *pr = (ProcRef) cp;

    return 0;
}

int OpenLoopback(pr)
     ProcRef *pr;
{
    ChildProc *cp;
    int to[2], from[2];

    SetUpChildIO(to, from);

    cp = (ChildProc *) calloc(1, sizeof(ChildProc));
    cp->kind = CPLoop;
    cp->pid = 0;
    cp->fdFrom = to[0];		/* note not from[0]; we are doing a loopback */
    cp->fdTo = to[1];
    *pr = (ProcRef) cp;

    return 0;
}

int OpenRcmd(host, user, cmd, pr)
     char *host, *user, *cmd;
     ProcRef *pr;
{
    DisplayFatalError(_("internal rcmd not implemented for Unix"), 0, 1);
    return -1;
}

#define INPUT_SOURCE_BUF_SIZE 8192

typedef struct {
    CPKind kind;
    int fd;
    int lineByLine;
    char *unused;
    InputCallback func;
    guint sid;
    char buf[INPUT_SOURCE_BUF_SIZE];
    VOIDSTAR closure;
} InputSource;

void
DoInputCallback(io,cond,data)
     GIOChannel   *io;
     GIOCondition  cond;
     gpointer *data;
{
  /* read input from one of the input source (for example a chess program, ICS, etc).
   * and call a function that will handle the input
   */

  int count; /* how many bytes did we read */
  int error; 
  char *p, *q;
  
  /* All information (callback function, file descriptor, etc) is
   * saved in an InputSource structure 
   */
  InputSource *is = (InputSource *) data; 
  
  if (is->lineByLine) 
    {
      count = read(is->fd, is->unused,
		   INPUT_SOURCE_BUF_SIZE - (is->unused - is->buf));

      if (count <= 0) 
	{
	  (is->func)(is, is->closure, is->buf, count, count ? errno : 0);
	  return;
	}
      is->unused += count;
      p = is->buf;
      /* break input into lines and call the callback function on each
       * line 
       */
      while (p < is->unused) 
	{
	  q = memchr(p, '\n', is->unused - p);
	  if (q == NULL) break;
	  q++;
	  (is->func)(is, is->closure, p, q - p, 0);
	  p = q;
	}
      /* remember not yet used part of the buffer */
      q = is->buf;
      while (p < is->unused) 
	{
	  *q++ = *p++;
	}
      is->unused = q;
    }
  else 
    {
      /* read maximum length of input buffer and send the whole buffer
       * to the callback function 
       */
      count = read(is->fd, is->buf, INPUT_SOURCE_BUF_SIZE);
      if (count == -1)
	error = errno;
      else
	error = 0;
      (is->func)(is, is->closure, is->buf, count, error);
    }
  
  return;
}

InputSourceRef AddInputSource(pr, lineByLine, func, closure)
     ProcRef pr;
     int lineByLine;
     InputCallback func;
     VOIDSTAR closure;
{
    InputSource *is;
    GIOChannel *channel;
    ChildProc *cp = (ChildProc *) pr;

    is = (InputSource *) calloc(1, sizeof(InputSource));
    is->lineByLine = lineByLine;
    is->func = func;
    if (pr == NoProc) {
	is->kind = CPReal;
	is->fd = fileno(stdin);
    } else {
	is->kind = cp->kind;
	is->fd = cp->fdFrom;
    }
    if (lineByLine) 
      is->unused = is->buf;
    else
      is->unused = NULL;

//    is->xid = XtAppAddInput(appContext, is->fd,
//			    (XtPointer) (XtInputReadMask),
//			    (XtInputCallbackProc) DoInputCallback,
//			    (XtPointer) is);
//

    /* TODO: will this work on windows?*/
    printf("DEBUG: fd=%d %d\n",is->fd,is);

    channel = g_io_channel_unix_new(is->fd);
    g_io_channel_set_close_on_unref (channel, TRUE);
    is->sid = g_io_add_watch(channel, G_IO_IN,(GIOFunc) DoInputCallback, is);
    is->closure = closure;
    return (InputSourceRef) is;
}

void
RemoveInputSource(isr)
     InputSourceRef isr;
{
    InputSource *is = (InputSource *) isr;

    if (is->sid == 0) return;
    g_source_remove(is->sid);
    is->sid = 0;
    return;
}

int OutputToProcess(pr, message, count, outError)
     ProcRef pr;
     char *message;
     int count;
     int *outError;
{
    ChildProc *cp = (ChildProc *) pr;
    int outCount;

    if (pr == NoProc)
      outCount = fwrite(message, 1, count, stdout);
    else
      outCount = write(cp->fdTo, message, count);

    if (outCount == -1)
      *outError = errno;
    else
      *outError = 0;

    return outCount;
}

/* Output message to process, with "ms" milliseconds of delay
   between each character. This is needed when sending the logon
   script to ICC, which for some reason doesn't like the
   instantaneous send. */
int OutputToProcessDelayed(pr, message, count, outError, msdelay)
     ProcRef pr;
     char *message;
     int count;
     int *outError;
     long msdelay;
{
    ChildProc *cp = (ChildProc *) pr;
    int outCount = 0;
    int r;

    while (count--) {
	r = write(cp->fdTo, message++, 1);
	if (r == -1) {
	    *outError = errno;
	    return outCount;
	}
	++outCount;
	if (msdelay >= 0)
	  TimeDelay(msdelay);
    }

    return outCount;
}

/****	Animation code by Hugh Fisher, DCS, ANU.

	Known problem: if a window overlapping the board is
	moved away while a piece is being animated underneath,
	the newly exposed area won't be updated properly.
	I can live with this.

	Known problem: if you look carefully at the animation
	of pieces in mono mode, they are being drawn as solid
	shapes without interior detail while moving. Fixing
	this would be a major complication for minimal return.
****/

/*	Masks for XPM pieces. Black and white pieces can have
	different shapes, but in the interest of retaining my
	sanity pieces must have the same outline on both light
	and dark squares, and all pieces must use the same
	background square colors/images.		*/

static int xpmDone = 0;

static void
CreateAnimMasks (pieceDepth)
     int pieceDepth;
{
  ChessSquare   piece;
  Pixmap	buf;
  GC		bufGC, maskGC;
  int		kind, n;
  unsigned long	plane;
  XGCValues	values;

  /* just return for gtk at the moment */
  return;

  /* Need a bitmap just to get a GC with right depth */
  buf = XCreatePixmap(xDisplay, xBoardWindow,
			8, 8, 1);
  values.foreground = 1;
  values.background = 0;
  /* Don't use XtGetGC, not read only */
  maskGC = XCreateGC(xDisplay, buf,
		    GCForeground | GCBackground, &values);
  XFreePixmap(xDisplay, buf);

  buf = XCreatePixmap(xDisplay, xBoardWindow,
		      squareSize, squareSize, pieceDepth);
  values.foreground = XBlackPixel(xDisplay, xScreen);
  values.background = XWhitePixel(xDisplay, xScreen);
  bufGC = XCreateGC(xDisplay, buf,
		    GCForeground | GCBackground, &values);

  for (piece = WhitePawn; piece <= BlackKing; piece++) {
    /* Begin with empty mask */
    if(!xpmDone) // [HGM] pieces: keep using existing
    xpmMask[piece] = XCreatePixmap(xDisplay, xBoardWindow,
				 squareSize, squareSize, 1);
    XSetFunction(xDisplay, maskGC, GXclear);
    XFillRectangle(xDisplay, xpmMask[piece], maskGC,
		   0, 0, squareSize, squareSize);

    /* Take a copy of the piece */
    if (White(piece))
      kind = 0;
    else
      kind = 2;
    XSetFunction(xDisplay, bufGC, GXcopy);
    XCopyArea(xDisplay, xpmPieceBitmap[kind][((int)piece) % (int)BlackPawn],
	      buf, bufGC,
	      0, 0, squareSize, squareSize, 0, 0);

    /* XOR the background (light) over the piece */
    XSetFunction(xDisplay, bufGC, GXxor);
    if (useImageSqs)
      XCopyArea(xDisplay, xpmLightSquare, buf, bufGC,
		0, 0, squareSize, squareSize, 0, 0);
    else {
      XSetForeground(xDisplay, bufGC, lightSquareColor);
      XFillRectangle(xDisplay, buf, bufGC, 0, 0, squareSize, squareSize);
    }

    /* We now have an inverted piece image with the background
       erased. Construct mask by just selecting all the non-zero
       pixels - no need to reconstruct the original image.	*/
    XSetFunction(xDisplay, maskGC, GXor);
    plane = 1;
    /* Might be quicker to download an XImage and create bitmap
       data from it rather than this N copies per piece, but it
       only takes a fraction of a second and there is a much
       longer delay for loading the pieces.	   	*/
    for (n = 0; n < pieceDepth; n ++) {
      XCopyPlane(xDisplay, buf, xpmMask[piece], maskGC,
		 0, 0, squareSize, squareSize,
		 0, 0, plane);
      plane = plane << 1;
    }
  }
  /* Clean up */
  XFreePixmap(xDisplay, buf);
  XFreeGC(xDisplay, bufGC);
  XFreeGC(xDisplay, maskGC);
}

static void
InitAnimState (anim, info)
  AnimState * anim;
  XWindowAttributes * info;
{
  XtGCMask  mask;
  XGCValues values;

  /* Each buffer is square size, same depth as window */
//  anim->saveBuf = XCreatePixmap(xDisplay, xBoardWindow,
//			squareSize, squareSize, info->depth);
//  anim->newBuf = XCreatePixmap(xDisplay, xBoardWindow,
//			squareSize, squareSize, info->depth);
//
//  /* Create a plain GC for blitting */
//  mask = GCForeground | GCBackground | GCFunction |
//         GCPlaneMask | GCGraphicsExposures;
//  values.foreground = XBlackPixel(xDisplay, xScreen);
//  values.background = XWhitePixel(xDisplay, xScreen);
//  values.function   = GXcopy;
//  values.plane_mask = AllPlanes;
//  values.graphics_exposures = False;
//  anim->blitGC = XCreateGC(xDisplay, xBoardWindow, mask, &values);
//
//  /* Piece will be copied from an existing context at
//     the start of each new animation/drag. */
//  anim->pieceGC = XCreateGC(xDisplay, xBoardWindow, 0, &values);
//
//  /* Outline will be a read-only copy of an existing */
//  anim->outlineGC = None;
}

static void
CreateAnimVars ()
{
  static VariantClass old = (VariantClass) -1; // [HGM] pieces: redo every time variant changes
  XWindowAttributes info;

  /* for gtk at the moment just ... */
  return;

  if (xpmDone && gameInfo.variant == old) return;
  if(xpmDone) old = gameInfo.variant; // first time pieces might not be created yet
  //  XGetWindowAttributes(xDisplay, xBoardWindow, &info);

  //  InitAnimState(&game, &info);
  //  InitAnimState(&player, &info);

  /* For XPM pieces, we need bitmaps to use as masks. */
  //  if (useImages)
  //    CreateAnimMasks(info.depth);
   xpmDone = 1;
}

#ifndef HAVE_USLEEP

static Boolean frameWaiting;

static RETSIGTYPE FrameAlarm (sig)
     int sig;
{
  frameWaiting = False;
  /* In case System-V style signals.  Needed?? */
  signal(SIGALRM, FrameAlarm);
}

static void
FrameDelay (time)
     int time;
{
  struct itimerval delay;

  XSync(xDisplay, False);

  if (time > 0) {
    frameWaiting = True;
    signal(SIGALRM, FrameAlarm);
    delay.it_interval.tv_sec =
      delay.it_value.tv_sec = time / 1000;
    delay.it_interval.tv_usec =
      delay.it_value.tv_usec = (time % 1000) * 1000;
    setitimer(ITIMER_REAL, &delay, NULL);
    while (frameWaiting) pause();
    delay.it_interval.tv_sec = delay.it_value.tv_sec = 0;
    delay.it_interval.tv_usec = delay.it_value.tv_usec = 0;
    setitimer(ITIMER_REAL, &delay, NULL);
  }
}

#else

static void
FrameDelay (time)
     int time;
{
  //  XSync(xDisplay, False);
  if (time > 0)
    usleep(time * 1000);
}

#endif

/*	Convert board position to corner of screen rect and color	*/

static void
ScreenSquare(column, row, pt, color)
     int column; int row; XPoint * pt; int * color;
{
  if (flipView) {
    pt->x = lineGap + ((BOARD_WIDTH-1)-column) * (squareSize + lineGap);
    pt->y = lineGap + row * (squareSize + lineGap);
  } else {
    pt->x = lineGap + column * (squareSize + lineGap);
    pt->y = lineGap + ((BOARD_HEIGHT-1)-row) * (squareSize + lineGap);
  }
  *color = SquareColor(row, column);
}

/*	Convert window coords to square			*/

static void
BoardSquare(x, y, column, row)
     int x; int y; int * column; int * row;
{
  *column = EventToSquare(x, BOARD_WIDTH);
  if (flipView && *column >= 0)
    *column = BOARD_WIDTH - 1 - *column;
  *row = EventToSquare(y, BOARD_HEIGHT);
  if (!flipView && *row >= 0)
    *row = BOARD_HEIGHT - 1 - *row;
}

/*   Utilities	*/

#undef Max  /* just in case */
#undef Min
#define Max(a, b) ((a) > (b) ? (a) : (b))
#define Min(a, b) ((a) < (b) ? (a) : (b))

static void
SetRect(rect, x, y, width, height)
     XRectangle * rect; int x; int y; int width; int height;
{
  rect->x = x;
  rect->y = y;
  rect->width  = width;
  rect->height = height;
}

/*	Test if two frames overlap. If they do, return
	intersection rect within old and location of
	that rect within new. */

static Boolean
Intersect(old, new, size, area, pt)
     XPoint * old; XPoint * new;
     int size; XRectangle * area; XPoint * pt;
{
  if (old->x > new->x + size || new->x > old->x + size ||
      old->y > new->y + size || new->y > old->y + size) {
    return False;
  } else {
    SetRect(area, Max(new->x - old->x, 0), Max(new->y - old->y, 0),
            size - abs(old->x - new->x), size - abs(old->y - new->y));
    pt->x = Max(old->x - new->x, 0);
    pt->y = Max(old->y - new->y, 0);
    return True;
  }
}

/*	For two overlapping frames, return the rect(s)
	in the old that do not intersect with the new.   */

static void
CalcUpdateRects(old, new, size, update, nUpdates)
     XPoint * old; XPoint * new; int size;
     XRectangle update[]; int * nUpdates;
{
  int	     count;

  /* If old = new (shouldn't happen) then nothing to draw */
  if (old->x == new->x && old->y == new->y) {
    *nUpdates = 0;
    return;
  }
  /* Work out what bits overlap. Since we know the rects
     are the same size we don't need a full intersect calc. */
  count = 0;
  /* Top or bottom edge? */
  if (new->y > old->y) {
    SetRect(&(update[count]), old->x, old->y, size, new->y - old->y);
    count ++;
  } else if (old->y > new->y) {
    SetRect(&(update[count]), old->x, old->y + size - (old->y - new->y),
			      size, old->y - new->y);
    count ++;
  }
  /* Left or right edge - don't overlap any update calculated above. */
  if (new->x > old->x) {
    SetRect(&(update[count]), old->x, Max(new->y, old->y),
			      new->x - old->x, size - abs(new->y - old->y));
    count ++;
  } else if (old->x > new->x) {
    SetRect(&(update[count]), new->x + size, Max(new->y, old->y),
			      old->x - new->x, size - abs(new->y - old->y));
    count ++;
  }
  /* Done */
  *nUpdates = count;
}

/*	Generate a series of frame coords from start->mid->finish.
	The movement rate doubles until the half way point is
	reached, then halves back down to the final destination,
	which gives a nice slow in/out effect. The algorithmn
	may seem to generate too many intermediates for short
	moves, but remember that the purpose is to attract the
	viewers attention to the piece about to be moved and
	then to where it ends up. Too few frames would be less
	noticeable.						*/

static void
Tween(start, mid, finish, factor, frames, nFrames)
     XPoint * start; XPoint * mid;
     XPoint * finish; int factor;
     XPoint frames[]; int * nFrames;
{
  int fraction, n, count;

  count = 0;

  /* Slow in, stepping 1/16th, then 1/8th, ... */
  fraction = 1;
  for (n = 0; n < factor; n++)
    fraction *= 2;
  for (n = 0; n < factor; n++) {
    frames[count].x = start->x + (mid->x - start->x) / fraction;
    frames[count].y = start->y + (mid->y - start->y) / fraction;
    count ++;
    fraction = fraction / 2;
  }

  /* Midpoint */
  frames[count] = *mid;
  count ++;

  /* Slow out, stepping 1/2, then 1/4, ... */
  fraction = 2;
  for (n = 0; n < factor; n++) {
    frames[count].x = finish->x - (finish->x - mid->x) / fraction;
    frames[count].y = finish->y - (finish->y - mid->y) / fraction;
    count ++;
    fraction = fraction * 2;
  }
  *nFrames = count;
}

/*	Draw a piece on the screen without disturbing what's there	*/

static void
SelectGCMask(piece, clip, outline, mask)
     ChessSquare piece; GC * clip; GC * outline; Pixmap * mask;
{
  GC source;

  /* Bitmap for piece being moved. */
  if (appData.monoMode) {
      *mask = *pieceToSolid(piece);
  } else if (useImages) {
#if HAVE_LIBXPM
      *mask = xpmMask[piece];
#else
      *mask = ximMaskPm[piece];
#endif
  } else {
      *mask = *pieceToSolid(piece);
  }

  /* GC for piece being moved. Square color doesn't matter, but
     since it gets modified we make a copy of the original. */
  if (White(piece)) {
    if (appData.monoMode)
      source = bwPieceGC;
    else
      source = wlPieceGC;
  } else {
    if (appData.monoMode)
      source = wbPieceGC;
    else
      source = blPieceGC;
  }
  //  XCopyGC(xDisplay, source, 0xFFFFFFFF, *clip);

  /* Outline only used in mono mode and is not modified */
  if (White(piece))
    *outline = bwPieceGC;
  else
    *outline = wbPieceGC;
}

static void
OverlayPiece(piece, clip, outline,  dest)
     ChessSquare piece; GC clip; GC outline; Drawable dest;
{
  int	kind;

  if (!useImages) {
    /* Draw solid rectangle which will be clipped to shape of piece */
//    XFillRectangle(xDisplay, dest, clip,
//		   0, 0, squareSize, squareSize)
;
    if (appData.monoMode)
      /* Also draw outline in contrasting color for black
	 on black / white on white cases		*/
//      XCopyPlane(xDisplay, *pieceToOutline(piece), dest, outline,
//		 0, 0, squareSize, squareSize, 0, 0, 1)
;
  } else {
    /* Copy the piece */
    if (White(piece))
      kind = 0;
    else
      kind = 2;
//    XCopyArea(xDisplay, xpmPieceBitmap[kind][piece],
//	      dest, clip,
//	      0, 0, squareSize, squareSize,
//	      0, 0);
  }
}

/* Animate the movement of a single piece */

static void
BeginAnimation(anim, piece, startColor, start)
     AnimState *anim;
     ChessSquare piece;
     int startColor;
     XPoint * start;
{
  Pixmap mask;

  /* The old buffer is initialised with the start square (empty) */
  BlankSquare(0, 0, startColor, EmptySquare, anim->saveBuf);
  anim->prevFrame = *start;

  /* The piece will be drawn using its own bitmap as a matte	*/
//  SelectGCMask(piece, &anim->pieceGC, &anim->outlineGC, &mask);
//  XSetClipMask(xDisplay, anim->pieceGC, mask);
}

static void
AnimationFrame(anim, frame, piece)
     AnimState *anim;
     XPoint *frame;
     ChessSquare piece;
{
  XRectangle updates[4];
  XRectangle overlap;
  XPoint     pt;
  int	     count, i;

  /* Save what we are about to draw into the new buffer */
//  XCopyArea(xDisplay, xBoardWindow, anim->newBuf, anim->blitGC,
//	    frame->x, frame->y, squareSize, squareSize,
//	    0, 0);

  /* Erase bits of the previous frame */
  if (Intersect(&anim->prevFrame, frame, squareSize, &overlap, &pt)) {
    /* Where the new frame overlapped the previous,
       the contents in newBuf are wrong. */
//    XCopyArea(xDisplay, anim->saveBuf, anim->newBuf, anim->blitGC,
//	      overlap.x, overlap.y,
//	      overlap.width, overlap.height,
//	      pt.x, pt.y);
    /* Repaint the areas in the old that don't overlap new */
    CalcUpdateRects(&anim->prevFrame, frame, squareSize, updates, &count);
    for (i = 0; i < count; i++)
//      XCopyArea(xDisplay, anim->saveBuf, xBoardWindow, anim->blitGC,
//		updates[i].x - anim->prevFrame.x,
//		updates[i].y - anim->prevFrame.y,
//		updates[i].width, updates[i].height,
//		updates[i].x, updates[i].y)
;
  } else {
    /* Easy when no overlap */
//    XCopyArea(xDisplay, anim->saveBuf, xBoardWindow, anim->blitGC,
//		  0, 0, squareSize, squareSize,
//		  anim->prevFrame.x, anim->prevFrame.y);
  }

  /* Save this frame for next time round */
//  XCopyArea(xDisplay, anim->newBuf, anim->saveBuf, anim->blitGC,
//		0, 0, squareSize, squareSize,
//		0, 0);
  anim->prevFrame = *frame;

  /* Draw piece over original screen contents, not current,
     and copy entire rect. Wipes out overlapping piece images. */
  OverlayPiece(piece, anim->pieceGC, anim->outlineGC, anim->newBuf);
//  XCopyArea(xDisplay, anim->newBuf, xBoardWindow, anim->blitGC,
//		0, 0, squareSize, squareSize,
//		frame->x, frame->y);
}

static void
EndAnimation (anim, finish)
     AnimState *anim;
     XPoint *finish;
{
  XRectangle updates[4];
  XRectangle overlap;
  XPoint     pt;
  int	     count, i;

  /* The main code will redraw the final square, so we
     only need to erase the bits that don't overlap.	*/
  if (Intersect(&anim->prevFrame, finish, squareSize, &overlap, &pt)) {
    CalcUpdateRects(&anim->prevFrame, finish, squareSize, updates, &count);
    for (i = 0; i < count; i++)
//      XCopyArea(xDisplay, anim->saveBuf, xBoardWindow, anim->blitGC,
//		updates[i].x - anim->prevFrame.x,
//		updates[i].y - anim->prevFrame.y,
//		updates[i].width, updates[i].height,
//		updates[i].x, updates[i].y)
;
  } else {
//    XCopyArea(xDisplay, anim->saveBuf, xBoardWindow, anim->blitGC,
//		0, 0, squareSize, squareSize,
//		anim->prevFrame.x, anim->prevFrame.y);
  }
}

static void
FrameSequence(anim, piece, startColor, start, finish, frames, nFrames)
     AnimState *anim;
     ChessSquare piece; int startColor;
     XPoint * start; XPoint * finish;
     XPoint frames[]; int nFrames;
{
  int n;

  BeginAnimation(anim, piece, startColor, start);
  for (n = 0; n < nFrames; n++) {
    AnimationFrame(anim, &(frames[n]), piece);
    FrameDelay(appData.animSpeed);
  }
  EndAnimation(anim, finish);
}

/* Main control logic for deciding what to animate and how */

void
AnimateMove(board, fromX, fromY, toX, toY)
     Board board;
     int fromX;
     int fromY;
     int toX;
     int toY;
{
  ChessSquare piece;
  int hop;
  XPoint      start, finish, mid;
  XPoint      frames[kFactor * 2 + 1];
  int	      nFrames, startColor, endColor;

  /* Are we animating? */
  if (!appData.animate || appData.blindfold)
    return;

  if(board[toY][toX] == WhiteRook && board[fromY][fromX] == WhiteKing ||
     board[toY][toX] == BlackRook && board[fromY][fromX] == BlackKing)
	return; // [HGM] FRC: no animtion of FRC castlings, as to-square is not true to-square

  if (fromY < 0 || fromX < 0 || toX < 0 || toY < 0) return;
  piece = board[fromY][fromX];
  if (piece >= EmptySquare) return;

#if DONT_HOP
  hop = FALSE;
#else
  hop = (piece == WhiteKnight || piece == BlackKnight);
#endif

  if (appData.debugMode) {
      fprintf(debugFP, hop ? _("AnimateMove: piece %d hops from %d,%d to %d,%d \n") :
                             _("AnimateMove: piece %d slides from %d,%d to %d,%d \n"),
             piece, fromX, fromY, toX, toY);  }

  ScreenSquare(fromX, fromY, &start, &startColor);
  ScreenSquare(toX, toY, &finish, &endColor);

  if (hop) {
    /* Knight: make diagonal movement then straight */
    if (abs(toY - fromY) < abs(toX - fromX)) {
       mid.x = start.x + (finish.x - start.x) / 2;
       mid.y = finish.y;
     } else {
       mid.x = finish.x;
       mid.y = start.y + (finish.y - start.y) / 2;
     }
  } else {
    mid.x = start.x + (finish.x - start.x) / 2;
    mid.y = start.y + (finish.y - start.y) / 2;
  }

  /* Don't use as many frames for very short moves */
  if (abs(toY - fromY) + abs(toX - fromX) <= 2)
    Tween(&start, &mid, &finish, kFactor - 1, frames, &nFrames);
  else
    Tween(&start, &mid, &finish, kFactor, frames, &nFrames);
  FrameSequence(&game, piece, startColor, &start, &finish, frames, nFrames);

  /* Be sure end square is redrawn */
  damage[toY][toX] = True;
}

void
DragPieceBegin(x, y)
     int x; int y;
{
    int	 boardX, boardY, color;
    XPoint corner;

    /* Are we animating? */
    if (!appData.animateDragging || appData.blindfold)
      return;

    /* Figure out which square we start in and the
       mouse position relative to top left corner. */
    BoardSquare(x, y, &boardX, &boardY);
    player.startBoardX = boardX;
    player.startBoardY = boardY;
    ScreenSquare(boardX, boardY, &corner, &color);
    player.startSquare  = corner;
    player.startColor   = color;
    /* As soon as we start dragging, the piece will jump slightly to
       be centered over the mouse pointer. */
    player.mouseDelta.x = squareSize/2;
    player.mouseDelta.y = squareSize/2;
    /* Initialise animation */
    player.dragPiece = PieceForSquare(boardX, boardY);
    /* Sanity check */
    if (player.dragPiece >= 0 && player.dragPiece < EmptySquare) {
	player.dragActive = True;
	BeginAnimation(&player, player.dragPiece, color, &corner);
	/* Mark this square as needing to be redrawn. Note that
	   we don't remove the piece though, since logically (ie
	   as seen by opponent) the move hasn't been made yet. */
           if(boardX == BOARD_RGHT+1 && PieceForSquare(boardX-1, boardY) > 1 ||
              boardX == BOARD_LEFT-2 && PieceForSquare(boardX+1, boardY) > 1)
//           XCopyArea(xDisplay, xBoardWindow, player.saveBuf, player.blitGC,
//	             corner.x, corner.y, squareSize, squareSize,
//	             0, 0); // [HGM] zh: unstack in stead of grab
	damage[boardY][boardX] = True;
    } else {
	player.dragActive = False;
    }
}

static void
DragPieceMove(x, y)
     int x; int y;
{
    XPoint corner;

    /* Are we animating? */
    if (!appData.animateDragging || appData.blindfold)
      return;

    /* Sanity check */
    if (! player.dragActive)
      return;
    /* Move piece, maintaining same relative position
       of mouse within square	 */
    corner.x = x - player.mouseDelta.x;
    corner.y = y - player.mouseDelta.y;
    AnimationFrame(&player, &corner, player.dragPiece);
#if HIGHDRAG
    if (appData.highlightDragging) {
	int boardX, boardY;
	BoardSquare(x, y, &boardX, &boardY);
	SetHighlights(fromX, fromY, boardX, boardY);
    }
#endif
}

void
DragPieceEnd(x, y)
     int x; int y;
{
    int boardX, boardY, color;
    XPoint corner;

    /* Are we animating? */
    if (!appData.animateDragging || appData.blindfold)
      return;

    /* Sanity check */
    if (! player.dragActive)
      return;
    /* Last frame in sequence is square piece is
       placed on, which may not match mouse exactly. */
    BoardSquare(x, y, &boardX, &boardY);
    ScreenSquare(boardX, boardY, &corner, &color);
    EndAnimation(&player, &corner);

    /* Be sure end square is redrawn */
    damage[boardY][boardX] = True;

    /* This prevents weird things happening with fast successive
       clicks which on my Sun at least can cause motion events
       without corresponding press/release. */
    player.dragActive = False;
}

/* Handle expose event while piece being dragged */

static void
DrawDragPiece ()
{
  if (!player.dragActive || appData.blindfold)
    return;

  /* What we're doing: logically, the move hasn't been made yet,
     so the piece is still in it's original square. But visually
     it's being dragged around the board. So we erase the square
     that the piece is on and draw it at the last known drag point. */
  BlankSquare(player.startSquare.x, player.startSquare.y,
		player.startColor, EmptySquare, xBoardWindow);
  AnimationFrame(&player, &player.prevFrame, player.dragPiece);
  damage[player.startBoardY][player.startBoardX] = TRUE;
}

void
SetProgramStats( FrontEndProgramStats * stats )
{
  // [HR] TODO
  // [HGM] done, but perhaps backend should call this directly?
    EngineOutputUpdate( stats );
}
