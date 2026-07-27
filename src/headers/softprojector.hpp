/***************************************************************************
//
//    softProjector - an open source media projection software
//    Copyright (C) 2017  Vladislav Kobzar
//
//    This program is free software: you can redistribute it and/or modify
//    it under the terms of the GNU General Public License as published by
//    the Free Software Foundation version 3 of the License.
//
//    This program is distributed in the hope that it will be useful,
//    but WITHOUT ANY WARRANTY; without even the implied warranty of
//    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//    GNU General Public License for more details.
//
//    You should have received a copy of the GNU General Public License
//    along with this program.  If not, see <http://www.gnu.org/licenses/>.
//
***************************************************************************/

#ifndef SOFTPROJECTOR_HPP
#define SOFTPROJECTOR_HPP

#include <QMainWindow>
#include "projectordisplayscreen.hpp"
#include "editwidget.hpp"
#include "bible.hpp"
#include "managedatadialog.hpp"
#include "settingsdialog.hpp"
#include "songcounter.hpp"
#include "settings.hpp"
#include "printpreviewdialog.hpp"
#include "helpdialog.hpp"
#include "slideshow.hpp"
#include "videoinfo.hpp"
#include "slideshoweditor.hpp"
#include "schedule.hpp"
#include "workspace.hpp"

class QQuickWidget;
class QActionGroup;

enum PresentationType
{
    BIBLE,
    SONG,
    ANNOUCEMENT,
    PICTURE,
    VIDEO
};

// The main window is a shell around the QML workspace. It keeps everything
// that talks to the projector — display screens, themes, image generation,
// schedule files, printing — and owns a Workspace view-model that the QML
// binds to.
class SoftProjector : public QMainWindow
{
    Q_OBJECT

public:
    SoftProjector(QWidget *parent = 0);
    ~SoftProjector();

    EditWidget *editWidget;
    ManageDataDialog *manageDialog;
    ProjectorDisplayScreen *pds1;
    ProjectorDisplayScreen *pds2;
    ProjectorDisplayScreen *pds3;
    ProjectorDisplayScreen *pds4;

    bool showing; // whether we are currently showing to the projector
    Song current_song;
    int current_song_verse;
    Verse current_verse;
    Announcement currentAnnounce;
    QString version_string;
    Theme theme;
    Settings mySettings;

public slots:
    void updateSetting(GeneralSettings &g,Theme &t, SlideShowSettings &ssets,
                       BibleVersionSettings &bsets, BibleVersionSettings &bsets2,
                       BibleVersionSettings &bsets3, BibleVersionSettings &bsets4);
    void saveSettings();
    void positionDisplayWindow();
    void updateScreen();

    void setWaitCursor();
    void setArrowCursor();
    void setAppDataDir(QDir d){appDataDir = d;}

private:
    QQuickWidget *quickView;
    Workspace *workspace;
    SettingsDialog *settingsDialog;
    HelpDialog *helpDialog;
    PresentationType pType;
    bool new_list;
    QActionGroup *languageGroup;
    QString languagePath;
    QTranslator translator;

    // Replaces the old checkable "close display" toolbar action.
    bool displayVisible;

    //For saving and opening schedule files
    QString schedule_file_path;
    bool is_schedule_saved;
    QString cur_locale;
    bool isSingleScreen;
    bool hasDisplayScreen2;
    bool hasDisplayScreen3;
    bool hasDisplayScreen4;

    // Pictures
    QList<SlideShowItem> pictureShowList;

    // video items
    VideoInfo currentVideo;

    // Schedule list
    QList<Schedule> schedule;
    QDir appDataDir;

    // ── QML plumbing ────────────────────────────────────────────────────────
    void setupQuickView();
    void publishLiveState(const QString &kind, const QString &title,
                          const QStringList &slides, int row, const QString &notes = QString());
    void publishActionState();
    int currentShowRow() const;
    int showCount() const;
    QList<int> selectedShowRows() const;
    void setShowRow(int row);
    void countSongUse(const Song &song);

private slots:
    void showDisplayScreen(bool show);

    void applySetting(GeneralSettings &g, Theme &t, SlideShowSettings &s,
                      BibleVersionSettings &b1, BibleVersionSettings &b2,
                      BibleVersionSettings &b3, BibleVersionSettings &b4);
    void on_actionSong_Counter_triggered();
    void updateEditActions();
    void on_actionNew_triggered();
    void on_actionEdit_triggered();
    void on_actionCopy_triggered();
    void on_actionDelete_triggered();
    void updateWindowText();
    void showBible();
    void showSong(int currentRow);
    void showAnnounce(int currentRow);
    void showPicture(int currentRow);
    void showVideo();

    void retranslateUis();
    void createLanguageActions();
    void switchLanguage(QAction *action);
    void on_actionDonate_triggered();
    void on_action_Help_triggered();
    void on_actionManage_Database_triggered();
    void on_actionAbout_triggered();
    void on_actionSettings_triggered();
    void newSong();
    void copySong();
    void editSong();
    void deleteSong();
    void newSlideShow();
    void editSlideShow();
    void deleteSlideShow();
    void removeMediaFromLibrary();
    void newAnnouncement();
    void editAnnouncement();
    void copyAnnouncement();
    void deleteAnnoucement();
    void on_actionShow_triggered();
    void on_actionHide_triggered();
    void on_actionClose_triggered();
    void setSongList(Song song, int row);
    void setAnnounceText(Announcement announce, int row);
    void setChapterList(QStringList chapter_list, QString caption, const QList<int> &rows);
    void setPictureList(QList<SlideShowItem> &image_list, int row, QString name);
    void setVideo(VideoInfo &video);
    void playVideo();
    void pauseVideo();
    void stopVideo();
    void setVideoPosition(qint64 position);
    void videoStopped();

    void onLiveIndexChanged();
    void onMultiVerseChanged();
    void onWorkspaceAction(const QString &action);
    void onWorkspaceTransport(const QString &what, qint64 value);
    void onScheduleAddRequested(const QString &kind);

    void on_actionPrint_triggered();
    void on_actionPrintSchedule_triggered();

    void nextSlide();
    void prevSlide();

    // schedule functions
    void on_actionScheduleAdd_triggered();
    void on_actionScheduleRemove_triggered();
    void on_actionScheduleClear_triggered();
    void addToShcedule(BibleHistory &b);
    void addToShcedule(Song &s);
    void addToShcedule(SlideShow &s);
    void addToShcedule(VideoInfo &v);
    void addToShcedule(Announcement &a);
    void reloadShceduleList();
    void openScheduleEntry(int row, bool goLive);
    void on_actionNewSchedule_triggered();
    void on_actionOpenSchedule_triggered();
    void on_actionSaveSchedule_triggered();
    void on_actionSaveScheduleAs_triggered();
    void on_actionCloseSchedule_triggered();
    void openSchedule();
    void saveSchedule(bool overWrite);
    void saveScheduleNew(QSqlQuery &q);
    void saveScheduleItemNew(QSqlQuery &q, int scid, const BibleHistory &b);
    void saveScheduleItemNew(QSqlQuery &q, int scid, const Song &s);
    void saveScheduleItemNew(QSqlQuery &q, int scid, const SlideShow &s);
    void saveScheduleItemNew(QSqlQuery &q, int scid, const VideoInfo &v);
    void saveScheduleItemNew(QSqlQuery &q, int scid, const Announcement &a);
    void saveScheduleUpdate(QSqlQuery &q);
    void saveScheduleItemUpdate(QSqlQuery &q, int scid, const BibleHistory &b);
    void saveScheduleItemUpdate(QSqlQuery &q, int scid, const Song &s);
    void saveScheduleItemUpdate(QSqlQuery &q, int scid, const SlideShow &s);
    void saveScheduleItemUpdate(QSqlQuery &q, int scid, const VideoInfo &v);
    void saveScheduleItemUpdate(QSqlQuery &q, int scid, const Announcement &a);
    void openScheduleItem(QSqlQuery &q, const int scid, BibleHistory &b);
    void openScheduleItem(QSqlQuery &q, const int scid, Song &s);
    void openScheduleItem(QSqlQuery &q, const int scid, SlideShow &s);
    void openScheduleItem(QSqlQuery &q, const int scid, VideoInfo &v);
    void openScheduleItem(QSqlQuery &q, const int scid, Announcement &a);

    void on_actionClear_triggered();

    void on_actionCloseDisplay_triggered();

protected:
    void closeEvent(QCloseEvent *event);
    void keyPressEvent(QKeyEvent *event);
};

#endif // SOFTPROJECTOR_HPP
