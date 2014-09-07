#ifndef LEVEL_H
#define LEVEL_H

#include <QList>
#include <QMap>
#include <QString>
#include <cstdint>

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
    int32_t data[13];
};

struct data5_t {
    int32_t data[6];
};

struct LevelData {
    uint width, height;
    QString musicName;

    QList<QList<mapblock_t>> blocks;

    QMap<QString, enemystate_t> states;
    QList<enemy_t> enemies;

    QList<object_t> objects;
    QList<QString> objectNames;

    QList<data5_t> data5;
};

#endif // LEVEL_H
