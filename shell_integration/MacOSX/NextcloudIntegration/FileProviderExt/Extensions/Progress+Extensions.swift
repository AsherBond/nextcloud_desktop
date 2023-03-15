/*
 * Copyright (C) 2023 by Claudio Cambra <claudio.cambra@nextcloud.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
 * or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License
 * for more details.
 */

import Foundation

extension Progress {
    func copyCurrentStateToProgress(_ otherProgress: Progress) {
        otherProgress.cancellationHandler = self.cancellationHandler
        otherProgress.pausingHandler = self.pausingHandler
        otherProgress.totalUnitCount = self.totalUnitCount
        otherProgress.completedUnitCount = self.completedUnitCount
        otherProgress.estimatedTimeRemaining = self.estimatedTimeRemaining
        otherProgress.localizedDescription = self.localizedAdditionalDescription
        otherProgress.localizedAdditionalDescription = self.localizedAdditionalDescription
        otherProgress.isCancellable = self.isCancellable
        otherProgress.isPausable = self.isPausable
        otherProgress.fileCompletedCount = self.fileCompletedCount
        otherProgress.fileURL = self.fileURL
        otherProgress.fileTotalCount = self.fileTotalCount
        otherProgress.fileCompletedCount = self.fileCompletedCount
        otherProgress.fileOperationKind = self.fileOperationKind
        otherProgress.kind = self.kind
        otherProgress.throughput = self.throughput

        for (key, object) in self.userInfo {
            otherProgress.setUserInfoObject(object, forKey: key)
        }
    }

    func copyOfCurrentState() -> Progress {
        let newProgress = Progress()
        copyCurrentStateToProgress(newProgress)
        return newProgress
    }
}
