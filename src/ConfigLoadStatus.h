#pragma once

namespace ConfigLoadStatus {
enum class Status {
    Loaded,
    UsedDefaults
};

enum class Reason {
    LoadedFromFile,
    MissingFile,
    MountFailed,
    InvalidJson,
    FileTooLarge
};

struct Outcome {
    Status status;
    Reason reason;
    bool usedDefaults;
};

Outcome loaded();
Outcome missingFile();
Outcome mountFailed();
Outcome invalidJson();
Outcome fileTooLarge();

const char *statusLabel(Status status);
const char *reasonLabel(Reason reason);
}
