#ifndef LEVEL_H
#define LEVEL_H

#include <QVector>
#include <QMap>
#include <QString>
#include <cstdint>

class QFile;

struct mapblock_t {
    int16_t  breakable;
    uint32_t collision;

    struct {
        int16_t  first;
        uint16_t second;
    } visual[3];
};

struct enemystate_t {
    int32_t data[8];
};

struct enemy_t {
    QString name, state;
};

struct object_t {
    uint32_t x, y, type;
    int32_t unknown;
    int32_t enabled;
    int32_t params[8];
};

struct item_t {
    int32_t data[3];
    uint32_t x, y, data2;
};

struct LevelData {
    uint width, height;
    QString musicName;

    QVector<QVector<mapblock_t>> blocks;

    // from chunk 4
    uint32_t unknown1, unknown2;

    QMap<QString, enemystate_t> states;
    QVector<enemy_t> enemies;

    QVector<object_t> objects;
    QVector<QString> objectNames;

    QVector<item_t> items;

    void open(QFile&);
    void clear();
};

#endif // LEVEL_H
