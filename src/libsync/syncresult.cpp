/*
 * SPDX-FileCopyrightText: 2018 Nextcloud GmbH and Nextcloud contributors
 * SPDX-FileCopyrightText: 2014 ownCloud GmbH
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "syncresult.h"
#include "progressdispatcher.h"

namespace OCC {

SyncResult::SyncResult() = default;

SyncResult::Status SyncResult::status() const
{
    return _status;
}

void SyncResult::reset()
{
    *this = SyncResult();
}

QString SyncResult::statusString() const
{
    QString re;
    Status stat = status();

    switch (stat) {
    case Undefined:
        re = QLatin1String("Undefined");
        break;
    case NotYetStarted:
        re = QLatin1String("Not yet started");
        break;
    case SyncRunning:
        re = QLatin1String("Sync running");
        break;
    case Success:
        re = QLatin1String("Success");
        break;
    case Error:
        re = QLatin1String("Error");
        break;
    case SetupError:
        re = QLatin1String("Setup error");
        break;
    case SyncPrepare:
        re = QLatin1String("Preparing to sync");
        break;
    case Problem:
        re = QLatin1String("Success, some files were ignored.");
        break;
    case SyncAbortRequested:
        re = QLatin1String("Sync request cancelled");
        break;
    case Paused:
        re = QLatin1String("Sync paused");
        break;
    }
    return re;
}

void SyncResult::setStatus(Status stat)
{
    _status = stat;
    _syncTime = QDateTime::currentDateTimeUtc();
}

QDateTime SyncResult::syncTime() const
{
    return _syncTime;
}

QStringList SyncResult::errorStrings() const
{
    return _errors;
}

void SyncResult::appendErrorString(const QString &err)
{
    _errors.append(err);
}

QString SyncResult::errorString() const
{
    if (_errors.isEmpty())
        return QString();
    return _errors.first();
}

void SyncResult::clearErrors()
{
    _errors.clear();
}

void SyncResult::setFolder(const QString &folder)
{
    _folder = folder;
}

QString SyncResult::folder() const
{
    return _folder;
}

void SyncResult::processCompletedItem(const SyncFileItemPtr &item)
{
    if (Progress::isWarningKind(item->_status)) {
        // Count any error conditions, error strings will have priority anyway.
        _foundFilesNotSynced = true;
    }

    if (item->isDirectory() && (item->_instruction == CSYNC_INSTRUCTION_NEW
                                  || item->_instruction == CSYNC_INSTRUCTION_TYPE_CHANGE
                                  || item->_instruction == CSYNC_INSTRUCTION_REMOVE
                                  || item->_instruction == CSYNC_INSTRUCTION_RENAME)) {
        _folderStructureWasChanged = true;
    }

    if(item->_status == SyncFileItem::FileLocked){
        _numLockedItems++;
        if (!_firstItemLocked) {
            _firstItemLocked = item;
        }
    }

    // Process the item to the gui
    if (item->_status == SyncFileItem::FatalError || item->_status == SyncFileItem::NormalError) {
        //: this displays an error string (%2) for a file %1
        appendErrorString(QObject::tr("%1: %2").arg(item->_file, item->_errorString));
        _numErrorItems++;
        if (!_firstItemError) {
            _firstItemError = item;
        }
    } else if (item->_status == SyncFileItem::Conflict || item->_status == SyncFileItem::FileNameInvalid || item->_status == SyncFileItem::FileNameInvalidOnServer || item->_status == SyncFileItem::FileNameClash) {
        if (item->_instruction == CSYNC_INSTRUCTION_CONFLICT) {
            _numNewConflictItems++;
            if (!_firstNewConflictItem) {
                _firstNewConflictItem = item;
            }
        } else {
            _numOldConflictItems++;
        }
    } else {
        if (!item->hasErrorStatus() && item->_status != SyncFileItem::FileIgnored && item->_direction == SyncFileItem::Down) {
            switch (item->_instruction) {
            case CSYNC_INSTRUCTION_NEW:
            case CSYNC_INSTRUCTION_TYPE_CHANGE:
                _numNewItems++;
                if (!_firstItemNew)
                    _firstItemNew = item;
                break;
            case CSYNC_INSTRUCTION_REMOVE:
                _numRemovedItems++;
                if (!_firstItemDeleted)
                    _firstItemDeleted = item;
                break;
            case CSYNC_INSTRUCTION_SYNC:
                _numUpdatedItems++;
                if (!_firstItemUpdated)
                    _firstItemUpdated = item;
                break;
            case CSYNC_INSTRUCTION_RENAME:
                if (!_firstItemRenamed) {
                    _firstItemRenamed = item;
                }
                _numRenamedItems++;
                break;
            default:
                // nothing.
                break;
            }
        } else if (item->_instruction == CSYNC_INSTRUCTION_IGNORE) {
            _foundFilesNotSynced = true;
        }
    }
}

} // ns mirall
