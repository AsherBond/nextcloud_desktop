/*
 * SPDX-FileCopyrightText: 2017 Nextcloud GmbH and Nextcloud contributors
 * SPDX-License-Identifier: GPL-2.0-or-later
 */
#ifndef CLIENTSIDEENCRYPTIONJOBS_H
#define CLIENTSIDEENCRYPTIONJOBS_H

#include "networkjobs.h"
#include "accountfwd.h"

#include <QSslKey>
#include <QString>
#include <QJsonDocument>

namespace OCC {
/* Here are all of the network jobs for the client side encryption.
 * anything that goes thru the server and expects a response, is here.
 */

/*
 * @brief Job to sigh the CSR that return JSON
 *
 * To be used like this:
 * \code
 * _job = new SignPublicKeyApiJob(account, QLatin1String("ocs/v1.php/foo/bar"), this);
 * _job->setCsr( csr );
 * connect(_job...);
 * _job->start();
 * \encode
 *
 * @ingroup libsync
 */

class SyncJournalDb;
class OWNCLOUDSYNC_EXPORT SignPublicKeyApiJob : public AbstractNetworkJob
{
    Q_OBJECT
public:
    explicit SignPublicKeyApiJob(const AccountPtr &account, const QString &path, QObject *parent = nullptr);

    /**
     * @brief setCsr - the CSR with the public key.
     * This function needs to be called before start() obviously.
     */
    void setCsr(const QByteArray& csr);

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

private:
    QBuffer _csr;
};

/*
 * @brief Job to upload the PublicKey that return JSON
 *
 * To be used like this:
 * \code
 * _job = new StorePublicKeyApiJob(account, QLatin1String("ocs/v1.php/foo/bar"), this);
 * _job->setPublicKey( privKey );
 * connect(_job...);
 * _job->start();
 * \encode
 *
 * @ingroup libsync
 */
class OWNCLOUDSYNC_EXPORT StorePublicKeyApiJob : public AbstractNetworkJob
{
    Q_OBJECT
public:
    explicit StorePublicKeyApiJob(const AccountPtr &account, const QString &path, QObject *parent = nullptr);

    /**
     * @brief setCsr - the CSR with the public key.
     * This function needs to be called before start() obviously.
     */
    void setPublicKey(const QByteArray& publicKey);

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

private:
    QBuffer _publicKey;
};

/*
 * @brief Job to upload the PrivateKey that return JSON
 *
 * To be used like this:
 * \code
 * _job = new StorePrivateKeyApiJob(account, QLatin1String("ocs/v1.php/foo/bar"), this);
 * _job->setPrivateKey( privKey );
 * connect(_job...);
 * _job->start();
 * \encode
 *
 * @ingroup libsync
 */
class OWNCLOUDSYNC_EXPORT StorePrivateKeyApiJob : public AbstractNetworkJob
{
    Q_OBJECT
public:
    explicit StorePrivateKeyApiJob(const AccountPtr &account, const QString &path, QObject *parent = nullptr);

    /**
     * @brief setCsr - the CSR with the public key.
     * This function needs to be called before start() obviously.
     */
    void setPrivateKey(const QByteArray& privateKey);

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

private:
    QBuffer _privKey;
};


/*
 * @brief Job to mark a folder as encrypted JSON
 *
 * To be used like this:
 * \code
 * _job = new SetEncryptionFlagApiJob(account, 2, this);
  * connect(_job...);
 * _job->start();
 * \encode
 *
 * @ingroup libsync
 */
class OWNCLOUDSYNC_EXPORT SetEncryptionFlagApiJob : public AbstractNetworkJob
{
    Q_OBJECT
public:
    enum FlagAction {
        Clear = 0,
        Set = 1
    };

    explicit SetEncryptionFlagApiJob(const AccountPtr &account, const QByteArray &fileId, FlagAction flagAction = Set, QObject *parent = nullptr);

public slots:
    void start() override;

protected:
    bool finished() override;

signals:
    void success(const QByteArray &fileId);
    void error(const QByteArray &fileId,
               const int httpReturnCode,
               const QString &errorMessage);

private:
    QByteArray _fileId;
    FlagAction _flagAction = Set;
};

class OWNCLOUDSYNC_EXPORT LockEncryptFolderApiJob : public AbstractNetworkJob
{
    Q_OBJECT
public:
    explicit LockEncryptFolderApiJob(const AccountPtr &account,
                                     const QByteArray &fileId,
                                     const QByteArray &certificateSha256Fingerprint,
                                     SyncJournalDb *journalDb,
                                     const QSslKey &sslkey,
                                     QObject *parent = nullptr);

    void setCounter(const quint64 counter);

public slots:
    void start() override;

protected:
    bool finished() override;

signals:
    void success(const QByteArray& fileId, const QByteArray& token);
    void error(const QByteArray& fileId,
               const int httpErrorCode,
               const QString &errorMessage);

private:
    QByteArray _fileId;
    QByteArray _certificateSha256Fingerprint;
    QPointer<SyncJournalDb> _journalDb;
    QSslKey _publicKey;
    quint64 _counter = 0;
};


class OWNCLOUDSYNC_EXPORT UnlockEncryptFolderApiJob : public AbstractNetworkJob
{
    Q_OBJECT
public:
    explicit UnlockEncryptFolderApiJob (
        const AccountPtr &account,
        const QByteArray& fileId,
        const QByteArray& token,
        SyncJournalDb *journalDb,
        QObject *parent = nullptr);

    [[nodiscard]] bool shouldRollbackMetadataChanges() const;

public slots:
    void start() override;
    void setShouldRollbackMetadataChanges(bool shouldRollbackMetadataChanges);

protected:
    bool finished() override;

signals:
    void success(const QByteArray& fileId);
    void error(const QByteArray& fileId,
               const int httpReturnCode,
               const QString &errorMessage);
    void done();

private:
    QByteArray _fileId;
    QByteArray _token;
    QBuffer *_tokenBuf = nullptr;
    QPointer<SyncJournalDb> _journalDb;
    bool _shouldRollbackMetadataChanges = false;
};


class OWNCLOUDSYNC_EXPORT StoreMetaDataApiJob : public AbstractNetworkJob
{
    Q_OBJECT
public:
    explicit StoreMetaDataApiJob (
        const AccountPtr &account,
        const QByteArray& fileId,
        const QByteArray &token,
        const QByteArray& b64Metadata,
        const QByteArray &signature,
        QObject *parent = nullptr);

public slots:
    void start() override;

protected:
    bool finished() override;

signals:
    void success(const QByteArray& fileId);
    void error(const QByteArray& fileId, int httpReturnCode);

private:
    QByteArray _fileId;
    QByteArray _token;
    QByteArray _b64Metadata;
    QByteArray _signature;
};

class OWNCLOUDSYNC_EXPORT UpdateMetadataApiJob : public AbstractNetworkJob
{
    Q_OBJECT
public:
    explicit UpdateMetadataApiJob (
        const AccountPtr &account,
        const QByteArray& fileId,
        const QByteArray& b64Metadata,
        const QByteArray& lockedToken,
        const QByteArray &signature,
        QObject *parent = nullptr);

public slots:
    void start() override;

protected:
    bool finished() override;

signals:
    void success(const QByteArray& fileId);
    void error(const QByteArray& fileId, int httpReturnCode);

private:
    QByteArray _fileId;
    QByteArray _b64Metadata;
    QByteArray _token;
    QByteArray _signature;
};


class OWNCLOUDSYNC_EXPORT GetMetadataApiJob : public AbstractNetworkJob
{
    Q_OBJECT
public:
    explicit GetMetadataApiJob (
        const AccountPtr &account,
        const QByteArray& fileId,
        QObject *parent = nullptr);

    [[nodiscard]] const QByteArray &signature() const;

public slots:
    void start() override;

protected:
    bool finished() override;

signals:
    void jsonReceived(const QJsonDocument &json, int statusCode);
    void error(const QByteArray& fileId, int httpReturnCode);

private:
    QByteArray _fileId;
    QByteArray _signature;
};

class OWNCLOUDSYNC_EXPORT DeleteMetadataApiJob : public AbstractNetworkJob
{
    Q_OBJECT
public:
    explicit DeleteMetadataApiJob (
        const AccountPtr &account,
        const QByteArray& fileId,
        const QByteArray& token,
        QObject *parent = nullptr);

public slots:
    void start() override;

protected:
    bool finished() override;

signals:
    void success(const QByteArray& fileId);
    void error(const QByteArray& fileId, int httpErrorCode);

private:
    QByteArray _fileId;
    QByteArray _token;
};

}
#endif
