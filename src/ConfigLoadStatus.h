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
    InvalidJson
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

const char *statusLabel(Status status);
const char *reasonLabel(Reason reason);
}
