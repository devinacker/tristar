#include <QFile>
#include <QtEndian>
#include <QMessageBox>
#include "level.h"

#define CHUNK_TABLE 0x14

// TODO: spin this shit into an actual file class like KALE/KDCE have
bool bigEndian = false;

typedef uint16_t u16;
typedef int16_t i16;
typedef uint32_t u32;
typedef int32_t i32;

template <typename type> type readNum(QFile &file) {
    type num;
    file.read((char*)&num, sizeof(type));

    if (bigEndian)
        return qFromBigEndian<type>((uchar*)&num);

    return qFromLittleEndian<type>((uchar*)&num);
}

uint chunkOffset(QFile &file, uint chunk) {
    file.seek(CHUNK_TABLE + (4 * chunk));
    return readNum<u32>(file);
}

void seekChunk(QFile& file, uint chunk) {
    file.seek(chunkOffset(file, chunk));
}

void LevelData::loadBreakable(QFile &file, uint chunk) {

    seekChunk(file, chunk);
    // read width/height
    uint width = readNum<u32>(file);
    uint height = readNum<u32>(file);

    this->width = width;
    this->height = height;

    this->blocks.resize(height);
    // read info
    for (int y = height - 1; y >= 0; y--) {
        this->blocks[y].resize(width);
        for (uint x = 0; x < width; x++) {
            this->blocks[y][x].breakable = readNum<i16>(file);
        }
    }
}

void LevelData::loadCollision(QFile &file, uint chunk) {

    seekChunk(file, chunk);
    // read pointer
    uint ptr = readNum<u32>(file);
    file.seek(ptr);
    // read info
    uint width = readNum<u32>(file);
    uint height = readNum<u32>(file);

    // is there a mismatch between the width/height given here and elsewhere?
    if (width != this->width || height != this->height)
        printf("data3 size mismatch: (%u, %u) != (%u, %u)\n",
               this->width, this->height, width, height);

    for (int y = height - 1; y >= 0; y--) {
         for (uint x = 0; x < width; x++) {
             this->blocks[y][x].collision = readNum<u32>(file);
         }
    }
}

void LevelData::loadCollisionRTDL(QFile &file, uint chunk) {

    seekChunk(file, chunk);

    // dunno what these are
    // (this is the only difference between this part and its
    //  corresponding part in Triple Deluxe)
    printf("RTDL chunk 3 unknown = 0x%X\n", readNum<u32>(file));

    // read pointer
    uint ptr = readNum<u32>(file);
    file.seek(ptr);

    // read info
    uint width = readNum<u32>(file);
    uint height = readNum<u32>(file);

    // do this here since it's the first chunk read for RTDL right now
    this->width = width;
    this->height = height;
    this->blocks.resize(height);

    // is there a mismatch between the width/height given here and elsewhere?
    if (width != this->width || height != this->height)
        printf("data3 size mismatch: (%u, %u) != (%u, %u)\n",
               this->width, this->height, width, height);

    for (int y = height - 1; y >= 0; y--) {
        // do this here too for now
        this->blocks[y].resize(width);
        for (uint x = 0; x < width; x++) {
            uint tileInfo = readNum<u32>(file);
            // i don't know how this shit works but this should at least give
            // us something to look at (pretty sure it's a bitfield but it doesn't
            // work the same way as TDX's collision data does for drawing purposes
            // but this might give us the same sort of view...
            this->blocks[y][x].collision = tileInfo >> 24;

            // and this (TODO: read the actual breakable data from somewhere)
            this->blocks[y][x].breakable = -1;
        }
    }
}

void LevelData::loadVisual(QFile &file, uint chunk) {
    uint ptrs[3];

    seekChunk(file, chunk);
    // get two unknown values
    this->unknown1 = readNum<u32>(file);
    this->unknown2 = readNum<u32>(file);
    printf("chunk 4 unknown 1 = 0x%X unknown 2 = 0x%X\n", this->unknown1, this->unknown2);
    // get pointers to body
    ptrs[0] = readNum<u32>(file);
    ptrs[1] = readNum<u32>(file);
    ptrs[2] = readNum<u32>(file);
    // get data
    for (uint i = 0; i < 3; i++) {
        file.seek(ptrs[i]);
        uint width = readNum<u32>(file);
        uint height = readNum<u32>(file);

        // is there a mismatch between the width/height given here and elsewhere?
        if (width != this->width || height != this->height)
            printf("data4 size mismatch (body %u): (%u, %u) != (%u, %u)\n",
                   i, this->width, this->height, width, height);

        this->blocks.resize(height);
        for (int y = height - 1; y >= 0; y--) {
             for (uint x = 0; x < width && this->width; x++) {
                 this->blocks[y].resize(width);
                 this->blocks[y][x].visual[i].first = readNum<i16>(file);
                 this->blocks[y][x].visual[i].second = readNum<u16>(file);
             }
        }
    }

}

void LevelData::loadEnemies(QFile &file, uint chunk) {

    seekChunk(file, chunk);
    uint count = readNum<u32>(file);
    for (uint i = 0; i < count; i++) {
        enemy_t enemy;

        // save position, read name
        uint namePtr = readNum<u32>(file);
        uint tempPtr = file.pos();
        file.seek(namePtr);
        uint size = readNum<u32>(file);
        enemy.name = file.read(size);

        // seek back
        file.seek(tempPtr);

        // TODO: update with known fields
        enemy.data1[0] = readNum<i32>(file);
        enemy.data1[1] = readNum<i32>(file);
        enemy.data1[2] = readNum<i32>(file);

        enemy.type = readNum<i32>(file);
        enemy.x = readNum<i32>(file);
        enemy.y = readNum<i32>(file);

        enemy.data2[0] = readNum<i32>(file);
        enemy.data2[1] = readNum<i32>(file);

        this->enemies.push_back(enemy);
    }
}

void LevelData::loadEnemyTypes(QFile &file, uint chunk) {

    seekChunk(file, chunk);
    uint count = readNum<u32>(file);
    for (uint i = 0; i < count; i++) {
        enemytype_t type;
        // name pointer
        uint namePtr = readNum<u32>(file);
        // state pointer
        uint statePtr = readNum<u32>(file);

        // save pos
        uint tempPtr = file.pos();

        // get name
        file.seek(namePtr);
        uint size = readNum<u32>(file);
        type.name = file.read(size);
        // get state
        file.seek(statePtr);
        size = readNum<u32>(file);
        type.state = file.read(size);

        this->enemyTypes.push_back(type);

        // seek back
        file.seek(tempPtr);
    }
}

void LevelData::loadMusic(QFile &file, uint chunk) {

    // TODO: other stuff besides filename, maybe
    seekChunk(file, chunk);
    uint ptr = readNum<u32>(file);
    file.seek(ptr);
    uint count= readNum<u32>(file);
    this->musicName = file.read(count);
}

void LevelData::loadObjects(QFile &file, uint chunk) {

    seekChunk(file, chunk);
    uint objListPtr = readNum<u32>(file);
    uint nameListPtr = readNum<u32>(file);
    // object table
    file.seek(objListPtr);
    uint count = readNum<u32>(file);

    this->objects.resize(count);
    for (uint i = 0; i < count; i++) {
        // TODO: update with known fields
        this->objects[i].x = readNum<u32>(file);
        this->objects[i].y = readNum<u32>(file);
        this->objects[i].type = readNum<u32>(file);

        this->objects[i].unknown = readNum<i32>(file);
        this->objects[i].enabled = readNum<i32>(file);

        for (uint j = 0; j < 8; j++) {
            this->objects[i].params[j] = readNum<i32>(file);
        }
    }
    // object names
    file.seek(nameListPtr);
    count = readNum<u32>(file);

    this->objectNames.resize(count);
    for (uint i = 0; i < count; i++) {
        uint namePtr = readNum<u32>(file);
        uint tempPtr = file.pos();
        file.seek(namePtr);
        uint size = readNum<u32>(file);
        this->objectNames[i] = file.read(size);
        file.seek(tempPtr);
    }
}

void LevelData::loadItems(QFile &file, uint chunk) {

    seekChunk(file, chunk);
    uint count = readNum<u32>(file);
    this->items.resize(count);
    for (uint i = 0; i < count; i++) {
        for (uint j = 0; j < 3; j++) {
            this->items[i].data[j] = readNum<i32>(file);
        }
        this->items[i].x = readNum<u32>(file);
        this->items[i].y = readNum<u32>(file);
        this->items[i].data2 = readNum<u32>(file);
    }
}

bool LevelData::open(QFile& file) {
    this->clear();

    file.seek(4);
    bigEndian = file.read(2) == "\x12\x34";

    if (!bigEndian && chunkOffset(file, 9) == 0x12345678) {
        // main game map
        loadBreakable(file, 0);
        // TODO: check if map data 2 is ever actually used
        loadCollision(file, 2);
        loadVisual(file, 3);
        loadEnemies(file, 4);
        loadEnemyTypes(file, 5);
        loadMusic(file, 6);
        loadObjects(file, 7);
        loadItems(file, 8);
    } else if (!bigEndian && chunkOffset(file, 5) == 0x12345678) {
        // Kirby Fighters map
        loadBreakable(file, 0);
        loadCollision(file, 1);
        loadVisual(file, 2);
        loadMusic(file, 3);
        loadObjects(file, 4);
    } else if (bigEndian && chunkOffset(file, 9) == 0x12345678) {
        // Return to Dream Land map
        // just a test...
        loadCollisionRTDL(file, 2);
        loadVisual(file, 4);
    } else {
        QMessageBox::critical(0, "Load Map", "Unrecognized map format.",
                              QMessageBox::Ok);
        return false;
    }

    fflush(stdout);
    return true;
}

void LevelData::clear() {
    this->width = 0;
    this->height = 0;

    this->musicName = "";

    this->blocks.clear();
    this->enemyTypes.clear();
    this->enemies.clear();
    this->objects.clear();
    this->objectNames.clear();
    this->items.clear();
}
