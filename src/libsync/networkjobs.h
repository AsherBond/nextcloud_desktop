/*
 * SPDX-FileCopyrightText: 2017 Nextcloud GmbH and Nextcloud contributors
 * SPDX-FileCopyrightText: 2013 ownCloud GmbH
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef NETWORKJOBS_H
#define NETWORKJOBS_H

#include "config.h"

#include "abstractnetworkjob.h"

#include "common/result.h"

#include <QBuffer>
#include <QUrlQuery>

class QUrl;
class QJsonObject;
class QJsonDocument;
class QDomDocument;

namespace OCC {

constexpr auto HttpErrorCodeNone = 0;
constexpr auto HttpErrorCodeSuccess = 200;
constexpr auto HttpErrorCodeSuccessCreated = 201;
constexpr auto HttpErrorCodeSuccessNoContent = 204;
constexpr auto HttpErrorCodeBadRequest = 400;
constexpr auto HttpErrorCodeUnsupportedMediaType = 415;

struct HttpError
{
    int code; // HTTP error code
    QString message;
};

template <typename T>
using HttpResult = Result<T, HttpError>;

/**
 * @brief The EntityExistsJob class
 * @ingroup libsync
 */
class OWNCLOUDSYNC_EXPORT EntityExistsJob : public AbstractNetworkJob
{
    Q_OBJECT
public:
    explicit EntityExistsJob(AccountPtr account, const QString &path, QObject *parent = nullptr);
    void start() override;

signals:
    void exists(QNetworkReply *);

private slots:
    bool finished() override;
};

/**
 * @brief A basic file manipulation job
 * @ingroup libsync
 */
class OWNCLOUDSYNC_EXPORT SimpleFileJob : public AbstractNetworkJob
{
    Q_OBJECT
public:
    explicit SimpleFileJob(AccountPtr account, const QString &filePath, QObject *parent = nullptr);

    QNetworkReply *startRequest(
        const QByteArray &verb, const QNetworkRequest req = QNetworkRequest(), QIODevice *requestBody = nullptr);

    QNetworkReply *startRequest(const QByteArray &verb, const QUrl &url, const QNetworkRequest req = QNetworkRequest(),
        QIODevice *requestBody = nullptr);

signals:
    void finishedSignal(QNetworkReply *reply);
protected slots:
    bool finished() override;

private:
    QByteArray _verb;
};

/**
 * @brief sends a DELETE http request to a url.
 *
 * See Nextcloud API usage for the possible DELETE requests.
 *
 * This does *not* delete files, it does a http request.
 */
class OWNCLOUDSYNC_EXPORT DeleteApiJob : public SimpleFileJob
{
    Q_OBJECT
public:
    explicit DeleteApiJob(AccountPtr account, const QString &path, QObject *parent = nullptr);
    void start() override;

signals:
    void result(int httpCode);

private slots:
    bool finished() override;
};

struct ExtraFolderInfo {
    QByteArray fileId;
    qint64 size = -1;
};

/**
 * @brief The LsColJob class
 * @ingroup libsync
 */
class OWNCLOUDSYNC_EXPORT LsColXMLParser : public QObject
{
    Q_OBJECT
public:
    explicit LsColXMLParser();

    bool parse(const QByteArray &xml,
               QHash<QString, ExtraFolderInfo> *sizes,
               const QString &expectedPath);

signals:
    void directoryListingSubfolders(const QStringList &items);
    void directoryListingIterated(const QString &name, const QMap<QString, QString> &properties);
    void finishedWithError(QNetworkReply *reply);
    void finishedWithoutError();
};

class OWNCLOUDSYNC_EXPORT LsColJob : public AbstractNetworkJob
{
    Q_OBJECT
public:
    explicit LsColJob(AccountPtr account, const QString &path);
    explicit LsColJob(AccountPtr account, const QUrl &url);
    void start() override;
    QHash<QString, ExtraFolderInfo> _folderInfos;

    /**
     * Used to specify which properties shall be retrieved.
     *
     * The properties can
     *  - contain no colon: they refer to a property in the DAV: namespace
     *  - contain a colon: and thus specify an explicit namespace,
     *    e.g. "ns:with:colons:bar", which is "bar" in the "ns:with:colons" namespace
     */
    void setProperties(QList<QByteArray> properties);
    [[nodiscard]] QList<QByteArray> properties() const;

signals:
    void directoryListingSubfolders(const QStringList &items);
    void directoryListingIterated(const QString &name, const QMap<QString, QString> &properties);
    void finishedWithError(QNetworkReply *reply);
    void finishedWithoutError();

private slots:
    bool finished() override;

private:
    QList<QByteArray> _properties;
    QUrl _url; // Used instead of path() if the url is specified in the constructor
};

/**
 * @brief The PropfindJob class
 *
 * Setting the desired properties with setProperties() is mandatory.
 *
 * Note that this job is only for querying one item.
 * There is also the LsColJob which can be used to list collections
 *
 * @ingroup libsync
 */
class OWNCLOUDSYNC_EXPORT PropfindJob : public AbstractNetworkJob
{
    Q_OBJECT
public:
    explicit PropfindJob(AccountPtr account, const QString &path, QObject *parent = nullptr);
    void start() override;

    /**
     * Used to specify which properties shall be retrieved.
     *
     * The properties can
     *  - contain no colon: they refer to a property in the DAV: namespace
     *  - contain a colon: and thus specify an explicit namespace,
     *    e.g. "ns:with:colons:bar", which is "bar" in the "ns:with:colons" namespace
     */
    void setProperties(QList<QByteArray> properties);
    [[nodiscard]] QList<QByteArray> properties() const;

signals:
    void result(const QVariantMap &values);
    void finishedWithError(QNetworkReply *reply = nullptr);

private slots:
    bool finished() override;

private:
    static QVariantMap processPropfindDomDocument(const QDomDocument &domDocument);
    static QStringList processTagsInPropfindDomDocument(const QDomDocument &domDocument);
    static QVariantList processSystemTagsInPropfindDomDocument(const QDomDocument &domDocument);

    QList<QByteArray> _properties;
};

#ifndef TOKEN_AUTH_ONLY
/**
 * @brief Retrieves the account users avatar from the server using a GET request.
 *
 * If the server does not have the avatar, the result Pixmap is empty.
 *
 * @ingroup libsync
 */
class OWNCLOUDSYNC_EXPORT AvatarJob : public AbstractNetworkJob
{
    Q_OBJECT
public:
    /**
     * @param userId The user for which to obtain the avatar
     * @param size The size of the avatar (square so size*size)
     */
    explicit AvatarJob(AccountPtr account, const QString &userId, int size, QObject *parent = nullptr);

    void start() override;

    /** The retrieved avatar images don't have the circle shape by default */
    static QImage makeCircularAvatar(const QImage &baseAvatar);

signals:
    /**
     * @brief avatarPixmap - returns either a valid pixmap or not.
     */

    void avatarPixmap(const QImage &);

private slots:
    bool finished() override;

private:
    QUrl _avatarUrl;
};
#endif

/**
 * @brief Send a Proppatch request
 *
 * Setting the desired properties with setProperties() is mandatory.
 *
 * WARNING: Untested!
 *
 * @ingroup libsync
 */
class OWNCLOUDSYNC_EXPORT ProppatchJob : public AbstractNetworkJob
{
    Q_OBJECT
public:
    explicit ProppatchJob(AccountPtr account, const QString &path, QObject *parent = nullptr);
    void start() override;

    /**
     * Used to specify which properties shall be set.
     *
     * The property keys can
     *  - contain no colon: they refer to a property in the DAV: namespace
     *  - contain a colon: and thus specify an explicit namespace,
     *    e.g. "ns:with:colons:bar", which is "bar" in the "ns:with:colons" namespace
     */
    void setProperties(QMap<QByteArray, QByteArray> properties);
    [[nodiscard]] QMap<QByteArray, QByteArray> properties() const;

signals:
    void success();
    void finishedWithError();

private slots:
    bool finished() override;

private:
    QMap<QByteArray, QByteArray> _properties;
};

/**
 * @brief The MkColJob class
 * @ingroup libsync
 */
class OWNCLOUDSYNC_EXPORT MkColJob : public AbstractNetworkJob
{
    Q_OBJECT
    QUrl _url; // Only used if the constructor taking a url is taken.
    QMap<QByteArray, QByteArray> _extraHeaders;

public:
    explicit MkColJob(AccountPtr account, const QString &path, QObject *parent = nullptr);
    explicit MkColJob(AccountPtr account, const QString &path, const QMap<QByteArray, QByteArray> &extraHeaders, QObject *parent = nullptr);
    explicit MkColJob(AccountPtr account, const QUrl &url,
        const QMap<QByteArray, QByteArray> &extraHeaders, QObject *parent = nullptr);
    void start() override;

signals:
    void finishedWithError(QNetworkReply *reply);
    void finishedWithoutError();

private:
    bool finished() override;
};

/**
 * @brief The CheckServerJob class
 * @ingroup libsync
 */
class OWNCLOUDSYNC_EXPORT CheckServerJob : public AbstractNetworkJob
{
    Q_OBJECT
public:
    explicit CheckServerJob(AccountPtr account, QObject *parent = nullptr);
    void start() override;

    static QString version(const QJsonObject &info);
    static QString versionString(const QJsonObject &info);
    static bool installed(const QJsonObject &info);

signals:
    /** Emitted when a status.php was successfully read.
     *
     * \a url see _serverStatusUrl (does not include "/status.php")
     * \a info The status.php reply information
     */
    void instanceFound(const QUrl &url, const QJsonObject &info);

    /** Emitted on invalid status.php reply.
     *
     * \a reply is never null
     */
    void instanceNotFound(QNetworkReply *reply);

    /** A timeout occurred.
     *
     * \a url The specific url where the timeout happened.
     */
    void timeout(const QUrl &url);

private:
    bool finished() override;
    void onTimedOut() override;
private slots:
    virtual void metaDataChangedSlot();
    virtual void encryptedSlot();
    void slotRedirected(QNetworkReply *reply, const QUrl &targetUrl, int redirectCount);

private:
    bool _subdirFallback = false;

    /** The permanent-redirect adjusted account url.
     *
     * Note that temporary redirects or a permanent redirect behind a temporary
     * one do not affect this url.
     */
    QUrl _serverUrl;

    /** Keep track of how many permanent redirect were applied. */
    int _permanentRedirects = 0;
};

/**
 * @brief The CheckRedirectCostFreeUrlJob class
 * @ingroup libsync
 */
class OWNCLOUDSYNC_EXPORT CheckRedirectCostFreeUrlJob : public AbstractNetworkJob
{
    Q_OBJECT
public:
    explicit CheckRedirectCostFreeUrlJob(const AccountPtr &account, QObject *parent = nullptr);
    void start() override;

signals:
    /**
    * a check is finished
    * \a statusCode cost-free URL GET HTTP response code
    */
    void jobFinished(int statusCode);
    /** A timeout occurred.
     *
     * \a url The specific url where the timeout happened.
     */
    void timeout(const QUrl &url);

private:
    bool finished() override;
    void onTimedOut() override;
};


/**
 * @brief The RequestEtagJob class
 */
class OWNCLOUDSYNC_EXPORT RequestEtagJob : public AbstractNetworkJob
{
    Q_OBJECT
public:
    explicit RequestEtagJob(AccountPtr account, const QString &path, QObject *parent = nullptr);
    void start() override;

signals:
    void etagRetrieved(const QByteArray &etag, const QDateTime &time);
    void finishedWithResult(const HttpResult<QByteArray> &etag);

private slots:
    bool finished() override;
};

class OWNCLOUDSYNC_EXPORT SimpleApiJob : public AbstractNetworkJob
{
    Q_OBJECT
public:
    enum class Verb {
        Get,
        Post,
        Put,
        Delete,
        };

    explicit SimpleApiJob(const AccountPtr &account, const QString &path, QObject *parent = nullptr);

    void setBody(const QByteArray &body);

    void setVerb(Verb value);

    /**
     * @brief addQueryParams - add more parameters to the ocs call
     * @param params: list pairs of strings containing the parameter name and the value.
     *
     * All parameters from the passed list are appended to the query. Note
     * that the format=json parameter is added automatically and does not
     * need to be set this way.
     *
     * This function needs to be called before start() obviously.
     */
    void addQueryParams(const QUrlQuery &params);
    void addRawHeader(const QByteArray &headerName, const QByteArray &value);

public slots:
    void start() override;

Q_SIGNALS:

    void resultReceived(int statusCode);

protected:
    bool finished() override;

    [[nodiscard]] QNetworkRequest& request();
    [[nodiscard]] QByteArray& body();
    [[nodiscard]] QUrlQuery& additionalParams();
    [[nodiscard]] QByteArray verbToString() const;

private:
    QByteArray _body;
    QUrlQuery _additionalParams;
    QNetworkRequest _request;
    Verb _verb = Verb::Get;
};

/**
 * @brief Job to check an API that return JSON
 *
 * Note! you need to be in the connected state before calling this because of a server bug:
 * https://github.com/owncloud/core/issues/12930
 *
 * To be used like this:
 * \code
 * _job = new JsonApiJob(account, QLatin1String("ocs/v1.php/foo/bar"), this);
 * connect(job, SIGNAL(jsonReceived(QJsonDocument)), ...)
 * The received QVariantMap is null in case of error
 * \encode
 *
 * @ingroup libsync
 */
class OWNCLOUDSYNC_EXPORT JsonApiJob : public SimpleApiJob
{
    Q_OBJECT
public:
    explicit JsonApiJob(const AccountPtr &account, const QString &path, QObject *parent = nullptr);

    void setBody(const QJsonDocument &body);

public slots:
    void start() override;

protected:
    bool finished() override;
signals:

    /**
     * @brief jsonReceived - signal to report the json answer from ocs
     * @param json - the parsed json document
     * @param statusCode - the OCS status code: 100 (!) for success
     */
    void jsonReceived(const QJsonDocument &json, int statusCode);

    /**
     * @brief etagResponseHeaderReceived - signal to report the ETag response header value
     * from ocs api v2
     * @param value - the ETag response header value
     * @param statusCode - the OCS status code: 100 (!) for success
     */
    void etagResponseHeaderReceived(const QByteArray &value, int statusCode);
};

/**
 * @brief Checks with auth type to use for a server
 * @ingroup libsync
 */
class OWNCLOUDSYNC_EXPORT DetermineAuthTypeJob : public QObject
{
    Q_OBJECT
public:
    enum AuthType {
        NoAuthType, // used only before we got a chance to probe the server
#ifdef WITH_WEBENGINE
        WebViewFlow,
#endif // WITH_WEBENGINE
        Basic, // also the catch-all fallback for backwards compatibility reasons
        LoginFlowV2
    };
    Q_ENUM(AuthType)

    explicit DetermineAuthTypeJob(AccountPtr account, QObject *parent = nullptr);
    void start();
signals:
    void authType(OCC::DetermineAuthTypeJob::AuthType);

private:
    void checkAllDone();

    AccountPtr _account;
    AuthType _resultGet = NoAuthType;
    AuthType _resultPropfind = NoAuthType;
    AuthType _resultOldFlow = NoAuthType;
    bool _getDone = false;
    bool _propfindDone = false;
    bool _oldFlowDone = false;
    bool useFlow2 = false;
};

/**
 * @brief A basic job around a network request without extra functionality
 * @ingroup libsync
 *
 * Primarily adds timeout and redirection handling.
 */
class OWNCLOUDSYNC_EXPORT SimpleNetworkJob : public AbstractNetworkJob
{
    Q_OBJECT
public:
    explicit SimpleNetworkJob(AccountPtr account, QObject *parent = nullptr);

    QNetworkReply *startRequest(const QByteArray &verb, const QUrl &url,
        QNetworkRequest req = QNetworkRequest(),
        QIODevice *requestBody = nullptr);

signals:
    void finishedSignal(QNetworkReply *reply);
private slots:
    bool finished() override;
};

/**
 * @brief Runs a PROPFIND to figure out the private link url
 *
 * The numericFileId is used only to build the deprecatedPrivateLinkUrl
 * locally as a fallback. If it's empty and the PROPFIND fails, targetFun
 * will be called with an empty string.
 *
 * The job and signal connections are parented to the target QObject.
 *
 * Note: targetFun is guaranteed to be called only through the event
 * loop and never directly.
 */
void OWNCLOUDSYNC_EXPORT fetchPrivateLinkUrl(
    AccountPtr account, const QString &remotePath,
    const QByteArray &numericFileId, QObject *target,
    std::function<void(const QString &url)> targetFun);

} // namespace OCC

#endif // NETWORKJOBS_H
