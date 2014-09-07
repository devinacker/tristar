#ifndef LEVEL_H
#define LEVEL_H

#include <QVector>
#include <QMap>
#include <QString>
#include <cstdint>

class QFile;

struct mapblock_t {
    int16_t  data1;
    uint32_t data3;

    struct {
        int16_t  first;
        uint16_t second;
    } data4[3];
};

struct enemystate_t {
    int32_t data[8];
};

struct enemy_t {
    QString name, state;
};

struct object_t {
    uint32_t x, y, type;
    int32_t data[10];
};

struct data5_t {
    int32_t data[3];
    uint32_t x, y, type;
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

    QVector<data5_t> data5;

    void open(QFile&);
    void clear();
};

#endif // LEVEL_H
