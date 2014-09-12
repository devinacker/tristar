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

struct enemy_t {
    QString name;
    int32_t data1[3];
    int32_t type, x, y;
    int32_t data2[2];
};

struct enemytype_t {
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

    QVector<enemy_t> enemies;
    QVector<enemytype_t> enemyTypes;

    QVector<object_t> objects;
    QVector<QString> objectNames;

    QVector<item_t> items;

    bool open(QFile&);
    void clear();

private:
    void loadBreakable(QFile&, uint);
    void loadCollision(QFile&, uint);
    void loadCollisionRTDL(QFile&, uint);
    void loadVisual(QFile&, uint);
    void loadEnemies(QFile&, uint);
    void loadEnemyTypes(QFile&, uint);
    void loadMusic(QFile&, uint);
    void loadObjects(QFile&, uint);
    void loadItems(QFile&, uint);
};

#endif // LEVEL_H
