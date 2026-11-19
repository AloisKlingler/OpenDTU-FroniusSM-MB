// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (C) 2024-2026 Thomas Basler and others
 */
#include "CommandQueue.h"
#include "../inverters/InverterAbstract.h"
#include <algorithm>

void CommandQueue::removeAllEntriesForInverter(InverterAbstract* inv)
{
    std::lock_guard<std::mutex> lock(_mutex);

    auto it = std::remove_if(_queue.begin(), _queue.end(),
        [&](const auto& v) { return v->getTargetAddress() == inv->serial(); });
    _queue.erase(it, _queue.end());
}

void CommandQueue::removeDuplicatedEntries(std::shared_ptr<CommandAbstract> cmd)
{
    std::lock_guard<std::mutex> lock(_mutex);

    if (_queue.empty()) {
        return;
    }

    auto it = std::remove_if(_queue.begin() + 1, _queue.end(),
        [&](const auto& v) {
            return cmd->areSameParameter(v.get())
                && cmd.get()->getQueueInsertType() == QueueInsertType::RemoveOldest;
        });
    _queue.erase(it, _queue.end());
}

void CommandQueue::replaceEntries(std::shared_ptr<CommandAbstract> cmd)
{
    std::lock_guard<std::mutex> lock(_mutex);

    if (_queue.empty()) {
        return;
    }

    std::replace_if(_queue.begin() + 1, _queue.end(),
        [&](const auto& v) {
            return cmd.get()->getQueueInsertType() == QueueInsertType::ReplaceExistent
                && cmd->areSameParameter(v.get());
            },
        cmd
    );
}

uint8_t CommandQueue::countSimilarCommands(std::shared_ptr<CommandAbstract> cmd)
{
    std::lock_guard<std::mutex> lock(_mutex);

    return std::count_if(_queue.begin(), _queue.end(),
        [&](const auto& v) {
            return cmd->areSameParameter(v.get());
        });
}

uint8_t CommandQueue::removePendingGridProfileWriteCommands(InverterAbstract* inv)
{
    std::lock_guard<std::mutex> lock(_mutex);

    if (_queue.size() <= 1) {
        return 0;
    }
    const auto invSerial = inv->serial();
    auto it = std::remove_if(_queue.begin() + 1, _queue.end(),
        [&](const auto& v) {
            return v->getTargetAddress() == invSerial
                && v->isGridProfileWriteCommand();
        });
    const uint8_t removed = static_cast<uint8_t>(std::distance(it, _queue.end()));
    _queue.erase(it, _queue.end());
    return removed;
}

bool CommandQueue::hasGridProfileWriteCommands(InverterAbstract* inv)
{
    std::lock_guard<std::mutex> lock(_mutex);

    const auto invSerial = inv->serial();
    return std::any_of(_queue.begin(), _queue.end(),
        [&](const auto& v) {
            return v->getTargetAddress() == invSerial
                && v->isGridProfileWriteCommand();
        });
}
