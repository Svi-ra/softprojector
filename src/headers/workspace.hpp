/***************************************************************************
//
//    SoftProjector - an open source media projection software
//
//    Workspace is the view-model behind the QML interface. It owns the
//    browsing state of every module panel (Bible, Songs, Pictures, Media,
//    Announcements) and the schedule, and exposes it as properties QML binds
//    to.
//
//    It deliberately does NOT drive the projector. When the operator sends
//    something live, Workspace emits a request signal; SoftProjector — which
//    owns the display screens, themes and image generators — performs it and
//    pushes the resulting live state back in through setLive*(). That keeps
//    the projection pipeline exactly where it already was.
//
***************************************************************************/

#ifndef WORKSPACE_HPP
#define WORKSPACE_HPP

#include <QObject>
#include <QVariantList>
#include <QStringList>
#include <QUrl>
#include <QHash>
#include <QPixmap>

#include "bible.hpp"
#include "song.hpp"
#include "announcement.hpp"
#include "slideshow.hpp"
#include "videoinfo.hpp"
#include "schedule.hpp"
#include "settings.hpp"

class Workspace : public QObject
{
    Q_OBJECT

    // ── Shell ───────────────────────────────────────────────────────────────
    Q_PROPERTY(QString module READ module WRITE setModule NOTIFY moduleChanged)
    Q_PROPERTY(QString statusText READ statusText NOTIFY statusTextChanged)
    Q_PROPERTY(int screenCount READ screenCount NOTIFY displayChanged)
    Q_PROPERTY(bool displayOn READ displayOn NOTIFY displayChanged)
    Q_PROPERTY(QVariantList languages READ languages NOTIFY languagesChanged)
    Q_PROPERTY(QString language READ language NOTIFY languagesChanged)
    Q_PROPERTY(QString versionString READ versionString CONSTANT)

    // ── Live / preview column ───────────────────────────────────────────────
    Q_PROPERTY(bool live READ live NOTIFY liveChanged)
    Q_PROPERTY(QString liveKind READ liveKind NOTIFY liveChanged)
    Q_PROPERTY(QString liveTitle READ liveTitle NOTIFY liveChanged)
    Q_PROPERTY(QString liveIcon READ liveIcon NOTIFY liveChanged)
    Q_PROPERTY(QString liveNotes READ liveNotes NOTIFY liveChanged)
    Q_PROPERTY(QVariantList liveSlides READ liveSlides NOTIFY liveChanged)
    Q_PROPERTY(int liveIndex READ liveIndex WRITE setLiveIndex NOTIFY liveIndexChanged)
    Q_PROPERTY(QVariantList liveSelection READ liveSelection NOTIFY liveIndexChanged)
    Q_PROPERTY(bool multiVerse READ multiVerse WRITE setMultiVerse NOTIFY multiVerseChanged)
    Q_PROPERTY(bool canShow READ canShow NOTIFY actionStateChanged)
    Q_PROPERTY(bool canHide READ canHide NOTIFY actionStateChanged)
    Q_PROPERTY(bool canClear READ canClear NOTIFY actionStateChanged)

    // ── Schedule ────────────────────────────────────────────────────────────
    Q_PROPERTY(QVariantList schedule READ schedule NOTIFY scheduleChanged)
    Q_PROPERTY(int scheduleIndex READ scheduleIndex WRITE setScheduleIndex NOTIFY scheduleIndexChanged)
    Q_PROPERTY(QString scheduleName READ scheduleName NOTIFY scheduleChanged)
    Q_PROPERTY(bool scheduleDirty READ scheduleDirty NOTIFY scheduleChanged)

    // ── Bible ───────────────────────────────────────────────────────────────
    Q_PROPERTY(QStringList bibleVersions READ bibleVersions NOTIFY bibleVersionsChanged)
    Q_PROPERTY(int bibleVersionIndex READ bibleVersionIndex WRITE setBibleVersionIndex NOTIFY bibleVersionsChanged)
    Q_PROPERTY(QVariantList bibleBooks READ bibleBooks NOTIFY bibleBooksChanged)
    Q_PROPERTY(int bibleBookIndex READ bibleBookIndex WRITE setBibleBookIndex NOTIFY bibleBookChanged)
    Q_PROPERTY(int bibleChapterCount READ bibleChapterCount NOTIFY bibleBookChanged)
    Q_PROPERTY(int bibleChapterIndex READ bibleChapterIndex WRITE setBibleChapterIndex NOTIFY bibleChapterChanged)
    Q_PROPERTY(QVariantList bibleVerses READ bibleVerses NOTIFY bibleChapterChanged)
    Q_PROPERTY(QString bibleBookFilter READ bibleBookFilter WRITE setBibleBookFilter NOTIFY bibleBooksChanged)
    Q_PROPERTY(QVariantList bibleSearchResults READ bibleSearchResults NOTIFY bibleSearchChanged)
    Q_PROPERTY(bool bibleSearching READ bibleSearching NOTIFY bibleSearchChanged)
    Q_PROPERTY(QVariantList bibleHistory READ bibleHistory NOTIFY bibleHistoryChanged)

    // ── Songs ───────────────────────────────────────────────────────────────
    Q_PROPERTY(QStringList songbooks READ songbooks NOTIFY songbooksChanged)
    Q_PROPERTY(int songbookIndex READ songbookIndex WRITE setSongbookIndex NOTIFY songFilterChanged)
    Q_PROPERTY(QStringList songCategories READ songCategories NOTIFY songbooksChanged)
    Q_PROPERTY(int songCategoryIndex READ songCategoryIndex WRITE setSongCategoryIndex NOTIFY songFilterChanged)
    Q_PROPERTY(int songMatchMode READ songMatchMode WRITE setSongMatchMode NOTIFY songFilterChanged)
    Q_PROPERTY(QString songQuery READ songQuery WRITE setSongQuery NOTIFY songFilterChanged)
    Q_PROPERTY(QVariantList songs READ songs NOTIFY songsChanged)
    Q_PROPERTY(int songIndex READ songIndex WRITE setSongIndex NOTIFY songSelectionChanged)
    Q_PROPERTY(QString songTitle READ songTitle NOTIFY songSelectionChanged)
    Q_PROPERTY(QString songInfo READ songInfo NOTIFY songSelectionChanged)
    Q_PROPERTY(QString songNotes READ songNotes NOTIFY songSelectionChanged)
    Q_PROPERTY(QVariantList songStanzas READ songStanzas NOTIFY songSelectionChanged)

    // ── Announcements ───────────────────────────────────────────────────────
    Q_PROPERTY(QString announceQuery READ announceQuery WRITE setAnnounceQuery NOTIFY announcementsChanged)
    Q_PROPERTY(QVariantList announcements READ announcements NOTIFY announcementsChanged)
    Q_PROPERTY(int announceIndex READ announceIndex WRITE setAnnounceIndex NOTIFY announceSelectionChanged)
    Q_PROPERTY(QString announceTitle READ announceTitle NOTIFY announceSelectionChanged)
    Q_PROPERTY(QVariantList announceSlides READ announceSlides NOTIFY announceSelectionChanged)

    // ── Pictures ────────────────────────────────────────────────────────────
    Q_PROPERTY(QVariantList slideshows READ slideshows NOTIFY slideshowsChanged)
    Q_PROPERTY(int slideshowIndex READ slideshowIndex WRITE setSlideshowIndex NOTIFY slideshowSelectionChanged)
    Q_PROPERTY(QString slideshowName READ slideshowName NOTIFY slideshowSelectionChanged)
    Q_PROPERTY(QVariantList slides READ slides NOTIFY slideshowSelectionChanged)
    Q_PROPERTY(int slideIndex READ slideIndex WRITE setSlideIndex NOTIFY slideIndexChanged)

    // ── Media ───────────────────────────────────────────────────────────────
    Q_PROPERTY(QVariantList mediaLibrary READ mediaLibrary NOTIFY mediaLibraryChanged)
    Q_PROPERTY(int mediaIndex READ mediaIndex WRITE setMediaIndex NOTIFY mediaSelectionChanged)
    Q_PROPERTY(QString mediaName READ mediaName NOTIFY mediaSelectionChanged)
    Q_PROPERTY(bool mediaPlaying READ mediaPlaying NOTIFY transportChanged)
    Q_PROPERTY(qint64 mediaPosition READ mediaPosition NOTIFY transportChanged)
    Q_PROPERTY(qint64 mediaDuration READ mediaDuration NOTIFY transportChanged)
    Q_PROPERTY(int mediaVolume READ mediaVolume WRITE setMediaVolume NOTIFY transportChanged)
    Q_PROPERTY(bool mediaMuted READ mediaMuted WRITE setMediaMuted NOTIFY transportChanged)

public:
    explicit Workspace(QObject *parent = nullptr);

    // ── Readers ─────────────────────────────────────────────────────────────
    QString module() const { return mModule; }
    QString statusText() const { return mStatusText; }
    int screenCount() const { return mScreenCount; }
    bool displayOn() const { return mDisplayOn; }
    QVariantList languages() const { return mLanguages; }
    QString language() const { return mLanguage; }
    QString versionString() const;

    bool live() const { return mLive; }
    QString liveKind() const { return mLiveKind; }
    QString liveTitle() const { return mLiveTitle; }
    QString liveIcon() const;
    QString liveNotes() const { return mLiveNotes; }
    QVariantList liveSlides() const { return mLiveSlides; }
    int liveIndex() const { return mLiveIndex; }
    QVariantList liveSelection() const;
    bool multiVerse() const { return mMultiVerse; }
    bool canShow() const { return mCanShow; }
    bool canHide() const { return mCanHide; }
    bool canClear() const { return mCanClear; }

    QVariantList schedule() const { return mScheduleView; }
    int scheduleIndex() const { return mScheduleIndex; }
    QString scheduleName() const { return mScheduleName; }
    bool scheduleDirty() const { return mScheduleDirty; }

    QStringList bibleVersions() const { return mBibleVersions; }
    int bibleVersionIndex() const { return mBibleVersionIndex; }
    QVariantList bibleBooks() const { return mBibleBooksView; }
    int bibleBookIndex() const { return mBibleBookIndex; }
    int bibleChapterCount() const;
    int bibleChapterIndex() const { return mBibleChapterIndex; }
    QVariantList bibleVerses() const { return mBibleVerses; }
    QString bibleBookFilter() const { return mBibleBookFilter; }
    QVariantList bibleSearchResults() const { return mBibleSearchView; }
    bool bibleSearching() const { return mBibleSearching; }
    QVariantList bibleHistory() const { return mBibleHistoryView; }

    QStringList songbooks() const { return mSongbooks; }
    int songbookIndex() const { return mSongbookIndex; }
    QStringList songCategories() const { return mSongCategories; }
    int songCategoryIndex() const { return mSongCategoryIndex; }
    int songMatchMode() const { return mSongMatchMode; }
    QString songQuery() const { return mSongQuery; }
    QVariantList songs() const { return mSongsView; }
    int songIndex() const { return mSongIndex; }
    QString songTitle() const { return mSongTitle; }
    QString songInfo() const { return mSongInfo; }
    QString songNotes() const { return mSongNotes; }
    QVariantList songStanzas() const { return mSongStanzas; }

    QString announceQuery() const { return mAnnounceQuery; }
    QVariantList announcements() const { return mAnnounceView; }
    int announceIndex() const { return mAnnounceIndex; }
    QString announceTitle() const { return mAnnounceTitle; }
    QVariantList announceSlides() const { return mAnnounceSlides; }

    QVariantList slideshows() const { return mSlideshowView; }
    int slideshowIndex() const { return mSlideshowIndex; }
    QString slideshowName() const { return mCurrentSlideShow.name; }
    QVariantList slides() const { return mSlidesView; }
    int slideIndex() const { return mSlideIndex; }

    QVariantList mediaLibrary() const { return mMediaView; }
    int mediaIndex() const { return mMediaIndex; }
    QString mediaName() const;
    bool mediaPlaying() const { return mMediaPlaying; }
    qint64 mediaPosition() const { return mMediaPosition; }
    qint64 mediaDuration() const { return mMediaDuration; }
    int mediaVolume() const { return mMediaVolume; }
    bool mediaMuted() const { return mMediaMuted; }

    // ── Writers bound from QML ──────────────────────────────────────────────
    void setModule(const QString &m);
    void setLiveIndex(int i);
    void setMultiVerse(bool on);
    void setScheduleIndex(int i);
    void setBibleVersionIndex(int i);
    void setBibleBookIndex(int i);
    void setBibleChapterIndex(int i);
    void setBibleBookFilter(const QString &f);
    void setSongbookIndex(int i);
    void setSongCategoryIndex(int i);
    void setSongMatchMode(int m);
    void setSongQuery(const QString &q);
    void setSongIndex(int i);
    void setAnnounceQuery(const QString &q);
    void setAnnounceIndex(int i);
    void setSlideshowIndex(int i);
    void setSlideIndex(int i);
    void setMediaIndex(int i);
    void setMediaVolume(int v);
    void setMediaMuted(bool m);

    // ── Called by SoftProjector to publish live state ───────────────────────
    void setLiveContent(const QString &kind, const QString &title,
                        const QVariantList &slides, int index,
                        const QString &notes = QString());
    void setLiveSelection(const QList<int> &rows);
    void clearLive();
    void setActionState(bool canShow, bool canHide, bool canClear);
    void setDisplayState(bool on, int screens);
    void setStatusText(const QString &t);
    void setScheduleModel(const QList<Schedule> &items, const QString &name, bool dirty);
    void setLanguageList(const QVariantList &langs, const QString &current);
    void setTransport(bool playing, qint64 position, qint64 duration);

    // Selection accessors used by SoftProjector when acting on a request.
    QList<int> selectedLiveRows() const { return mLiveSelectedRows; }
    Song currentSong() const { return mCurrentSong; }
    Announcement currentAnnouncement() const { return mCurrentAnnounce; }
    SlideShow currentSlideShow() const { return mCurrentSlideShow; }
    VideoInfo currentMedia() const;
    Bible *bibleEngine() { return &mBible; }
    BibleHistory bibleRefFor(const QList<int> &rows) const;
    BibleHistory currentBibleRef() const { return bibleRefFor(mLiveSelectedRows); }
    QStringList currentChapterList() const { return mBibleChapterList; }
    QString currentBibleBook() const;

    void applyBibleSettings(const BibleVersionSettings &s);
    void applySlideSettings(const SlideShowSettings &s) { mSlideSettings = s; }

public slots:
    // ── Data loading ────────────────────────────────────────────────────────
    void reloadAll();
    void reloadBibles();
    void reloadSongs();
    void reloadSongbooks();
    void reloadAnnouncements();
    void reloadSlideshows();
    void reloadMediaLibrary();

    // ── Panel actions invoked from QML ──────────────────────────────────────
    void bibleSearch(const QString &query, int scope, bool allWords);
    void bibleClearSearch();
    void bibleOpenSearchResult(int i);
    void bibleOpenHistory(int i);
    void bibleClearHistory();
    void bibleGoLive(const QVariantList &rows);
    void bibleAddToSchedule();
    void bibleStep(int delta);

    void songGoLive(int stanza);
    void songAddToSchedule();
    void songJumpToNumber(int number);

    void announceGoLive(int slide);
    void announceAddToSchedule();

    void slideshowGoLive(int slide);
    void slideshowAddToSchedule();

    void mediaGoLive();
    void mediaAddToSchedule();
    void mediaAddFiles(const QVariantList &urls);
    void mediaRemoveSelected();

    void mediaSeek(qint64 ms);

    // ── Shell actions invoked from QML ──────────────────────────────────────
    void invoke(const QString &action);
    void setLiveRows(const QVariantList &rows);
    void scheduleActivate(int i);
    void scheduleMove(int from, int to);

signals:
    void moduleChanged();
    void statusTextChanged();
    void displayChanged();
    void languagesChanged();
    void liveChanged();
    void liveIndexChanged();
    void multiVerseChanged();
    void actionStateChanged();
    void scheduleChanged();
    void scheduleIndexChanged();
    void bibleVersionsChanged();
    void bibleBooksChanged();
    void bibleBookChanged();
    void bibleChapterChanged();
    void bibleSearchChanged();
    void bibleHistoryChanged();
    void songbooksChanged();
    void songFilterChanged();
    void songsChanged();
    void songSelectionChanged();
    void announcementsChanged();
    void announceSelectionChanged();
    void slideshowsChanged();
    void slideshowSelectionChanged();
    void slideIndexChanged();
    void mediaLibraryChanged();
    void mediaSelectionChanged();
    void transportChanged();

    // ── Requests handled by SoftProjector ───────────────────────────────────
    void requestBibleLive(const QStringList &chapterList, const QString &caption,
                          const QList<int> &rows);
    void requestSongLive(const Song &song, int stanza);
    void requestAnnounceLive(const Announcement &announce, int slide);
    void requestSlideShowLive(const SlideShow &show, int slide);
    void requestMediaLive(const VideoInfo &video);
    void requestScheduleAdd(const QString &kind);
    void requestAction(const QString &action);
    void requestTransport(const QString &what, qint64 value);
    void toast(const QString &message, const QString &tone);

private:
    void rebuildBibleBookView();
    void rebuildSongView();
    void rebuildAnnounceView();
    void loadBibleChapter();
    void loadSongPreview();
    void loadAnnouncePreview();
    void loadSlideShow(int id);
    QString bibleCaption(const QList<int> &rows) const;

    // Shell
    QString mModule = QStringLiteral("bible");
    QString mStatusText;
    int mScreenCount = 1;
    bool mDisplayOn = false;
    QVariantList mLanguages;
    QString mLanguage = QStringLiteral("en");

    // Live
    bool mLive = false;
    QString mLiveKind;
    QString mLiveTitle;
    QString mLiveNotes;
    QVariantList mLiveSlides;
    int mLiveIndex = -1;
    QList<int> mLiveSelectedRows;
    bool mMultiVerse = false;
    bool mCanShow = false, mCanHide = false, mCanClear = false;

    // Schedule
    QVariantList mScheduleView;
    int mScheduleIndex = -1;
    QString mScheduleName;
    bool mScheduleDirty = false;

    // Bible
    Bible mBible;
    BibleVersionSettings mBibleSettings;
    QStringList mBibleVersions;
    QList<int> mBibleVersionIds;
    int mBibleVersionIndex = 0;
    QVariantList mBibleBooksView;      // filtered view
    QList<int> mBibleBookMap;          // view row -> Bible::books index
    QString mBibleBookFilter;
    int mBibleBookIndex = -1;
    int mBibleChapterIndex = 0;
    QVariantList mBibleVerses;
    QStringList mBibleChapterList;
    QList<BibleSearch> mBibleSearch;
    QVariantList mBibleSearchView;
    bool mBibleSearching = false;
    QList<BibleHistory> mBibleHistory;
    QVariantList mBibleHistoryView;

    // Songs
    SongDatabase mSongDb;
    QList<Song> mAllSongs;
    QVariantList mSongsView;
    QList<int> mSongMap;               // view row -> mAllSongs index
    QStringList mSongbooks;
    QStringList mSongbookIds;
    int mSongbookIndex = 0;
    QStringList mSongCategories;
    QList<int> mSongCategoryIds;
    int mSongCategoryIndex = 0;
    int mSongMatchMode = 0;            // 0 contains, 1 starts with, 2 exact
    QString mSongQuery;
    int mSongIndex = -1;
    Song mCurrentSong;
    QString mSongTitle, mSongInfo, mSongNotes;
    QVariantList mSongStanzas;

    // Announcements
    QList<Announcement> mAllAnnounce;
    QVariantList mAnnounceView;
    QList<int> mAnnounceMap;
    QString mAnnounceQuery;
    int mAnnounceIndex = -1;
    Announcement mCurrentAnnounce;
    QString mAnnounceTitle;
    QVariantList mAnnounceSlides;

    // Pictures
    QList<SlideShowInfo> mSlideshows;
    QVariantList mSlideshowView;
    int mSlideshowIndex = -1;
    SlideShow mCurrentSlideShow;
    QVariantList mSlidesView;
    int mSlideIndex = 0;
    SlideShowSettings mSlideSettings;

    // Media
    QList<QUrl> mMediaPaths;
    QStringList mMediaNames;
    QVariantList mMediaView;
    int mMediaIndex = -1;
    bool mMediaPlaying = false;
    qint64 mMediaPosition = 0;
    qint64 mMediaDuration = 0;
    int mMediaVolume = 100;
    bool mMediaMuted = false;
};

#endif // WORKSPACE_HPP
