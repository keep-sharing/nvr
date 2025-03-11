#ifndef VIDEOPOSITION_H
#define VIDEOPOSITION_H

class QDebug;
class VideoPosition;

QDebug operator<<(QDebug debug, const VideoPosition &c);

class VideoPosition
{
public:
    VideoPosition();
    VideoPosition(int r, int c, int rSpan = 1, int cSpan = 1);

    bool operator<(const VideoPosition &other) const;
    bool operator==(const VideoPosition &other) const;

    int index = -1;
    int channel = -1;

    int row = 0;
    int column = 0;
    int rowSpan = 0;
    int columnSpan = 0;
};

#endif // VIDEOPOSITION_H
