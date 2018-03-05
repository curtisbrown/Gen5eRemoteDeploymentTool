#ifndef ENUMERATIONS_H
#define ENUMERATIONS_H

#include <QObject>
#include <QMetaType>

class Enums : public QObject {
    Q_OBJECT
    Q_ENUMS(UpdateStatus)

public:
    enum UpdateStatus {
        UPDATE_NONE = 0,
        UPDATE_IN_PROGRESS,
        UPDATE_SUCCESS,
        UPDATE_FAILED
    };
};

#endif // ENUMERATIONS_H
