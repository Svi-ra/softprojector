/***************************************************************************
//
//    SoftProjector - an open source media projection software
//
***************************************************************************/

#include "../headers/workspace.hpp"
#include "../headers/editwidget.hpp"
#include "../headers/spfunctions.hpp"
#include "../headers/thumbprovider.hpp"

#include <QRegularExpression>
#include <QSqlQuery>
#include <QFileInfo>
#include <QCoreApplication>

namespace
{
// Icon used in the schedule and live header for each kind of content.
QString iconForKind(const QString &kind)
{
    if (kind == QLatin1String("bible"))    return QStringLiteral("book-open");
    if (kind == QLatin1String("song"))     return QStringLiteral("music-4");
    if (kind == QLatin1String("announce")) return QStringLiteral("megaphone");
    if (kind == QLatin1String("picture"))  return QStringLiteral("images");
    if (kind == QLatin1String("video"))    return QStringLiteral("clapperboard");
    return QStringLiteral("circle-dot");
}

QVariantMap row(const QString &text, const QString &secondary = QString(),
                const QString &trailing = QString(), const QString &icon = QString())
{
    QVariantMap m;
    m.insert(QStringLiteral("text"), text);
    m.insert(QStringLiteral("secondary"), secondary);
    m.insert(QStringLiteral("trailing"), trailing);
    m.insert(QStringLiteral("icon"), icon);
    return m;
}
}

Workspace::Workspace(QObject *parent)
    : QObject(parent)
{
}

QString Workspace::versionString() const
{
    return QCoreApplication::applicationVersion();
}

QString Workspace::liveIcon() const
{
    return iconForKind(mLiveKind);
}

QVariantList Workspace::liveSelection() const
{
    QVariantList l;
    for (int r : mLiveSelectedRows)
        l.append(r);
    return l;
}

// ─────────────────────────────────────────────────────────────────────────────
//  Shell
// ─────────────────────────────────────────────────────────────────────────────

void Workspace::setModule(const QString &m)
{
    if (mModule == m)
        return;
    mModule = m;
    emit moduleChanged();
}

void Workspace::setStatusText(const QString &t)
{
    if (mStatusText == t)
        return;
    mStatusText = t;
    emit statusTextChanged();
}

void Workspace::setDisplayState(bool on, int screens)
{
    if (mDisplayOn == on && mScreenCount == screens)
        return;
    mDisplayOn = on;
    mScreenCount = screens;
    emit displayChanged();
}

void Workspace::setLanguageList(const QVariantList &langs, const QString &current)
{
    mLanguages = langs;
    mLanguage = current;
    emit languagesChanged();
}

// ─────────────────────────────────────────────────────────────────────────────
//  Live column
// ─────────────────────────────────────────────────────────────────────────────

void Workspace::setLiveContent(const QString &kind, const QString &title,
                               const QVariantList &slides, int index, const QString &notes)
{
    mLive = true;
    mLiveKind = kind;
    mLiveTitle = title;
    mLiveSlides = slides;
    mLiveNotes = notes;
    mLiveIndex = index;
    mLiveSelectedRows.clear();
    if (index >= 0)
        mLiveSelectedRows.append(index);

    if (kind != QLatin1String("bible") && mMultiVerse)
    {
        mMultiVerse = false;
        emit multiVerseChanged();
    }

    emit liveChanged();
    emit liveIndexChanged();
}

void Workspace::setLiveSelection(const QList<int> &rows)
{
    mLiveSelectedRows = rows;
    mLiveIndex = rows.isEmpty() ? -1 : rows.first();
    emit liveIndexChanged();
}

void Workspace::clearLive()
{
    mLive = false;
    mLiveKind.clear();
    mLiveTitle.clear();
    mLiveNotes.clear();
    mLiveSlides.clear();
    mLiveIndex = -1;
    mLiveSelectedRows.clear();
    emit liveChanged();
    emit liveIndexChanged();
}

void Workspace::setLiveIndex(int i)
{
    if (mLiveIndex == i)
        return;
    mLiveIndex = i;
    mLiveSelectedRows.clear();
    if (i >= 0)
        mLiveSelectedRows.append(i);
    emit liveIndexChanged();
}

void Workspace::setMultiVerse(bool on)
{
    if (mMultiVerse == on)
        return;
    mMultiVerse = on;
    emit multiVerseChanged();
}

void Workspace::setActionState(bool canShow, bool canHide, bool canClear)
{
    if (mCanShow == canShow && mCanHide == canHide && mCanClear == canClear)
        return;
    mCanShow = canShow;
    mCanHide = canHide;
    mCanClear = canClear;
    emit actionStateChanged();
}

// ─────────────────────────────────────────────────────────────────────────────
//  Schedule
// ─────────────────────────────────────────────────────────────────────────────

void Workspace::setScheduleModel(const QList<Schedule> &items, const QString &name, bool dirty)
{
    mScheduleView.clear();
    for (const Schedule &s : items)
    {
        QVariantMap m = row(s.name, QString(), QString(), iconForKind(s.stype));
        m.insert(QStringLiteral("kind"), s.stype);
        mScheduleView.append(m);
    }
    mScheduleName = name;
    mScheduleDirty = dirty;
    if (mScheduleIndex >= mScheduleView.count())
        mScheduleIndex = mScheduleView.count() - 1;
    emit scheduleChanged();
    emit scheduleIndexChanged();
}

void Workspace::setScheduleIndex(int i)
{
    if (mScheduleIndex == i)
        return;
    mScheduleIndex = i;
    emit scheduleIndexChanged();
}

void Workspace::scheduleActivate(int i)
{
    setScheduleIndex(i);
    emit requestAction(QStringLiteral("scheduleOpenItem"));
}

void Workspace::scheduleMove(int from, int to)
{
    if (from < 0 || to < 0 || from >= mScheduleView.count() || to >= mScheduleView.count()
            || from == to)
        return;
    mScheduleView.move(from, to);
    mScheduleIndex = to;
    emit scheduleChanged();
    emit scheduleIndexChanged();
}

// ─────────────────────────────────────────────────────────────────────────────
//  Loading
// ─────────────────────────────────────────────────────────────────────────────

void Workspace::reloadAll()
{
    reloadBibles();
    reloadSongbooks();
    reloadSongs();
    reloadAnnouncements();
    reloadSlideshows();
    reloadMediaLibrary();
}

void Workspace::applyBibleSettings(const BibleVersionSettings &s)
{
    const QString previous = mBibleSettings.operatorBible;
    mBibleSettings = s;
    if (mBibleSettings.operatorBible == QLatin1String("same"))
        mBibleSettings.operatorBible = mBibleSettings.primaryBible;

    if (previous != mBibleSettings.operatorBible)
        reloadBibles();
}

void Workspace::reloadBibles()
{
    mBibleVersions.clear();
    mBibleVersionIds.clear();

    QSqlQuery sq;
    sq.exec(QStringLiteral("SELECT id, bible_name FROM BibleVersions ORDER BY bible_name"));
    while (sq.next())
    {
        mBibleVersionIds.append(sq.value(0).toInt());
        mBibleVersions.append(sq.value(1).toString());
    }

    const int wanted = mBibleSettings.operatorBible.toInt();
    mBibleVersionIndex = qMax(0, mBibleVersionIds.indexOf(wanted));
    emit bibleVersionsChanged();

    if (mBibleVersions.isEmpty())
    {
        mBibleBooksView.clear();
        mBibleBookMap.clear();
        emit bibleBooksChanged();
        return;
    }

    QString id = QString::number(mBibleVersionIds.value(mBibleVersionIndex));
    mBible.setBiblesId(id);
    mBible.loadOperatorBible();

    rebuildBibleBookView();
    if (!mBibleBookMap.isEmpty())
        setBibleBookIndex(0);
}

void Workspace::setBibleVersionIndex(int i)
{
    if (i < 0 || i >= mBibleVersionIds.count() || i == mBibleVersionIndex)
        return;
    mBibleVersionIndex = i;
    QString id = QString::number(mBibleVersionIds.at(i));
    mBibleSettings.operatorBible = id;
    mBible.setBiblesId(id);
    mBible.loadOperatorBible();
    emit bibleVersionsChanged();

    rebuildBibleBookView();
    setBibleBookIndex(0);
}

void Workspace::rebuildBibleBookView()
{
    mBibleBooksView.clear();
    mBibleBookMap.clear();

    const QStringList all = mBible.getBooks();
    for (int i = 0; i < all.count(); ++i)
    {
        if (!mBibleBookFilter.isEmpty()
                && !all.at(i).contains(mBibleBookFilter, Qt::CaseInsensitive))
            continue;

        QVariantMap m = row(all.at(i), QString(),
                            QString::number(mBible.books.at(i).chapterCount));
        mBibleBookMap.append(i);
        mBibleBooksView.append(m);
    }
    emit bibleBooksChanged();
}

void Workspace::setBibleBookFilter(const QString &f)
{
    if (mBibleBookFilter == f)
        return;
    mBibleBookFilter = f;
    const int previous = mBibleBookIndex >= 0 && mBibleBookIndex < mBibleBookMap.count()
                         ? mBibleBookMap.at(mBibleBookIndex) : -1;
    rebuildBibleBookView();

    // Keep the same book selected if it survived the filter, else take the first.
    int next = mBibleBookMap.indexOf(previous);
    setBibleBookIndex(next >= 0 ? next : (mBibleBookMap.isEmpty() ? -1 : 0));
}

int Workspace::bibleChapterCount() const
{
    if (mBibleBookIndex < 0 || mBibleBookIndex >= mBibleBookMap.count())
        return 0;
    return mBible.books.at(mBibleBookMap.at(mBibleBookIndex)).chapterCount;
}

void Workspace::setBibleBookIndex(int i)
{
    mBibleBookIndex = i;
    emit bibleBookChanged();

    mBibleChapterIndex = 0;
    loadBibleChapter();
}

void Workspace::setBibleChapterIndex(int i)
{
    if (i < 0 || i >= bibleChapterCount())
        return;
    mBibleChapterIndex = i;
    loadBibleChapter();
}

void Workspace::loadBibleChapter()
{
    mBibleVerses.clear();
    mBibleChapterList.clear();

    if (mBibleBookIndex >= 0 && mBibleBookIndex < mBibleBookMap.count())
    {
        const BibleBook &bk = mBible.books.at(mBibleBookMap.at(mBibleBookIndex));
        mBibleChapterList = mBible.getChapter(bk.bookId.toInt(), mBibleChapterIndex + 1);

        for (int i = 0; i < mBibleChapterList.count(); ++i)
        {
            QString t = mBibleChapterList.at(i);
            // Verses arrive as "12. text"; split the number out so it can be
            // set in tabular figures beside the text.
            const int dot = t.indexOf(QLatin1Char('.'));
            QVariantMap m;
            m.insert(QStringLiteral("number"), dot > 0 ? t.left(dot) : QString::number(i + 1));
            m.insert(QStringLiteral("text"), dot > 0 ? t.mid(dot + 1).trimmed() : t);
            mBibleVerses.append(m);
        }
    }

    emit bibleChapterChanged();
}

QString Workspace::currentBibleBook() const
{
    if (mBibleBookIndex < 0 || mBibleBookIndex >= mBibleBookMap.count())
        return QString();
    return mBible.books.at(mBibleBookMap.at(mBibleBookIndex)).book;
}

QString Workspace::bibleCaption(const QList<int> &rows) const
{
    if (mBibleBookIndex < 0 || mBibleBookIndex >= mBibleBookMap.count() || rows.isEmpty())
        return QString();
    const QString book = mBible.books.at(mBibleBookMap.at(mBibleBookIndex)).book;
    return QStringLiteral("%1 %2").arg(book).arg(mBibleChapterIndex + 1);
}

BibleHistory Workspace::bibleRefFor(const QList<int> &rows) const
{
    BibleHistory b;
    if (rows.isEmpty() || mBibleBookIndex < 0 || mBibleBookIndex >= mBibleBookMap.count())
        return b;

    const QString book = mBible.books.at(mBibleBookMap.at(mBibleBookIndex)).book;
    const QString chapter = QString::number(mBibleChapterIndex + 1);

    QStringList ids;
    for (int r : rows)
        if (r >= 0 && r < mBible.previewIdList.count())
            ids.append(mBible.previewIdList.at(r));

    const int first = rows.first() + 1;
    const int last = rows.last() + 1;

    b.verseIds = ids.join(QLatin1Char(','));
    b.caption = book + " " + chapter + ":" + QString::number(first)
                + (first == last ? QString() : "-" + QString::number(last));
    b.captionLong = b.caption;
    return b;
}

void Workspace::bibleSearch(const QString &query, int scope, bool allWords)
{
    QString text = clean(query);
    if (text.isEmpty())
    {
        bibleClearSearch();
        return;
    }

    QRegularExpression rx;
    rx.setPatternOptions(QRegularExpression::CaseInsensitiveOption);
    text.replace(QLatin1String(" "), QLatin1String("\\W*"));
    if (allWords)
    {
        text.replace(QLatin1String("\\W*"), QLatin1String("|"));
        rx.setPattern("\\b(" + text + ")\\b");
    }
    else
    {
        rx.setPattern(text);
    }

    const int book = (mBibleBookIndex >= 0 && mBibleBookIndex < mBibleBookMap.count())
                     ? mBible.books.at(mBibleBookMap.at(mBibleBookIndex)).bookId.toInt() : 0;

    if (scope == 1 && book)
        mBibleSearch = mBible.searchBible(allWords, rx, book);
    else if (scope == 2 && book)
        mBibleSearch = mBible.searchBible(allWords, rx, book, mBibleChapterIndex + 1);
    else
        mBibleSearch = mBible.searchBible(allWords, rx);

    mBibleSearchView.clear();
    for (const BibleSearch &s : mBibleSearch)
    {
        mBibleSearchView.append(row(s.verse_text,
                                    QStringLiteral("%1 %2:%3").arg(s.book, s.chapter, s.verse)));
    }
    mBibleSearching = true;
    emit bibleSearchChanged();

    if (mBibleSearch.isEmpty())
        emit toast(tr("No verses match that search."), QStringLiteral("warning"));
}

void Workspace::bibleClearSearch()
{
    mBibleSearch.clear();
    mBibleSearchView.clear();
    mBibleSearching = false;
    emit bibleSearchChanged();
}

void Workspace::bibleOpenSearchResult(int i)
{
    if (i < 0 || i >= mBibleSearch.count())
        return;
    const BibleSearch &s = mBibleSearch.at(i);

    const QStringList all = mBible.getBooks();
    const int bookRow = all.indexOf(s.book);
    const int viewRow = mBibleBookMap.indexOf(bookRow);
    if (viewRow < 0)
    {
        // The book is hidden by the current filter; drop the filter so the
        // result the operator clicked is actually reachable.
        setBibleBookFilter(QString());
    }
    mBibleBookIndex = mBibleBookMap.indexOf(bookRow);
    emit bibleBookChanged();

    mBibleChapterIndex = qMax(0, s.chapter.toInt() - 1);
    loadBibleChapter();

    const int verse = qMax(0, s.first_v - 1);
    setLiveIndex(verse);
}

void Workspace::bibleOpenHistory(int i)
{
    if (i < 0 || i >= mBibleHistory.count())
        return;
    const BibleHistory &h = mBibleHistory.at(i);

    QString bk;
    int ch = 0, vr = 0;
    mBible.getVerseRef(h.verseIds, bk, ch, vr);
    const int last = mBible.getVerseNumberLast(h.verseIds);

    const int bookRow = mBible.getBooks().indexOf(bk);
    int viewRow = mBibleBookMap.indexOf(bookRow);
    if (viewRow < 0)
    {
        setBibleBookFilter(QString());
        viewRow = mBibleBookMap.indexOf(bookRow);
    }
    mBibleBookIndex = viewRow;
    emit bibleBookChanged();

    mBibleChapterIndex = qMax(0, ch - 1);
    loadBibleChapter();

    QList<int> rows;
    for (int v = vr; v <= qMax(vr, last); ++v)
        rows.append(v - 1);
    setLiveSelection(rows);
    bibleGoLive(liveSelection());
}

void Workspace::bibleClearHistory()
{
    mBibleHistory.clear();
    mBibleHistoryView.clear();
    emit bibleHistoryChanged();
}

void Workspace::bibleGoLive(const QVariantList &rows)
{
    QList<int> r;
    for (const QVariant &v : rows)
        r.append(v.toInt());
    if (r.isEmpty())
        return;
    std::sort(r.begin(), r.end());

    mBible.currentIdList = mBible.previewIdList;
    emit requestBibleLive(mBible.verseList, bibleCaption(r), r);

    // Record it in history, most recent first, without duplicates.
    const BibleHistory h = bibleRefFor(r);

    if (!h.verseIds.isEmpty())
    {
        for (int i = mBibleHistory.count() - 1; i >= 0; --i)
            if (mBibleHistory.at(i).verseIds == h.verseIds)
                mBibleHistory.removeAt(i);

        mBibleHistory.prepend(h);
        mBibleHistoryView.clear();
        for (const BibleHistory &e : mBibleHistory)
            mBibleHistoryView.append(row(e.caption, QString(), QString(),
                                         QStringLiteral("book-open")));
        emit bibleHistoryChanged();
    }
}

void Workspace::bibleAddToSchedule()
{
    emit requestScheduleAdd(QStringLiteral("bible"));
}

void Workspace::bibleStep(int delta)
{
    const int chapters = bibleChapterCount();
    int c = mBibleChapterIndex + delta;
    if (c >= 0 && c < chapters)
    {
        setBibleChapterIndex(c);
        return;
    }
    // Walk into the neighbouring book rather than stopping dead at the edge.
    int b = mBibleBookIndex + (delta > 0 ? 1 : -1);
    if (b < 0 || b >= mBibleBookMap.count())
        return;
    mBibleBookIndex = b;
    emit bibleBookChanged();
    mBibleChapterIndex = delta > 0 ? 0 : qMax(0, bibleChapterCount() - 1);
    loadBibleChapter();
}

// ─────────────────────────────────────────────────────────────────────────────
//  Songs
// ─────────────────────────────────────────────────────────────────────────────

void Workspace::reloadSongbooks()
{
    mSongbooks.clear();
    mSongbookIds.clear();
    mSongbooks.append(tr("All songbooks"));
    mSongbookIds.append(QStringLiteral("ALL"));

    QSqlQuery sq;
    sq.exec(QStringLiteral("SELECT id, name FROM Songbooks"));
    while (sq.next())
    {
        mSongbookIds.append(sq.value(0).toString());
        mSongbooks.append(sq.value(1).toString());
    }

    mSongCategories.clear();
    mSongCategoryIds.clear();
    mSongCategories.append(tr("All categories"));
    mSongCategoryIds.append(-1);

    const QStringList cats = EditWidget::categories();
    QMap<QString, int> sorted;
    for (int i = 0; i < cats.count(); ++i)
        sorted.insert(cats.at(i), i);
    mSongCategories.append(sorted.keys());
    mSongCategoryIds.append(sorted.values());

    emit songbooksChanged();
}

void Workspace::reloadSongs()
{
    mAllSongs = mSongDb.getSongs();
    rebuildSongView();
}

// ponytail: the song list is filtered by walking every row. At a few thousand
// songs on a keystroke this is well under a frame; swap in SongProxyModel if a
// library ever gets big enough to feel it.
void Workspace::rebuildSongView()
{
    mSongsView.clear();
    mSongMap.clear();

    const QString songbookFilter = mSongbookIds.value(mSongbookIndex, QStringLiteral("ALL"));
    const int categoryFilter = mSongCategoryIds.value(mSongCategoryIndex, -1);

    QRegularExpression rx;
    rx.setPatternOptions(QRegularExpression::CaseInsensitiveOption);
    QString pattern = mSongQuery;
    pattern.replace(QLatin1String(" "), QLatin1String("\\W*"));
    if (mSongMatchMode == 1)
        rx.setPattern("^" + pattern);
    else
        rx.setPattern(pattern);

    for (int i = 0; i < mAllSongs.count(); ++i)
    {
        const Song &s = mAllSongs.at(i);

        if (songbookFilter != QLatin1String("ALL") && s.songbook_id != songbookFilter)
            continue;
        if (categoryFilter != -1 && s.category != categoryFilter)
            continue;

        if (!mSongQuery.isEmpty())
        {
            const QString num = QString::number(s.number);
            bool hit;
            if (mSongMatchMode == 2)
                hit = num.compare(mSongQuery, Qt::CaseInsensitive) == 0
                      || s.title.compare(mSongQuery, Qt::CaseInsensitive) == 0;
            else
                hit = num.contains(rx) || s.title.contains(rx);
            if (!hit)
                continue;
        }

        mSongMap.append(i);
        mSongsView.append(row(s.title, s.songbook_name, QString::number(s.number),
                              QStringLiteral("music-4")));
    }

    emit songsChanged();

    // Keep something selected so the preview column is never mysteriously blank.
    setSongIndex(mSongsView.isEmpty() ? -1 : 0);
}

void Workspace::setSongbookIndex(int i)
{
    if (mSongbookIndex == i) return;
    mSongbookIndex = i;
    emit songFilterChanged();
    rebuildSongView();
}

void Workspace::setSongCategoryIndex(int i)
{
    if (mSongCategoryIndex == i) return;
    mSongCategoryIndex = i;
    emit songFilterChanged();
    rebuildSongView();
}

void Workspace::setSongMatchMode(int m)
{
    if (mSongMatchMode == m) return;
    mSongMatchMode = m;
    emit songFilterChanged();
    rebuildSongView();
}

void Workspace::setSongQuery(const QString &q)
{
    if (mSongQuery == q) return;
    mSongQuery = q;
    emit songFilterChanged();
    rebuildSongView();
}

void Workspace::setSongIndex(int i)
{
    mSongIndex = i;
    loadSongPreview();
    emit songSelectionChanged();
}

void Workspace::loadSongPreview()
{
    mSongStanzas.clear();
    mSongTitle.clear();
    mSongInfo.clear();
    mSongNotes.clear();

    if (mSongIndex < 0 || mSongIndex >= mSongMap.count())
    {
        mCurrentSong = Song();
        return;
    }

    mCurrentSong = mAllSongs.at(mSongMap.at(mSongIndex));
    mCurrentSong.readData();

    mSongTitle = mCurrentSong.title;
    mSongNotes = mCurrentSong.notes;

    QStringList bits;
    if (mCurrentSong.number > 0)
        bits << QStringLiteral("#%1").arg(mCurrentSong.number);
    if (!mCurrentSong.songbook_name.isEmpty())
        bits << mCurrentSong.songbook_name;
    if (!mCurrentSong.wordsBy.isEmpty())
        bits << tr("Words: %1").arg(mCurrentSong.wordsBy);
    if (!mCurrentSong.musicBy.isEmpty())
        bits << tr("Music: %1").arg(mCurrentSong.musicBy);
    mSongInfo = bits.join(QStringLiteral("  ·  "));

    const QStringList text = mCurrentSong.getSongTextList();
    for (int i = 0; i < text.count(); ++i)
        mSongStanzas.append(row(text.at(i), QString(), QString::number(i + 1)));
}

void Workspace::songJumpToNumber(int number)
{
    for (int i = 0; i < mSongMap.count(); ++i)
    {
        if (mAllSongs.at(mSongMap.at(i)).number == number)
        {
            setSongIndex(i);
            return;
        }
    }
}

void Workspace::songGoLive(int stanza)
{
    if (mSongIndex < 0 || mSongIndex >= mSongMap.count())
        return;
    emit requestSongLive(mCurrentSong, qMax(0, stanza));
}

void Workspace::songAddToSchedule()
{
    if (mSongIndex < 0) return;
    emit requestScheduleAdd(QStringLiteral("song"));
}

// ─────────────────────────────────────────────────────────────────────────────
//  Announcements
// ─────────────────────────────────────────────────────────────────────────────

void Workspace::reloadAnnouncements()
{
    mAllAnnounce.clear();

    QSqlQuery sq;
    sq.exec(QStringLiteral("SELECT id FROM Announcements ORDER BY title"));
    while (sq.next())
        mAllAnnounce.append(Announcement(sq.value(0).toInt()));

    rebuildAnnounceView();
}

void Workspace::rebuildAnnounceView()
{
    mAnnounceView.clear();
    mAnnounceMap.clear();

    for (int i = 0; i < mAllAnnounce.count(); ++i)
    {
        const Announcement &a = mAllAnnounce.at(i);
        if (!mAnnounceQuery.isEmpty()
                && !a.title.contains(mAnnounceQuery, Qt::CaseInsensitive)
                && !a.text.contains(mAnnounceQuery, Qt::CaseInsensitive))
            continue;

        mAnnounceMap.append(i);
        mAnnounceView.append(row(a.title,
                                 a.text.left(80).simplified(),
                                 a.useAutoNext ? tr("auto") : QString(),
                                 QStringLiteral("megaphone")));
    }

    emit announcementsChanged();
    setAnnounceIndex(mAnnounceView.isEmpty() ? -1 : 0);
}

void Workspace::setAnnounceQuery(const QString &q)
{
    if (mAnnounceQuery == q) return;
    mAnnounceQuery = q;
    rebuildAnnounceView();
}

void Workspace::setAnnounceIndex(int i)
{
    mAnnounceIndex = i;
    loadAnnouncePreview();
    emit announceSelectionChanged();
}

void Workspace::loadAnnouncePreview()
{
    mAnnounceSlides.clear();
    mAnnounceTitle.clear();

    if (mAnnounceIndex < 0 || mAnnounceIndex >= mAnnounceMap.count())
    {
        mCurrentAnnounce = Announcement();
        return;
    }

    mCurrentAnnounce = mAllAnnounce.at(mAnnounceMap.at(mAnnounceIndex));
    mAnnounceTitle = mCurrentAnnounce.title;

    const QStringList slides = mCurrentAnnounce.getAnnounceList();
    for (int i = 0; i < slides.count(); ++i)
        mAnnounceSlides.append(row(slides.at(i), QString(), QString::number(i + 1)));
}

void Workspace::announceGoLive(int slide)
{
    if (mAnnounceIndex < 0) return;
    emit requestAnnounceLive(mCurrentAnnounce, qMax(0, slide));
}

void Workspace::announceAddToSchedule()
{
    if (mAnnounceIndex < 0) return;
    emit requestScheduleAdd(QStringLiteral("announce"));
}

// ─────────────────────────────────────────────────────────────────────────────
//  Pictures
// ─────────────────────────────────────────────────────────────────────────────

void Workspace::reloadSlideshows()
{
    mSlideshows.clear();
    mSlideshowView.clear();

    QSqlQuery sq;
    sq.exec(QStringLiteral("SELECT id, name, info FROM SlideShows"));
    while (sq.next())
    {
        SlideShowInfo ssi;
        ssi.slideSwId = sq.value(0).toInt();
        ssi.name = sq.value(1).toString();
        ssi.info = sq.value(2).toString();
        mSlideshows.append(ssi);

        QSqlQuery cq;
        cq.exec(QStringLiteral("SELECT COUNT(*) FROM Slides WHERE ss_id = %1").arg(ssi.slideSwId));
        const int n = cq.next() ? cq.value(0).toInt() : 0;

        mSlideshowView.append(row(ssi.name, ssi.info,
                                  tr("%n slide(s)", "", n), QStringLiteral("images")));
    }

    emit slideshowsChanged();
    setSlideshowIndex(mSlideshows.isEmpty() ? -1 : 0);
}

void Workspace::setSlideshowIndex(int i)
{
    mSlideshowIndex = i;
    if (i >= 0 && i < mSlideshows.count())
        loadSlideShow(mSlideshows.at(i).slideSwId);
    else
    {
        mCurrentSlideShow = SlideShow();
        mSlidesView.clear();
    }
    mSlideIndex = 0;
    emit slideshowSelectionChanged();
    emit slideIndexChanged();
}

void Workspace::loadSlideShow(int id)
{
    mCurrentSlideShow = SlideShow();
    mCurrentSlideShow.loadSlideShow(id);

    mSlidesView.clear();
    ThumbProvider::instance()->clear();

    for (int i = 0; i < mCurrentSlideShow.slides.count(); ++i)
    {
        const SlideShowItem &s = mCurrentSlideShow.slides.at(i);
        const QString key = QStringLiteral("slide%1_%2").arg(id).arg(i);
        ThumbProvider::instance()->put(key, s.imageSmall);

        QVariantMap m = row(s.name.isEmpty() ? tr("Slide %1").arg(i + 1) : s.name);
        m.insert(QStringLiteral("thumb"), QStringLiteral("image://spthumb/") + key);
        mSlidesView.append(m);
    }
}

void Workspace::setSlideIndex(int i)
{
    if (mSlideIndex == i) return;
    mSlideIndex = i;
    emit slideIndexChanged();
}

void Workspace::slideshowGoLive(int slide)
{
    if (mSlideshowIndex < 0 || mCurrentSlideShow.slides.isEmpty())
        return;
    mSlideIndex = qMax(0, slide);
    emit requestSlideShowLive(mCurrentSlideShow, mSlideIndex);
}

void Workspace::slideshowAddToSchedule()
{
    if (mSlideshowIndex < 0) return;
    emit requestScheduleAdd(QStringLiteral("picture"));
}

// ─────────────────────────────────────────────────────────────────────────────
//  Media
// ─────────────────────────────────────────────────────────────────────────────

void Workspace::reloadMediaLibrary()
{
    mMediaPaths.clear();
    mMediaNames.clear();
    mMediaView.clear();

    QSqlQuery sq;
    sq.exec(QStringLiteral("SELECT long_path, short_path FROM Media"));
    while (sq.next())
    {
        const QUrl url(sq.value(0).toString());
        const QString name = sq.value(1).toString();
        mMediaPaths.append(url);
        mMediaNames.append(name);

        const QString suffix = QFileInfo(url.toLocalFile()).suffix().toUpper();
        const bool missing = !QFileInfo::exists(url.toLocalFile());
        QVariantMap m = row(name,
                            missing ? tr("File not found") : url.toLocalFile(),
                            suffix,
                            QStringLiteral("clapperboard"));
        m.insert(QStringLiteral("missing"), missing);
        mMediaView.append(m);
    }

    emit mediaLibraryChanged();
    setMediaIndex(mMediaView.isEmpty() ? -1 : 0);
}

QString Workspace::mediaName() const
{
    return mMediaNames.value(mMediaIndex);
}

VideoInfo Workspace::currentMedia() const
{
    VideoInfo v;
    if (mMediaIndex >= 0 && mMediaIndex < mMediaPaths.count())
    {
        v.filePath = mMediaPaths.at(mMediaIndex);
        v.fileName = mMediaNames.at(mMediaIndex);
    }
    return v;
}

void Workspace::setMediaIndex(int i)
{
    if (mMediaIndex == i) return;
    mMediaIndex = i;
    emit mediaSelectionChanged();
}

void Workspace::mediaGoLive()
{
    if (mMediaIndex < 0) return;
    VideoInfo v = currentMedia();
    if (!QFileInfo::exists(v.filePath.toLocalFile()))
    {
        emit toast(tr("That media file no longer exists."), QStringLiteral("danger"));
        return;
    }
    emit requestMediaLive(v);
}

void Workspace::mediaAddToSchedule()
{
    if (mMediaIndex < 0) return;
    emit requestScheduleAdd(QStringLiteral("video"));
}

void Workspace::mediaAddFiles(const QVariantList &urls)
{
    QSqlQuery sq;
    int added = 0;
    for (const QVariant &u : urls)
    {
        const QUrl url = u.toUrl();
        if (url.isEmpty() || mMediaPaths.contains(url))
            continue;
        sq.prepare(QStringLiteral("INSERT INTO Media (long_path, short_path) VALUES (?,?)"));
        sq.addBindValue(url.toString());
        sq.addBindValue(QFileInfo(url.toLocalFile()).fileName());
        sq.exec();
        ++added;
    }
    if (added)
    {
        reloadMediaLibrary();
        emit toast(tr("%n file(s) added to the library", "", added), QStringLiteral("live"));
    }
}

void Workspace::mediaRemoveSelected()
{
    if (mMediaIndex < 0 || mMediaIndex >= mMediaPaths.count())
        return;
    QSqlQuery sq;
    sq.prepare(QStringLiteral("DELETE FROM Media WHERE long_path = ?"));
    sq.addBindValue(mMediaPaths.at(mMediaIndex).toString());
    sq.exec();
    const int keep = mMediaIndex;
    reloadMediaLibrary();
    setMediaIndex(qMin(keep, mMediaView.count() - 1));
}

void Workspace::setMediaVolume(int v)
{
    if (mMediaVolume == v) return;
    mMediaVolume = v;
    emit transportChanged();
    emit requestTransport(QStringLiteral("volume"), v);
}

void Workspace::setMediaMuted(bool m)
{
    if (mMediaMuted == m) return;
    mMediaMuted = m;
    emit transportChanged();
    emit requestTransport(QStringLiteral("mute"), m ? 1 : 0);
}

void Workspace::setTransport(bool playing, qint64 position, qint64 duration)
{
    if (mMediaPlaying == playing && mMediaPosition == position && mMediaDuration == duration)
        return;
    mMediaPlaying = playing;
    mMediaPosition = position;
    mMediaDuration = duration;
    emit transportChanged();
}

// ─────────────────────────────────────────────────────────────────────────────
//  Generic action dispatch
// ─────────────────────────────────────────────────────────────────────────────

void Workspace::mediaSeek(qint64 ms)
{
    emit requestTransport(QStringLiteral("seek"), ms);
}

void Workspace::setLiveRows(const QVariantList &rows)
{
    QList<int> r;
    for (const QVariant &v : rows)
        r.append(v.toInt());
    std::sort(r.begin(), r.end());
    mLiveSelectedRows = r;
    if (!r.isEmpty())
        mLiveIndex = r.first();
    emit liveIndexChanged();
}

void Workspace::invoke(const QString &action)
{
    // Transport verbs are handled here; everything else is the window's job.
    if (action == QLatin1String("mediaPlayPause"))
        emit requestTransport(mMediaPlaying ? QStringLiteral("pause") : QStringLiteral("play"), 0);
    else if (action == QLatin1String("mediaStop"))
        emit requestTransport(QStringLiteral("stop"), 0);
    else
        emit requestAction(action);
}
