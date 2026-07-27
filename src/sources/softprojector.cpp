/***************************************************************************
//
//    SoftProjector - an open source media projection software
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

#include <QWidget>
#include <QQuickWidget>
#include <QQmlContext>
#include <QQmlEngine>
#include <QQuickItem>
#include <QActionGroup>
#include <QScreen>

#include "../headers/softprojector.hpp"
#include "../headers/aboutdialog.hpp"
#include "../headers/editannouncementdialog.hpp"
#include "../headers/lucideprovider.hpp"
#include "../headers/thumbprovider.hpp"
#include "../headers/spfunctions.hpp"

namespace
{
// The nav rail keys, in the order the old tab bar used, so anything that
// still thinks in tab indexes keeps working.
const char *const kModuleKeys[] = { "bible", "songs", "pictures", "media", "announce" };

int moduleIndex(const QString &key)
{
    for (int i = 0; i < 5; ++i)
        if (key == QLatin1String(kModuleKeys[i]))
            return i;
    return 0;
}

QVariantList toRows(const QStringList &items)
{
    QVariantList rows;
    for (int i = 0; i < items.count(); ++i)
    {
        QVariantMap m;
        m.insert(QStringLiteral("text"), items.at(i));
        m.insert(QStringLiteral("trailing"), QString::number(i + 1));
        rows.append(m);
    }
    return rows;
}
}

SoftProjector::SoftProjector(QWidget *parent)
    : QMainWindow(parent)
{
    version_string = "2.2";
    QCoreApplication::setApplicationVersion(version_string);

    // Load settings
    mySettings.loadSettings();
    theme.setThemeId(mySettings.general.currentThemeId);
    theme.loadTheme();
    // Reset current theme id if initial was 0
    mySettings.general.currentThemeId = theme.getThemeId();

    // Update Themes Bible Versions
    theme.bible.versions = mySettings.bibleSets;
    theme.bible2.versions = mySettings.bibleSets2;
    theme.bible3.versions = mySettings.bibleSets3;
    theme.bible4.versions = mySettings.bibleSets4;

    // Setting up the Display Screens
    pds1 = new ProjectorDisplayScreen();
    pds2 = new ProjectorDisplayScreen();
    pds3 = new ProjectorDisplayScreen();
    pds4 = new ProjectorDisplayScreen();

    editWidget = new EditWidget;
    manageDialog = new ManageDataDialog(this);
    settingsDialog = new SettingsDialog(this);
    helpDialog = new HelpDialog();

    workspace = new Workspace(this);
    displayVisible = false;
    showing = false;
    new_list = false;
    pType = BIBLE;
    is_schedule_saved = true;

    setupQuickView();

    languagePath = qApp->applicationDirPath() + QString(QDir::separator())
                   + "translations" + QString(QDir::separator());
    createLanguageActions();

    // Apply Settings
    applySetting(mySettings.general, theme, mySettings.slideSets, mySettings.bibleSets,
                 mySettings.bibleSets2, mySettings.bibleSets3, mySettings.bibleSets4);

    positionDisplayWindow();

    workspace->reloadAll();

    // ── Workspace → window ──────────────────────────────────────────────────
    connect(workspace, &Workspace::requestBibleLive, this, &SoftProjector::setChapterList);
    connect(workspace, &Workspace::requestSongLive, this,
            [this](const Song &s, int stanza) {
                Song song = s;
                setSongList(song, stanza);
                countSongUse(song);
            });
    connect(workspace, &Workspace::requestAnnounceLive, this,
            [this](const Announcement &a, int slide) {
                Announcement an = a;
                setAnnounceText(an, slide);
            });
    connect(workspace, &Workspace::requestSlideShowLive, this,
            [this](const SlideShow &s, int slide) {
                SlideShow show = s;
                setPictureList(show.slides, slide, show.name);
            });
    connect(workspace, &Workspace::requestMediaLive, this,
            [this](const VideoInfo &v) {
                VideoInfo video = v;
                setVideo(video);
            });
    connect(workspace, &Workspace::requestAction, this, &SoftProjector::onWorkspaceAction);
    connect(workspace, &Workspace::requestTransport, this, &SoftProjector::onWorkspaceTransport);
    connect(workspace, &Workspace::requestScheduleAdd, this, &SoftProjector::onScheduleAddRequested);
    connect(workspace, &Workspace::liveIndexChanged, this, &SoftProjector::onLiveIndexChanged);
    connect(workspace, &Workspace::multiVerseChanged, this, &SoftProjector::onMultiVerseChanged);
    connect(workspace, &Workspace::moduleChanged, this, &SoftProjector::updateEditActions);

    // ── Editors → workspace ─────────────────────────────────────────────────
    connect(editWidget, SIGNAL(updateSongFromDatabase(int,int)), workspace, SLOT(reloadSongs()));
    connect(editWidget, SIGNAL(addedNew(Song,int)), workspace, SLOT(reloadSongs()));
    connect(manageDialog, SIGNAL(setMainArrowCursor()), this, SLOT(setArrowCursor()));
    connect(manageDialog, SIGNAL(setMainWaitCursor()), this, SLOT(setWaitCursor()));
    connect(languageGroup, SIGNAL(triggered(QAction*)), this, SLOT(switchLanguage(QAction*)));

    // ── Display screens ─────────────────────────────────────────────────────
    connect(pds1, SIGNAL(exitSlide()), this, SLOT(on_actionHide_triggered()));
    connect(pds1, SIGNAL(nextSlide()), this, SLOT(nextSlide()));
    connect(pds1, SIGNAL(prevSlide()), this, SLOT(prevSlide()));
    connect(pds1, qOverload<qint64>(&ProjectorDisplayScreen::videoPositionChanged), this,
            [this](qint64 pos) {
                workspace->setTransport(workspace->mediaPlaying(), pos, workspace->mediaDuration());
            });
    connect(pds1, qOverload<qint64>(&ProjectorDisplayScreen::videoDurationChanged), this,
            [this](qint64 dur) {
                workspace->setTransport(workspace->mediaPlaying(), workspace->mediaPosition(), dur);
            });
    connect(pds1, qOverload<QMediaPlayer::PlaybackState>(&ProjectorDisplayScreen::videoPlaybackStateChanged), this,
            [this](QMediaPlayer::PlaybackState s) {
                workspace->setTransport(s == QMediaPlayer::PlayingState,
                                        workspace->mediaPosition(), workspace->mediaDuration());
            });
    connect(pds1, SIGNAL(videoStopped()), this, SLOT(videoStopped()));

    connect(settingsDialog, SIGNAL(updateSettings(GeneralSettings&,Theme&,SlideShowSettings&,
                                                  BibleVersionSettings&,BibleVersionSettings&,
                                                  BibleVersionSettings&,BibleVersionSettings&)),
            this, SLOT(updateSetting(GeneralSettings&,Theme&,SlideShowSettings&,
                                     BibleVersionSettings&,BibleVersionSettings&,
                                     BibleVersionSettings&,BibleVersionSettings&)));
    connect(settingsDialog, SIGNAL(positionsDisplayWindow()), this, SLOT(positionDisplayWindow()));
    connect(settingsDialog, SIGNAL(updateScreen()), this, SLOT(updateScreen()));

    publishActionState();
    updateWindowText();
}

SoftProjector::~SoftProjector()
{
    saveSettings();
    delete editWidget;
    delete manageDialog;
    delete pds1;
    delete pds2;
    delete pds3;
    delete pds4;
    delete languageGroup;
    delete settingsDialog;
    delete helpDialog;
}

// ─────────────────────────────────────────────────────────────────────────────
//  QML host
// ─────────────────────────────────────────────────────────────────────────────

void SoftProjector::setupQuickView()
{
    quickView = new QQuickWidget(this);
    quickView->setResizeMode(QQuickWidget::SizeRootObjectToView);

    // A transparent-free opaque surface: the QML paints its own background and
    // the widget behind it never shows through during a resize.
    quickView->setClearColor(QColor("#0B1120"));

    quickView->engine()->addImageProvider(QStringLiteral("lucide"), new LucideProvider);
    quickView->engine()->addImageProvider(QStringLiteral("spthumb"), ThumbProvider::instance());
    quickView->rootContext()->setContextProperty(QStringLiteral("App"), workspace);

    quickView->setSource(QUrl(QStringLiteral("qrc:/qt/qml/SoftProjector/Ui/AppShell.qml")));

    if (quickView->status() == QQuickWidget::Error)
    {
        const auto errors = quickView->errors();
        for (const QQmlError &e : errors)
            qWarning("QML: %s", qPrintable(e.toString()));
    }

    setCentralWidget(quickView);
    setMinimumSize(1100, 680);
    setWindowIcon(QIcon(QStringLiteral(":/icons/icons/softProjector.png")));
}

// Convert a plain list of slide strings into the row maps the QML lists expect,
// and hand the whole live state over in one go.
void SoftProjector::publishLiveState(const QString &kind, const QString &title,
                                     const QStringList &slides, int row, const QString &notes)
{
    workspace->setLiveContent(kind, title, toRows(slides), row, notes);
}

void SoftProjector::publishActionState()
{
    // Show is only meaningful once something has actually been sent across;
    // with an empty live column there is nothing to put on the projector.
    const bool hasContent = workspace->live();

    workspace->setActionState(hasContent && (!showing || workspace->multiVerse()),
                              hasContent && showing,
                              hasContent && showing
                                  && (pType == BIBLE || pType == SONG || pType == ANNOUCEMENT));
    workspace->setDisplayState(displayVisible, QApplication::screens().count());
}

// Song usage statistics live in the SongCounter dialog's model; bumping the
// count needs an instance, but not a visible one.
void SoftProjector::countSongUse(const Song &song)
{
    SongCounter counter(this, cur_locale);
    counter.addSongCount(song);
}

int SoftProjector::currentShowRow() const   { return workspace->liveIndex(); }
int SoftProjector::showCount() const        { return workspace->liveSlides().count(); }
QList<int> SoftProjector::selectedShowRows() const { return workspace->selectedLiveRows(); }

void SoftProjector::setShowRow(int row)
{
    if (row < 0 || row >= showCount())
        return;
    workspace->setLiveIndex(row);
    updateScreen();
}

void SoftProjector::onLiveIndexChanged()
{
    // The operator moved the highlight in the live column. In multi-verse mode
    // the projector waits for an explicit Show; otherwise it follows along.
    if (!workspace->multiVerse())
        updateScreen();
    else
        publishActionState();
}

void SoftProjector::onMultiVerseChanged()
{
    publishActionState();
    if (!workspace->multiVerse())
        updateScreen();
}

// ─────────────────────────────────────────────────────────────────────────────
//  Display screens
// ─────────────────────────────────────────────────────────────────────────────

void SoftProjector::positionDisplayWindow()
{
    // Position the display window as needed (including setting "always on top"
    // flag, showing full screen / normal mode, and positioning it on the right
    // screen)

    if (mySettings.general.displayIsOnTop)
    {
        pds1->setWindowFlags(Qt::WindowStaysOnTopHint);
        pds2->setWindowFlags(Qt::WindowStaysOnTopHint);
        pds3->setWindowFlags(Qt::WindowStaysOnTopHint);
        pds4->setWindowFlags(Qt::WindowStaysOnTopHint);
    }

    const auto siblings = QApplication::primaryScreen()->virtualSiblings();

    if (siblings.count() > 1)
    {
        pds1->setGeometry(siblings.at(qBound(0, mySettings.general.displayScreen,
                                             siblings.count() - 1))->geometry());
        pds1->setCursor(Qt::BlankCursor);
        pds1->resetImGenSize();
        pds1->renderPassiveText(theme.passive.backgroundPix, theme.passive.useBackground);
        pds1->setControlsVisible(false);

        if (mySettings.general.displayOnStartUp)
        {
            pds1->showFullScreen();
            displayVisible = true;
        }

        struct { int index; ProjectorDisplayScreen *screen; TextSettings *sets; bool *flag; }
        extras[] = {
            { mySettings.general.displayScreen2, pds2, &theme.passive2, &hasDisplayScreen2 },
            { mySettings.general.displayScreen3, pds3, &theme.passive3, &hasDisplayScreen3 },
            { mySettings.general.displayScreen4, pds4, &theme.passive4, &hasDisplayScreen4 }
        };

        for (auto &e : extras)
        {
            if (e.index >= 0 && e.index < siblings.count())
            {
                *e.flag = true;
                e.screen->setGeometry(siblings.at(e.index)->geometry());
                e.screen->resetImGenSize();
                e.screen->setCursor(Qt::BlankCursor);
                e.screen->renderPassiveText(e.sets->backgroundPix, e.sets->useBackground);
                e.screen->setControlsVisible(false);
                if (mySettings.general.displayOnStartUp)
                    e.screen->showFullScreen();
            }
            else
            {
                *e.flag = false;
                e.screen->hide();
            }
        }

        isSingleScreen = false;
    }
    else
    {
        // Single monitor only: do not show on start up. It will be shown only
        // when items are sent to the projector.
        pds1->setGeometry(siblings.at(0)->geometry());
        pds1->resetImGenSize();
        showDisplayScreen(false);
        isSingleScreen = true;
        hasDisplayScreen2 = false;
        hasDisplayScreen3 = false;
        hasDisplayScreen4 = false;
    }

    publishActionState();
}

void SoftProjector::showDisplayScreen(bool show)
{
    if (show)
    {
        pds1->showFullScreen();
        pds1->positionControls(mySettings.general.displayControls);
        pds1->setControlsVisible(true);
        displayVisible = true;
    }
    else
    {
        pds1->hide();
        QPixmap p;
        pds1->renderPassiveText(p, false);
        displayVisible = false;
    }
    publishActionState();
}

void SoftProjector::on_actionCloseDisplay_triggered()
{
    displayVisible = !displayVisible;

    ProjectorDisplayScreen *all[] = { pds1, pds2, pds3, pds4 };
    const bool has[] = { true, hasDisplayScreen2, hasDisplayScreen3, hasDisplayScreen4 };

    for (int i = 0; i < 4; ++i)
    {
        if (!has[i])
            continue;
        if (displayVisible)
            all[i]->showFullScreen();
        else
            all[i]->hide();
    }

    if (!displayVisible)
        showing = false;

    publishActionState();
}

void SoftProjector::on_actionClear_triggered()
{
    pds1->renderNotText();
    if (hasDisplayScreen2) pds2->renderNotText();
    if (hasDisplayScreen3) pds3->renderNotText();
    if (hasDisplayScreen4) pds4->renderNotText();

    workspace->setActionState(true, workspace->canHide(), false);
}

// ─────────────────────────────────────────────────────────────────────────────
//  Settings
// ─────────────────────────────────────────────────────────────────────────────

void SoftProjector::saveSettings()
{
    mySettings.spMain.isWindowMaximized = this->isMaximized();
    mySettings.spMain.uiTranslation = cur_locale.isEmpty() ? QStringLiteral("en") : cur_locale;
    mySettings.saveSettings();
    theme.saveThemeUpdate();
}

void SoftProjector::updateSetting(GeneralSettings &g, Theme &t, SlideShowSettings &ssets,
                                  BibleVersionSettings &bsets, BibleVersionSettings &bsets2,
                                  BibleVersionSettings &bsets3, BibleVersionSettings &bsets4)
{
    mySettings.general = g;
    mySettings.slideSets = ssets;
    mySettings.bibleSets = bsets;
    mySettings.bibleSets2 = bsets2;
    mySettings.bibleSets3 = bsets3;
    mySettings.bibleSets4 = bsets4;
    mySettings.saveSettings();
    theme = t;

    workspace->applyBibleSettings(mySettings.bibleSets);
    workspace->applySlideSettings(mySettings.slideSets);

    theme.bible.versions = mySettings.bibleSets;
    theme.bible2.versions = mySettings.bibleSets2;
    theme.bible3.versions = mySettings.bibleSets3;
    theme.bible4.versions = mySettings.bibleSets4;
}

void SoftProjector::applySetting(GeneralSettings &g, Theme &t, SlideShowSettings &s,
                                 BibleVersionSettings &b1, BibleVersionSettings &b2,
                                 BibleVersionSettings &b3, BibleVersionSettings &b4)
{
    updateSetting(g, t, s, b1, b2, b3, b4);

    if (mySettings.spMain.isWindowMaximized)
        this->setWindowState(Qt::WindowMaximized);

    cur_locale = mySettings.spMain.uiTranslation.isEmpty()
                 ? QStringLiteral("en") : mySettings.spMain.uiTranslation;

    const QList<QAction *> la = languageGroup->actions();
    for (QAction *a : la)
        if (a->data().toString() == cur_locale)
            a->setChecked(true);

    retranslateUis();
}

void SoftProjector::createLanguageActions()
{
    languageGroup = new QActionGroup(this);
    languageGroup->setExclusive(true);

    QVariantList langs;

    auto addLanguage = [&](const QString &code, const QString &name) {
        QAction *a = new QAction(name, languageGroup);
        a->setCheckable(true);
        a->setData(code);
        languageGroup->addAction(a);

        QVariantMap m;
        m.insert(QStringLiteral("code"), code);
        m.insert(QStringLiteral("name"), name);
        langs.append(m);
    };

    addLanguage(QStringLiteral("en"), QStringLiteral("English"));

    QDir dir(languagePath);
    const QStringList files = dir.entryList(QStringList("softpro_*.qm"));
    for (const QString &f : files)
    {
        QString code = f;
        code.remove(0, code.indexOf('_') + 1);
        code.chop(3);

        QLocale locale(code);
        QString name = QLocale::languageToString(locale.language());
        if (name.isEmpty() || name == QLatin1String("C"))
            name = code;
        addLanguage(code, name);
    }

    workspace->setLanguageList(langs, cur_locale.isEmpty() ? QStringLiteral("en") : cur_locale);
}

void SoftProjector::retranslateUis()
{
    editWidget->retranslateUis();
    updateWindowText();
}

// ─────────────────────────────────────────────────────────────────────────────
//  Live content
// ─────────────────────────────────────────────────────────────────────────────

void SoftProjector::setAnnounceText(Announcement announce, int row)
{
    currentAnnounce = announce;
    pType = ANNOUCEMENT;
    showing = true;
    new_list = true;

    publishLiveState(QStringLiteral("announce"), currentAnnounce.title,
                     currentAnnounce.getAnnounceList(), row);

    new_list = false;
    updateScreen();
}

void SoftProjector::setSongList(Song song, int row)
{
    current_song = song;
    current_song_verse = row;
    pType = SONG;
    showing = true;
    new_list = true;

    publishLiveState(QStringLiteral("song"), song.title, song.getSongTextList(), row,
                     song.notes.isEmpty() ? QString()
                                          : tr("Notes:", "Notes to songs") + " " + song.notes);

    new_list = false;
    updateScreen();
}

void SoftProjector::setChapterList(QStringList chapter_list, QString caption,
                                   const QList<int> &rows)
{
    // Called to show a bible verse from a chapter in the preview list
    pType = BIBLE;
    showing = true;
    new_list = true;

    publishLiveState(QStringLiteral("bible"), caption, chapter_list,
                     rows.isEmpty() ? 0 : rows.first());
    workspace->setMultiVerse(rows.count() > 1);
    workspace->setLiveSelection(rows);

    new_list = false;
    updateScreen();
}

void SoftProjector::setPictureList(QList<SlideShowItem> &image_list, int row, QString name)
{
    pType = PICTURE;
    showing = true;
    new_list = true;
    pictureShowList = image_list;

    QStringList labels;
    for (int i = 0; i < image_list.count(); ++i)
        labels.append(image_list.at(i).name.isEmpty() ? tr("Slide %1").arg(i + 1)
                                                      : image_list.at(i).name);

    publishLiveState(QStringLiteral("picture"), name, labels, row);

    new_list = false;
    updateScreen();
}

void SoftProjector::setVideo(VideoInfo &video)
{
    pType = VIDEO;
    currentVideo = video;
    showing = true;
    new_list = true;

    publishLiveState(QStringLiteral("video"), currentVideo.fileName, QStringList(), 0);

    new_list = false;
    updateScreen();
}

void SoftProjector::updateScreen()
{
    const int currentRow = currentShowRow();

    if (!showing)
    {
        pds1->renderPassiveText(theme.passive.backgroundPix, theme.passive.useBackground);

        if (isSingleScreen)
            showDisplayScreen(false);

        struct { bool has; ProjectorDisplayScreen *screen; TextSettings *sets; }
        extras[] = {
            { hasDisplayScreen2, pds2, &theme.passive2 },
            { hasDisplayScreen3, pds3, &theme.passive3 },
            { hasDisplayScreen4, pds4, &theme.passive4 }
        };
        for (auto &e : extras)
        {
            if (!e.has)
                continue;
            if (!e.sets->useDisp1settings)
                e.screen->renderPassiveText(e.sets->backgroundPix, e.sets->useBackground);
            else
                e.screen->renderPassiveText(theme.passive.backgroundPix, theme.passive.useBackground);
        }

        stopVideo();
        publishActionState();
    }
    else if ((currentRow >= 0 || pType == VIDEO) && !new_list)
    {
        if (isSingleScreen)
        {
            if (pds1->isHidden())
                showDisplayScreen(true);
        }
        else if (pds1->isHidden() || !displayVisible)
        {
            on_actionCloseDisplay_triggered();
        }

        switch (pType)
        {
        case BIBLE:      showBible();               break;
        case SONG:       showSong(currentRow);      break;
        case ANNOUCEMENT: showAnnounce(currentRow); break;
        case PICTURE:    showPicture(currentRow);   break;
        case VIDEO:      showVideo();               break;
        default: break;
        }

        publishActionState();
    }
}

void SoftProjector::showBible()
{
    const QList<int> currentRows = selectedShowRows();
    Bible *bible = workspace->bibleEngine();

    pds1->renderBibleText(bible->getCurrentVerseAndCaption(currentRows, theme.bible,
                                                           mySettings.bibleSets),
                          theme.bible);

    struct { bool has; ProjectorDisplayScreen *screen; BibleSettings *sets;
             BibleVersionSettings *versions; }
    extras[] = {
        { hasDisplayScreen2, pds2, &theme.bible2, &mySettings.bibleSets2 },
        { hasDisplayScreen3, pds3, &theme.bible3, &mySettings.bibleSets3 },
        { hasDisplayScreen4, pds4, &theme.bible4, &mySettings.bibleSets4 }
    };

    for (auto &e : extras)
    {
        if (!e.has)
            continue;
        if (!e.sets->useDisp1settings)
            e.screen->renderBibleText(
                bible->getCurrentVerseAndCaption(currentRows, *e.sets, *e.versions), *e.sets);
        else
            e.screen->renderBibleText(
                bible->getCurrentVerseAndCaption(currentRows, theme.bible, mySettings.bibleSets),
                theme.bible);
    }
}

// ─────────────────────────────────────────────────────────────────────────────
//  Slide stepping
// ─────────────────────────────────────────────────────────────────────────────

void SoftProjector::nextSlide()
{
    int current = currentShowRow();
    if (workspace->multiVerse())
    {
        const QList<int> sel = selectedShowRows();
        if (!sel.isEmpty())
            current = sel.last();
        if (current < showCount() - 1)
            workspace->setMultiVerse(false);
    }
    if (current < showCount() - 1)
        setShowRow(current + 1);
}

void SoftProjector::prevSlide()
{
    int current = currentShowRow();
    if (workspace->multiVerse())
    {
        const QList<int> sel = selectedShowRows();
        if (!sel.isEmpty())
            current = sel.first();
        if (current > 0)
            workspace->setMultiVerse(false);
    }
    if (current > 0)
        setShowRow(current - 1);
}

void SoftProjector::keyPressEvent(QKeyEvent *event)
{
    switch (event->key())
    {
    case Qt::Key_Left:
    case Qt::Key_Back:
        prevSlide();
        break;
    case Qt::Key_Right:
    case Qt::Key_Forward:
    case Qt::Key_Return:
    case Qt::Key_Enter:
        nextSlide();
        break;
    default:
        QMainWindow::keyPressEvent(event);
    }
}

// ─────────────────────────────────────────────────────────────────────────────
//  Editing
// ─────────────────────────────────────────────────────────────────────────────

void SoftProjector::updateEditActions()
{
    // The QML enables its own buttons from the workspace properties; nothing to
    // push here beyond keeping the projector action state honest.
    publishActionState();
}

void SoftProjector::on_actionNew_triggered()
{
    switch (moduleIndex(workspace->module()))
    {
    case 0: break;                       // Bible has nothing to create
    case 1: newSong();          break;
    case 2: newSlideShow();     break;
    case 3: break;                       // Media is added from the panel
    case 4: newAnnouncement();  break;
    }
}

void SoftProjector::on_actionEdit_triggered()
{
    switch (moduleIndex(workspace->module()))
    {
    case 1: editSong();         break;
    case 2: editSlideShow();    break;
    case 4: editAnnouncement(); break;
    default: break;
    }
}

void SoftProjector::on_actionCopy_triggered()
{
    switch (moduleIndex(workspace->module()))
    {
    case 1: copySong();         break;
    case 4: copyAnnouncement(); break;
    default: break;
    }
}

void SoftProjector::on_actionDelete_triggered()
{
    switch (moduleIndex(workspace->module()))
    {
    case 1: deleteSong();       break;
    case 2: deleteSlideShow();  break;
    case 3: removeMediaFromLibrary(); break;
    case 4: deleteAnnoucement(); break;
    default: break;
    }
}

void SoftProjector::editSong()
{
    if (!editWidget->isHidden())
    {
        QMessageBox ms(this);
        ms.setWindowTitle(tr("Cannot edit song"));
        ms.setText(tr("Another song is already been edited."));
        ms.setInformativeText(tr("Please save and/or close current edited song "
                                 "before edited a different song."));
        ms.setIcon(QMessageBox::Information);
        ms.exec();
        return;
    }

    Song song = workspace->currentSong();
    if (!song.isValid())
        return;

    editWidget->show();
    editWidget->setEdit(song);
    editWidget->activateWindow();
}

void SoftProjector::copySong()
{
    if (!editWidget->isHidden())
    {
        QMessageBox ms(this);
        ms.setWindowTitle(tr("Cannot copy song"));
        ms.setText(tr("Another song is already been edited."));
        ms.setIcon(QMessageBox::Information);
        ms.exec();
        return;
    }

    Song song = workspace->currentSong();
    if (!song.isValid())
        return;

    editWidget->show();
    editWidget->setCopy(song);
    editWidget->activateWindow();
}

void SoftProjector::deleteSong()
{
    Song song = workspace->currentSong();
    if (!song.isValid())
        return;

    QMessageBox mb(this);
    mb.setWindowTitle(tr("Delete song?"));
    mb.setText(tr("Delete \"%1\"?").arg(song.title));
    mb.setInformativeText(tr("This cannot be undone."));
    mb.setIcon(QMessageBox::Question);
    mb.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
    mb.setDefaultButton(QMessageBox::No);

    if (mb.exec() == QMessageBox::Yes)
    {
        SongDatabase db;
        db.deleteSong(song.songID);
        workspace->reloadSongs();
    }
}

void SoftProjector::newSlideShow()
{
    SlideShowEditor editor(this);
    editor.setSettings(mySettings.slideSets);
    editor.setSlideShow(SlideShow());
    if (editor.exec() == QDialog::Accepted)
        workspace->reloadSlideshows();
}

void SoftProjector::editSlideShow()
{
    SlideShow show = workspace->currentSlideShow();
    if (show.slideShowId <= 0)
        return;

    SlideShowEditor editor(this);
    editor.setSettings(mySettings.slideSets);
    editor.setSlideShow(show);
    if (editor.exec() == QDialog::Accepted)
        workspace->reloadSlideshows();
}

void SoftProjector::deleteSlideShow()
{
    SlideShow show = workspace->currentSlideShow();
    if (show.slideShowId <= 0)
        return;

    QMessageBox mb(this);
    mb.setWindowTitle(tr("Delete slide show?"));
    mb.setText(tr("Delete \"%1\" and all its slides?").arg(show.name));
    mb.setIcon(QMessageBox::Question);
    mb.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
    mb.setDefaultButton(QMessageBox::No);

    if (mb.exec() == QMessageBox::Yes)
    {
        QSqlQuery sq;
        sq.exec(QString("DELETE FROM Slides WHERE ss_id = %1").arg(show.slideShowId));
        sq.exec(QString("DELETE FROM SlideShows WHERE id = %1").arg(show.slideShowId));
        workspace->reloadSlideshows();
    }
}

void SoftProjector::removeMediaFromLibrary()
{
    workspace->mediaRemoveSelected();
}

void SoftProjector::newAnnouncement()
{
    EditAnnouncementDialog dialog(this);
    dialog.setNewAnnouce();
    if (dialog.exec() == QDialog::Accepted)
        workspace->reloadAnnouncements();
}

void SoftProjector::editAnnouncement()
{
    Announcement a = workspace->currentAnnouncement();
    if (a.idNum <= 0)
        return;

    EditAnnouncementDialog dialog(this);
    dialog.setEditAnnouce(a);
    if (dialog.exec() == QDialog::Accepted)
        workspace->reloadAnnouncements();
}

void SoftProjector::copyAnnouncement()
{
    Announcement a = workspace->currentAnnouncement();
    if (a.idNum <= 0)
        return;

    EditAnnouncementDialog dialog(this);
    a.title = tr("%1 (copy)").arg(a.title);
    dialog.setCopyAnnouce(a);
    if (dialog.exec() == QDialog::Accepted)
        workspace->reloadAnnouncements();
}

void SoftProjector::deleteAnnoucement()
{
    Announcement a = workspace->currentAnnouncement();
    if (a.idNum <= 0)
        return;

    QMessageBox mb(this);
    mb.setWindowTitle(tr("Delete announcement?"));
    mb.setText(tr("Delete \"%1\"?").arg(a.title));
    mb.setIcon(QMessageBox::Question);
    mb.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
    mb.setDefaultButton(QMessageBox::No);

    if (mb.exec() == QMessageBox::Yes)
    {
        a.deleteAnnouce();
        workspace->reloadAnnouncements();
    }
}

void SoftProjector::on_actionManage_Database_triggered()
{
    manageDialog->load_songbooks();
    manageDialog->exec();
    workspace->reloadAll();
}

// ─────────────────────────────────────────────────────────────────────────────
//  Printing
// ─────────────────────────────────────────────────────────────────────────────

void SoftProjector::on_actionPrint_triggered()
{
    switch (moduleIndex(workspace->module()))
    {
    case 1:
    {
        Song song = workspace->currentSong();
        if (!song.isValid())
            return;
        PrintPreviewDialog dialog(this);
        dialog.setText(song);
        dialog.exec();
        break;
    }
    case 0:
    {
        PrintPreviewDialog dialog(this);
        dialog.setText(workspace->bibleEngine()->getBibleName(),
                       workspace->currentBibleBook(), workspace->bibleChapterIndex() + 1);
        dialog.exec();
        break;
    }
    default:
        on_actionPrintSchedule_triggered();
    }
}

// ─────────────────────────────────────────────────────────────────────────────
//  Schedule
// ─────────────────────────────────────────────────────────────────────────────

void SoftProjector::reloadShceduleList()
{
    is_schedule_saved = false;
    QFileInfo fi(schedule_file_path);
    QString name = fi.fileName();
    name.remove(".spsc");
    workspace->setScheduleModel(schedule, name, !is_schedule_saved);
    updateWindowText();
}

void SoftProjector::onScheduleAddRequested(const QString &kind)
{
    Q_UNUSED(kind)
    on_actionScheduleAdd_triggered();
}

void SoftProjector::on_actionScheduleAdd_triggered()
{
    switch (moduleIndex(workspace->module()))
    {
    case 0:
    {
        BibleHistory b = workspace->currentBibleRef();
        if (!b.verseIds.isEmpty())
            addToShcedule(b);
        break;
    }
    case 1:
    {
        Song s = workspace->currentSong();
        if (s.isValid())
            addToShcedule(s);
        break;
    }
    case 2:
    {
        SlideShow s = workspace->currentSlideShow();
        if (s.slideShowId > 0)
            addToShcedule(s);
        break;
    }
    case 3:
    {
        VideoInfo v = workspace->currentMedia();
        if (!v.fileName.isEmpty())
            addToShcedule(v);
        break;
    }
    case 4:
    {
        Announcement a = workspace->currentAnnouncement();
        if (a.idNum > 0)
            addToShcedule(a);
        break;
    }
    }

    workspace->setScheduleIndex(schedule.count() - 1);
}

void SoftProjector::on_actionScheduleRemove_triggered()
{
    const int cRow = workspace->scheduleIndex();
    if (cRow >= 0 && cRow < schedule.count())
    {
        schedule.takeAt(cRow);
        reloadShceduleList();
    }
}

void SoftProjector::openScheduleEntry(int row, bool goLive)
{
    if (row < 0 || row >= schedule.count())
        return;

    Schedule s = schedule.at(row);

    if (s.stype == "bible")
    {
        workspace->setModule(QStringLiteral("bible"));
        BibleHistory b = s.bible;
        QString bk;
        int ch = 0, vr = 0;
        Bible *bible = workspace->bibleEngine();
        bible->getVerseRef(b.verseIds, bk, ch, vr);
        const int last = bible->getVerseNumberLast(b.verseIds);

        QList<int> rows;
        for (int v = vr; v <= qMax(vr, last); ++v)
            rows.append(v - 1);

        if (goLive)
        {
            bible->currentIdList = bible->previewIdList;
            setChapterList(bible->getChapter(
                               bible->books.at(bible->getCurrentBookRow(bk)).bookId.toInt(), ch),
                           b.caption, rows);
        }
    }
    else if (s.stype == "song")
    {
        workspace->setModule(QStringLiteral("songs"));
        if (goLive)
        {
            setSongList(s.song, 0);
            countSongUse(s.song);
        }
    }
    else if (s.stype == "slideshow")
    {
        workspace->setModule(QStringLiteral("pictures"));
        if (goLive)
            setPictureList(s.slideshow.slides, 0, s.slideshow.name);
    }
    else if (s.stype == "media")
    {
        workspace->setModule(QStringLiteral("media"));
        if (goLive)
            setVideo(s.media);
    }
    else if (s.stype == "announce")
    {
        workspace->setModule(QStringLiteral("announce"));
        if (goLive)
            setAnnounceText(s.announce, 0);
    }
}

// ─────────────────────────────────────────────────────────────────────────────
//  Action dispatch from the QML shell
// ─────────────────────────────────────────────────────────────────────────────

void SoftProjector::onWorkspaceAction(const QString &action)
{
    if      (action == "show")            on_actionShow_triggered();
    else if (action == "hide")            on_actionHide_triggered();
    else if (action == "clear")           on_actionClear_triggered();
    else if (action == "refreshScreen")   updateScreen();
    else if (action == "nextSlide")       nextSlide();
    else if (action == "prevSlide")       prevSlide();
    else if (action == "toggleDisplay")   on_actionCloseDisplay_triggered();
    else if (action == "settings")        on_actionSettings_triggered();
    else if (action == "manageData")      on_actionManage_Database_triggered();
    else if (action == "help")            on_action_Help_triggered();
    else if (action == "about")           on_actionAbout_triggered();
    else if (action == "donate")          on_actionDonate_triggered();
    else if (action == "songCounter")     on_actionSong_Counter_triggered();
    else if (action == "quit")            on_actionClose_triggered();
    else if (action == "print")           on_actionPrint_triggered();
    else if (action == "printSchedule")   on_actionPrintSchedule_triggered();
    else if (action == "newSchedule")     on_actionNewSchedule_triggered();
    else if (action == "openSchedule")    on_actionOpenSchedule_triggered();
    else if (action == "saveSchedule")    on_actionSaveSchedule_triggered();
    else if (action == "saveScheduleAs")  on_actionSaveScheduleAs_triggered();
    else if (action == "closeSchedule")   on_actionCloseSchedule_triggered();
    else if (action == "scheduleRemove")  on_actionScheduleRemove_triggered();
    else if (action == "scheduleClear")   on_actionScheduleClear_triggered();
    else if (action == "scheduleOpenItem") openScheduleEntry(workspace->scheduleIndex(), true);
    else if (action == "editNew")         on_actionNew_triggered();
    else if (action == "editEdit")        on_actionEdit_triggered();
    else if (action == "editCopy")        on_actionCopy_triggered();
    else if (action == "editDelete")      on_actionDelete_triggered();
    else if (action == "songNew")         newSong();
    else if (action == "songEdit")        editSong();
    else if (action == "songCopy")        copySong();
    else if (action == "songDelete")      deleteSong();
    else if (action == "slideshowNew")    newSlideShow();
    else if (action == "slideshowEdit")   editSlideShow();
    else if (action == "slideshowDelete") deleteSlideShow();
    else if (action == "announceNew")     newAnnouncement();
    else if (action == "announceEdit")    editAnnouncement();
    else if (action == "announceCopy")    copyAnnouncement();
    else if (action == "announceDelete")  deleteAnnoucement();
    else if (action == "focusSearch")     { /* handled inside the QML panels */ }
    else qWarning("Unhandled workspace action: %s", qPrintable(action));
}

void SoftProjector::onWorkspaceTransport(const QString &what, qint64 value)
{
    if      (what == "play")   playVideo();
    else if (what == "pause")  pauseVideo();
    else if (what == "stop")   stopVideo();
    else if (what == "seek")   setVideoPosition(value);
    else if (what == "volume")
    {
        pds1->setVideoVolume(int(value));
        if (hasDisplayScreen2) pds2->setVideoVolume(int(value));
        if (hasDisplayScreen3) pds3->setVideoVolume(int(value));
        if (hasDisplayScreen4) pds4->setVideoVolume(int(value));
    }
    else if (what == "mute")
    {
        pds1->setVideoMuted(value != 0);
        if (hasDisplayScreen2) pds2->setVideoMuted(value != 0);
        if (hasDisplayScreen3) pds3->setVideoMuted(value != 0);
        if (hasDisplayScreen4) pds4->setVideoMuted(value != 0);
    }
}
void SoftProjector::closeEvent(QCloseEvent *event)
{
    if(is_schedule_saved || schedule_file_path.isEmpty())
    {
        QCoreApplication::exit(0);
        event->accept();
    }
    else
    {
        QMessageBox mb(this);
        mb.setWindowTitle(tr("Schedule not saved"));
        mb.setText(tr("Do you want to save current schedule?"));
        mb.setIcon(QMessageBox::Question);
        mb.setStandardButtons(QMessageBox::Save | QMessageBox::Cancel | QMessageBox::Discard);
        mb.setDefaultButton(QMessageBox::Save);
        int ret = mb.exec();

        switch (ret)
        {
        case QMessageBox::Save:
            // Save Schedule and close
            on_actionSaveSchedule_triggered();
            QCoreApplication::exit(0);
            event->accept();
            break;
        case QMessageBox::Cancel:
            // Cancel was clicked, do nothing
            event->ignore();
            break;
        case QMessageBox::Discard:
            // Close without saving
            QCoreApplication::exit(0);
            event->accept();
            break;
        default:
            // should never be reached
            break;
        }
    }
}

void SoftProjector::on_actionClose_triggered()
{
    close();
}

void SoftProjector::playVideo()
{
    pds1->playVideo();
    if(hasDisplayScreen2)
    {
        pds2->playVideo();
    }
    if(hasDisplayScreen3)
    {
        pds3->playVideo();
    }
    if(hasDisplayScreen4)
    {
        pds4->playVideo();
    }
}

void SoftProjector::pauseVideo()
{
    pds1->pauseVideo();
    if(hasDisplayScreen2)
    {
        pds2->pauseVideo();
    }
    if(hasDisplayScreen3)
    {
        pds3->pauseVideo();
    }
    if(hasDisplayScreen4)
    {
        pds4->pauseVideo();
    }
}

void SoftProjector::stopVideo()
{
    pds1->stopVideo();
    if(hasDisplayScreen2)
    {
        pds2->stopVideo();
    }
    if(hasDisplayScreen3)
    {
        pds3->stopVideo();
    }
    if(hasDisplayScreen4)
    {
        pds4->stopVideo();
    }
}

void SoftProjector::setVideoPosition(qint64 position)
{
    pds1->setVideoPosition(position);
    if(hasDisplayScreen2)
    {
        pds2->setVideoPosition(position);
    }
    if(hasDisplayScreen3)
    {
        pds3->setVideoPosition(position);
    }
    if(hasDisplayScreen4)
    {
        pds4->setVideoPosition(position);
    }
}

void SoftProjector::videoStopped()
{
    showing = false;
    updateScreen();
}

void SoftProjector::showSong(int currentRow)
{
    // Get Song Settings
    SongSettings s1 = theme.song;
    SongSettings s2 = theme.song2;
    SongSettings s3 = theme.song3;
    SongSettings s4 = theme.song4;

    // Apply Song specific settings if there is one
    if(current_song.usePrivateSettings)
    {
        current_song.getSettings(s1);
        current_song.getSettings(s2);
        current_song.getSettings(s3);
        current_song.getSettings(s4);
    }

    pds1->renderSongText(current_song.getStanza(currentRow),s1);
    if(hasDisplayScreen2)
    {
        if(!theme.song2.useDisp1settings)
        {
            pds2->renderSongText(current_song.getStanza(currentRow),s2);
        }
        else
        {
            pds2->renderSongText(current_song.getStanza(currentRow),s1);
        }
    }
    if(hasDisplayScreen3)
    {
        if(!theme.song3.useDisp1settings)
        {
            pds3->renderSongText(current_song.getStanza(currentRow),s3);
        }
        else
        {
            pds3->renderSongText(current_song.getStanza(currentRow),s1);
        }
    }
    if(hasDisplayScreen4)
    {
        if(!theme.song4.useDisp1settings)
        {
            pds4->renderSongText(current_song.getStanza(currentRow),s4);
        }
        else
        {
            pds4->renderSongText(current_song.getStanza(currentRow),s1);
        }
    }

}

void SoftProjector::showAnnounce(int currentRow)
{
    pds1->renderAnnounceText(currentAnnounce.getAnnounceSlide(currentRow),theme.announce);
    if(hasDisplayScreen2)
    {
        if(!theme.announce2.useDisp1settings)
        {
            pds2->renderAnnounceText(currentAnnounce.getAnnounceSlide(currentRow),theme.announce2);
        }
        else
        {
            pds2->renderAnnounceText(currentAnnounce.getAnnounceSlide(currentRow),theme.announce);
        }
    }
    if(hasDisplayScreen3)
    {
        if(!theme.announce3.useDisp1settings)
        {
            pds3->renderAnnounceText(currentAnnounce.getAnnounceSlide(currentRow),theme.announce3);
        }
        else
        {
            pds3->renderAnnounceText(currentAnnounce.getAnnounceSlide(currentRow),theme.announce);
        }
    }
    if(hasDisplayScreen4)
    {
        if(!theme.announce4.useDisp1settings)
        {
            pds4->renderAnnounceText(currentAnnounce.getAnnounceSlide(currentRow),theme.announce4);
        }
        else
        {
            pds4->renderAnnounceText(currentAnnounce.getAnnounceSlide(currentRow),theme.announce);
        }
    }
}

void SoftProjector::showPicture(int currentRow)
{
    pds1->renderSlideShow(pictureShowList.at(currentRow).image,mySettings.slideSets);
    if(hasDisplayScreen2)
    {
        pds2->renderSlideShow(pictureShowList.at(currentRow).image,mySettings.slideSets);
    }
    if(hasDisplayScreen3)
    {
        pds3->renderSlideShow(pictureShowList.at(currentRow).image,mySettings.slideSets);
    }
    if(hasDisplayScreen4)
    {
        pds4->renderSlideShow(pictureShowList.at(currentRow).image,mySettings.slideSets);
    }
}

void SoftProjector::showVideo()
{
    pds1->renderVideo(currentVideo);
    pds1->setVideoVolume(100);
    if(hasDisplayScreen2)
    {
        pds2->setVideoVolume(0);
        pds2->setVideoMuted(true);
        pds2->renderVideo(currentVideo);
    }
    if(hasDisplayScreen3)
    {
        pds3->setVideoVolume(0);
        pds3->setVideoMuted(true);
        pds3->renderVideo(currentVideo);
    }
    if(hasDisplayScreen4)
    {
        pds4->setVideoVolume(0);
        pds4->setVideoMuted(true);
        pds4->renderVideo(currentVideo);
    }
}

void SoftProjector::on_actionShow_triggered()
{
    showing = true;
    updateScreen();
}

void SoftProjector::on_actionHide_triggered()
{
    showing = false;
    updateScreen();
}

void SoftProjector::on_actionSettings_triggered()
{
    settingsDialog->loadSettings(mySettings.general,theme,mySettings.slideSets, mySettings.bibleSets,mySettings.bibleSets2,mySettings.bibleSets3,mySettings.bibleSets4);
    settingsDialog->exec();
}

void SoftProjector::on_actionDonate_triggered()
{
    QDesktopServices::openUrl(QUrl("https://www.paypal.com/cgi-bin/webscr?cmd=_s-xclick&hosted_button_id=FPCLPXFMH9XP4"));
}

void SoftProjector::on_actionAbout_triggered()
{
    AboutDialog *aboutDialog;
    aboutDialog = new AboutDialog(this, version_string);
    aboutDialog->exec();
    delete aboutDialog;
}

void SoftProjector::on_action_Help_triggered()
{
    //helpDialog->show();
     QDesktopServices::openUrl(QUrl("http://softprojector.org/help/index.html"));
}

void SoftProjector::newSong()
{
    if (!editWidget->isHidden()) //Prohibits editing a song when a different song already been edited.
    {
        QMessageBox ms(this);
        ms.setWindowTitle(tr("Cannot create a new song"));
        ms.setText(tr("Another song is already been edited."));
        ms.setInformativeText(tr("Please save and/or close current edited song before edited a different song."));
        ms.setIcon(QMessageBox::Information);
        ms.exec();
    }
    else
    {
        editWidget->show();
        editWidget->setNew();
        editWidget->activateWindow();
    }
}

void SoftProjector::setArrowCursor()
{
    this->setCursor(Qt::ArrowCursor);
}

void SoftProjector::setWaitCursor()
{
    this->setCursor(Qt::WaitCursor);
}

void SoftProjector::switchLanguage(QAction *action)
{
    cur_locale = action->data().toString();
    retranslateUis();
}

void SoftProjector::on_actionSong_Counter_triggered()
{
    SongCounter *songCounter;
    songCounter = new SongCounter(this, cur_locale);
    songCounter->exec();
    delete songCounter;
}

void SoftProjector::updateWindowText()
{
    QFileInfo fi(schedule_file_path);
    QString file = fi.fileName();

    if(!file.isEmpty())
    {
        file.remove(".spsc");
        if(is_schedule_saved)
            this->setWindowTitle(file + " - SoftProjector " + version_string);
        else
            this->setWindowTitle(file + "* - SoftProjector " + version_string);
    }
    else
    {
        this->setWindowTitle("SoftProjector " + version_string);
    }
}

void SoftProjector::on_actionPrintSchedule_triggered()
{
    PrintPreviewDialog* p = new PrintPreviewDialog(this);
    p->setSchedule(schedule_file_path,schedule,false);
    p->exec();
    delete p;
}

void SoftProjector::on_actionScheduleClear_triggered()
{
    schedule.clear();
    reloadShceduleList();
}

void SoftProjector::addToShcedule(BibleHistory &b)
{
    Schedule d(b);
    schedule.append(d);
    reloadShceduleList();
}

void SoftProjector::addToShcedule(Song &s)
{
    Schedule d(s);
    schedule.append(d);
    reloadShceduleList();
}

void SoftProjector::addToShcedule(SlideShow &s)
{
    Schedule d(s);
    schedule.append(d);
    reloadShceduleList();
}

void SoftProjector::addToShcedule(VideoInfo &v)
{
    Schedule d(v);
    schedule.append(d);
    reloadShceduleList();
}

void SoftProjector::addToShcedule(Announcement &a)
{
    Schedule d(a);
    schedule.append(d);
    reloadShceduleList();
}

void SoftProjector::on_actionNewSchedule_triggered()
{
    if(!is_schedule_saved && !schedule_file_path.isEmpty())
    {
        QMessageBox mb(this);
        mb.setWindowTitle(tr("Save Schedule?"));
        mb.setText(tr("Do you want to save current schedule before creating a new schedule?"));
        mb.setIcon(QMessageBox::Question);
        mb.setStandardButtons(QMessageBox::Yes | QMessageBox::No | QMessageBox::Cancel);
        mb.setDefaultButton(QMessageBox::Yes);
        int ret = mb.exec();

        switch (ret)
        {
        case QMessageBox::Yes:
            // Yes to save unsaved schedule and contiue creating new schedule
            on_actionSaveSchedule_triggered();
            break;
        case QMessageBox::No:
            // No to save unsaved schedule and continue to create new schedule
            break;
        case QMessageBox::Cancel:
            // Cancel all
            return;
        default:
            // should never be reached
            break;
        }
    }

    schedule_file_path = "untitled.spsc";
    schedule.clear();
    reloadShceduleList();
    is_schedule_saved = false;
    updateWindowText();
}

void SoftProjector::on_actionOpenSchedule_triggered()
{
    if(!is_schedule_saved && !schedule_file_path.isEmpty())
    {
        QMessageBox mb(this);
        mb.setWindowTitle(tr("Save Schedule?"));
        mb.setText(tr("Do you want to save current schedule before opening a new schedule?"));
        mb.setIcon(QMessageBox::Question);
        mb.setStandardButtons(QMessageBox::Yes | QMessageBox::No | QMessageBox::Cancel);
        mb.setDefaultButton(QMessageBox::Yes);
        int ret = mb.exec();

        switch (ret)
        {
        case QMessageBox::Yes:
            // Yes to save unsaved schedule and contiue opening new schedule
            on_actionSaveSchedule_triggered();
            break;
        case QMessageBox::No:
            // No to save unsaved schedule and continue opening new schedule
            break;
        case QMessageBox::Cancel:
            // Cancel all
            return;
        default:
            // should never be reached
            break;
        }
    }

    QString path = QFileDialog::getOpenFileName(this,tr("Open SoftProjector schedule:"),".",
                                                tr("SoftProjector schedule file ") + "(*.spsc)");
    if(!path.isEmpty())
    {
        schedule_file_path = path;
        openSchedule();
        is_schedule_saved = true;
        updateWindowText();
    }
}

void SoftProjector::on_actionSaveSchedule_triggered()
{
    if(schedule_file_path.isEmpty() || schedule_file_path.startsWith("untitled"))
        on_actionSaveScheduleAs_triggered();
    else
        saveSchedule(false);
    updateWindowText();
}

void SoftProjector::on_actionSaveScheduleAs_triggered()
{
    QString path = QFileDialog::getSaveFileName(this,tr("Save SoftProjector schedule as:"),".",
                                                tr("SoftProjector schedule file ") + "(*.spsc)");
    if(!path.isEmpty())
    {
        if(path.endsWith(".spsc"))
            schedule_file_path = path;
        else
            schedule_file_path = path + ".spsc";
        saveSchedule(true);
    }

    updateWindowText();
}

void SoftProjector::on_actionCloseSchedule_triggered()
{
    if(!is_schedule_saved && !schedule_file_path.isEmpty())
    {
        QMessageBox mb(this);
        mb.setWindowTitle(tr("Save Schedule?"));
        mb.setText(tr("Do you want to save current schedule before closing it?"));
        mb.setIcon(QMessageBox::Question);
        mb.setStandardButtons(QMessageBox::Yes | QMessageBox::No | QMessageBox::Cancel);
        mb.setDefaultButton(QMessageBox::Yes);
        int ret = mb.exec();

        switch (ret)
        {
        case QMessageBox::Yes:
            // Yes to save unsaved schedule and contiue creating new schedule
            on_actionSaveSchedule_triggered();
            break;
        case QMessageBox::No:
            // No to save unsaved schedule and continue to create new schedule
            break;
        case QMessageBox::Cancel:
            // Cancel all
            return;
        default:
            // should never be reached
            break;
        }
    }

    schedule_file_path.clear();
    schedule.clear();
    reloadShceduleList();
    is_schedule_saved = true;
    updateWindowText();
}

void SoftProjector::saveSchedule(bool overWrite)
{
    // Save schedule as s SQLite database file
    QProgressDialog progress;
    progress.setMaximum(0);
    progress.setLabelText(tr("Saving schedule file..."));
    progress.show();
    {
        bool db_exist = QFile::exists(schedule_file_path);
        if(db_exist && overWrite)
        {
            if(!QFile::remove(schedule_file_path))
            {
                QMessageBox mb(this);
                mb.setText(tr("An error has ocured when overwriting existing file.\n"
                              "Please try again with different file name."));
                mb.setIcon(QMessageBox::Information);
                mb.setStandardButtons(QMessageBox::Ok);
                mb.exec();
                return;
            }
            else
                db_exist = false;
        }
        QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE","spsc");
        db.setDatabaseName(schedule_file_path);
        if(db.open())
        {
            QSqlQuery sq(db);
            sq.exec("PRAGMA user_version = 2");
            sq.exec("CREATE TABLE IF NOT EXISTS 'schedule' ('id' INTEGER PRIMARY KEY  AUTOINCREMENT  NOT NULL, "
                    "'stype' TEXT, 'name' TEXT, 'sorder' INTEGER )");
            sq.exec("CREATE TABLE IF NOT EXISTS 'bible' ('scid' INTEGER, 'verseIds' TEXT, 'caption' TEXT, 'captionLong' TEXT)");
            sq.exec("CREATE TABLE IF NOT EXISTS 'song' ('scid' INTEGER, 'songid' INTEGER, 'sbid' INTEGER, 'sbName' TEXT, "
                    "'number' INTEGER, 'title' TEXT, 'category' INTEGER, 'tune' TEXT, 'wordsBy' TEXT, 'musicBy' TEXT, "
                    "'songText' TEXT, 'notes' TEXT, 'usePrivate' BOOL, 'alignV' INTEGER, 'alignH' INTEGER, 'color' INTEGER, "
                    "'font' TEXT, 'infoColor' INTEGER, 'infoFont' TEXT, 'endingColor' INTEGER, 'endingFont' TEXT, "
                    "'useBack' BOOL, 'backImage' BLOB, 'backName' TEXT)");
            sq.exec("CREATE TABLE IF NOT EXISTS 'slideshow' ('scid' INTEGER, 'ssid' INTEGER, 'name' TEXT, 'info' TEXT)");
            sq.exec("CREATE TABLE IF NOT EXISTS 'slides' ('scid' INTEGER, 'sid' INTEGER, 'name' TEXT, 'path' TEXT, "
                    "'porder' INTEGER, 'image' BLOB, 'imageSmall' BLOB, 'imagePreview' BLOB)");
            sq.exec("CREATE TABLE IF NOT EXISTS 'media' ('scid' INTEGER, 'name' TEXT, 'path' TEXT, 'aRatio' INTEGER)");
            sq.exec("CREATE TABLE IF NOT EXISTS 'announce' ('scid' INTEGER, 'aId' INTEGER, 'title' TEXT, 'aText' TEXT, "
                    "'usePrivate' BOOL, 'useAuto' BOOL, 'loop' BOOL, 'slideTimer' INTEGER, 'font' TEXT, 'color' INTEGER, "
                    "'useBack' BOOL, 'backImage' BLOB, 'backPath' TEXT, 'alignV' INTEGER, 'alignH' INTEGER)");
            if(db_exist)
                saveScheduleUpdate(sq);
            else
                saveScheduleNew(sq);
        }
    }
    QSqlDatabase::removeDatabase("spsc");
    is_schedule_saved = true;
    progress.close();
}

void SoftProjector::saveScheduleNew(QSqlQuery &q)
{
    for(int i(0);i < schedule.count();++i)
    {
        Schedule sc =  schedule.at(i);
        q.exec(QString("INSERT INTO schedule (stype,name,sorder) VALUES('%1','%2',%3)")
               .arg(sc.stype).arg(sc.name).arg(i+1));
        q.exec("SELECT seq FROM sqlite_sequence WHERE name = 'schedule'");
        q.first();
        sc.scid = q.value(0).toInt();
        q.clear();
        if(sc.stype == "bible")
            saveScheduleItemNew(q,sc.scid,sc.bible);
        else if(sc.stype == "song")
            saveScheduleItemNew(q,sc.scid,sc.song);
        else if(sc.stype == "slideshow")
            saveScheduleItemNew(q,sc.scid,sc.slideshow);
        else if(sc.stype == "media")
            saveScheduleItemNew(q,sc.scid,sc.media);
        else if(sc.stype == "announce")
            saveScheduleItemNew(q,sc.scid,sc.announce);
        schedule.replace(i,sc);
    }
}

void SoftProjector::saveScheduleItemNew(QSqlQuery &q, int scid, const BibleHistory &b)
{
    q.prepare("INSERT INTO bible (scid,verseIds,caption,captionLong) VALUES(?,?,?,?)");
    q.addBindValue(scid);
    q.addBindValue(b.verseIds);
    q.addBindValue(b.caption);
    q.addBindValue(b.captionLong);
    q.exec();
}

void SoftProjector::saveScheduleItemNew(QSqlQuery &q, int scid, const Song &s)
{
    q.prepare("INSERT INTO song (scid,songid,sbid,sbName,number,title,category,tune,wordsBy,musicBy,"
              "songText,notes,usePrivate,alignV,alignH,color,font,infoColor,infoFont,endingColor,"
              "endingFont,useBack,backImage,backName) "
              "VALUES(?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?)");
    q.addBindValue(scid);
    q.addBindValue(s.songID);
    q.addBindValue(s.songbook_id);
    q.addBindValue(s.songbook_name);
    q.addBindValue(s.number);
    q.addBindValue(s.title);
    q.addBindValue(s.category);
    q.addBindValue(s.tune);
    q.addBindValue(s.wordsBy);
    q.addBindValue(s.musicBy);
    q.addBindValue(s.songText);
    q.addBindValue(s.notes);
    q.addBindValue(s.usePrivateSettings);
    q.addBindValue(s.alignmentV);
    q.addBindValue(s.alignmentH);
    q.addBindValue((unsigned int)(s.color.rgb()));
    q.addBindValue(s.font.toString());
    q.addBindValue((unsigned int)(s.infoColor.rgb()));
    q.addBindValue(s.infoFont.toString());
    q.addBindValue((unsigned int)(s.endingColor.rgb()));
    q.addBindValue(s.endingFont.toString());
    q.addBindValue(s.useBackground);
    q.addBindValue(pixToByte(s.background));
    q.addBindValue(s.backgroundName);
    q.exec();
}

void SoftProjector::saveScheduleItemNew(QSqlQuery &q, int scid, const SlideShow &s)
{
    q.prepare("INSERT INTO slideshow (scid,ssid,name,info) VALUES (?,?,?,?)");
    q.addBindValue(scid);
    q.addBindValue(s.slideShowId);
    q.addBindValue(s.name);
    q.addBindValue(s.info);
    q.exec();

    foreach(const SlideShowItem & si,s.slides)
    {
        q.prepare("INSERT INTO slides (scid,sid,name,path,porder,image,imageSmall,imagePreview) VALUES(?,?,?,?,?,?,?,?)");
        q.addBindValue(scid);
        q.addBindValue(si.slideId);
        q.addBindValue(si.name);
        q.addBindValue(si.path);
        q.addBindValue(si.order);
        q.addBindValue(pixToByte(si.image));
        q.addBindValue(pixToByte(si.imageSmall));
        q.addBindValue(pixToByte(si.imagePreview));
        q.exec();
    }
}

void SoftProjector::saveScheduleItemNew(QSqlQuery &q, int scid, const VideoInfo &v)
{
    q.prepare("INSERT INTO media (scid,name,path,aRatio) VALUES(?,?,?,?)");
    q.addBindValue(scid);
    q.addBindValue(v.fileName);
    q.addBindValue(v.filePath);
    q.addBindValue(v.aspectRatio);
    q.exec();
}

void SoftProjector::saveScheduleItemNew(QSqlQuery &q, int scid, const Announcement &a)
{
    q.prepare("INSERT INTO announce (scid,aId,title,aText,usePrivate,useAuto,loop,slideTimer,font,"
                        "color,useBack,backImage,backPath,alignV,alignH) "
                        "VALUES(?,?,?,?,?,?,?,?,?,?,?,?,?,?,?)");
    q.addBindValue(scid);
    q.addBindValue(a.idNum);
    q.addBindValue(a.title);
    q.addBindValue(a.text);
    q.addBindValue(a.usePrivateSettings);
    q.addBindValue(a.useAutoNext);
    q.addBindValue(a.loop);
    q.addBindValue(a.slideTimer);
    q.addBindValue(a.font.toString());
    unsigned int tci = (unsigned int)(a.color.rgb());
    q.addBindValue(tci);
    q.addBindValue(a.useBackground);
    q.addBindValue(pixToByte(QPixmap()));
    q.addBindValue(a.backgroundPath);
    q.addBindValue(a.alignmentV);
    q.addBindValue(a.alignmentH);
    q.exec();
}

void SoftProjector::saveScheduleUpdate(QSqlQuery &q)
{
    for(int i(0);i < schedule.count();++i)
    {
        Schedule sc =  schedule.at(i);
        if(sc.scid == -1)   // Save new schedule item that was not saved yet
        {
            q.exec(QString("INSERT INTO schedule (stype,name,sorder) VALUES('%1','%2',%3)")
                   .arg(sc.stype).arg(sc.name).arg(i+1));
            q.exec("SELECT seq FROM sqlite_sequence WHERE name = 'schedule'");
            q.first();
            sc.scid = q.value(0).toInt();
            q.clear();
            if(sc.stype == "bible")
                saveScheduleItemNew(q,sc.scid,sc.bible);
            else if(sc.stype == "song")
                saveScheduleItemNew(q,sc.scid,sc.song);
            else if(sc.stype == "slideshow")
                saveScheduleItemNew(q,sc.scid,sc.slideshow);
            else if(sc.stype == "media")
                saveScheduleItemNew(q,sc.scid,sc.media);
            else if(sc.stype == "announce")
                saveScheduleItemNew(q,sc.scid,sc.announce);
            schedule.replace(i,sc);
        }
        else    // Update existing schedule item
        {
            q.exec(QString("UPDATE schedule SET sorder = %1 WHERE id = %2").arg(i+1).arg(sc.scid));
            //if(sc.stype == "bible")
            //    saveScheduleItemUpdate(q,sc.scid,sc.bible);
            //else if(sc.stype == "song")
            //    saveScheduleItemUpdate(q,sc.scid,sc.song);
            //else if(sc.stype == "slideshow")
            //    saveScheduleItemUpdate(q,sc.scid,sc.slideshow);
            //else if(sc.stype == "media")
            //    saveScheduleItemUpdate(q,sc.scid,sc.media);
            //else if(sc.stype == "announce")
            //    saveScheduleItemUpdate(q,sc.scid,sc.announce);
        }
    }

    // Delete any shcedule items from file that have removed from schedule
    q.exec("SELECT id,stype FROM schedule");
    QSqlQuery sq = q;
    while(q.next())
    {
        int scid = q.value(0).toInt();
        bool toDelete = false;
        QString stype = q.value(1).toString();
        foreach(const Schedule &s,schedule)
        {
            if(scid == s.scid)
            {
                toDelete = false;
                break;
            }
            else
                toDelete = true;
        }

        if(toDelete)
        {
            sq.exec("DELETE FROM schedule WHERE id = " + QString::number(scid));
            sq.exec("DELETE FROM " + stype + " WHERE scid = " + QString::number(scid));
            if(stype == "slideshow")
                sq.exec("DELETE FROM slides WHERE scid = " + QString::number(scid));
        }
    }
}

void SoftProjector::saveScheduleItemUpdate(QSqlQuery &q, int scid, const BibleHistory &b)
{

}

void SoftProjector::saveScheduleItemUpdate(QSqlQuery &q, int scid, const Song &s)
{

}

void SoftProjector::saveScheduleItemUpdate(QSqlQuery &q, int scid, const SlideShow &s)
{

}

void SoftProjector::saveScheduleItemUpdate(QSqlQuery &q, int scid, const VideoInfo &v)
{

}

void SoftProjector::saveScheduleItemUpdate(QSqlQuery &q, int scid, const Announcement &a)
{

}

void SoftProjector::openSchedule()
{
    QProgressDialog progress;
    progress.setMaximum(0);
    progress.setLabelText(tr("Opening schedule file..."));
    progress.show();
    {
        QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE","spsc");
        db.setDatabaseName(schedule_file_path);
        if(db.open())
        {
            QSqlQuery sq(db);
            sq.exec("PRAGMA user_version");
            sq.first();
            int scVer = sq.value(0).toInt();
            if(scVer == 2)
            {
                schedule.clear();
                sq.exec("SELECT id, stype, name FROM schedule ORDER BY sorder");
                QSqlQuery sqsc = sq;
                while(sq.next())
                {
                    int scid = sq.value(0).toInt();
                    QString stype = sq.value(1).toString();
                    QString name = sq.value(2).toString();

                    if(stype == "bible")
                    {
                        BibleHistory bib;
                        openScheduleItem(sqsc,scid,bib);
                        Schedule sc(bib);
                        sc.name = name;
                        sc.scid = scid;
                        schedule.append(sc);
                    }
                    else if(stype == "song")
                    {
                        Song song;
                        openScheduleItem(sqsc,scid,song);
                        Schedule sc(song);
                        sc.name = name;
                        sc.scid = scid;
                        schedule.append(sc);
                    }
                    else if(stype == "slideshow")
                    {
                        SlideShow ss;
                        openScheduleItem(sqsc,scid,ss);
                        Schedule sc(ss);
                        sc.name = name;
                        sc.scid = scid;
                        schedule.append(sc);
                    }
                    else if(stype == "media")
                    {
                        VideoInfo vi;
                        openScheduleItem(sqsc,scid,vi);
                        Schedule sc(vi);
                        sc.name = name;
                        sc.scid = scid;
                        schedule.append(sc);
                    }
                    else if(stype == "announce")
                    {
                        Announcement announce;
                        openScheduleItem(sqsc,scid,announce);
                        Schedule sc(announce);
                        sc.name = name;
                        sc.scid = scid;
                        schedule.append(sc);
                    }
                }
                reloadShceduleList();
            }
            else
            {
                QMessageBox mb(this);
                mb.setText(tr("The schedule file you are trying to open is of uncompatible version.\n"
                              "Compatible version: 2\n"
                              "This schedule file version: ") + QString::number(scVer));
                mb.setIcon(QMessageBox::Information);
                mb.setStandardButtons(QMessageBox::Ok);
                mb.exec();
                schedule_file_path.clear();
                updateWindowText();
            }
        }
    }
    QSqlDatabase::removeDatabase("spsc");
    progress.close();
}

void SoftProjector::openScheduleItem(QSqlQuery &q, const int scid, BibleHistory &b)
{
    q.exec("SELECT verseIds, caption, captionLong FROM bible WHERE scid = " + QString::number(scid));
    q.first();
    b.verseIds = q.value(0).toString();
    b.caption = q.value(1).toString();
    b.caption = q.value(2).toString();
}

void SoftProjector::openScheduleItem(QSqlQuery &q, const int scid, Song &s)
{
    q.exec("SELECT * FROM song WHERE scid = " + QString::number(scid));
    q.first();
    QSqlRecord r = q.record();
    s.songID = r.field("songid").value().toInt();
    s.songbook_id = r.field("sbid").value().toString();
    s.songbook_name = r.field("sbName").value().toString();
    s.number = r.field("number").value().toInt();
    s.title = r.field("title").value().toString();
    s.category = r.field("category").value().toInt();
    s.tune = r.field("tune").value().toString();
    s.wordsBy = r.field("wordsBy").value().toString();
    s.musicBy = r.field("musicBy").value().toString();
    s.songText = r.field("songText").value().toString();
    s.notes = r.field("notes").value().toString();
    s.usePrivateSettings = r.field("usePrivate").value().toBool();
    s.alignmentV = r.field("alignV").value().toInt();
    s.alignmentH = r.field("alignH").value().toInt();
    s.color = QColor::fromRgb(r.field("color").value().toUInt());
    s.font.fromString(r.field("font").value().toString());
    s.infoColor = QColor::fromRgb(r.field("infoColor").value().toUInt());
    s.infoFont.fromString(r.field("infoFont").value().toString());
    s.endingColor = QColor::fromRgb(r.field("endingColor").value().toUInt());
    s.endingFont.fromString(r.field("endingFont").value().toString());
    s.useBackground = r.field("useBack").value().toBool();
    s.background.loadFromData(r.field("backImage").value().toByteArray());
    s.backgroundName = r.field("backName").value().toString();
}

void SoftProjector::openScheduleItem(QSqlQuery &q, const int scid, SlideShow &s)
{
    q.exec("SELECT ssid, name, info FROM slideshow WHERE scid = " + QString::number(scid));
    q.first();
    s.slideShowId = q.value(0).toInt();
    s.name = q.value(1).toString();
    s.info = q.value(2).toString();

    q.exec("SELECT sid, name, path, porder, image, imageSmall, imagePreview FROM slides WHERE scid = " + QString::number(scid));
    while(q.next())
    {
        SlideShowItem si;
        si.slideId = q.value(0).toInt();
        si.name = q.value(1).toString();
        si.path = q.value(2).toString();
        si.order = q.value(3).toInt();
        si.image.loadFromData(q.value(4).toByteArray());
        si.imageSmall.loadFromData(q.value(5).toByteArray());
        si.imagePreview.loadFromData(q.value(6).toByteArray());
        s.slides.append(si);
    }
}

void SoftProjector::openScheduleItem(QSqlQuery &q, const int scid, VideoInfo &v)
{
    q.exec("SELECT name, path, aRatio FROM media WHERE scid = " + QString::number(scid));
    q.first();
    v.fileName = q.value(0).toString();
    v.filePath = q.value(1).toString();
    v.aspectRatio = q.value(2).toInt();
}

void SoftProjector::openScheduleItem(QSqlQuery &q, const int scid, Announcement &a)
{
    q.exec("SELECT aId, title, aText, usePrivate, useAuto, loop, slideTimer, font, color, useBack, "
           "backImage, backPath, alignV, alignH FROM announce WHERE scid = " + QString::number(scid));
    q.first();
    a.idNum = q.value(0).toInt();
    a.title = q.value(1).toString();
    a.text = q.value(2).toString();
    a.usePrivateSettings = q.value(3).toBool();
    a.useAutoNext = q.value(4).toBool();
    a.loop = q.value(5).toBool();
    a.slideTimer = q.value(6).toInt();
    a.font.fromString(q.value(7).toString());
    a.color = QColor::fromRgb(q.value(8).toUInt());
    a.useBackground = q.value(9).toBool();
//    a.backImage = QPixmap
    a.backgroundPath = q.value(11).toString();
    a.alignmentV = q.value(12).toInt();
    a.alignmentH = q.value(13).toInt();
}
