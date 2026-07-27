#include "ConfigLoadStatus.h"

namespace ConfigLoadStatus {
Outcome loaded() {
    return {Status::Loaded, Reason::LoadedFromFile, false};
}

Outcome missingFile() {
    return {Status::UsedDefaults, Reason::MissingFile, true};
}

Outcome mountFailed() {
    return {Status::UsedDefaults, Reason::MountFailed, true};
}

Outcome invalidJson() {
    return {Status::UsedDefaults, Reason::InvalidJson, true};
}

Outcome fileTooLarge() {
    return {Status::UsedDefaults, Reason::FileTooLarge, true};
}

const char *statusLabel(Status status) {
    switch (status) {
    case Status::Loaded:
        return "loaded";
    case Status::UsedDefaults:
        return "used_defaults";
    }
    return "used_defaults";
}

const char *reasonLabel(Reason reason) {
    switch (reason) {
    case Reason::LoadedFromFile:
        return "loaded_from_file";
    case Reason::MissingFile:
        return "missing_file";
    case Reason::MountFailed:
        return "mount_failed";
    case Reason::InvalidJson:
        return "invalid_json";
    case Reason::FileTooLarge:
        return "file_too_large";
    }
    return "invalid_json";
}
}
