#ifndef MSQTVCA_H
#define MSQTVCA_H

namespace MsQtVca {
enum ObjectVcaType {
    RegionEntrance = 0,
    RegionExiting,
    LineCrossing,
    Loitering,
    HumanDetection,
    PeopleCounting,
    ObjectLeftRemoved,
    AdvancedMotionDetection
};
enum ObjectSizeType {
    MinSize,
    MaxSize
};
}

#endif // MSQTVCA_H
